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

/**
 * @file test_plugin_interface.cpp
 * @brief Unit tests for PluginInterface component
 * @author Trevor Bakker
 * @date 2025
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "ComponentInfo.hpp"
#include "PluginInterface.hpp"
#include "SBOMGenerator.hpp"

namespace heimdall {
namespace test {

/**
 * @brief Concrete test implementation of PluginInterface
 */
class TestPluginInterface : public PluginInterface {
public:
    TestPluginInterface() = default;
    ~TestPluginInterface() override = default;

    // Implement pure virtual methods for testing
    bool initialize() override {
        return true;
    }

    void cleanup() override {
        // Clean up test resources
    }

    void processInputFile(const std::string& filePath) override {
        ComponentInfo component(extractComponentName(filePath), filePath);
        addComponent(component);
    }

    void processLibrary(const std::string& libraryPath) override {
        ComponentInfo component(extractComponentName(libraryPath), libraryPath);
        component.fileType = FileType::SharedLibrary;
        addComponent(component);
    }

    void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) override {
        SymbolInfo symbol;
        symbol.name = symbolName;
        symbol.address = address;
        symbol.size = size;

        // Add to the last processed component
        if (!processedComponents.empty()) {
            processedComponents.back().addSymbol(symbol);
        }
    }

    void setOutputPath(const std::string& path) override {
        if (sbomGenerator) {
            sbomGenerator->setOutputPath(path);
        }
    }

    void setFormat(const std::string& format) override {
        if (sbomGenerator) {
            sbomGenerator->setFormat(format);
        }
    }

    void setCycloneDXVersion(const std::string& version) override {
        if (sbomGenerator) {
            sbomGenerator->setCycloneDXVersion(version);
        }
    }

    void setSPDXVersion(const std::string& version) override {
        if (sbomGenerator) {
            sbomGenerator->setSPDXVersion(version);
        }
    }

    void generateSBOM() override {
        if (sbomGenerator) {
            sbomGenerator->generateSBOM();
        }
    }

    void setVerbose(bool verbose) override {
        this->verbose = verbose;
    }

    void setExtractDebugInfo(bool extract) override {
        this->extractDebugInfo = extract;
    }

    void setIncludeSystemLibraries(bool include) override {
        this->includeSystemLibraries = include;
    }

    size_t getComponentCount() const override {
        return processedComponents.size();
    }

    void printStatistics() const override {
        // Print test statistics
        std::cout << "Test Plugin Statistics:" << std::endl;
        std::cout << "  Components processed: " << processedComponents.size() << std::endl;
        std::cout << "  Verbose mode: " << (verbose ? "enabled" : "disabled") << std::endl;
        std::cout << "  Debug info extraction: " << (extractDebugInfo ? "enabled" : "disabled")
                  << std::endl;
        std::cout << "  System libraries: " << (includeSystemLibraries ? "included" : "excluded")
                  << std::endl;
    }

    // Expose protected methods for testing
    using PluginInterface::addComponent;
    using PluginInterface::extractComponentName;
    using PluginInterface::extractDebugInfo;
    using PluginInterface::includeSystemLibraries;
    using PluginInterface::processedComponents;
    using PluginInterface::shouldProcessFile;
    using PluginInterface::updateComponent;
    using PluginInterface::verbose;
};

/**
 * @brief Test fixture for PluginInterface tests
 */
class PluginInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin = std::make_unique<TestPluginInterface>();

        // Create temporary test files
        createTestFiles();
    }

    void TearDown() override {
        // Clean up test files
        cleanupTestFiles();
    }

    void createTestFiles() {
        // Create test object file
        std::ofstream objFile("test_object.o");
        objFile << "test object file content";
        objFile.close();

        // Create test library file
        std::ofstream libFile("libtest.so");
        libFile << "test library file content";
        libFile.close();

        // Create test executable
        std::ofstream exeFile("test_executable.exe");
        exeFile << "test executable content";
        exeFile.close();

        // Create test archive
        std::ofstream archiveFile("libtest.a");
        archiveFile << "test archive content";
        archiveFile.close();
    }

    void cleanupTestFiles() {
        std::filesystem::remove("test_object.o");
        std::filesystem::remove("libtest.so");
        std::filesystem::remove("test_executable.exe");
        std::filesystem::remove("libtest.a");
        std::filesystem::remove("test_config.json");
        std::filesystem::remove("test_output.json");
    }

    std::unique_ptr<TestPluginInterface> plugin;
};

