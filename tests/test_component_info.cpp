/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "ComponentInfo.hpp"

using namespace heimdall;

class ComponentInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_component_test";
        std::filesystem::create_directories(test_dir);
        test_file = test_dir / "libtest.so";
        std::ofstream(test_file) << "dummy content";
    }
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    std::filesystem::path test_dir;
    std::filesystem::path test_file;
};

TEST_F(ComponentInfoTest, DefaultConstructor) {
    ComponentInfo component;
    EXPECT_EQ(component.name, "");
    EXPECT_EQ(component.fileType, FileType::Unknown);
    EXPECT_EQ(component.symbols.size(), 0u);
    EXPECT_EQ(component.sections.size(), 0u);
}

TEST_F(ComponentInfoTest, PathConstructorAndFileType) {
    ComponentInfo component("libtest", test_file.string());
    EXPECT_EQ(component.name, "libtest");
    EXPECT_EQ(component.filePath, test_file.string());
    EXPECT_EQ(component.fileType, FileType::SharedLibrary);
    EXPECT_GT(component.fileSize, 0u);
    EXPECT_FALSE(component.checksum.empty());
}

TEST_F(ComponentInfoTest, AddSymbolAndSection) {
    ComponentInfo component;
    SymbolInfo symbol;
    symbol.name = "main";
    symbol.address = 0x1000;
    symbol.size = 42;
    symbol.isDefined = true;
    symbol.isGlobal = true;
    symbol.section = ".text";
    component.addSymbol(symbol);
    EXPECT_EQ(component.symbols.size(), 1u);
    EXPECT_EQ(component.symbols[0].name, "main");
    EXPECT_EQ(component.symbols[0].section, ".text");

    SectionInfo section;
    section.name = ".text";
    section.address = 0x1000;
    section.size = 100;
    section.type = "code";
    component.addSection(section);
    EXPECT_EQ(component.sections.size(), 1u);
    EXPECT_EQ(component.sections[0].name, ".text");
}

TEST_F(ComponentInfoTest, AddDependencyAndSourceFile) {
    ComponentInfo component;
    component.addDependency("libssl.so");
    component.addDependency("libssl.so"); // duplicate
    EXPECT_EQ(component.dependencies.size(), 1u);
    component.addSourceFile("main.cpp");
    component.addSourceFile("main.cpp"); // duplicate
    EXPECT_EQ(component.sourceFiles.size(), 1u);
}

TEST_F(ComponentInfoTest, SettersAndFlags) {
    ComponentInfo component;
    component.setVersion("1.2.3");
    component.setSupplier("TestOrg");
    component.setDownloadLocation("https://example.com");
    component.setHomepage("https://homepage");
    component.setLicense("MIT");
    component.setPackageManager("conan");
    component.setDetectedBy(LinkerType::LLD);
    component.markAsSystemLibrary();
    component.setContainsDebugInfo(true);
    component.setStripped(true);
    EXPECT_EQ(component.version, "1.2.3");
    EXPECT_EQ(component.supplier, "TestOrg");
    EXPECT_EQ(component.downloadLocation, "https://example.com");
    EXPECT_EQ(component.homepage, "https://homepage");
    EXPECT_EQ(component.license, "MIT");
    EXPECT_EQ(component.packageManager, "conan");
    EXPECT_EQ(component.detectedBy, LinkerType::LLD);
    EXPECT_TRUE(component.isSystemLibrary);
    EXPECT_TRUE(component.containsDebugInfo);
    EXPECT_TRUE(component.isStripped);
}

TEST_F(ComponentInfoTest, HasSymbolAndSection) {
    ComponentInfo component;
    SymbolInfo symbol;
    symbol.name = "foo";
    component.addSymbol(symbol);
    EXPECT_TRUE(component.hasSymbol("foo"));
    EXPECT_FALSE(component.hasSymbol("bar"));
    SectionInfo section;
    section.name = ".data";
    component.addSection(section);
    EXPECT_TRUE(component.hasSection(".data"));
    EXPECT_FALSE(component.hasSection(".bss"));
}

TEST_F(ComponentInfoTest, GetFileTypeString) {
    ComponentInfo component;
    EXPECT_EQ(component.getFileTypeString(), "Unknown");
    component.fileType = FileType::Object;
    EXPECT_EQ(component.getFileTypeString(), "Object");
    component.fileType = FileType::StaticLibrary;
    EXPECT_EQ(component.getFileTypeString(), "StaticLibrary");
    component.fileType = FileType::SharedLibrary;
    EXPECT_EQ(component.getFileTypeString(), "SharedLibrary");
    component.fileType = FileType::Executable;
    EXPECT_EQ(component.getFileTypeString(), "Executable");
} 