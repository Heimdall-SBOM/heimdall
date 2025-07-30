/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUTHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include "src/extractors/AdaExtractor.hpp"
#include "src/common/ComponentInfo.hpp"
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class AdaExtractorTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      // Create a temporary test directory and change to it (same approach as PluginInterfaceTest)
      // Use process ID to make directory unique and avoid test interference
      auto pid = std::to_string(getpid());
      test_dir = fs::temp_directory_path() / ("heimdall_ada_test_" + pid);

      // Clean up any existing test directory first
      if (fs::exists(test_dir))
      {
         fs::remove_all(test_dir);
      }

      fs::create_directories(test_dir);

      fs::current_path(test_dir);

      // Create a dummy ali file named my_package.ali in test directory
      std::ofstream ali_file("my_package.ali");
      if (!ali_file.is_open())
      {
         std::cerr << "ERROR: Failed to create dummy ALI file" << std::endl;
      }
      else
      {
         ali_file << "V \"GNAT Lib v2022\"" << std::endl;
         ali_file << "W my_package%b main.adb main.ali" << std::endl;
         ali_file.close();
      }
   }

   void TearDown() override
   {
      // Clean up the test directory
      test_utils::safeRemoveDirectory(test_dir);
   }

   fs::path test_dir;
};

TEST_F(AdaExtractorTest, FindAliFiles)
{
   AdaExtractor             extractor;
   std::vector<std::string> aliFiles;
   extractor.findAliFiles(test_dir.string(), aliFiles);
   ASSERT_EQ(aliFiles.size(), 1);
       EXPECT_EQ(fs::path(aliFiles[0]).filename(), "my_package.ali");
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata)
{
   AdaExtractor             extractor;
   ComponentInfo            component;
   std::vector<std::string> aliFiles;
   extractor.findAliFiles(test_dir.string(), aliFiles);
   extractor.extractAdaMetadata(component, aliFiles);
   ASSERT_EQ(component.packageManager, "GNAT");
   ASSERT_EQ(component.dependencies.size(), 1);
   EXPECT_EQ(component.dependencies[0], "my_package");
   ASSERT_EQ(component.sourceFiles.size(), 1);
   EXPECT_EQ(component.sourceFiles[0], "main.adb");
}

TEST_F(AdaExtractorTest, IsAliFile)
{
   AdaExtractor extractor;
   EXPECT_TRUE(extractor.isAliFile("test.ali"));
   EXPECT_FALSE(extractor.isAliFile("test.txt"));
   EXPECT_FALSE(extractor.isAliFile("testali"));
}