// Constructor and Destructor Tests
TEST_F(PluginInterfaceTest, Constructor) {
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_EQ(plugin->getComponentCount(), 0u);
    EXPECT_FALSE(plugin->verbose);
    EXPECT_TRUE(plugin->extractDebugInfo);
    EXPECT_FALSE(plugin->includeSystemLibraries);
}

TEST_F(PluginInterfaceTest, Destructor) {
    // Test that destructor doesn't crash
    plugin.reset();
    EXPECT_TRUE(plugin == nullptr);
}

// Initialization and Cleanup Tests
TEST_F(PluginInterfaceTest, Initialize) {
    EXPECT_TRUE(plugin->initialize());
}

TEST_F(PluginInterfaceTest, Cleanup) {
    // Test that cleanup doesn't crash
    EXPECT_NO_THROW(plugin->cleanup());
}

// Component Processing Tests
TEST_F(PluginInterfaceTest, ProcessInputFile) {
    plugin->processInputFile("test_object.o");
    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].name, "test_object");
    EXPECT_EQ(plugin->processedComponents[0].filePath, "test_object.o");
}

TEST_F(PluginInterfaceTest, ProcessLibrary) {
    plugin->processLibrary("libtest.so");
    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].name, "test");
    EXPECT_EQ(plugin->processedComponents[0].filePath, "libtest.so");
    EXPECT_EQ(plugin->processedComponents[0].fileType, FileType::SharedLibrary);
}

TEST_F(PluginInterfaceTest, ProcessSymbol) {
    plugin->processInputFile("test_object.o");
    plugin->processSymbol("test_function", 0x1000, 64);

    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].symbols.size(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].symbols[0].name, "test_function");
    EXPECT_EQ(plugin->processedComponents[0].symbols[0].address, 0x1000u);
    EXPECT_EQ(plugin->processedComponents[0].symbols[0].size, 64u);
}

TEST_F(PluginInterfaceTest, ProcessMultipleComponents) {
    plugin->processInputFile("test_object.o");
    plugin->processLibrary("libtest.so");
    plugin->processInputFile("test_executable.exe");

    EXPECT_EQ(plugin->getComponentCount(), 3u);
    EXPECT_EQ(plugin->processedComponents[0].name, "test_object");
    EXPECT_EQ(plugin->processedComponents[1].name, "test");
    EXPECT_EQ(plugin->processedComponents[2].name, "test_executable");
}

// Configuration Tests
TEST_F(PluginInterfaceTest, SetOutputPath) {
    plugin->setOutputPath("/tmp/test_output.json");
    // Note: We can't easily test the internal SBOM generator path setting
    // but we can test that the method doesn't crash
    EXPECT_NO_THROW(plugin->setOutputPath("/tmp/test_output.json"));
}

TEST_F(PluginInterfaceTest, SetFormat) {
    plugin->setFormat("spdx");
    plugin->setFormat("cyclonedx");
    // Test that the method doesn't crash
    EXPECT_NO_THROW(plugin->setFormat("json"));
}

TEST_F(PluginInterfaceTest, SetCycloneDXVersion) {
    plugin->setCycloneDXVersion("1.4");
    plugin->setCycloneDXVersion("1.5");
    plugin->setCycloneDXVersion("1.6");
    // Test that the method doesn't crash
    EXPECT_NO_THROW(plugin->setCycloneDXVersion("2.0"));
}

TEST_F(PluginInterfaceTest, SetSPDXVersion) {
    plugin->setSPDXVersion("2.3");
    plugin->setSPDXVersion("3.0");
    // Test that the method doesn't crash
    EXPECT_NO_THROW(plugin->setSPDXVersion("2.4"));
}

