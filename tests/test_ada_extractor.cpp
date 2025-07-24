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
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <cerrno>
#include <cstring>
#include "common/AdaExtractor.hpp"
#include "common/ComponentInfo.hpp"
#include "common/Utils.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class AdaExtractorTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "DEBUG: AdaExtractorTest::SetUp() starting" << std::endl;
        
        // Create a temporary test directory and change to it (same approach as PluginInterfaceTest)
        test_dir = std::filesystem::temp_directory_path() / "heimdall_ada_test";
        std::cout << "DEBUG: Test directory path: " << test_dir << std::endl;
        
        // Clean up any existing test directory first
        if (std::filesystem::exists(test_dir)) {
            std::cout << "DEBUG: Removing existing test directory..." << std::endl;
            std::filesystem::remove_all(test_dir);
        }
        
        std::cout << "DEBUG: Creating test directory..." << std::endl;
        std::filesystem::create_directories(test_dir);
        std::cout << "DEBUG: Test directory created successfully" << std::endl;
        
        std::cout << "DEBUG: Changing current directory to test directory..." << std::endl;
        std::filesystem::current_path(test_dir);
        std::cout << "DEBUG: Current directory changed to: " << std::filesystem::current_path() << std::endl;
        
        // Create a dummy ali file named my_package.ali in test directory
        std::cout << "DEBUG: Creating dummy ALI file..." << std::endl;
        std::ofstream ali_file("my_package.ali");
        if (!ali_file.is_open()) {
            std::cerr << "ERROR: Failed to create dummy ALI file" << std::endl;
        } else {
            ali_file << "V \"GNAT Lib v2022\"" << std::endl;
            ali_file << "W my_package%b main.adb main.ali" << std::endl;
            ali_file.close();
            std::cout << "DEBUG: Dummy ALI file created successfully" << std::endl;
        }
        std::cout << "DEBUG: AdaExtractorTest::SetUp() completed" << std::endl;
    }

    void TearDown() override {
        // Clean up the test directory
        test_utils::safeRemoveDirectory(test_dir);
    }

    std::filesystem::path test_dir;
};

TEST_F(AdaExtractorTest, FindAliFiles) {
    AdaExtractor extractor;
    std::vector<std::string> aliFiles;
    extractor.findAliFiles(test_dir.string(), aliFiles);
    ASSERT_EQ(aliFiles.size(), 1);
    EXPECT_EQ(std::filesystem::path(aliFiles[0]).filename(), "my_package.ali");
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::vector<std::string> aliFiles;
    extractor.findAliFiles(test_dir.string(), aliFiles);
    extractor.extractAdaMetadata(component, aliFiles);
    ASSERT_EQ(component.packageManager, "GNAT");
    ASSERT_EQ(component.dependencies.size(), 1);
    EXPECT_EQ(component.dependencies[0], "my_package");
    ASSERT_EQ(component.sourceFiles.size(), 1);
    EXPECT_EQ(component.sourceFiles[0], "main.adb");
}

TEST_F(AdaExtractorTest, IsAliFile) {
    AdaExtractor extractor;
    EXPECT_TRUE(extractor.isAliFile("test.ali"));
    EXPECT_FALSE(extractor.isAliFile("test.txt"));
    EXPECT_FALSE(extractor.isAliFile("testali"));
}

