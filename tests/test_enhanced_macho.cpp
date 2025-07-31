/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "common/ComponentInfo.hpp"
#include "common/MetadataExtractor.hpp"

class EnhancedMachOTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      // Create a simple test Mach-O file for testing
      createTestMachOFile();
   }

   void TearDown() override
   {
      // Clean up test files
      if (std::filesystem::exists(testMachOFile))
      {
         std::filesystem::remove(testMachOFile);
      }
   }

   void createTestMachOFile()
   {
      // Create a minimal Mach-O file for testing
      std::ofstream file(testMachOFile, std::ios::binary);
      if (file.is_open())
      {
         // Write Mach-O magic number (64-bit)
         uint32_t magic = 0xFEEDFACF;  // MH_MAGIC_64
         file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

         // Write minimal Mach-O header
         struct
         {
            uint32_t cputype    = 0x1000007;  // CPU_TYPE_X86_64
            uint32_t cpusubtype = 0x3;        // CPU_SUBTYPE_X86_64_ALL
            uint32_t filetype   = 0x2;        // MH_EXECUTE
            uint32_t ncmds      = 1;          // One load command
            uint32_t sizeofcmds = 56;         // Size of load commands
            uint32_t flags      = 0x800000;   // MH_HARDENED_RUNTIME
            uint32_t reserved   = 0;          // Reserved for 64-bit
         } header;

         file.write(reinterpret_cast<const char*>(&header), sizeof(header));

         // Write a minimal load command (LC_UUID)
         struct
         {
            uint32_t cmd      = 0x1B;  // LC_UUID
            uint32_t cmdsize  = 24;    // Size of command
            uint8_t  uuid[16] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                                 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
         } uuid_cmd;

         file.write(reinterpret_cast<const char*>(&uuid_cmd), sizeof(uuid_cmd));

         file.close();
      }
   }

   std::string testMachOFile = "test_macho.bin";
};

TEST_F(EnhancedMachOTest, MachODetection)
{
   heimdall::MetadataExtractor extractor;

#ifdef __APPLE__
   EXPECT_TRUE(extractor.isMachO(testMachOFile));
#else
   EXPECT_FALSE(extractor.isMachO(testMachOFile));
#endif
}

TEST_F(EnhancedMachOTest, VersionExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   // Test version extraction (should fail for our minimal test file)
   bool result = extractor.extractVersionMetadata(component);
   EXPECT_FALSE(result);  // Our minimal file doesn't have version info
#endif
}

TEST_F(EnhancedMachOTest, UUIDExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractEnhancedMachOMetadata(component);
   EXPECT_TRUE(result);
   // UUID extraction is part of enhanced metadata
   // The test file has a UUID, so it should be extracted
#endif
}

TEST_F(EnhancedMachOTest, CodeSignInfoExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOCodeSignInfo(component);
   EXPECT_FALSE(result);  // Our minimal file doesn't have code signing
#endif
}

TEST_F(EnhancedMachOTest, PlatformInfoExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOPlatformInfo(component);
   EXPECT_TRUE(result);
   EXPECT_EQ(component.platformInfo.architecture, "x86_64");
   // Platform info might be set from flags, so we don't check if it's empty
#endif
}

TEST_F(EnhancedMachOTest, ArchitectureExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor             extractor;
   heimdall::ComponentInfo                 component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOArchitectures(component);
   EXPECT_TRUE(result);
   EXPECT_EQ(component.architectures.size(), 1);
   EXPECT_EQ(component.architectures[0].name, "x86_64");
   EXPECT_EQ(component.architectures[0].cpuType, 0x1000007);  // CPU_TYPE_X86_64
#endif
}

TEST_F(EnhancedMachOTest, EnhancedMetadataExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractEnhancedMachOMetadata(component);
   EXPECT_TRUE(result);

   // Check that platform info was extracted
   EXPECT_EQ(component.platformInfo.architecture, "x86_64");

   // Check that architectures were extracted
   EXPECT_EQ(component.architectures.size(), 1);
   EXPECT_EQ(component.architectures[0].name, "x86_64");
#endif
}

TEST_F(EnhancedMachOTest, NonMachOFileHandling)
{
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = "nonexistent_file";

   // Should handle non-Mach-O files gracefully
   bool result = extractor.extractEnhancedMachOMetadata(component);
   EXPECT_FALSE(result);
}

TEST_F(EnhancedMachOTest, FrameworkExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOFrameworks(component);
   EXPECT_FALSE(result);  // Our minimal file doesn't have framework dependencies
   EXPECT_TRUE(component.frameworks.empty());
#endif
}

TEST_F(EnhancedMachOTest, EntitlementsExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOEntitlements(component);
   EXPECT_FALSE(result);  // Our minimal file doesn't have entitlements
   EXPECT_TRUE(component.entitlements.empty());
#endif
}

TEST_F(EnhancedMachOTest, BuildConfigExtraction)
{
#ifdef __APPLE__
   heimdall::MetadataExtractor extractor;
   heimdall::ComponentInfo     component;
   component.filePath = testMachOFile;

   bool result = extractor.extractMachOBuildConfig(component);
   EXPECT_FALSE(result);  // Our minimal file doesn't have build config
#endif
}
