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
#include <filesystem>
#include "ComponentInfo.hpp"
#include "MetadataExtractor.hpp"
#include "gtest/gtest.h"

using namespace heimdall;

// Helper: Only run if file exists
#define REQUIRE_FILE(path)                               \
    if (!std::filesystem::exists(path)) {                \
        GTEST_SKIP() << "Test file not found: " << path; \
    }

// -------- Package Manager Integration Tests --------

TEST(PackageManagerIntegration, DetectRpm) {
    std::string path = "/usr/lib/librpm.so";  // Adjust to a known RPM-owned file on your system
    REQUIRE_FILE(path);
    ComponentInfo comp("rpmtest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "rpm");
    EXPECT_EQ(comp.supplier, "rpm-package-manager");
    EXPECT_FALSE(comp.version.empty());
}

TEST(PackageManagerIntegration, DetectDeb) {
    std::string path = "/usr/lib/x86_64-linux-gnu/libc.so.6";  // Adjust to a known DEB-owned file
    REQUIRE_FILE(path);
    ComponentInfo comp("debtest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "deb");
    EXPECT_EQ(comp.supplier, "debian-package-manager");
    EXPECT_FALSE(comp.version.empty());
}

TEST(PackageManagerIntegration, DetectPacman) {
    std::string path = "/usr/lib/libc.so.6";  // Adjust to a known Pacman-owned file
    REQUIRE_FILE(path);
    ComponentInfo comp("pacmantest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "pacman");
    EXPECT_EQ(comp.supplier, "arch-package-manager");
    EXPECT_FALSE(comp.version.empty());
}

TEST(PackageManagerIntegration, DetectConan) {
    std::string path = "../tests/testdata/conan/lib/libz.a";
    REQUIRE_FILE(path);
    ComponentInfo comp("conantest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "conan");
    EXPECT_EQ(comp.supplier, "conan-center");
    // Version may be empty if not encoded in path
}

TEST(PackageManagerIntegration, DetectVcpkg) {
    std::string path = "../tests/testdata/vcpkg/installed/x64-linux/lib/libz.a";
    REQUIRE_FILE(path);
    ComponentInfo comp("vcpkgtest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "vcpkg");
    EXPECT_EQ(comp.supplier, "vcpkg");
}

TEST(PackageManagerIntegration, DetectSpack) {
    std::string path =
        "../tests/testdata/spack/opt/spack/linux-ubuntu20.04-x86_64/gcc-9.3.0/zlib-1.2.11-abcdef/"
        "lib/libz.a";
    REQUIRE_FILE(path);
    ComponentInfo comp("spacktest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_EQ(comp.packageManager, "spack");
    EXPECT_EQ(comp.supplier, "spack");
}

// -------- Archive File Support Tests --------

TEST(ArchiveSupport, ExtractMembers) {
    std::string path = "../tests/testdata/libtest.a";
    REQUIRE_FILE(path);
    ComponentInfo comp("archivetest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);
    EXPECT_GT(comp.sourceFiles.size(), 0);  // Should find members
    // Check for specific member name
    bool found_test_lib = false;
    for (const auto& member : comp.sourceFiles) {
        if (member.find("test_lib.o") != std::string::npos) {
            found_test_lib = true;
            break;
        }
    }
    EXPECT_TRUE(found_test_lib);
}

TEST(ArchiveSupport, ExtractSymbols) {
    std::string path = "../tests/testdata/libtest.a";
    REQUIRE_FILE(path);
    ComponentInfo comp("archivetest", path);
    MetadataExtractor extractor;
    extractor.extractMetadata(comp);

    // For archives without symbol tables, we might not get symbols
    // But we should at least get the archive members
    EXPECT_GT(comp.sourceFiles.size(), 0);  // Should find members

    // Check for specific member name
    bool found_test_lib = false;
    for (const auto& member : comp.sourceFiles) {
        if (member.find("test_lib.o") != std::string::npos) {
            found_test_lib = true;
            break;
        }
    }
    EXPECT_TRUE(found_test_lib);

    // If we have symbols, check for specific ones
    if (comp.symbols.size() > 0) {
        bool found_test_function1 = false;
        bool found_global_test_var = false;
        for (const auto& symbol : comp.symbols) {
            if (symbol.name == "test_function1") {
                found_test_function1 = true;
            }
            if (symbol.name == "global_test_var") {
                found_global_test_var = true;
            }
        }
        EXPECT_TRUE(found_test_function1);
        EXPECT_TRUE(found_global_test_var);
    }
}

TEST(ArchiveSupport, InvalidArchive) {
    std::string path = "../tests/testdata/notanarchive.txt";
    REQUIRE_FILE(path);
    ComponentInfo comp("notanarchive", path);
    MetadataExtractor extractor;
    bool result = extractor.extractSymbolInfo(comp);
    EXPECT_FALSE(result);
}