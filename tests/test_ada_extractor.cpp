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
#include "common/AdaExtractor.hpp"
#include "common/ComponentInfo.hpp"
#include "common/Utils.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class AdaExtractorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_ada_test";
        std::filesystem::create_directories(test_dir);
        // Create a dummy ali file named my_package.ali
        std::ofstream ali_file(test_dir / "my_package.ali");
        ali_file << "V \"GNAT Lib v2022\"" << std::endl;
        ali_file << "W my_package%b main.adb main.ali" << std::endl;
        ali_file.close();
    }

    void TearDown() override {
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
    std::string empty_ali = (test_dir / "empty.ali").string();
    std::ofstream(empty_ali).close();
    std::vector<std::string> aliFiles = {empty_ali};
    EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_CorruptAliFile) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string corrupt_ali = (test_dir / "corrupt.ali").string();
    std::ofstream(corrupt_ali) << "This is not a valid ALI file";
    std::vector<std::string> aliFiles = {corrupt_ali};
    EXPECT_FALSE(extractor.extractAdaMetadata(component, aliFiles));
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_RuntimeOnly) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string runtime_ali = (test_dir / "runtime.ali").string();
    std::ofstream(runtime_ali) << "V \"GNAT Lib v2022\"\nW ada%b runtime.adb runtime.ali\n";
    std::vector<std::string> aliFiles = {runtime_ali};
    extractor.extractAdaMetadata(component, aliFiles);
    // Should not add runtime package as dependency
    EXPECT_TRUE(component.dependencies.empty());
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_DuplicateDependencies) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string dup_ali = (test_dir / "dup.ali").string();
    std::ofstream(dup_ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {dup_ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
    EXPECT_EQ(component.dependencies[0], "my_package");
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_MissingWLine) {
    AdaExtractor extractor;
    ComponentInfo component;
    std::string no_w_ali = (test_dir / "no_w.ali").string();
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
        std::string ali1 = (test_dir / "pkg1.ali").string();
        std::string ali2 = (test_dir / "pkg2.ali").string();
        
        std::cout << "DEBUG: Test directory: " << test_dir << std::endl;
        std::cout << "DEBUG: ali1 path: " << ali1 << std::endl;
        std::cout << "DEBUG: ali2 path: " << ali2 << std::endl;
        
        // Create test files with proper synchronization
        std::cout << "DEBUG: Creating first ALI file..." << std::endl;
        std::ofstream file1(ali1);
        if (!file1.is_open()) {
            std::cerr << "ERROR: Failed to create " << ali1 << std::endl;
            FAIL() << "Failed to create " << ali1;
        }
        file1 << "V \"GNAT Lib v2022\"\nW pkg1%b file1.adb file1.ali\n";
        file1.close();
        std::cout << "DEBUG: First ALI file created successfully" << std::endl;
        
        std::cout << "DEBUG: Creating second ALI file..." << std::endl;
        std::ofstream file2(ali2);
        if (!file2.is_open()) {
            std::cerr << "ERROR: Failed to create " << ali2 << std::endl;
            FAIL() << "Failed to create " << ali2;
        }
        file2 << "V \"GNAT Lib v2022\"\nW pkg2%b file2.adb file2.ali\n";
        file2.close();
        std::cout << "DEBUG: Second ALI file created successfully" << std::endl;
        
        // Ensure files are written to disk
        std::cout << "DEBUG: Flushing files to disk..." << std::endl;
        file1.flush();
        file2.flush();
        
        // Small delay to ensure filesystem synchronization in CI environments
        std::cout << "DEBUG: Waiting for filesystem synchronization..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Verify files were created
        std::cout << "DEBUG: Verifying file creation..." << std::endl;
        if (!std::filesystem::exists(ali1)) {
            std::cerr << "ERROR: File " << ali1 << " was not created" << std::endl;
            FAIL() << "File " << ali1 << " was not created";
        }
        std::cout << "DEBUG: ali1 exists" << std::endl;
        
        if (!std::filesystem::exists(ali2)) {
            std::cerr << "ERROR: File " << ali2 << " was not created" << std::endl;
            FAIL() << "File " << ali2 << " was not created";
        }
        std::cout << "DEBUG: ali2 exists" << std::endl;
        
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
    std::string ali = (test_dir / "verbose.ali").string();
    std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
}

TEST_F(AdaExtractorTest, ExtractAdaMetadata_TestMode) {
    AdaExtractor::setTestMode(true);
    AdaExtractor extractor;
    ComponentInfo component;
    std::string ali = (test_dir / "testmode.ali").string();
    std::ofstream(ali) << "V \"GNAT Lib v2022\"\nW my_package%b main.adb main.ali\n";
    std::vector<std::string> aliFiles = {ali};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 1);
    AdaExtractor::setTestMode(false);
} 