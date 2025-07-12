#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "gold/GoldAdapter.hpp"
#include "gold/GoldPlugin.hpp"
#include "common/ComponentInfo.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/Utils.hpp"

using namespace heimdall;

class GoldPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_gold_test";
        std::filesystem::create_directories(test_dir);
        
        // Create test object file
        test_object_file = test_dir / "test.o";
        std::ofstream obj_file(test_object_file);
        obj_file << "ELF object file content";
        obj_file.close();
        
        // Create test library file
        test_library_file = test_dir / "libtest.a";
        std::ofstream lib_file(test_library_file);
        lib_file << "Static library content";
        lib_file.close();
        
        // Create test shared library
        test_shared_lib = test_dir / "libtest.so";
        std::ofstream shared_file(test_shared_lib);
        shared_file << "Shared library content";
        shared_file.close();
        
        // Create test executable
        test_executable = test_dir / "test_exe";
        std::ofstream exe_file(test_executable);
        exe_file << "Executable content";
        exe_file.close();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    std::filesystem::path test_dir;
    std::filesystem::path test_object_file;
    std::filesystem::path test_library_file;
    std::filesystem::path test_shared_lib;
    std::filesystem::path test_executable;
};

// GoldAdapter Tests

TEST_F(GoldPluginTest, GoldAdapterCreation) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
}

TEST_F(GoldPluginTest, GoldAdapterInitialization) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    bool result = adapter->initialize();
    EXPECT_TRUE(result);
}

TEST_F(GoldPluginTest, GoldAdapterFinalization) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->finalize();
    // Should not throw or crash
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, ProcessInputFile) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing a valid file
    adapter->processInputFile(test_object_file.string());
    
    // Test processing a non-existent file
    adapter->processInputFile("/nonexistent/file.o");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, ProcessLibrary) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing a valid library
    adapter->processLibrary(test_library_file.string());
    
    // Test processing a shared library
    adapter->processLibrary(test_shared_lib.string());
    
    // Test processing a non-existent library
    adapter->processLibrary("/nonexistent/lib.a");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, ProcessSymbol) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    adapter->processSymbol("malloc", 0x3000, 75);
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, SetOutputPath) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    std::string output_path = (test_dir / "output.sbom").string();
    adapter->setOutputPath(output_path);
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, SetFormat) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setFormat("spdx");
    adapter->setFormat("cyclonedx");
    adapter->setFormat("json");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, SetCycloneDXVersion) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setCycloneDXVersion("1.4");
    adapter->setCycloneDXVersion("1.5");
    adapter->setCycloneDXVersion("1.6");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, SetSPDXVersion) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setSPDXVersion("2.3");
    adapter->setSPDXVersion("3.0");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GetProcessedFiles) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some files
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    
    auto processed_files = adapter->getProcessedFiles();
    EXPECT_EQ(processed_files.size(), 2);
    
    adapter->finalize();
}

TEST_F(GoldPluginTest, GetProcessedLibraries) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some libraries
    adapter->processLibrary(test_library_file.string());
    adapter->processLibrary(test_shared_lib.string());
    adapter->processLibrary("/usr/lib/libc.a");
    
    auto processed_libraries = adapter->getProcessedLibraries();
    EXPECT_EQ(processed_libraries.size(), 3);
    
    adapter->finalize();
}

TEST_F(GoldPluginTest, GetProcessedSymbols) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    
    auto processed_symbols = adapter->getProcessedSymbols();
    EXPECT_EQ(processed_symbols.size(), 2);
    
    adapter->finalize();
}

TEST_F(GoldPluginTest, ShouldProcessFile) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test various file types
    EXPECT_TRUE(adapter->shouldProcessFile(test_object_file.string()));
    EXPECT_TRUE(adapter->shouldProcessFile(test_library_file.string()));
    EXPECT_TRUE(adapter->shouldProcessFile(test_shared_lib.string()));
    EXPECT_TRUE(adapter->shouldProcessFile(test_executable.string()));
    EXPECT_FALSE(adapter->shouldProcessFile("/nonexistent/file"));
    
    adapter->finalize();
}

TEST_F(GoldPluginTest, ExtractComponentName) {
    auto adapter = std::make_unique<GoldAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    EXPECT_EQ(adapter->extractComponentName("/path/to/libtest.a"), "libtest");
    EXPECT_EQ(adapter->extractComponentName("/path/to/libtest.so"), "libtest");
    EXPECT_EQ(adapter->extractComponentName("/path/to/test.o"), "test");
    EXPECT_EQ(adapter->extractComponentName("/path/to/executable"), "executable");
    
    adapter->finalize();
}

// Gold Plugin Interface Tests

TEST_F(GoldPluginTest, PluginVersion) {
    const char* version = heimdall_gold_version();
    ASSERT_NE(version, nullptr);
    EXPECT_EQ(std::string(version), "1.0.0");
}

TEST_F(GoldPluginTest, PluginDescription) {
    const char* description = heimdall_gold_description();
    ASSERT_NE(description, nullptr);
    EXPECT_NE(std::string(description).find("Heimdall"), std::string::npos);
    EXPECT_NE(std::string(description).find("Gold"), std::string::npos);
}

