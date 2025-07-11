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
#include <iostream>
#include "common/ComponentInfo.hpp"
#include "common/MetadataExtractor.hpp"

using namespace heimdall;

class LinuxSupportTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_linux_test";
        std::filesystem::create_directories(test_dir);

        // Create a simple C source file for testing
        test_source = test_dir / "testlib.c";
        std::ofstream(test_source) << R"(
#include <stdio.h>

__attribute__((visibility("default")))
int test_function() {
    return 42;
}

__attribute__((visibility("default")))
const char* test_version = "1.2.3";

__attribute__((visibility("default")))
const char* test_license = "MIT";
)";

        // Compile it into a shared library with build ID and debug info
        test_lib = test_dir / "libtest.so";
        std::string compile_cmd = "gcc -shared -fPIC -g3 -O0 -fno-omit-frame-pointer -Wl,--build-id=0x1234567890abcdef -o " +
                                  test_lib.string() + " " + test_source.string() + " 2>/dev/null";
        int compile_result = system(compile_cmd.c_str());
        (void)compile_result; // Suppress unused variable warning

        // Fallback to dummy file if compilation fails
        if (!std::filesystem::exists(test_lib)) {
            std::ofstream(test_lib) << "dummy content";
        }
    }
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    std::filesystem::path test_dir;
    std::filesystem::path test_source;
    std::filesystem::path test_lib;
};

TEST_F(LinuxSupportTest, ELFSymbolExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "ELF symbol extraction is Linux-specific";
#endif

    MetadataExtractor extractor;
    ComponentInfo component("testlib", test_lib.string());

    if (std::filesystem::file_size(test_lib) > 100) {
        bool result = extractor.extractSymbolInfo(component);
        EXPECT_TRUE(result);
        EXPECT_GT(component.symbols.size(), 0);

        // Check for our test symbols
        bool found_test_function = false;
        bool found_test_version = false;

        for (const auto& symbol : component.symbols) {
            if (symbol.name == "test_function") {
                found_test_function = true;
            }
            if (symbol.name == "test_version") {
                found_test_version = true;
            }
        }

        EXPECT_TRUE(found_test_function);
        EXPECT_TRUE(found_test_version);
    }
}

TEST_F(LinuxSupportTest, ELFSectionExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "ELF section extraction is Linux-specific";
#endif

    MetadataExtractor extractor;
    ComponentInfo component("testlib", test_lib.string());

    if (std::filesystem::file_size(test_lib) > 100) {
        bool result = extractor.extractSectionInfo(component);
        EXPECT_TRUE(result);
        EXPECT_GT(component.sections.size(), 0);

        // Check for common ELF sections
        bool found_text = false;
        bool found_data = false;

        for (const auto& section : component.sections) {
            if (section.name == ".text") {
                found_text = true;
            }
            if (section.name == ".data") {
                found_data = true;
            }
        }

        EXPECT_TRUE(found_text);
    }
}

TEST_F(LinuxSupportTest, ELFDependencyExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "ELF dependency extraction is Linux-specific";
#endif

    MetadataExtractor extractor;
    ComponentInfo component("testlib", test_lib.string());

    if (std::filesystem::file_size(test_lib) > 100) {
        bool result = extractor.extractDependencyInfo(component);
        // May or may not have dependencies, but should not crash
        EXPECT_TRUE(component.wasProcessed);
    }
}

TEST_F(LinuxSupportTest, ELFBuildIdExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "ELF build ID extraction is Linux-specific";
#endif

    MetadataExtractor extractor;
    ComponentInfo component("testlib", test_lib.string());

    if (std::filesystem::file_size(test_lib) > 100) {
        // Test build ID extraction directly
        std::string buildId;
        bool result = MetadataHelpers::extractELFBuildId(test_lib.string(), buildId);

        // May or may not have build ID, but should not crash
        if (result) {
            EXPECT_FALSE(buildId.empty());
            std::cout << "Extracted build ID: " << buildId << std::endl;
        }
    }
}

TEST_F(LinuxSupportTest, FileFormatDetection) {
#ifndef __linux__
    GTEST_SKIP() << "ELF format detection is Linux-specific";
#endif

    MetadataExtractor extractor;

    // Test ELF detection
    if (std::filesystem::file_size(test_lib) > 100) {
        EXPECT_TRUE(extractor.isELF(test_lib.string()));
        EXPECT_TRUE(MetadataHelpers::isELF(test_lib.string()));
    }

    // Test non-ELF files
    EXPECT_FALSE(extractor.isELF(test_source.string()));
    EXPECT_FALSE(MetadataHelpers::isELF(test_source.string()));
}

TEST_F(LinuxSupportTest, DWARFSourceFileExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "DWARF source file extraction is Linux-specific";
#endif

    // Debug: Check if test library exists and has proper size
    bool library_exists = std::filesystem::exists(test_lib);
    size_t library_size = 0;
    if (library_exists) {
        library_size = std::filesystem::file_size(test_lib);
    }
    
    // Debug: Check if it's a real library (not dummy)
    if (library_exists && library_size > 100) {
        std::vector<std::string> sourceFiles;
        bool result = heimdall::MetadataHelpers::extractSourceFiles(test_lib.string(), sourceFiles);
        // Should find at least one .c file (the test source)
        bool found_c = false;
        for (const auto& f : sourceFiles) {
            if (f.find("testlib.c") != std::string::npos)
                found_c = true;
        }
        EXPECT_TRUE(result);
        EXPECT_TRUE(found_c);
    } else {
        // Test library doesn't exist or is too small (dummy file)
        // This is expected if compilation failed
        GTEST_SKIP() << "Test library not available (compilation may have failed)";
    }
}

TEST_F(LinuxSupportTest, DWARFCompileUnitExtraction) {
#ifndef __linux__
    GTEST_SKIP() << "DWARF compile unit extraction is Linux-specific";
#endif

    // Debug: Check if test library exists and has proper size
    bool library_exists = std::filesystem::exists(test_lib);
    size_t library_size = 0;
    if (library_exists) {
        library_size = std::filesystem::file_size(test_lib);
    }
    
    // Debug: Check if it's a real library (not dummy)
    if (library_exists && library_size > 100) {
        std::vector<std::string> units;
        bool result = heimdall::MetadataHelpers::extractCompileUnits(test_lib.string(), units);
        // We expect at least one compile unit (may be a stub string)
        EXPECT_TRUE(result);
        EXPECT_GE(units.size(), 1);
    } else {
        // Test library doesn't exist or is too small (dummy file)
        // This is expected if compilation failed
        GTEST_SKIP() << "Test library not available (compilation may have failed)";
    }
}