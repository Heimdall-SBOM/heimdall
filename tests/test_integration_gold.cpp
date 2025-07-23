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

/**
 * @file test_integration_gold.cpp
 * @brief Comprehensive integration tests for Gold plugin
 * @author Trevor Bakker
 * @date 2025
 */

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
#include "test_plugin_interface.hpp"
#include "test_utils.hpp"

using namespace heimdall;

#define SUPPRESS_WARNINGS(adapter) (adapter)->setSuppressWarnings(true)

class GoldIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = test_utils::getUniqueTestDirectory("heimdall_gold_test");
        std::filesystem::create_directories(test_dir);
        
        // Create test files
        createTestFiles();
    }
    
    void TearDown() override {
        test_utils::safeRemoveDirectory(test_dir);
    }
    
    std::filesystem::path test_dir;
    std::filesystem::path test_object_file;
    std::filesystem::path test_library_file;
    std::filesystem::path test_shared_lib;
    std::filesystem::path test_executable;
    std::filesystem::path test_archive;

    void createTestFiles() {
        test_object_file = test_dir / "test.o";
        test_library_file = test_dir / "libtest.a";
        test_shared_lib = test_dir / "libtest.so";
        test_executable = test_dir / "test_executable";
        test_archive = test_dir / "archive.a";
        
        std::ofstream obj_file(test_object_file);
        obj_file << "ELF object file content";
        obj_file.close();
        
        std::ofstream lib_file(test_library_file);
        lib_file << "Archive library content";
        lib_file.close();
        
        std::ofstream shared_file(test_shared_lib);
        shared_file << "Shared library content";
        shared_file.close();
        
        std::ofstream exe_file(test_executable);
        exe_file << "Executable content";
        exe_file.close();
        
        std::ofstream archive_file(test_archive);
        archive_file << "Archive content";
        archive_file.close();
    }
};

// End-to-End Workflow Tests

TEST_F(GoldIntegrationTest, CompleteSBOMGenerationWorkflow) {
    // Initialize the Gold adapter
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    // Configure the adapter
    adapter->initialize();
    adapter->setOutputPath((test_dir / "complete_workflow.sbom").string());
    adapter->setFormat("spdx");
    adapter->setSPDXVersion("2.3");
    adapter->setVerbose(true);
    
    // Process various file types
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    adapter->processLibrary(test_shared_lib.string());
    adapter->processInputFile(test_executable.string());
    
    // Process symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    adapter->processSymbol("malloc", 0x3000, 75);
    adapter->processSymbol("free", 0x4000, 60);
    
    // Generate SBOM
    adapter->finalize();
    
    // Verify SBOM file was created
    std::filesystem::path sbom_file = test_dir / "complete_workflow.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    EXPECT_GT(std::filesystem::file_size(sbom_file), 0);
    
    // Verify processed files
    auto processed_files = adapter->getProcessedFiles();
    EXPECT_EQ(processed_files.size(), 2);
    
    auto processed_libraries = adapter->getProcessedLibraries();
    EXPECT_EQ(processed_libraries.size(), 2);
}

TEST_F(GoldIntegrationTest, CycloneDXFormatWorkflow) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "cyclonedx_workflow.sbom").string());
    adapter->setFormat("cyclonedx");
    adapter->setCycloneDXVersion("1.6");
    adapter->setVerbose(true);
    
    // Process files
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    adapter->processLibrary(test_shared_lib.string());
    
    // Process symbols
    for (int i = 0; i < 100; ++i) {
        std::string symbol_name = "symbol_" + std::to_string(i);
        adapter->processSymbol(symbol_name, i * 1000, 50 + (i % 50));
    }
    
    adapter->finalize();
    
    // Verify SBOM file
    std::filesystem::path sbom_file = test_dir / "cyclonedx_workflow.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    EXPECT_GT(std::filesystem::file_size(sbom_file), 0);
}

TEST_F(GoldIntegrationTest, LargeScaleProcessingWorkflow) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "large_scale.sbom").string());
    adapter->setFormat("spdx");
    adapter->setVerbose(false); // Disable verbose for performance
    
    // Create and process many files
    std::vector<std::filesystem::path> test_files;
    for (int i = 0; i < 50; ++i) {
        std::filesystem::path file_path = test_dir / ("file_" + std::to_string(i) + ".o");
        std::ofstream file(file_path);
        file << "Content for file " << i;
        file.close();
        test_files.push_back(file_path);
        
        adapter->processInputFile(file_path.string());
    }
    
    // Process many symbols
    for (int i = 0; i < 1000; ++i) {
        std::string symbol_name = "symbol_" + std::to_string(i);
        adapter->processSymbol(symbol_name, i * 1000, 50 + (i % 100));
    }
    
    adapter->finalize();
    
    // Verify results
    std::filesystem::path sbom_file = test_dir / "large_scale.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    EXPECT_GT(std::filesystem::file_size(sbom_file), 0);
    
    auto processed_files = adapter->getProcessedFiles();
    EXPECT_EQ(processed_files.size(), 50);
}

