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