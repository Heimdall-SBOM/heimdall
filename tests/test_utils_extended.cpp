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
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Utils.hpp"

using namespace heimdall;

class UtilsExtendedTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_utils_extended_test";
        std::filesystem::create_directories(test_dir);

        // Create test files
        test_file = test_dir / "test.txt";
        std::ofstream(test_file) << "test content";

        // Create a larger file for checksum testing
        large_file = test_dir / "large.bin";
        std::ofstream large_stream(large_file, std::ios::binary);
        for (int i = 0; i < 1000; ++i) {
            large_stream.write(reinterpret_cast<const char*>(&i), sizeof(i));
        }
        large_stream.close();

        // Create test directories
        sub_dir = test_dir / "subdir";
        std::filesystem::create_directories(sub_dir);

        // Set environment variable for testing
        setenv("TEST_VAR", "test_value", 1);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
        unsetenv("TEST_VAR");
    }

    std::filesystem::path test_dir;
    std::filesystem::path test_file;
    std::filesystem::path large_file;
    std::filesystem::path sub_dir;
};

TEST_F(UtilsExtendedTest, NormalizePath) {
    EXPECT_EQ(Utils::normalizePath("/usr/lib/../lib64"), "/usr/lib64");
    EXPECT_EQ(Utils::normalizePath("./test/../file.txt"), "file.txt");
    EXPECT_EQ(Utils::normalizePath("//usr//lib//"), "/usr/lib/");
    EXPECT_EQ(Utils::normalizePath(""), "");
}

TEST_F(UtilsExtendedTest, SplitPath) {
    auto parts = Utils::splitPath("/usr/lib/x86_64-linux-gnu");
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_EQ(parts[0], "/");
    EXPECT_EQ(parts[1], "usr");
    EXPECT_EQ(parts[2], "lib");
    EXPECT_EQ(parts[3], "x86_64-linux-gnu");

    auto relative_parts = Utils::splitPath("test/subdir/file.txt");
    ASSERT_EQ(relative_parts.size(), 3u);
    EXPECT_EQ(relative_parts[0], "test");
    EXPECT_EQ(relative_parts[1], "subdir");
    EXPECT_EQ(relative_parts[2], "file.txt");
}

TEST_F(UtilsExtendedTest, GetFileSize) {
    EXPECT_GT(Utils::getFileSize(test_file.string()), 0u);
    EXPECT_EQ(Utils::getFileSize((test_dir / "nonexistent.txt").string()), 0u);
    EXPECT_EQ(Utils::getFileSize(large_file.string()), 4000u);  // 1000 * sizeof(int)
}

TEST_F(UtilsExtendedTest, GetFileChecksum) {
    std::string checksum = Utils::getFileChecksum(test_file.string());
    EXPECT_FALSE(checksum.empty());
    EXPECT_EQ(checksum.length(), 64u);  // SHA256 is 32 bytes = 64 hex chars

    // Same file should have same checksum
    std::string checksum2 = Utils::getFileChecksum(test_file.string());
    EXPECT_EQ(checksum, checksum2);

    // Different files should have different checksums
    std::string large_checksum = Utils::getFileChecksum(large_file.string());
    EXPECT_NE(checksum, large_checksum);

    // Non-existent file should return empty string
    EXPECT_TRUE(Utils::getFileChecksum((test_dir / "nonexistent.txt").string()).empty());
}

TEST_F(UtilsExtendedTest, StringManipulation) {
    EXPECT_EQ(Utils::toLower("Hello World"), "hello world");
    EXPECT_EQ(Utils::toUpper("hello world"), "HELLO WORLD");
    EXPECT_EQ(Utils::trim("  test  "), "test");
    EXPECT_EQ(Utils::trim("\t\n\r test \t\n\r"), "test");
    EXPECT_EQ(Utils::trim(""), "");
    EXPECT_EQ(Utils::trim("no_spaces"), "no_spaces");
}

TEST_F(UtilsExtendedTest, StringOperations) {
    EXPECT_TRUE(Utils::startsWith("hello world", "hello"));
    EXPECT_FALSE(Utils::startsWith("hello world", "world"));
    EXPECT_TRUE(Utils::endsWith("hello world", "world"));
    EXPECT_FALSE(Utils::endsWith("hello world", "hello"));

    EXPECT_EQ(Utils::replace("hello world", "world", "universe"), "hello universe");
    EXPECT_EQ(Utils::replace("hello hello", "hello", "hi"), "hi hi");
    EXPECT_EQ(Utils::replace("no change", "missing", "replacement"), "no change");
}

TEST_F(UtilsExtendedTest, SplitAndJoin) {
    auto parts = Utils::split("a:b:c", ':');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");

    EXPECT_EQ(Utils::join(parts, ":"), "a:b:c");
    EXPECT_EQ(Utils::join(parts, "-"), "a-b-c");
    EXPECT_EQ(Utils::join({}, ":"), "");
    EXPECT_EQ(Utils::join({"single"}, ":"), "single");
}

TEST_F(UtilsExtendedTest, FileTypeDetection) {
    EXPECT_TRUE(Utils::isObjectFile("test.o"));
    EXPECT_TRUE(Utils::isObjectFile("test.obj"));
    EXPECT_FALSE(Utils::isObjectFile("test.txt"));

    EXPECT_TRUE(Utils::isStaticLibrary("libtest.a"));
    EXPECT_TRUE(Utils::isStaticLibrary("test.lib"));
    EXPECT_FALSE(Utils::isStaticLibrary("test.txt"));

    EXPECT_TRUE(Utils::isSharedLibrary("libtest.so"));
    EXPECT_TRUE(Utils::isSharedLibrary("test.dylib"));
    EXPECT_TRUE(Utils::isSharedLibrary("test.dll"));
    EXPECT_FALSE(Utils::isSharedLibrary("test.txt"));

    EXPECT_TRUE(Utils::isExecutable("test.exe"));
    EXPECT_TRUE(Utils::isExecutable("bin/test"));
    EXPECT_FALSE(Utils::isExecutable("test.txt"));
}

