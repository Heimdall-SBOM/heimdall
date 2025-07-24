/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <gtest/gtest.h>
#include "src/compat/compatibility.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "common/Utils.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = heimdall::compat::fs::temp_directory_path() / "heimdall_utils_test";
        heimdall::compat::fs::create_directories(test_dir);
        test_file = test_dir / "file.txt";
        std::ofstream(test_file) << "test content";
    }
    void TearDown() override {
        test_utils::safeRemoveDirectory(test_dir);
    }
    heimdall::compat::fs::path test_dir;
    heimdall::compat::fs::path test_file;
};

TEST_F(UtilsTest, GetDirectory) {
    std::string dir = Utils::getDirectory(test_file.string());
    EXPECT_EQ(dir, test_dir.string());
}

TEST_F(UtilsTest, GetFileName) {
    std::string name = Utils::getFileName(test_file.string());
    EXPECT_EQ(name, "file.txt");
}

TEST_F(UtilsTest, GetFileExtension) {
    std::string ext = Utils::getFileExtension(test_file.string());
    EXPECT_EQ(ext, ".txt");
}

TEST_F(UtilsTest, FileExists) {
    EXPECT_TRUE(Utils::fileExists(test_file.string()));
    EXPECT_FALSE(Utils::fileExists((test_dir / "nonexistent.txt").string()));
}

TEST_F(UtilsTest, IsSystemLibrary) {
    // This is a heuristic; just check it doesn't crash
    EXPECT_FALSE(Utils::isSystemLibrary(test_file.string()));
}

TEST(UtilsJson, EscapeJsonString) {
    std::string raw = "Heimdall\n\"SBOM\"";
    std::string escaped = Utils::escapeJsonString(raw);
    EXPECT_NE(escaped.find("\\n"), std::string::npos);
    EXPECT_NE(escaped.find("\\\""), std::string::npos);
}

TEST(UtilsJson, FormatJsonValue) {
    std::string value = "test\nvalue";
    std::string formatted = Utils::formatJsonValue(value);
    EXPECT_TRUE(formatted.front() == '"' && formatted.back() == '"');
}
