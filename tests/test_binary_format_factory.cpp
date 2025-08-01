/**
 * @file test_binary_format_factory.cpp
 * @brief Unit tests for BinaryFormatFactory
 * @author Heimdall Development Team
 * @date 2025-07-29
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "compat/compatibility.hpp"

#include "factories/BinaryFormatFactory.hpp"
#include "interfaces/IBinaryExtractor.hpp"

namespace heimdall
{

class BinaryFormatFactoryTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      // Create test files with different formats
      createTestFiles();
   }

   void TearDown() override
   {
      // Clean up test files
      cleanupTestFiles();
   }

   private:
   void createTestFiles()
   {
      // Create a simple ELF file (just magic number) - ELF magic is always big-endian
      std::ofstream elfFile("test_elf.bin", std::ios::binary);
      const char    elfMagic[] = {0x7F, 0x45, 0x4C, 0x46};  // "\x7fELF" in big-endian
      elfFile.write(elfMagic, sizeof(elfMagic));
      elfFile.close();

      // Create a simple Mach-O file (just magic number) - Mach-O is little-endian on x86
      std::ofstream machoFile("test_macho.bin", std::ios::binary);
      uint32_t      machoMagic = 0xFEEDFACE;  // Mach-O 32-bit (little-endian on x86)
      machoFile.write(reinterpret_cast<const char*>(&machoMagic), sizeof(machoMagic));
      machoFile.close();

      // Create a simple PE file (just magic number) - PE is little-endian
      std::ofstream peFile("test_pe.bin", std::ios::binary);
      uint32_t      peMagic = 0x00004550;  // PE signature (little-endian)
      peFile.write(reinterpret_cast<const char*>(&peMagic), sizeof(peMagic));
      peFile.close();

      // Create a simple archive file
      std::ofstream archiveFile("test_archive.a", std::ios::binary);
      const char    archiveMagic[] = "!<arch>\n";
      archiveFile.write(archiveMagic, sizeof(archiveMagic) - 1);
      archiveFile.close();

      // Create a simple Java class file - Java is big-endian
      std::ofstream       javaFile("test_java.class", std::ios::binary);
      const unsigned char javaMagic[] = {0xCA, 0xFE, 0xBA, 0xBE};  // Java magic in big-endian
      javaFile.write(reinterpret_cast<const char*>(javaMagic), sizeof(javaMagic));
      javaFile.close();

      // Create a simple WebAssembly file - WebAssembly is little-endian
      std::ofstream wasmFile("test_wasm.wasm", std::ios::binary);
      uint32_t      wasmMagic = 0x6D736100;  // "\0asm" (little-endian)
      wasmFile.write(reinterpret_cast<const char*>(&wasmMagic), sizeof(wasmMagic));
      wasmFile.close();
   }

   void cleanupTestFiles()
   {
      std::vector<std::string> testFiles = {"test_elf.bin",   "test_macho.bin",  "test_pe.bin",
                                            "test_archive.a", "test_java.class", "test_wasm.wasm"};

      for (const auto& file : testFiles)
      {
         if (fs::exists(file))
         {
            fs::remove(file);
         }
      }
   }
};

TEST_F(BinaryFormatFactoryTest, DetectELFFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_elf.bin");
   EXPECT_EQ(format, BinaryFormatFactory::Format::ELF);
}

TEST_F(BinaryFormatFactoryTest, DetectMachOFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_macho.bin");
   EXPECT_EQ(format, BinaryFormatFactory::Format::MachO);
}

TEST_F(BinaryFormatFactoryTest, DetectPEFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_pe.bin");
   EXPECT_EQ(format, BinaryFormatFactory::Format::PE);
}

TEST_F(BinaryFormatFactoryTest, DetectArchiveFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_archive.a");
   EXPECT_EQ(format, BinaryFormatFactory::Format::Archive);
}

TEST_F(BinaryFormatFactoryTest, DetectJavaFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_java.class");
   EXPECT_EQ(format, BinaryFormatFactory::Format::Java);
}

TEST_F(BinaryFormatFactoryTest, DetectWasmFormat)
{
   auto format = BinaryFormatFactory::detectFormat("test_wasm.wasm");
   EXPECT_EQ(format, BinaryFormatFactory::Format::Wasm);
}

TEST_F(BinaryFormatFactoryTest, DetectUnknownFormat)
{
   auto format = BinaryFormatFactory::detectFormat("nonexistent_file.bin");
   EXPECT_EQ(format, BinaryFormatFactory::Format::Unknown);
}

TEST_F(BinaryFormatFactoryTest, CreateExtractorFromFormat)
{
   auto elfExtractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::ELF);
   EXPECT_NE(elfExtractor, nullptr);

   auto machoExtractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::MachO);
   EXPECT_NE(machoExtractor, nullptr);

   auto peExtractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::PE);
   EXPECT_NE(peExtractor, nullptr);

   auto archiveExtractor =
      BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::Archive);
   EXPECT_NE(archiveExtractor, nullptr);

   // Java and Wasm extractors not yet implemented
   auto javaExtractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::Java);
   EXPECT_EQ(javaExtractor, nullptr);

   auto wasmExtractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::Wasm);
   EXPECT_EQ(wasmExtractor, nullptr);
}

TEST_F(BinaryFormatFactoryTest, CreateExtractorFromFile)
{
   auto elfExtractor = BinaryFormatFactory::createExtractor("test_elf.bin");
   EXPECT_NE(elfExtractor, nullptr);

   auto unknownExtractor = BinaryFormatFactory::createExtractor("nonexistent_file.bin");
   EXPECT_EQ(unknownExtractor, nullptr);
}

TEST_F(BinaryFormatFactoryTest, GetAvailableExtractors)
{
   auto extractors = BinaryFormatFactory::getAvailableExtractors("test_elf.bin");
   EXPECT_FALSE(extractors.empty());
}

TEST_F(BinaryFormatFactoryTest, GetFormatName)
{
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::ELF), "ELF");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::MachO), "Mach-O");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::PE), "PE");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::Archive), "Archive");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::Java), "Java");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::Wasm), "WebAssembly");
   EXPECT_EQ(BinaryFormatFactory::getFormatName(BinaryFormatFactory::Format::Unknown), "Unknown");
}

TEST_F(BinaryFormatFactoryTest, GetFormatExtensions)
{
   auto elfExtensions = BinaryFormatFactory::getFormatExtensions(BinaryFormatFactory::Format::ELF);
   EXPECT_FALSE(elfExtensions.empty());
   EXPECT_TRUE(std::find(elfExtensions.begin(), elfExtensions.end(), ".so") != elfExtensions.end());

   auto machoExtensions =
      BinaryFormatFactory::getFormatExtensions(BinaryFormatFactory::Format::MachO);
   EXPECT_FALSE(machoExtensions.empty());
   EXPECT_TRUE(std::find(machoExtensions.begin(), machoExtensions.end(), ".dylib") !=
               machoExtensions.end());
}

TEST_F(BinaryFormatFactoryTest, IsExtensionForFormat)
{
   EXPECT_TRUE(BinaryFormatFactory::isExtensionForFormat(".so", BinaryFormatFactory::Format::ELF));
   EXPECT_TRUE(
      BinaryFormatFactory::isExtensionForFormat(".dylib", BinaryFormatFactory::Format::MachO));
   EXPECT_TRUE(BinaryFormatFactory::isExtensionForFormat(".exe", BinaryFormatFactory::Format::PE));
   EXPECT_TRUE(
      BinaryFormatFactory::isExtensionForFormat(".a", BinaryFormatFactory::Format::Archive));
   EXPECT_TRUE(
      BinaryFormatFactory::isExtensionForFormat(".class", BinaryFormatFactory::Format::Java));
   EXPECT_TRUE(
      BinaryFormatFactory::isExtensionForFormat(".wasm", BinaryFormatFactory::Format::Wasm));

   EXPECT_FALSE(
      BinaryFormatFactory::isExtensionForFormat(".txt", BinaryFormatFactory::Format::ELF));
   EXPECT_FALSE(BinaryFormatFactory::isExtensionForFormat(".so", BinaryFormatFactory::Format::PE));
}

TEST_F(BinaryFormatFactoryTest, GetSupportedFormats)
{
   auto formats = BinaryFormatFactory::getSupportedFormats();
   EXPECT_FALSE(formats.empty());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::ELF) !=
               formats.end());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::MachO) !=
               formats.end());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::PE) !=
               formats.end());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::Archive) !=
               formats.end());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::Java) !=
               formats.end());
   EXPECT_TRUE(std::find(formats.begin(), formats.end(), BinaryFormatFactory::Format::Wasm) !=
               formats.end());
}

TEST_F(BinaryFormatFactoryTest, IsFormatSupported)
{
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::ELF));
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::MachO));
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::PE));
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::Archive));
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::Java));
   EXPECT_TRUE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::Wasm));
   EXPECT_FALSE(BinaryFormatFactory::isFormatSupported(BinaryFormatFactory::Format::Unknown));
}

TEST_F(BinaryFormatFactoryTest, RegisteredExtractorCount)
{
   size_t initialCount = BinaryFormatFactory::getRegisteredExtractorCount();
   EXPECT_EQ(initialCount, 0u);
}

TEST_F(BinaryFormatFactoryTest, ClearRegisteredExtractors)
{
   BinaryFormatFactory::clearRegisteredExtractors();
   EXPECT_EQ(BinaryFormatFactory::getRegisteredExtractorCount(), 0u);
}

}  // namespace heimdall