TEST_F(UtilsExtendedTest, CalculateSHA256) {
    // Should be same as getFileChecksum
    std::string checksum1 = Utils::getFileChecksum(test_file.string());
    std::string checksum2 = Utils::calculateSHA256(test_file.string());
    EXPECT_EQ(checksum1, checksum2);
}

TEST_F(UtilsExtendedTest, LicenseDetection) {
    EXPECT_EQ(Utils::detectLicenseFromName("libssl"), "Apache-2.0");
    EXPECT_EQ(Utils::detectLicenseFromName("libcrypto"), "Apache-2.0");
    EXPECT_EQ(Utils::detectLicenseFromName("libpthread"), "MIT");
    EXPECT_EQ(Utils::detectLicenseFromName("libc"), "LGPL-2.1");
    EXPECT_EQ(Utils::detectLicenseFromName("libm"), "LGPL-2.1");
    EXPECT_EQ(Utils::detectLicenseFromName("libgcc"), "GPL-3.0");
    EXPECT_EQ(Utils::detectLicenseFromName("unknown_lib"), "NOASSERTION");
}

TEST_F(UtilsExtendedTest, LicenseDetectionFromPath) {
    EXPECT_EQ(Utils::detectLicenseFromPath("/usr/lib/libssl.so"), "LGPL-2.1");
    EXPECT_EQ(Utils::detectLicenseFromPath("/usr/lib/libc.so"), "LGPL-2.1");
    EXPECT_EQ(Utils::detectLicenseFromPath("/usr/lib/unknown.so"), "LGPL-2.1");
}

TEST_F(UtilsExtendedTest, EnvironmentVariables) {
    EXPECT_EQ(Utils::getEnvironmentVariable("TEST_VAR"), "test_value");
    EXPECT_EQ(Utils::getEnvironmentVariable("NONEXISTENT_VAR"), "");
    EXPECT_EQ(Utils::getEnvironmentVariable(""), "");
}

TEST_F(UtilsExtendedTest, CurrentWorkingDirectory) {
    std::string cwd = Utils::getCurrentWorkingDirectory();
    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(std::filesystem::exists(cwd));
}

TEST_F(UtilsExtendedTest, LibrarySearchPaths) {
    auto paths = Utils::getLibrarySearchPaths();
    EXPECT_FALSE(paths.empty());

    // Should contain standard paths
    bool has_usr_lib = false;
    bool has_usr_lib64 = false;
    for (const auto& path : paths) {
        if (path.find("/usr/lib") != std::string::npos)
            has_usr_lib = true;
        if (path.find("/usr/lib64") != std::string::npos)
            has_usr_lib64 = true;
    }
    EXPECT_TRUE(has_usr_lib || has_usr_lib64);
}

TEST_F(UtilsExtendedTest, FindLibrary) {
    // Test with a library that should exist
    std::string libc_path = Utils::findLibrary("libc.so");
    if (!libc_path.empty()) {
        EXPECT_TRUE(Utils::fileExists(libc_path));
    }

    // Test with non-existent library
    EXPECT_TRUE(Utils::findLibrary("nonexistent_library.so").empty());
}

TEST_F(UtilsExtendedTest, IsSystemLibrary) {
    // Test with system library paths
    EXPECT_TRUE(Utils::isSystemLibrary("/usr/lib/libc.so"));
    EXPECT_TRUE(Utils::isSystemLibrary("/usr/lib64/libm.so"));
    EXPECT_TRUE(Utils::isSystemLibrary("/lib/libpthread.so"));

    // Test with non-system paths
    EXPECT_FALSE(Utils::isSystemLibrary("/home/user/libtest.so"));
    EXPECT_FALSE(Utils::isSystemLibrary("./libtest.so"));
}

TEST_F(UtilsExtendedTest, ExtractPackageName) {
    EXPECT_EQ(Utils::extractPackageName("/usr/lib/libssl-1.1.so"), "ssl-1.1");
    EXPECT_EQ(Utils::extractPackageName("/usr/lib/libcrypto.so.1.1"), "crypto.so.1.1");
    EXPECT_EQ(Utils::extractPackageName("libtest.so"), "test");
    EXPECT_EQ(Utils::extractPackageName("test.o"), "test.o");
    EXPECT_EQ(Utils::extractPackageName(""), "");
}

TEST_F(UtilsExtendedTest, DebugPrint) {
    // Test that debug print doesn't crash
    Utils::debugPrint("Test debug message");
    Utils::debugPrint("");
    Utils::debugPrint("Message with special chars: \n\t\"\\");
}

TEST_F(UtilsExtendedTest, PathOperations) {
    // Test path operations with various inputs
    EXPECT_EQ(Utils::getFileName("/usr/lib/libtest.so"), "libtest.so");
    EXPECT_EQ(Utils::getFileName("libtest.so"), "libtest.so");
    EXPECT_EQ(Utils::getFileName(""), "");

    EXPECT_EQ(Utils::getFileExtension("/usr/lib/libtest.so"), ".so");
    EXPECT_EQ(Utils::getFileExtension("libtest.so"), ".so");
    EXPECT_EQ(Utils::getFileExtension("test"), "");
    EXPECT_EQ(Utils::getFileExtension(""), "");

    EXPECT_EQ(Utils::getDirectory("/usr/lib/libtest.so"), "/usr/lib");
    EXPECT_EQ(Utils::getDirectory("libtest.so"), "");
    EXPECT_EQ(Utils::getDirectory(""), "");
}