TEST_F(AdaExtractorTest, IsRuntimePackage) {
    AdaExtractor extractor;
    EXPECT_TRUE(extractor.isRuntimePackage("ada.strings"));
    EXPECT_TRUE(extractor.isRuntimePackage("system.io"));
    EXPECT_TRUE(extractor.isRuntimePackage("ada"));  // Test exact match
    EXPECT_FALSE(extractor.isRuntimePackage("my_package"));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_EmptyAliFile) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string empty_ali = "empty.ali";
    std::ofstream(empty_ali).close();
    std::vector<std::string> aliFiles = {empty_ali};
    EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_CorruptAliFile) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string corrupt_ali = "corrupt.ali";
    std::ofstream(corrupt_ali) << "This is not a valid ALI file";
    std::vector<std::string> aliFiles = {corrupt_ali};
    EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_RuntimeOnly) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string runtime_ali = "runtime.ali";
    std::ofstream(runtime_ali) << "V \"GNAT Lib v2022\"\nW ada%b runtime.adb runtime.ali\n";
    std::vector<std::string> aliFiles = {runtime_ali};
    extractor.extractAdaMetadata(component, aliFiles);
    // Should not add runtime package as dependency
    EXPECT_TRUE(component.dependencies.empty());
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_DuplicateDependencies) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string dup_ali = "dup.ali";
    std::ofstream(dup_ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {dup_ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
    EXPECT_EQ(component.dependencies[0], "my_package");
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_MissingWLine) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string no_w_ali = "no_w.ali";
    std::ofstream(no_w_ali) << "V \"GNAT Lib v2022\"\n";
    std::vector<std::string> aliFiles = {no_w_ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_TRUE(component.dependencies.empty());
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_MultipleAliFiles) {
    std::cout << "DEBUG: ExtractAdaMetadata_MultipleAliFiles test starting" << std::endl;
    try {
        AdaExtractor extractor;
        ComponentInfo component;
        std::string ali1 = "ada_test_pkg1.ali";
        std::string ali2 = "ada_test_pkg2.ali";
        
        std::cout << "DEBUG: Test directory: " << test_dir << std::endl;
        std::cout << "DEBUG: Test directory exists: " << (std::filesystem::exists(test_dir) ? "yes" : "no") << std::endl;
        std::cout << "DEBUG: Test directory is directory: " << (std::filesystem::is_directory(test_dir) ? "yes" : "no") << std::endl;
        std::cout << "DEBUG: Current working directory: " << std::filesystem::current_path() << std::endl;
        std::cout << "DEBUG: ali1 path: " << ali1 << std::endl;
        std::cout << "DEBUG: ali2 path: " << ali2 << std::endl;
        
        // Create test files with proper synchronization
        std::cout << "DEBUG: Creating first ALI file..." << std::endl;
        std::cout << "DEBUG: About to open file: " << ali1 << std::endl;
        std::cout << "DEBUG: Current directory before file creation: " << std::filesystem::current_path() << std::endl;
        std::ofstream file1(ali1);
        if (!file1.is_open()) {
            std::cerr << "ERROR: Failed to create " << ali1 << std::endl;
            std::cerr << "ERROR: errno: " << errno << " (" << std::strerror(errno) << ")" << std::endl;
            FAIL() << "Failed to create " << ali1;
        }
        std::cout << "DEBUG: File opened successfully, writing content..." << std::endl;
        file1 << "V \"GNAT Lib v2022\"\nW pkg1%b file1.adb file1.ali\n";
        file1.flush();
        file1.close();
        std::cout << "DEBUG: First ALI file created successfully" << std::endl;
        std::cout << "DEBUG: Current directory after file creation: " << std::filesystem::current_path() << std::endl;
        
        // Check if file exists immediately after creation
        std::cout << "DEBUG: Checking if file exists immediately after creation..." << std::endl;
        bool file_exists_immediately = std::filesystem::exists(ali1);
        std::cout << "DEBUG: File exists immediately: " << (file_exists_immediately ? "yes" : "no") << std::endl;
        if (file_exists_immediately) {
            try {
                std::cout << "DEBUG: File size immediately: " << std::filesystem::file_size(ali1) << std::endl;
            } catch (const std::exception& e) {
                std::cout << "DEBUG: Error getting file size: " << e.what() << std::endl;
            }
        }
        
        std::cout << "DEBUG: Creating second ALI file..." << std::endl;
        std::ofstream file2(ali2);
        if (!file2.is_open()) {
            std::cerr << "ERROR: Failed to create " << ali2 << std::endl;
            FAIL() << "Failed to create " << ali2;
        }
        file2 << "V \"GNAT Lib v2022\"\nW pkg2%b file2.adb file2.ali\n";
        file2.flush();
        file2.close();
        std::cout << "DEBUG: Second ALI file created successfully" << std::endl;
        
        // Check if both files exist after second file creation
        std::cout << "DEBUG: Checking both files after second file creation..." << std::endl;
        bool ali1_exists_after_second = std::filesystem::exists(ali1);
        bool ali2_exists_after_second = std::filesystem::exists(ali2);
        std::cout << "DEBUG: ali1 exists after second file: " << (ali1_exists_after_second ? "yes" : "no") << std::endl;
        std::cout << "DEBUG: ali2 exists after second file: " << (ali2_exists_after_second ? "yes" : "no") << std::endl;
        
        // Ensure files are written to disk and synchronized
        std::cout << "DEBUG: Ensuring filesystem synchronization..." << std::endl;
        
        // Small delay to ensure filesystem synchronization in CI environments
        std::cout << "DEBUG: Waiting for filesystem synchronization..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Debug: Check current directory and list files before verification
        std::cout << "DEBUG: Current directory before verification: " << std::filesystem::current_path() << std::endl;
        std::cout << "DEBUG: Listing files in current directory:" << std::endl;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(".")) {
                std::cout << "DEBUG:   " << entry.path().filename() << " (size: " << entry.file_size() << ")" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "DEBUG: Error listing directory: " << e.what() << std::endl;
        }
        
        // Verify files were created with retries for CI environments
        std::cout << "DEBUG: Verifying file creation..." << std::endl;
        
        // Try multiple times to verify file existence (CI filesystem timing issues)
        int retry_count = 0;
        const int max_retries = 5;
        bool ali1_exists = false;
        bool ali2_exists = false;
        
        while (retry_count < max_retries && (!ali1_exists || !ali2_exists)) {
            std::cout << "DEBUG: Verification attempt " << (retry_count + 1) << "/" << max_retries << std::endl;
            
            if (!ali1_exists) {
                ali1_exists = std::filesystem::exists(ali1);
                std::cout << "DEBUG: ali1 exists: " << (ali1_exists ? "yes" : "no") << std::endl;
            }
            
            if (!ali2_exists) {
                ali2_exists = std::filesystem::exists(ali2);
                std::cout << "DEBUG: ali2 exists: " << (ali2_exists ? "yes" : "no") << std::endl;
            }
            
            if (!ali1_exists || !ali2_exists) {
                std::cout << "DEBUG: Files not found, waiting before retry..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                retry_count++;
            }
        }
        
        if (!ali1_exists) {
            std::cerr << "ERROR: File " << ali1 << " was not created after " << max_retries << " attempts" << std::endl;
            FAIL() << "File " << ali1 << " was not created";
        }
        
        if (!ali2_exists) {
            std::cerr << "ERROR: File " << ali2 << " was not created after " << max_retries << " attempts" << std::endl;
            FAIL() << "File " << ali2 << " was not created";
        }
        
        std::cout << "DEBUG: Both files verified successfully" << std::endl;
        
        // Check file sizes
        std::cout << "DEBUG: ali1 file size: " << std::filesystem::file_size(ali1) << std::endl;
        std::cout << "DEBUG: ali2 file size: " << std::filesystem::file_size(ali2) << std::endl;
        
        std::vector<std::string> aliFiles = {ali1, ali2};
        std::cout << "DEBUG: About to call extractAdaMetadata with " << aliFiles.size() << " files..." << std::endl;
        
        extractor.extractAdaMetadata(component, aliFiles);
        
        std::cout << "DEBUG: extractAdaMetadata completed" << std::endl;
        std::cout << "DEBUG: Component dependencies count: " << component.dependencies.size() << std::endl;
        
        for (size_t i = 0; i < component.dependencies.size(); ++i) {
            std::cout << "DEBUG: Dependency " << i << ": '" << component.dependencies[i] << "'" << std::endl;
        }
        
        EXPECT_EQ(component.dependencies.size(), 2);
        
        auto pkg1_found = std::find(component.dependencies.begin(), component.dependencies.end(), "pkg1");
        std::cout << "DEBUG: pkg1 found: " << (pkg1_found != component.dependencies.end() ? "yes" : "no") << std::endl;
        EXPECT_NE(pkg1_found, component.dependencies.end());
        
        auto pkg2_found = std::find(component.dependencies.begin(), component.dependencies.end(), "pkg2");
        std::cout << "DEBUG: pkg2 found: " << (pkg2_found != component.dependencies.end() ? "yes" : "no") << std::endl;
        EXPECT_NE(pkg2_found, component.dependencies.end());
        
        std::cout << "DEBUG: ExtractAdaMetadata_MultipleAliFiles test completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception in ExtractAdaMetadata_MultipleAliFiles test: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "ERROR: Unknown exception in ExtractAdaMetadata_MultipleAliFiles test" << std::endl;
        throw;
    }
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_VerboseMode) {
    AdaExtractor extractor;
    extractor.setVerbose(true);
    ComponentInfo component;
    std::string ali = "verbose.ali";
    std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_TestMode) {
    AdaExtractor::setTestMode(true);
    AdaExtractor extractor;
    ComponentInfo component;
    std::string ali = "testmode.ali";
    std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
    AdaExtractor::setTestMode(false);
} 