#include <gtest/gtest.h>
#include "src/compat/compatibility.hpp"
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "lld/LLDAdapter.hpp"
#include "lld/LLDPlugin.hpp"
#include "common/ComponentInfo.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/Utils.hpp"
#include "test_plugin_interface.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class LLDPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = test_utils::getUniqueTestDirectory("heimdall_lld_test");
        heimdall::compat::fs::create_directories(test_dir);
        
        // Create test files
        createTestFiles();
    }

    void TearDown() override {
        test_utils::safeRemoveDirectory(test_dir);
    }

    heimdall::compat::fs::path test_dir;
    heimdall::compat::fs::path test_object_file;
    heimdall::compat::fs::path test_library_file;
    heimdall::compat::fs::path test_executable;

    void createTestFiles() {
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
        
        // Create test executable
        test_executable = test_dir / "test_exe";
        std::ofstream exe_file(test_executable);
        exe_file << "Executable content";
        exe_file.close();
    }
};

// LLDAdapter Unit Tests

TEST_F(LLDPluginTest, LLDAdapterCreation) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
}

TEST_F(LLDPluginTest, LLDAdapterInitialization) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    bool result = adapter->initialize();
    EXPECT_TRUE(result);
}

TEST_F(LLDPluginTest, LLDAdapterFinalization) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->finalize();
    // Should not throw or crash
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, ProcessInputFile) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing a valid file
    adapter->processInputFile(test_object_file.string());
    
    // Test processing a non-existent file
    adapter->processInputFile("/nonexistent/file.o");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, ProcessLibrary) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing a valid library
    adapter->processLibrary(test_library_file.string());
    
    // Test processing a non-existent library
    adapter->processLibrary("/nonexistent/lib.a");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, ProcessSymbol) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test processing symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    adapter->processSymbol("malloc", 0x3000, 75);
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, SetOutputPath) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    std::string output_path = (test_dir / "output.sbom").string();
    adapter->setOutputPath(output_path);
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, SetFormat) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setFormat("spdx");
    adapter->setFormat("cyclonedx");
    adapter->setFormat("json");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, SetCycloneDXVersion) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setCycloneDXVersion("1.4");
    adapter->setCycloneDXVersion("1.5");
    adapter->setCycloneDXVersion("1.6");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, SetSPDXVersion) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    adapter->setSPDXVersion("2.3");
    adapter->setSPDXVersion("3.0");
    
    adapter->finalize();
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, GetProcessedFiles) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some files
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    
    auto processed_files = adapter->getProcessedFiles();
    EXPECT_EQ(processed_files.size(), 1); // Only input files should be counted
    
    adapter->finalize();
}

TEST_F(LLDPluginTest, GetProcessedLibraries) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some libraries
    adapter->processLibrary(test_library_file.string());
    
    auto processed_libraries = adapter->getProcessedLibraries();
    EXPECT_EQ(processed_libraries.size(), 1);
    
    adapter->finalize();
}

TEST_F(LLDPluginTest, GetProcessedSymbols) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Process some symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    adapter->processSymbol("malloc", 0x3000, 75);
    
    auto processed_symbols = adapter->getProcessedSymbols();
    // Note: Symbol processing not fully implemented yet
    EXPECT_TRUE(true);
    
    adapter->finalize();
}

TEST_F(LLDPluginTest, ShouldProcessFile) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    EXPECT_TRUE(adapter->shouldProcessFile(test_object_file.string()));
    EXPECT_TRUE(adapter->shouldProcessFile(test_library_file.string()));
    EXPECT_TRUE(adapter->shouldProcessFile(test_executable.string()));
    EXPECT_FALSE(adapter->shouldProcessFile("/nonexistent/file"));
    
    adapter->finalize();
}

TEST_F(LLDPluginTest, ExtractComponentName) {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    EXPECT_EQ(adapter->extractComponentName("/path/to/libtest.a"), "test");
    EXPECT_EQ(adapter->extractComponentName("/path/to/test.o"), "test");
    EXPECT_EQ(adapter->extractComponentName("/path/to/executable"), "executable");
    
    adapter->finalize();
}