TEST_F(PluginInterfaceTest, SetVerbose) {
    EXPECT_FALSE(plugin->verbose);
    plugin->setVerbose(true);
    EXPECT_TRUE(plugin->verbose);
    plugin->setVerbose(false);
    EXPECT_FALSE(plugin->verbose);
}

TEST_F(PluginInterfaceTest, SetExtractDebugInfo) {
    EXPECT_TRUE(plugin->extractDebugInfo);
    plugin->setExtractDebugInfo(false);
    EXPECT_FALSE(plugin->extractDebugInfo);
    plugin->setExtractDebugInfo(true);
    EXPECT_TRUE(plugin->extractDebugInfo);
}

TEST_F(PluginInterfaceTest, SetIncludeSystemLibraries) {
    EXPECT_FALSE(plugin->includeSystemLibraries);
    plugin->setIncludeSystemLibraries(true);
    EXPECT_TRUE(plugin->includeSystemLibraries);
    plugin->setIncludeSystemLibraries(false);
    EXPECT_FALSE(plugin->includeSystemLibraries);
}

// Statistics Tests
TEST_F(PluginInterfaceTest, GetComponentCount) {
    EXPECT_EQ(plugin->getComponentCount(), 0u);
    plugin->processInputFile("test_object.o");
    EXPECT_EQ(plugin->getComponentCount(), 1u);
    plugin->processLibrary("libtest.so");
    EXPECT_EQ(plugin->getComponentCount(), 2u);
}

TEST_F(PluginInterfaceTest, PrintStatistics) {
    plugin->processInputFile("test_object.o");
    plugin->processLibrary("libtest.so");

    // Capture output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    plugin->printStatistics();

    std::cout.rdbuf(old);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Components processed: 2") != std::string::npos);
}

// Protected Method Tests
TEST_F(PluginInterfaceTest, AddComponent) {
    // Create the test file first
    std::ofstream testFile("test_file.o");
    testFile << "test file content";
    testFile.close();

    ComponentInfo component("test_component", "test_file.o");
    component.fileType = FileType::Object;

    plugin->addComponent(component);
    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].name, "test_component");
    EXPECT_EQ(plugin->processedComponents[0].filePath, "test_file.o");

    // Clean up
    std::filesystem::remove("test_file.o");
}

TEST_F(PluginInterfaceTest, UpdateComponent) {
    // Create the test file first
    std::ofstream testFile("test_file.o");
    testFile << "test file content";
    testFile.close();

    // Add initial component
    ComponentInfo component("test_component", "test_file.o");
    plugin->addComponent(component);

    // Update with symbols
    std::vector<SymbolInfo> symbols;
    SymbolInfo symbol1;
    symbol1.name = "function1";
    symbol1.address = 0x1000;
    symbol1.size = 64;
    symbols.push_back(symbol1);

    SymbolInfo symbol2;
    symbol2.name = "function2";
    symbol2.address = 0x2000;
    symbol2.size = 128;
    symbols.push_back(symbol2);

    plugin->updateComponent("test_component", "test_file.o", symbols);

    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].symbols.size(), 2u);
    EXPECT_EQ(plugin->processedComponents[0].symbols[0].name, "function1");
    EXPECT_EQ(plugin->processedComponents[0].symbols[1].name, "function2");

    // Clean up
    std::filesystem::remove("test_file.o");
}

TEST_F(PluginInterfaceTest, UpdateComponentNotFound) {
    // Create the test file first
    std::ofstream testFile("nonexistent.o");
    testFile << "test file content";
    testFile.close();

    // Try to update non-existent component
    std::vector<SymbolInfo> symbols;
    SymbolInfo symbol;
    symbol.name = "test_function";
    symbols.push_back(symbol);

    plugin->updateComponent("nonexistent", "nonexistent.o", symbols);

    // Should create new component
    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].name, "nonexistent");
    EXPECT_EQ(plugin->processedComponents[0].symbols.size(), 1u);

    // Clean up
    std::filesystem::remove("nonexistent.o");
}