TEST_F(AdaExtractorTest, IsRuntimePackage)
{
   AdaExtractor extractor;
   EXPECT_TRUE(extractor.isRuntimePackage("ada.strings"));
   EXPECT_TRUE(extractor.isRuntimePackage("system.io"));
   EXPECT_TRUE(extractor.isRuntimePackage("ada"));  // Test exact match
   EXPECT_FALSE(extractor.isRuntimePackage("my_package"));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_EmptyAliFile)
{
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   empty_ali = "empty.ali";
   std::ofstream(empty_ali).close();
   std::vector<std::string> aliFiles = {empty_ali};
   EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_CorruptAliFile)
{
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   corrupt_ali = "corrupt.ali";
   std::ofstream(corrupt_ali) << "This is not a valid ALI file";
   std::vector<std::string> aliFiles = {corrupt_ali};
   EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_RuntimeOnly)
{
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   runtime_ali = "runtime.ali";
   std::ofstream(runtime_ali) << "V \"GNAT Lib v2022\"\nW ada%b runtime.adb runtime.ali\n";
   std::vector<std::string> aliFiles = {runtime_ali};
   
   // Test default behavior: runtime packages should be included (like ELF extractors)
   extractor.extractAdaMetadata(component, aliFiles);
   // Should include runtime package as dependency by default
   EXPECT_EQ(component.dependencies.size(), 1);
   EXPECT_EQ(component.dependencies[0], "ada");
   
   // Test explicit exclusion
   ComponentInfo component2;
   extractor.setExcludeRuntimePackages(true);
   extractor.extractAdaMetadata(component2, aliFiles);
   // Should not add runtime package as dependency when explicitly excluded
   EXPECT_TRUE(component2.dependencies.empty());
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_DuplicateDependencies)
{
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   dup_ali = "dup.ali";
   std::ofstream(dup_ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\nW "
                             "my_package%b main.adb main.ali\n";
   std::vector<std::string> aliFiles = {dup_ali};
   extractor.extractAdaMetadata(component, aliFiles);
   EXPECT_EQ(component.dependencies.size(), 1);
   EXPECT_EQ(component.dependencies[0], "my_package");
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_MissingWLine)
{
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   no_w_ali = "no_w.ali";
   std::ofstream(no_w_ali) << "V \"GNAT Lib v2022\"\n";
   std::vector<std::string> aliFiles = {no_w_ali};
   extractor.extractAdaMetadata(component, aliFiles);
   EXPECT_TRUE(component.dependencies.empty());
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_MultipleAliFiles)
{
   try
   {
      AdaExtractor  extractor;
      ComponentInfo component;
      std::string   ali1 = "ada_test_pkg1.ali";
      std::string   ali2 = "ada_test_pkg2.ali";

      // Create test files with proper synchronization
      std::ofstream file1(ali1);
      if (!file1.is_open())
      {
         std::cerr << "ERROR: Failed to create " << ali1 << std::endl;
         std::cerr << "ERROR: errno: " << errno << " (" << std::strerror(errno) << ")" << std::endl;
         FAIL() << "Failed to create " << ali1;
      }
      file1 << "V \"GNAT Lib v2022\"\nW pkg1%b file1.adb file1.ali\n";
      file1.flush();
      file1.close();

      // Check if file exists immediately after creation
      bool file_exists_immediately = fs::exists(ali1);
      if (file_exists_immediately)
      {
         try
         {
         }
         catch (const std::exception& e)
         {
         }
      }

      std::ofstream file2(ali2);
      if (!file2.is_open())
      {
         std::cerr << "ERROR: Failed to create " << ali2 << std::endl;
         FAIL() << "Failed to create " << ali2;
      }
      file2 << "V \"GNAT Lib v2022\"\nW pkg2%b file2.adb file2.ali\n";
      file2.flush();
      file2.close();

      // Check if both files exist after second file creation
      bool ali1_exists_after_second = fs::exists(ali1);
      bool ali2_exists_after_second = fs::exists(ali2);

      // Ensure files are written to disk and synchronized

      // Small delay to ensure filesystem synchronization in CI environments
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // Debug: Check current directory and list files before verification
      // Note: Removed directory iteration to avoid copy issues with non-copyable iterator

      // Verify files were created with retries for CI environments

      // Try multiple times to verify file existence (CI filesystem timing issues)
      int       retry_count = 0;
      const int max_retries = 5;
      bool      ali1_exists = false;
      bool      ali2_exists = false;

      while (retry_count < max_retries && (!ali1_exists || !ali2_exists))
      {
         if (!ali1_exists)
         {
            ali1_exists = fs::exists(ali1);
         }

         if (!ali2_exists)
         {
            ali2_exists = fs::exists(ali2);
         }

         if (!ali1_exists || !ali2_exists)
         {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            retry_count++;
         }
      }

      if (!ali1_exists)
      {
         std::cerr << "ERROR: File " << ali1 << " was not created after " << max_retries
                   << " attempts" << std::endl;
         FAIL() << "File " << ali1 << " was not created";
      }

      if (!ali2_exists)
      {
         std::cerr << "ERROR: File " << ali2 << " was not created after " << max_retries
                   << " attempts" << std::endl;
         FAIL() << "File " << ali2 << " was not created";
      }

      // Check file sizes

      std::vector<std::string> aliFiles = {ali1, ali2};

      extractor.extractAdaMetadata(component, aliFiles);

      for (size_t i = 0; i < component.dependencies.size(); ++i)
      {
      }

      EXPECT_EQ(component.dependencies.size(), 2);

      auto pkg1_found =
         std::find(component.dependencies.begin(), component.dependencies.end(), "pkg1");
      EXPECT_NE(pkg1_found, component.dependencies.end());

      auto pkg2_found =
         std::find(component.dependencies.begin(), component.dependencies.end(), "pkg2");
      EXPECT_NE(pkg2_found, component.dependencies.end());
   }
   catch (const std::exception& e)
   {
      std::cerr << "ERROR: Exception in ExtractAdaMetadata_MultipleAliFiles test: " << e.what()
                << std::endl;
      throw;
   }
   catch (...)
   {
      std::cerr << "ERROR: Unknown exception in ExtractAdaMetadata_MultipleAliFiles test"
                << std::endl;
      throw;
   }
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_VerboseMode)
{
   AdaExtractor extractor;
   extractor.setVerbose(true);
   ComponentInfo component;
   std::string   ali = "verbose.ali";
   std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
   std::vector<std::string> aliFiles = {ali};
   extractor.extractAdaMetadata(component, aliFiles);
   EXPECT_EQ(component.dependencies.size(), 1);
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_TestMode)
{
   AdaExtractor::setTestMode(true);
   AdaExtractor  extractor;
   ComponentInfo component;
   std::string   ali = "testmode.ali";
   std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
   std::vector<std::string> aliFiles = {ali};
   extractor.extractAdaMetadata(component, aliFiles);
   EXPECT_EQ(component.dependencies.size(), 1);
   AdaExtractor::setTestMode(false);
}