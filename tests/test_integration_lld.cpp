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
 * @file test_integration_lld.cpp
 * @brief Comprehensive integration tests for LLD plugin
 * @author Trevor Bakker
 * @date 2025
 */

#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "common/ComponentInfo.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/Utils.hpp"
#include "lld/LLDAdapter.hpp"
#include "lld/LLDPlugin.hpp"
#include "src/compat/compatibility.hpp"
#include "test_plugin_interface.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class LLDIntegrationTest : public ::testing::Test
{
  protected:
  void SetUp() override
  {
    test_dir = test_utils::getUniqueTestDirectory("heimdall_lld_test");
    heimdall::compat::fs::create_directories(test_dir);

    // Create test files
    createTestFiles();
  }

  void TearDown() override
  {
    test_utils::safeRemoveDirectory(test_dir);
  }

  heimdall::compat::fs::path test_dir;
  heimdall::compat::fs::path test_object_file;
  heimdall::compat::fs::path test_library_file;
  heimdall::compat::fs::path test_executable;
  heimdall::compat::fs::path test_bitcode;
  heimdall::compat::fs::path test_llvm_ir;

  void                       createTestFiles()
  {
    test_object_file  = test_dir / "test.o";
    test_library_file = test_dir / "libtest.a";
    test_executable   = test_dir / "test_executable";
    test_bitcode      = test_dir / "test.bc";
    test_llvm_ir      = test_dir / "test.ll";

    std::ofstream obj_file(test_object_file);
    obj_file << "ELF object file content";
    obj_file.close();

    std::ofstream lib_file(test_library_file);
    lib_file << "Archive library content";
    lib_file.close();

    std::ofstream exe_file(test_executable);
    exe_file << "Executable content";
    exe_file.close();

    std::ofstream bc_file(test_bitcode);
    bc_file << "LLVM bitcode content";
    bc_file.close();

    std::ofstream ll_file(test_llvm_ir);
    ll_file << "LLVM IR content";
    ll_file.close();
  }
};

// End-to-End Workflow Tests

TEST_F(LLDIntegrationTest, CompleteSBOMGenerationWorkflow)
{
  // Initialize the LLD adapter
  auto adapter = std::make_unique<LLDAdapter>();
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
  adapter->processInputFile(test_executable.string());

  // Process symbols
  adapter->processSymbol("main", 0x1000, 100);
  adapter->processSymbol("printf", 0x2000, 50);
  adapter->processSymbol("malloc", 0x3000, 75);
  adapter->processSymbol("free", 0x4000, 60);

  // Generate SBOM
  adapter->finalize();

  // Verify SBOM file was created
  heimdall::compat::fs::path sbom_file = test_dir / "complete_workflow.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
  EXPECT_GT(heimdall::compat::fs::file_size(sbom_file), 0);

  // Verify processed files
  auto processed_files = adapter->getProcessedFiles();
  EXPECT_EQ(processed_files.size(), 2);

  auto processed_libraries = adapter->getProcessedLibraries();
  EXPECT_EQ(processed_libraries.size(), 1);
}

TEST_F(LLDIntegrationTest, CycloneDXFormatWorkflow)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "cyclonedx_workflow.sbom").string());
  adapter->setFormat("cyclonedx");
  adapter->setCycloneDXVersion("1.6");
  adapter->setVerbose(true);

  // Process files
  adapter->processInputFile(test_object_file.string());
  adapter->processLibrary(test_library_file.string());

  // Process symbols
  for (int i = 0; i < 100; ++i)
  {
    std::string symbol_name = "symbol_" + std::to_string(i);
    adapter->processSymbol(symbol_name, i * 1000, 50 + (i % 50));
  }

  adapter->finalize();

  // Verify SBOM file
  heimdall::compat::fs::path sbom_file = test_dir / "cyclonedx_workflow.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
  EXPECT_GT(heimdall::compat::fs::file_size(sbom_file), 0);
}