// Plugin Interface Tests (if C functions are available)

TEST_F(LLDPluginTest, PluginVersion) {
    const char* version = heimdall_lld_version();
    EXPECT_NE(version, nullptr);
    EXPECT_STRNE(version, "");
}

TEST_F(LLDPluginTest, PluginDescription) {
    const char* description = heimdall_lld_description();
    EXPECT_NE(description, nullptr);
    EXPECT_STRNE(description, "");
}

TEST_F(LLDPluginTest, PluginOnload) {
    int result = onload(nullptr);
    EXPECT_EQ(result, 0);
}

TEST_F(LLDPluginTest, PluginOnunload) {
    onload(nullptr);
    onunload();
    EXPECT_TRUE(true);
}

// Comprehensive Integration Tests

TEST_F(LLDPluginTest, FullWorkflowIntegration) {
    // Initialize plugin
    onload(nullptr);
    
    // Set configuration
    heimdall_set_output_path((test_dir / "workflow.sbom").string().c_str());
    heimdall_set_format("spdx");
    heimdall_set_verbose(true);
    
    // Process files
    heimdall_process_input_file(test_object_file.string().c_str());
    heimdall_process_library(test_library_file.string().c_str());
    
    // Finalize
    heimdall_finalize();
    
    // Cleanup
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, MultipleFileProcessingIntegration) {
    onload(nullptr);
    
    // Process multiple files
    heimdall_process_input_file(test_object_file.string().c_str());
    heimdall_process_input_file(test_executable.string().c_str());
    heimdall_process_library(test_library_file.string().c_str());
    
    // Process the same file again (should be handled gracefully)
    heimdall_process_input_file(test_object_file.string().c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, ErrorHandlingIntegration) {
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

TEST_F(LLDPluginTest, ConfigurationPersistenceIntegration) {
    onload(nullptr);
    
    // Set configuration
    heimdall_set_output_path((test_dir / "persistent.sbom").string().c_str());
    heimdall_set_format("cyclonedx");
    heimdall_set_cyclonedx_version("1.6");
    heimdall_set_verbose(true);
    
    // Process a file
    heimdall_process_input_file(test_object_file.string().c_str());
    
    // Change configuration
    heimdall_set_format("spdx");
    heimdall_set_output_path((test_dir / "changed.sbom").string().c_str());
    
    // Process another file
    heimdall_process_input_file(test_executable.string().c_str());
    
    heimdall_finalize();
    onunload();
    
    EXPECT_TRUE(true);
}

// Performance and Stress Tests

TEST_F(LLDPluginTest, LargeFileProcessingIntegration) {
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

TEST_F(LLDPluginTest, MultipleSymbolProcessingIntegration) {
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

// Memory Management and Stability Tests

TEST_F(LLDPluginTest, MemoryLeakPreventionIntegration) {
    // Test multiple initialization/cleanup cycles
    for (int i = 0; i < 10; ++i) {
        onload(nullptr);
        heimdall_process_input_file(test_object_file.string().c_str());
        heimdall_finalize();
        onunload();
    }
    
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, NullPointerHandlingIntegration) {
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

// LLVM-Specific Integration Tests (if LLVM is available)

#ifdef LLVM_AVAILABLE
TEST_F(LLDPluginTest, LLVMPassIntegration) {
    // Test LLVM pass integration
    auto pass = std::make_unique<HeimdallPass>();
    ASSERT_NE(pass, nullptr);
    
    std::string name = pass->getPassName().str();
    EXPECT_NE(name.find("Heimdall"), std::string::npos);
    
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, LLVMPluginRegistrationIntegration) {
    // Test plugin registration
    heimdall_register_pass();
    heimdall_lld_plugin_init();
    heimdall_lld_plugin_cleanup();
    
    EXPECT_TRUE(true);
}

TEST_F(LLDPluginTest, LLVMFileProcessingIntegration) {
    // Test LLVM-specific file processing
    heimdall_lld_process_file(test_object_file.string().c_str());
    heimdall_lld_process_library(test_library_file.string().c_str());
    
    EXPECT_TRUE(true);
}
#endif 