// Error Handling and Recovery Tests

TEST_F(GoldIntegrationTest, ErrorRecoveryWorkflow) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "error_recovery.sbom").string());
    adapter->setFormat("spdx");
    
    // Process valid files first
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    
    // Process invalid files (should not crash)
    adapter->processInputFile("/nonexistent/file1.o");
    adapter->processLibrary("/nonexistent/lib1.a");
    adapter->processInputFile("/nonexistent/file2.o");
    adapter->processLibrary("/nonexistent/lib2.so");
    
    // Process more valid files
    adapter->processInputFile(test_executable.string());
    adapter->processLibrary(test_shared_lib.string());
    
    // Process invalid symbols
    adapter->processSymbol("", 0, 0);
    
    adapter->finalize();
    
    // Should still generate a valid SBOM
    std::filesystem::path sbom_file = test_dir / "error_recovery.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    EXPECT_GT(std::filesystem::file_size(sbom_file), 0);
}

TEST_F(GoldIntegrationTest, ConfigurationErrorHandling) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test invalid configurations
    adapter->setOutputPath("");
    adapter->setOutputPath("/invalid/path/with/many/levels/that/does/not/exist/file.sbom");
    adapter->setFormat("invalid_format");
    adapter->setCycloneDXVersion("invalid_version");
    adapter->setSPDXVersion("invalid_version");
    
    // Should still be able to process files
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    
    // Set valid configuration
    adapter->setOutputPath((test_dir / "config_error.sbom").string());
    adapter->setFormat("spdx");
    
    adapter->finalize();
    
    // Should still generate SBOM
    std::filesystem::path sbom_file = test_dir / "config_error.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
}

// Performance and Stress Tests

TEST_F(GoldIntegrationTest, MemoryStressTest) {
    // Test memory usage under stress
    for (int cycle = 0; cycle < 5; ++cycle) {
        auto adapter = std::make_unique<GoldAdapter>();
        SUPPRESS_WARNINGS(adapter);
        ASSERT_NE(adapter, nullptr);
        
        adapter->initialize();
        adapter->setOutputPath((test_dir / ("stress_" + std::to_string(cycle) + ".sbom")).string());
        adapter->setFormat("spdx");
        
        // Process many files and symbols
        for (int i = 0; i < 100; ++i) {
            std::filesystem::path file_path = test_dir / ("stress_file_" + std::to_string(i) + ".o");
            std::ofstream file(file_path);
            file << "Stress test content " << i;
            file.close();
            
            adapter->processInputFile(file_path.string());
            
            // Process symbols for each file
            for (int j = 0; j < 10; ++j) {
                std::string symbol_name = "symbol_" + std::to_string(i) + "_" + std::to_string(j);
                adapter->processSymbol(symbol_name, (i * 1000) + j, 50 + (j % 50));
            }
        }
        
        adapter->finalize();
        
        // Verify SBOM was created
        std::filesystem::path sbom_file = test_dir / ("stress_" + std::to_string(cycle) + ".sbom");
        EXPECT_TRUE(std::filesystem::exists(sbom_file));
    }
}

TEST_F(GoldIntegrationTest, SequentialAdapterTest) {
    // Test that multiple adapters can work sequentially (not concurrently)
    std::vector<std::unique_ptr<GoldAdapter>> adapters;
    
    for (int i = 0; i < 3; ++i) {
        adapters.push_back(std::make_unique<GoldAdapter>());
        auto& adapter = adapters.back();
        SUPPRESS_WARNINGS(adapter);
        
        adapter->initialize();
        adapter->setOutputPath((test_dir / ("sequential_" + std::to_string(i) + ".sbom")).string());
        adapter->setFormat("spdx");
        
        // Process files sequentially for each adapter
        for (int j = 0; j < 10; ++j) {
            std::filesystem::path file_path = test_dir / ("sequential_file_" + std::to_string(i) + "_" + std::to_string(j) + ".o");
            std::ofstream file(file_path);
            file << "Sequential test content " << i << "_" << j;
            file.close();
            
            adapter->processInputFile(file_path.string());
            adapter->processSymbol("symbol_" + std::to_string(i) + "_" + std::to_string(j), j * 1000, 50);
        }
        adapter->finalize();
    }
    
    // Verify all SBOMs were created
    for (int i = 0; i < 3; ++i) {
        std::filesystem::path sbom_file = test_dir / ("sequential_" + std::to_string(i) + ".sbom");
        EXPECT_TRUE(std::filesystem::exists(sbom_file));
        EXPECT_GT(std::filesystem::file_size(sbom_file), 0);
    }
}