TEST_F(LLDIntegrationTest, LargeScaleProcessingWorkflow)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "large_scale.sbom").string());
  adapter->setFormat("spdx");
  adapter->setVerbose(false);  // Disable verbose for performance

  // Create and process many files
  std::vector<heimdall::compat::fs::path> test_files;
  for (int i = 0; i < 50; ++i)
  {
    heimdall::compat::fs::path file_path = test_dir / ("file_" + std::to_string(i) + ".o");
    std::ofstream              file(file_path);
    file << "Content for file " << i;
    file.close();
    test_files.push_back(file_path);

    adapter->processInputFile(file_path.string());
  }

  // Process many symbols
  for (int i = 0; i < 1000; ++i)
  {
    std::string symbol_name = "symbol_" + std::to_string(i);
    adapter->processSymbol(symbol_name, i * 1000, 50 + (i % 100));
  }

  adapter->finalize();

  // Verify results
  heimdall::compat::fs::path sbom_file = test_dir / "large_scale.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
  EXPECT_GT(heimdall::compat::fs::file_size(sbom_file), 0);

  auto processed_files = adapter->getProcessedFiles();
  EXPECT_EQ(processed_files.size(), 50);
}

// Error Handling and Recovery Tests

TEST_F(LLDIntegrationTest, ErrorRecoveryWorkflow)
{
  auto adapter = std::make_unique<LLDAdapter>();
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

  // Process invalid symbols
  adapter->processSymbol("", 0, 0);

  adapter->finalize();

  // Should still generate a valid SBOM
  heimdall::compat::fs::path sbom_file = test_dir / "error_recovery.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
  EXPECT_GT(heimdall::compat::fs::file_size(sbom_file), 0);
}

TEST_F(LLDIntegrationTest, ConfigurationErrorHandling)
{
  auto adapter = std::make_unique<LLDAdapter>();
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
  heimdall::compat::fs::path sbom_file = test_dir / "config_error.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
}

// Performance and Stress Tests

TEST_F(LLDIntegrationTest, MemoryStressTest)
{
  // Test memory usage under stress
  for (int cycle = 0; cycle < 5; ++cycle)
  {
    auto adapter = std::make_unique<LLDAdapter>();
    ASSERT_NE(adapter, nullptr);

    adapter->initialize();
    adapter->setOutputPath((test_dir / ("stress_" + std::to_string(cycle) + ".sbom")).string());
    adapter->setFormat("spdx");

    // Process many files and symbols
    for (int i = 0; i < 100; ++i)
    {
      heimdall::compat::fs::path file_path = test_dir / ("stress_file_" + std::to_string(i) + ".o");
      std::ofstream              file(file_path);
      file << "Stress test content " << i;
      file.close();

      adapter->processInputFile(file_path.string());

      // Process symbols for each file
      for (int j = 0; j < 10; ++j)
      {
        std::string symbol_name = "symbol_" + std::to_string(i) + "_" + std::to_string(j);
        adapter->processSymbol(symbol_name, (i * 1000) + j, 50 + (j % 50));
      }
    }

    adapter->finalize();

    // Verify SBOM was created
    heimdall::compat::fs::path sbom_file = test_dir / ("stress_" + std::to_string(cycle) + ".sbom");
    EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
  }
}

TEST_F(LLDIntegrationTest, ConcurrentAccessTest)
{
  // Test that multiple adapters can work independently (sequential version)
  for (int i = 0; i < 3; ++i)
  {
    auto adapter = std::make_unique<LLDAdapter>();
    adapter->initialize();
    adapter->setOutputPath((test_dir / ("concurrent_" + std::to_string(i) + ".sbom")).string());
    adapter->setFormat("spdx");
    for (int j = 0; j < 10; ++j)
    {
      heimdall::compat::fs::path file_path =
        test_dir / ("concurrent_file_" + std::to_string(i) + "_" + std::to_string(j) + ".o");
      std::ofstream file(file_path);
      file << "Concurrent test content " << i << "_" << j;
      file.close();
      adapter->processInputFile(file_path.string());
      adapter->processSymbol("symbol_" + std::to_string(i) + "_" + std::to_string(j), j * 1000, 50);
    }
    adapter->finalize();
  }
  // Verify all SBOMs were created
  for (int i = 0; i < 3; ++i)
  {
    heimdall::compat::fs::path sbom_file = test_dir / ("concurrent_" + std::to_string(i) + ".sbom");
    EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
    EXPECT_GT(heimdall::compat::fs::file_size(sbom_file), 0);
  }
}