TEST_F(PluginInterfaceTest, ShouldProcessFile) {
    // Valid files
    EXPECT_TRUE(plugin->shouldProcessFile("test_object.o"));
    EXPECT_TRUE(plugin->shouldProcessFile("libtest.so"));
    EXPECT_TRUE(plugin->shouldProcessFile("test_executable.exe"));
    EXPECT_TRUE(plugin->shouldProcessFile("libtest.a"));

    // Invalid files
    EXPECT_FALSE(plugin->shouldProcessFile("nonexistent.o"));
    EXPECT_FALSE(plugin->shouldProcessFile("test.txt"));
    EXPECT_FALSE(plugin->shouldProcessFile("test.c"));
    EXPECT_FALSE(plugin->shouldProcessFile(""));
}

TEST_F(PluginInterfaceTest, ShouldProcessFileSystemLibraries) {
    // Test system library exclusion
    EXPECT_FALSE(plugin->includeSystemLibraries);

    // Should not process system libraries by default
    EXPECT_FALSE(plugin->shouldProcessFile("/usr/lib/libc.so"));
    EXPECT_FALSE(plugin->shouldProcessFile("/usr/lib64/libstdc++.so"));

    // Enable system libraries
    plugin->setIncludeSystemLibraries(true);
    EXPECT_TRUE(plugin->includeSystemLibraries);

    // Should process system libraries when enabled
    // Try multiple possible paths for libc.so on different distributions
    std::vector<std::string> possible_libc_paths = {
        "/usr/lib/libc.so",
        "/usr/lib64/libc.so",
        "/usr/lib/x86_64-linux-gnu/libc.so",
        "/lib/x86_64-linux-gnu/libc.so",
        "/lib64/libc.so"
    };
    
    bool checked = false;
    for (const auto& path : possible_libc_paths) {
        if (std::filesystem::exists(path)) {
            EXPECT_TRUE(plugin->shouldProcessFile(path));
            checked = true;
            break;
        }
    }
    
    // If no libc.so found, skip this test
    if (!checked) {
        GTEST_SKIP() << "No libc.so found in standard locations";
    }

    // Test libstdc++ if it exists
    std::vector<std::string> possible_libstdcxx_paths = {
        "/usr/lib64/libstdc++.so",
        "/usr/lib/x86_64-linux-gnu/libstdc++.so",
        "/lib/x86_64-linux-gnu/libstdc++.so"
    };
    
    for (const auto& path : possible_libstdcxx_paths) {
        if (std::filesystem::exists(path)) {
            EXPECT_TRUE(plugin->shouldProcessFile(path));
            break;
        }
    }
}

TEST_F(PluginInterfaceTest, ExtractComponentName) {
    // Test various file name patterns
    EXPECT_EQ(plugin->extractComponentName("test.o"), "test");
    EXPECT_EQ(plugin->extractComponentName("libtest.so"), "test");
    EXPECT_EQ(plugin->extractComponentName("libtest.a"), "test");
    EXPECT_EQ(plugin->extractComponentName("test_executable"), "test_executable");
    EXPECT_EQ(plugin->extractComponentName("libtest-1.0.so"), "test-1.0");
    EXPECT_EQ(plugin->extractComponentName("libtest.so.1.0"), "test.so.1.0");
    EXPECT_EQ(plugin->extractComponentName("test.obj"), "test");
    EXPECT_EQ(plugin->extractComponentName("test.lib"), "test");
    EXPECT_EQ(plugin->extractComponentName("test.dll"), "test");
    EXPECT_EQ(plugin->extractComponentName("test.exe"), "test");
    EXPECT_EQ(plugin->extractComponentName("test.dylib"), "test");
}

// PluginUtils Tests
TEST_F(PluginInterfaceTest, PluginUtilsIsObjectFile) {
    EXPECT_TRUE(PluginUtils::isObjectFile("test.o"));
    EXPECT_TRUE(PluginUtils::isObjectFile("test.obj"));
    EXPECT_FALSE(PluginUtils::isObjectFile("test.so"));
    EXPECT_FALSE(PluginUtils::isObjectFile("test.a"));
    EXPECT_FALSE(PluginUtils::isObjectFile("test.exe"));
}