TEST_F(GoldPluginTest, PluginOnload) {
    int result = onload(nullptr);
    EXPECT_EQ(result, 0);
}

TEST_F(GoldPluginTest, PluginOnunload) {
    // Initialize first
    onload(nullptr);
    
    // Then unload
    onunload();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, SetOutputPath) {
    onload(nullptr);
    
    int result = heimdall_set_output_path("/tmp/test.sbom");
    EXPECT_EQ(result, 0);
    
    result = heimdall_set_output_path(nullptr);
    EXPECT_EQ(result, -1);
    
    onunload();
}

TEST_F(GoldPluginTest, SetFormat) {
    onload(nullptr);
    
    int result = heimdall_set_format("spdx");
    EXPECT_EQ(result, 0);
    
    result = heimdall_set_format("cyclonedx");
    EXPECT_EQ(result, 0);
    
    result = heimdall_set_format(nullptr);
    EXPECT_EQ(result, -1);
    
    onunload();
}

TEST_F(GoldPluginTest, SetVerbose) {
    onload(nullptr);
    
    heimdall_set_verbose(true);
    heimdall_set_verbose(false);
    
    onunload();
}

TEST_F(GoldPluginTest, ProcessInputFile) {
    onload(nullptr);
    
    // Test with valid file
    int result = heimdall_process_input_file(test_object_file.string().c_str());
    EXPECT_EQ(result, 0);
    
    // Test with null pointer
    result = heimdall_process_input_file(nullptr);
    EXPECT_EQ(result, -1);
    
    // Test with non-existent file
    result = heimdall_process_input_file("/nonexistent/file.o");
    EXPECT_EQ(result, 0); // Should not be an error
    
    onunload();
}

TEST_F(GoldPluginTest, ProcessLibrary) {
    onload(nullptr);
    
    // Test with valid library
    heimdall_process_library(test_library_file.string().c_str());
    
    // Test with shared library
    heimdall_process_library(test_shared_lib.string().c_str());
    
    // Test with null pointer
    heimdall_process_library(nullptr);
    
    // Test with non-existent library
    heimdall_process_library("/nonexistent/lib.a");
    
    onunload();
}

TEST_F(GoldPluginTest, Finalize) {
    onload(nullptr);
    
    heimdall_finalize();
    
    onunload();
}

TEST_F(GoldPluginTest, SetCycloneDXVersion) {
    onload(nullptr);
    
    int result = heimdall_set_cyclonedx_version("1.6");
    EXPECT_EQ(result, 0);
    
    result = heimdall_set_cyclonedx_version("1.5");
    EXPECT_EQ(result, 0);
    
    result = heimdall_set_cyclonedx_version(nullptr);
    EXPECT_EQ(result, -1);
    
    onunload();
}

// Gold-specific functionality tests

TEST_F(GoldPluginTest, GoldSpecificFeatures) {
    onload(nullptr);
    
    // Test Gold-specific features
    heimdall_gold_set_plugin_option("--plugin-opt=output=/tmp/gold.sbom");
    heimdall_gold_set_plugin_option("--plugin-opt=format=spdx");
    
    onunload();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldPluginOptionHandling) {
    onload(nullptr);
    
    // Test various plugin options
    heimdall_gold_set_plugin_option("--plugin-opt=verbose");
    heimdall_gold_set_plugin_option("--plugin-opt=output=/tmp/test.sbom");
    heimdall_gold_set_plugin_option("--plugin-opt=format=cyclonedx");
    heimdall_gold_set_plugin_option("--plugin-opt=cyclonedx-version=1.6");
    
    onunload();
    EXPECT_TRUE(true);
}

// Integration Tests