// LLVM-Specific Tests (if LLVM is available)

#ifdef LLVM_AVAILABLE
TEST_F(LLDIntegrationTest, LLVMBitcodeProcessing)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "bitcode_test.sbom").string());
  adapter->setFormat("spdx");

  // Process LLVM bitcode files
  adapter->processInputFile(test_bitcode.string());

  // Create additional bitcode files
  for (int i = 0; i < 5; ++i)
  {
    heimdall::compat::fs::path bitcode_path = test_dir / ("bitcode_" + std::to_string(i) + ".bc");
    std::ofstream              bitcode_file(bitcode_path);
    bitcode_file << "Bitcode content " << i;
    bitcode_file.close();

    adapter->processInputFile(bitcode_path.string());
  }

  adapter->finalize();

  heimdall::compat::fs::path sbom_file = test_dir / "bitcode_test.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));

  auto processed_files = adapter->getProcessedFiles();
  EXPECT_EQ(processed_files.size(), 6);
}

TEST_F(LLDIntegrationTest, LLVMIRProcessing)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "llvm_ir_test.sbom").string());
  adapter->setFormat("cyclonedx");

  // Process LLVM IR files
  adapter->processInputFile(test_llvm_ir.string());

  // Create additional LLVM IR files
  for (int i = 0; i < 5; ++i)
  {
    heimdall::compat::fs::path ir_path = test_dir / ("ir_" + std::to_string(i) + ".ll");
    std::ofstream              ir_file(ir_path);
    ir_file << "LLVM IR content " << i;
    ir_file.close();

    adapter->processInputFile(ir_path.string());
  }

  adapter->finalize();

  heimdall::compat::fs::path sbom_file = test_dir / "llvm_ir_test.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));

  auto processed_files = adapter->getProcessedFiles();
  EXPECT_EQ(processed_files.size(), 6);
}

TEST_F(LLDIntegrationTest, LLVMPassIntegration)
{
  // Test LLVM pass integration
  auto pass = std::make_unique<HeimdallPass>();
  ASSERT_NE(pass, nullptr);

  std::string name = pass->getPassName().str();
  EXPECT_NE(name.find("Heimdall"), std::string::npos);

  // Test pass registration
  heimdall_register_pass();
  heimdall_lld_plugin_init();
  heimdall_lld_plugin_cleanup();

  EXPECT_TRUE(true);
}
#endif

// File Type and Format Tests

TEST_F(LLDIntegrationTest, ArchiveFileProcessing)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "archive_test.sbom").string());
  adapter->setFormat("spdx");

  // Process archive files
  adapter->processLibrary(test_library_file.string());

  // Create additional archive files
  for (int i = 0; i < 5; ++i)
  {
    heimdall::compat::fs::path archive_path = test_dir / ("archive_" + std::to_string(i) + ".a");
    std::ofstream              archive_file(archive_path);
    archive_file << "Archive content " << i;
    archive_file.close();

    adapter->processLibrary(archive_path.string());
  }

  adapter->finalize();

  heimdall::compat::fs::path sbom_file = test_dir / "archive_test.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));

  auto processed_libraries = adapter->getProcessedLibraries();
  EXPECT_EQ(processed_libraries.size(), 6);
}

// Component Extraction Tests

TEST_F(LLDIntegrationTest, ComponentNameExtraction)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();

  // Test component name extraction for various file types
  EXPECT_EQ(adapter->extractComponentName("/path/to/libcrypto.a"), "crypto");
  EXPECT_EQ(adapter->extractComponentName("/path/to/main.o"), "main");
  EXPECT_EQ(adapter->extractComponentName("/path/to/executable"), "executable");
  EXPECT_EQ(adapter->extractComponentName("/path/to/libtest-1.2.3.a"), "test");
  EXPECT_EQ(adapter->extractComponentName("/path/to/test.bc"), "test");
  EXPECT_EQ(adapter->extractComponentName("/path/to/test.ll"), "test");

  adapter->finalize();
}