TEST_F(PluginInterfaceTest, PluginUtilsIsStaticLibrary) {
    EXPECT_TRUE(PluginUtils::isStaticLibrary("libtest.a"));
    EXPECT_TRUE(PluginUtils::isStaticLibrary("test.lib"));
    EXPECT_FALSE(PluginUtils::isStaticLibrary("test.o"));
    EXPECT_FALSE(PluginUtils::isStaticLibrary("test.so"));
    EXPECT_FALSE(PluginUtils::isStaticLibrary("test.exe"));
}

TEST_F(PluginInterfaceTest, PluginUtilsIsSharedLibrary) {
    EXPECT_TRUE(PluginUtils::isSharedLibrary("libtest.so"));
    EXPECT_TRUE(PluginUtils::isSharedLibrary("test.dll"));
    EXPECT_TRUE(PluginUtils::isSharedLibrary("test.dylib"));
    EXPECT_FALSE(PluginUtils::isSharedLibrary("test.o"));
    EXPECT_FALSE(PluginUtils::isSharedLibrary("test.a"));
    EXPECT_FALSE(PluginUtils::isSharedLibrary("test.exe"));
}

TEST_F(PluginInterfaceTest, PluginUtilsIsExecutable) {
    EXPECT_TRUE(PluginUtils::isExecutable("test.exe"));
    EXPECT_TRUE(PluginUtils::isExecutable("test"));
    EXPECT_FALSE(PluginUtils::isExecutable("test.o"));
    EXPECT_FALSE(PluginUtils::isExecutable("test.so"));
    EXPECT_FALSE(PluginUtils::isExecutable("test.a"));
}

TEST_F(PluginInterfaceTest, PluginUtilsIsSystemSymbol) {
    EXPECT_TRUE(PluginUtils::isSystemSymbol("__cxa_atexit"));
    EXPECT_TRUE(PluginUtils::isSystemSymbol("_start"));
    EXPECT_TRUE(PluginUtils::isSystemSymbol("main"));
    EXPECT_FALSE(PluginUtils::isSystemSymbol("my_function"));
    EXPECT_FALSE(PluginUtils::isSystemSymbol("test_symbol"));
}

TEST_F(PluginInterfaceTest, PluginUtilsIsWeakSymbol) {
    EXPECT_TRUE(PluginUtils::isWeakSymbol("weak_symbol"));
    EXPECT_FALSE(PluginUtils::isWeakSymbol("strong_symbol"));
    EXPECT_FALSE(PluginUtils::isWeakSymbol("my_function"));
}

TEST_F(PluginInterfaceTest, PluginUtilsExtractSymbolVersion) {
    EXPECT_EQ(PluginUtils::extractSymbolVersion("symbol@GLIBC_2.2.5"), "2.2.5");
    EXPECT_EQ(PluginUtils::extractSymbolVersion("symbol@@GLIBC_2.2.5"), "2.2.5");
    EXPECT_EQ(PluginUtils::extractSymbolVersion("symbol"), "");
    EXPECT_EQ(PluginUtils::extractSymbolVersion("symbol@"), "");
}

TEST_F(PluginInterfaceTest, PluginUtilsGetLibrarySearchPaths) {
    auto paths = PluginUtils::getLibrarySearchPaths();
    EXPECT_FALSE(paths.empty());

    // Should contain common library paths
    bool hasUsrLib = false;
    bool hasUsrLocalLib = false;

    for (const auto& path : paths) {
        if (path.find("/usr/lib") != std::string::npos) {
            hasUsrLib = true;
        }
        if (path.find("/usr/local/lib") != std::string::npos) {
            hasUsrLocalLib = true;
        }
    }

    EXPECT_TRUE(hasUsrLib || hasUsrLocalLib);
}

