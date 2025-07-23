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
#include "common/AdaExtractor.hpp"
#include "common/ComponentInfo.hpp"
#include "common/Utils.hpp"

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
        std::filesystem::remove_all(test_dir);
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
    AdaExtractor extractor;
    ComponentInfo component;
    std::string ali1 = (test_dir / "pkg1.ali").string();
    std::string ali2 = (test_dir / "pkg2.ali").string();
    std::ofstream(ali1) << "V \"GNAT Lib v2022\"\nW pkg1%b file1.adb file1.ali\n";
    std::ofstream(ali2) << "V \"GNAT Lib v2022\"\nW pkg2%b file2.adb file2.ali\n";
    std::vector<std::string> aliFiles = {ali1, ali2};
    extractor.extractAdaMetadata(component, aliFiles);
    EXPECT_EQ(component.dependencies.size(), 2);
    EXPECT_NE(std::find(component.dependencies.begin(), component.dependencies.end(), "pkg1"), component.dependencies.end());
    EXPECT_NE(std::find(component.dependencies.begin(), component.dependencies.end(), "pkg2"), component.dependencies.end());
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