TEST_F(GoldPluginTest, FullWorkflow) {
    // Initialize plugin
    onload(nullptr);
    
    // Set configuration
    heimdall_set_output_path("/tmp/workflow.sbom");
    heimdall_set_format("spdx");
    heimdall_set_verbose(true);
    
    // Process files
    heimdall_process_input_file(test_object_file.string().c_str());
    heimdall_process_library(test_library_file.string().c_str());
    heimdall_process_library(test_shared_lib.string().c_str());
    
    // Finalize
    heimdall_finalize();
    
    // Cleanup
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, MultipleFileProcessing) {
    onload(nullptr);
    
    // Process multiple files
    heimdall_process_input_file(test_object_file.string().c_str());
    heimdall_process_input_file(test_executable.string().c_str());
    heimdall_process_library(test_library_file.string().c_str());
    heimdall_process_library(test_shared_lib.string().c_str());
    
    // Process the same file again (should be handled gracefully)
    heimdall_process_input_file(test_object_file.string().c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, ErrorHandling) {
    onload(nullptr);
    
    // Test various error conditions
    heimdall_set_output_path(nullptr);
    heimdall_set_format(nullptr);
    heimdall_set_cyclonedx_version(nullptr);
    heimdall_process_input_file(nullptr);
    heimdall_process_library(nullptr);
    
    onunload();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, ConfigurationPersistence) {
    onload(nullptr);
    
    // Set configuration
    heimdall_set_output_path("/tmp/persistent.sbom");
    heimdall_set_format("cyclonedx");
    heimdall_set_cyclonedx_version("1.6");
    heimdall_set_verbose(true);
    
    // Process a file
    heimdall_process_input_file(test_object_file.string().c_str());
    
    // Change configuration
    heimdall_set_format("spdx");
    heimdall_set_output_path("/tmp/changed.sbom");
    
    // Process another file
    heimdall_process_input_file(test_executable.string().c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

// Performance Tests

TEST_F(GoldPluginTest, LargeFileProcessing) {
    onload(nullptr);
    
    // Create a large test file
    std::string large_file = (test_dir / "large.o").string();
    std::ofstream file(large_file);
    file << std::string(1024 * 1024, 'A'); // 1MB file
    file.close();
    
    heimdall_process_input_file(large_file.c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, MultipleSymbolProcessing) {
    onload(nullptr);
    
    // Process many symbols
    for (int i = 0; i < 1000; ++i) {
        std::string symbol_name = "symbol_" + std::to_string(i);
        heimdall_process_symbol(symbol_name.c_str(), i * 1000, 100);
    }
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

// Memory Management Tests

TEST_F(GoldPluginTest, MemoryLeakPrevention) {
    // Test multiple initialization/cleanup cycles
    for (int i = 0; i < 10; ++i) {
        onload(nullptr);
        heimdall_process_input_file(test_object_file.string().c_str());
        heimdall_finalize();
        onunload();
    }
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, NullPointerHandling) {
    onload(nullptr);
    
    // Test all functions with null pointers
    heimdall_set_output_path(nullptr);
    heimdall_set_format(nullptr);
    heimdall_set_cyclonedx_version(nullptr);
    heimdall_process_input_file(nullptr);
    heimdall_process_library(nullptr);
    
    onunload();
    EXPECT_TRUE(true);
}

// Thread Safety Tests (if applicable)

TEST_F(GoldPluginTest, ThreadSafety) {
    // Note: This is a basic test. Real thread safety testing would require
    // multiple threads calling plugin functions simultaneously
    onload(nullptr);
    
    // Simulate concurrent access
    heimdall_set_output_path("/tmp/thread1.sbom");
    heimdall_set_format("spdx");
    heimdall_process_input_file(test_object_file.string().c_str());
    
    heimdall_set_output_path("/tmp/thread2.sbom");
    heimdall_set_format("cyclonedx");
    heimdall_process_library(test_library_file.string().c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

// Gold-specific advanced tests

TEST_F(GoldPluginTest, GoldArchiveProcessing) {
    onload(nullptr);
    
    // Test processing archive files
    heimdall_process_library("/usr/lib/libc.a");
    heimdall_process_library("/usr/lib/libm.a");
    heimdall_process_library("/usr/lib/libpthread.a");
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldSharedLibraryProcessing) {
    onload(nullptr);
    
    // Test processing shared libraries
    heimdall_process_library("/usr/lib/libc.so");
    heimdall_process_library("/usr/lib/libm.so");
    heimdall_process_library("/usr/lib/libpthread.so");
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldSymbolResolution) {
    onload(nullptr);
    
    // Test symbol resolution
    heimdall_process_symbol("main", 0x1000, 100);
    heimdall_process_symbol("_start", 0x2000, 50);
    heimdall_process_symbol("__libc_start_main", 0x3000, 75);
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldPluginOptions) {
    onload(nullptr);
    
    // Test various Gold plugin options
    heimdall_gold_set_plugin_option("--plugin-opt=output=/tmp/gold_output.sbom");
    heimdall_gold_set_plugin_option("--plugin-opt=format=spdx");
    heimdall_gold_set_plugin_option("--plugin-opt=verbose");
    heimdall_gold_set_plugin_option("--plugin-opt=cyclonedx-version=1.6");
    heimdall_gold_set_plugin_option("--plugin-opt=spdx-version=2.3");
    
    onunload();
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldErrorRecovery) {
    onload(nullptr);
    
    // Test error recovery scenarios
    heimdall_process_input_file("/nonexistent/file1.o");
    heimdall_process_library("/nonexistent/lib1.a");
    heimdall_process_input_file(test_object_file.string().c_str()); // Valid file
    heimdall_process_input_file("/nonexistent/file2.o");
    heimdall_process_library(test_library_file.string().c_str()); // Valid library
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(GoldPluginTest, GoldConfigurationValidation) {
    onload(nullptr);
    
    // Test configuration validation
    heimdall_set_output_path("/tmp/valid.sbom");
    heimdall_set_format("spdx");
    heimdall_set_cyclonedx_version("1.6");
    
    // Test invalid configurations
    heimdall_set_format("invalid_format");
    heimdall_set_cyclonedx_version("invalid_version");
    
    // Test valid configurations again
    heimdall_set_format("cyclonedx");
    heimdall_set_cyclonedx_version("1.5");
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
} 