// File Type and Format Tests

TEST_F(GoldIntegrationTest, ArchiveFileProcessing) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "archive_test.sbom").string());
    adapter->setFormat("spdx");
    
    // Process archive files
    adapter->processLibrary(test_archive.string());
    adapter->processLibrary(test_library_file.string());
    
    // Create additional archive files
    for (int i = 0; i < 5; ++i) {
        std::filesystem::path archive_path = test_dir / ("archive_" + std::to_string(i) + ".a");
        std::ofstream archive_file(archive_path);
        archive_file << "Archive content " << i;
        archive_file.close();
        
        adapter->processLibrary(archive_path.string());
    }
    
    adapter->finalize();
    
    std::filesystem::path sbom_file = test_dir / "archive_test.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    
    auto processed_libraries = adapter->getProcessedLibraries();
    EXPECT_EQ(processed_libraries.size(), 7);
}

TEST_F(GoldIntegrationTest, SharedLibraryProcessing) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "shared_lib_test.sbom").string());
    adapter->setFormat("cyclonedx");
    
    // Process shared libraries
    adapter->processLibrary(test_shared_lib.string());
    
    // Create additional shared libraries
    for (int i = 0; i < 5; ++i) {
        std::filesystem::path shared_path = test_dir / ("libshared_" + std::to_string(i) + ".so");
        std::ofstream shared_file(shared_path);
        shared_file << "Shared library content " << i;
        shared_file.close();
        
        adapter->processLibrary(shared_path.string());
    }
    
    adapter->finalize();
    
    std::filesystem::path sbom_file = test_dir / "shared_lib_test.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    
    auto processed_libraries = adapter->getProcessedLibraries();
    EXPECT_EQ(processed_libraries.size(), 6);
}

// Component Extraction Tests

TEST_F(GoldIntegrationTest, ComponentNameExtraction) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    
    // Test component name extraction for various file types
    EXPECT_EQ(adapter->extractComponentName("/path/to/libcrypto.a"), "crypto");
    EXPECT_EQ(adapter->extractComponentName("/path/to/libssl.so"), "ssl");
    EXPECT_EQ(adapter->extractComponentName("/path/to/main.o"), "main");
    EXPECT_EQ(adapter->extractComponentName("/path/to/executable"), "executable");
    EXPECT_EQ(adapter->extractComponentName("/path/to/libtest-1.2.3.a"), "test");
    EXPECT_EQ(adapter->extractComponentName("/path/to/libtest_debug.so"), "test");
    
    adapter->finalize();
}

// SBOM Validation Tests

TEST_F(GoldIntegrationTest, SBOMContentValidation) {
    auto adapter = std::make_unique<GoldAdapter>();
    SUPPRESS_WARNINGS(adapter);
    ASSERT_NE(adapter, nullptr);
    
    adapter->initialize();
    adapter->setOutputPath((test_dir / "validation_test.sbom").string());
    adapter->setFormat("spdx");
    adapter->setSPDXVersion("2.3");
    
    // Process files with known content
    adapter->processInputFile(test_object_file.string());
    adapter->processLibrary(test_library_file.string());
    adapter->processLibrary(test_shared_lib.string());
    
    // Process symbols
    adapter->processSymbol("main", 0x1000, 100);
    adapter->processSymbol("printf", 0x2000, 50);
    adapter->processSymbol("malloc", 0x3000, 75);
    
    adapter->finalize();
    
    // Read and validate SBOM content
    std::filesystem::path sbom_file = test_dir / "validation_test.sbom";
    EXPECT_TRUE(std::filesystem::exists(sbom_file));
    
    std::ifstream file(sbom_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Basic validation of SBOM content
    EXPECT_NE(content.find("SPDX"), std::string::npos);
    EXPECT_NE(content.find("test.o"), std::string::npos);
    EXPECT_NE(content.find("libtest.a"), std::string::npos);
    EXPECT_NE(content.find("libtest.so"), std::string::npos);
} 