// Configuration Tests
TEST_F(PluginInterfaceTest, PluginConfigDefaultValues) {
    PluginConfig config;
    EXPECT_EQ(config.outputPath, "heimdall-sbom.json");
    EXPECT_EQ(config.format, "spdx");
    EXPECT_FALSE(config.verbose);
    EXPECT_TRUE(config.extractDebugInfo);
    EXPECT_FALSE(config.includeSystemLibraries);
    EXPECT_TRUE(config.generateChecksums);
    EXPECT_TRUE(config.extractMetadata);
    EXPECT_TRUE(config.excludePatterns.empty());
    EXPECT_TRUE(config.includePatterns.empty());
}

TEST_F(PluginInterfaceTest, PluginStatisticsDefaultValues) {
    PluginStatistics stats;
    EXPECT_EQ(stats.totalFiles, 0u);
    EXPECT_EQ(stats.objectFiles, 0u);
    EXPECT_EQ(stats.staticLibraries, 0u);
    EXPECT_EQ(stats.sharedLibraries, 0u);
    EXPECT_EQ(stats.executables, 0u);
    EXPECT_EQ(stats.systemLibraries, 0u);
    EXPECT_EQ(stats.totalSymbols, 0u);
    EXPECT_EQ(stats.processedComponents, 0u);
    EXPECT_EQ(stats.skippedFiles, 0u);
    EXPECT_EQ(stats.processingTime.count(), 0);
}

// Error Handling Tests
TEST_F(PluginInterfaceTest, ProcessNonExistentFile) {
    plugin->processInputFile("nonexistent.o");
    EXPECT_EQ(plugin->getComponentCount(), 0u);
}

TEST_F(PluginInterfaceTest, ProcessInvalidFileType) {
    // Create a text file
    std::ofstream textFile("test.txt");
    textFile << "This is a text file";
    textFile.close();

    plugin->processInputFile("test.txt");
    EXPECT_EQ(plugin->getComponentCount(), 0u);

    // Clean up
    std::filesystem::remove("test.txt");
}

TEST_F(PluginInterfaceTest, ProcessSymbolWithoutComponent) {
    // Process symbol without any component
    plugin->processSymbol("test_function", 0x1000, 64);
    EXPECT_EQ(plugin->getComponentCount(), 0u);
}

// Integration Tests
TEST_F(PluginInterfaceTest, FullWorkflow) {
    // Set up plugin
    plugin->setVerbose(true);
    plugin->setOutputPath("test_output.json");
    plugin->setFormat("spdx");

    // Process files
    plugin->processInputFile("test_object.o");
    plugin->processLibrary("libtest.so");
    plugin->processSymbol("function1", 0x1000, 64);
    plugin->processSymbol("function2", 0x2000, 128);

    // Verify results
    EXPECT_EQ(plugin->getComponentCount(), 2u);
    EXPECT_EQ(plugin->processedComponents[0].name, "test_object");
    EXPECT_EQ(plugin->processedComponents[1].name, "test");
    EXPECT_EQ(plugin->processedComponents[1].symbols.size(), 2u);

    // Generate SBOM (should not crash)
    EXPECT_NO_THROW(plugin->generateSBOM());

    // Clean up
    std::filesystem::remove("test_output.json");
}

TEST_F(PluginInterfaceTest, MultipleSymbolsPerComponent) {
    plugin->processInputFile("test_object.o");

    // Add multiple symbols
    for (int i = 0; i < 5; ++i) {
        std::string symbolName = "function" + std::to_string(i);
        plugin->processSymbol(symbolName, 0x1000 + i * 0x100, 64);
    }

    EXPECT_EQ(plugin->getComponentCount(), 1u);
    EXPECT_EQ(plugin->processedComponents[0].symbols.size(), 5u);

    // Verify symbol names
    for (int i = 0; i < 5; ++i) {
        std::string expectedName = "function" + std::to_string(i);
        EXPECT_EQ(plugin->processedComponents[0].symbols[i].name, expectedName);
    }
}

}  // namespace test
}  // namespace heimdall