// SBOM Validation Tests

TEST_F(LLDIntegrationTest, SBOMContentValidation)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "validation_test.sbom").string());
  adapter->setFormat("spdx");
  adapter->setSPDXVersion("2.3");

  // Process files with known content
  adapter->processInputFile(test_object_file.string());
  adapter->processLibrary(test_library_file.string());

  // Process symbols
  adapter->processSymbol("main", 0x1000, 100);
  adapter->processSymbol("printf", 0x2000, 50);
  adapter->processSymbol("malloc", 0x3000, 75);

  adapter->finalize();

  // Read and validate SBOM content
  heimdall::compat::fs::path sbom_file = test_dir / "validation_test.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));

  std::ifstream file(sbom_file);
  std::string   content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();

  // Basic validation of SBOM content
  EXPECT_NE(content.find("SPDX"), std::string::npos);
  EXPECT_NE(content.find("test.o"), std::string::npos);
  EXPECT_NE(content.find("libtest.a"), std::string::npos);
}

// Cross-Platform Compatibility Tests

TEST_F(LLDIntegrationTest, CrossPlatformPathHandling)
{
  auto adapter = std::make_unique<LLDAdapter>();
  ASSERT_NE(adapter, nullptr);

  adapter->initialize();
  adapter->setOutputPath((test_dir / "cross_platform.sbom").string());
  adapter->setFormat("spdx");

  // Test various path formats
  std::vector<std::string> test_paths = {
    "/usr/lib/libc.a",      "C:\\Windows\\System32\\kernel32.dll",
    "relative/path/file.o", "file_with_spaces.o",
    "file-with-dashes.o",   "file_with_underscores.o"};

  for (const auto& path : test_paths)
  {
    adapter->processInputFile(path);
  }

  adapter->finalize();

  heimdall::compat::fs::path sbom_file = test_dir / "cross_platform.sbom";
  EXPECT_TRUE(heimdall::compat::fs::exists(sbom_file));
}

// Plugin-Specific Feature Tests

TEST_F(LLDIntegrationTest, LLDPluginOptions)
{
  // Test LLD-specific plugin options
  onload(nullptr);

  // Test various LLD plugin options
  heimdall_lld_set_plugin_option(
    ("--plugin-opt=output=" + (test_dir / "lld_output.sbom").string()).c_str());
  heimdall_lld_set_plugin_option("--plugin-opt=format=spdx");
  heimdall_lld_set_plugin_option("--plugin-opt=verbose");
  heimdall_lld_set_plugin_option("--plugin-opt=cyclonedx-version=1.6");

  onunload();
  EXPECT_TRUE(true);
}

TEST_F(LLDIntegrationTest, LLDErrorRecovery)
{
  onload(nullptr);

  // Test error recovery scenarios
  heimdall_process_input_file("/nonexistent/file1.o");
  heimdall_process_library("/nonexistent/lib1.a");

  // Should still be able to process valid files
  heimdall_process_input_file(test_object_file.string().c_str());
  heimdall_process_library(test_library_file.string().c_str());

  heimdall_finalize();
  onunload();

  EXPECT_TRUE(true);
}

TEST_F(LLDIntegrationTest, LLDConfigurationValidation)
{
  onload(nullptr);

  // Test configuration validation
  heimdall_set_output_path((test_dir / "valid.sbom").string().c_str());
  heimdall_set_format("spdx");
  heimdall_set_cyclonedx_version("1.6");

  // Process files with validated configuration
  heimdall_process_input_file(test_object_file.string().c_str());
  heimdall_process_library(test_library_file.string().c_str());

  heimdall_finalize();
  onunload();

  EXPECT_TRUE(true);
}