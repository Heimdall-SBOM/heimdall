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
 * @file test_compiler_plugins.cpp
 * @brief Integration tests for compiler plugins with metadata collection
 * @author Trevor Bakker
 * @date 2025
 */

#include <gtest/gtest.h>
#include "../../src/compiler/common/CompilerMetadata.hpp"
#include "../../src/tools/EnhancedSBOMGenerator.hpp"
#include "../../src/gold/EnhancedGoldAdapter.hpp"
#include "../../src/common/Utils.hpp"
#include <filesystem>
#include <fstream>
#include <memory>

using namespace heimdall;
using namespace heimdall::compiler;

class CompilerPluginIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary test directories
        test_source_dir_ = std::filesystem::temp_directory_path() / "heimdall_test_src";
        test_metadata_dir_ = std::filesystem::temp_directory_path() / "heimdall_test_metadata";
        test_output_dir_ = std::filesystem::temp_directory_path() / "heimdall_test_output";
        
        std::filesystem::create_directories(test_source_dir_);
        std::filesystem::create_directories(test_metadata_dir_);
        std::filesystem::create_directories(test_output_dir_);
        
        // Create test source files
        createTestSourceFiles();
    }
    
    void TearDown() override
    {
        // Clean up test directories
        try {
            std::filesystem::remove_all(test_source_dir_);
            std::filesystem::remove_all(test_metadata_dir_);
            std::filesystem::remove_all(test_output_dir_);
        } catch (const std::exception& e) {
            // Ignore cleanup errors in tests
        }
    }
    
    void createTestSourceFiles()
    {
        // Create test main.cpp
        std::ofstream main_cpp(test_source_dir_ / "main.cpp");
        main_cpp << "/*\n"
                 << "Copyright 2025 Test Author\n"
                 << "Licensed under MIT License\n"
                 << "*/\n\n"
                 << "#include \"test_header.h\"\n"
                 << "#include <iostream>\n\n"
                 << "int main() {\n"
                 << "    std::cout << \"Hello World\" << std::endl;\n"
                 << "    test_function();\n"
                 << "    return 0;\n"
                 << "}\n";
        main_cpp.close();
        
        // Create test header
        std::ofstream test_header(test_source_dir_ / "test_header.h");
        test_header << "/*\n"
                   << "Copyright 2025 Test Author\n"
                   << "Licensed under MIT License\n"
                   << "*/\n\n"
                   << "#ifndef TEST_HEADER_H\n"
                   << "#define TEST_HEADER_H\n\n"
                   << "void test_function();\n\n"
                   << "#endif // TEST_HEADER_H\n";
        test_header.close();
        
        // Create test implementation
        std::ofstream test_impl(test_source_dir_ / "test_impl.cpp");
        test_impl << "/*\n"
                 << "Copyright 2025 Test Author\n"
                 << "Licensed under MIT License\n"
                 << "*/\n\n"
                 << "#include \"test_header.h\"\n"
                 << "#include <iostream>\n\n"
                 << "void test_function() {\n"
                 << "    std::cout << \"Test function called\" << std::endl;\n"
                 << "}\n";
        test_impl.close();
    }
    
    std::filesystem::path test_source_dir_;
    std::filesystem::path test_metadata_dir_;
    std::filesystem::path test_output_dir_;
};

TEST_F(CompilerPluginIntegrationTest, TestCompilerMetadataCollection)
{
    // Create metadata collector
    CompilerMetadataCollector collector;
    collector.setOutputDirectory(test_metadata_dir_.string());
    collector.setVerbose(true);
    
    // Initialize compilation unit
    collector.startCompilationUnit((test_source_dir_ / "main.cpp").string(), "gcc", "11.2.0");
    
    // Add files as they would be processed during compilation
    collector.addSourceFile((test_source_dir_ / "main.cpp").string());
    collector.addIncludeFile((test_source_dir_ / "test_header.h").string(), false);
    collector.addIncludeFile("/usr/include/iostream", true); // System header
    
    // Finish compilation unit
    collector.finishCompilationUnit();
    
    // Write metadata
    collector.writeMetadata();
    
    // Verify metadata was created
    auto metadata_stats = CompilerMetadataCollector::getMetadataStatistics(test_metadata_dir_.string());
    EXPECT_GT(metadata_stats.first, 0) << "Expected at least one metadata file";
    EXPECT_GT(metadata_stats.second, 0) << "Expected metadata files to have content";
    
    // Load and verify metadata
    auto loaded_metadata = CompilerMetadataCollector::loadMetadataFiles(test_metadata_dir_.string());
    EXPECT_FALSE(loaded_metadata.empty()) << "Expected to load metadata files";
    
    if (!loaded_metadata.empty()) {
        const auto& metadata = loaded_metadata[0];
        EXPECT_EQ(metadata.compiler_type, "gcc");
        EXPECT_EQ(metadata.compiler_version, "11.2.0");
        EXPECT_FALSE(metadata.source_files.empty()) << "Expected source files in metadata";
        EXPECT_FALSE(metadata.include_files.empty()) << "Expected include files in metadata";
        
        // Check file hashes were calculated
        for (const auto& source_file : metadata.source_files) {
            EXPECT_TRUE(source_file.hashes.isValid()) << "Expected valid hashes for source files";
        }
    }
}

TEST_F(CompilerPluginIntegrationTest, TestEnhancedSBOMGeneration)
{
    // First create some metadata
    CompilerMetadataCollector collector;
    collector.setOutputDirectory(test_metadata_dir_.string());
    collector.startCompilationUnit((test_source_dir_ / "main.cpp").string(), "gcc", "11.2.0");
    collector.addSourceFile((test_source_dir_ / "main.cpp").string());
    collector.addIncludeFile((test_source_dir_ / "test_header.h").string(), false);
    collector.finishCompilationUnit();
    collector.writeMetadata();
    
    // Create enhanced SBOM generator config
    EnhancedSBOMConfig config;
    config.plugin_path = "/fake/plugin.so"; // Won't be loaded in test
    config.binary_path = "/fake/binary";
    config.output_path = (test_output_dir_ / "test.spdx").string();
    config.format = "spdx";
    config.metadata_directory = test_metadata_dir_.string();
    config.include_compiler_metadata = true;
    config.verbose = true;
    
    // Test configuration
    EnhancedSBOMGenerator generator;
    generator.setConfig(config);
    
    // Test metadata loading
    EXPECT_TRUE(generator.loadCompilerMetadata(test_metadata_dir_.string())) 
        << "Expected to load compiler metadata successfully";
    EXPECT_TRUE(generator.hasCompilerMetadata()) 
        << "Expected compiler metadata to be available";
    EXPECT_GT(generator.getComponentCount(), 0) 
        << "Expected components from compiler metadata";
    
    // Test statistics
    generator.printStatistics(); // Should not crash
}

TEST_F(CompilerPluginIntegrationTest, TestMetadataCleanup)
{
    // Create some metadata files
    for (int i = 0; i < 5; i++) {
        std::ofstream metadata_file(test_metadata_dir_ / ("metadata_" + std::to_string(i) + ".json"));
        metadata_file << "{\"test\": \"data\"}";
        metadata_file.close();
    }
    
    // Get initial statistics
    auto initial_stats = CompilerMetadataCollector::getMetadataStatistics(test_metadata_dir_.string());
    EXPECT_EQ(initial_stats.first, 5) << "Expected 5 metadata files";
    
    // Test cleanup (no files should be old enough)
    size_t cleaned = CompilerMetadataCollector::cleanupOldMetadataFiles(test_metadata_dir_.string(), 1);
    EXPECT_EQ(cleaned, 0) << "Expected no files to be cleaned (too new)";
    
    // Test cleanup with 0 hour age (should clean all)
    cleaned = CompilerMetadataCollector::cleanupOldMetadataFiles(test_metadata_dir_.string(), 0);
    EXPECT_GT(cleaned, 0) << "Expected files to be cleaned with 0 hour age";
    
    // Verify cleanup worked
    auto final_stats = CompilerMetadataCollector::getMetadataStatistics(test_metadata_dir_.string());
    EXPECT_LT(final_stats.first, initial_stats.first) << "Expected fewer files after cleanup";
}

TEST_F(CompilerPluginIntegrationTest, TestEnhancedGoldAdapter)
{
    // Create metadata
    CompilerMetadataCollector collector;
    collector.setOutputDirectory(test_metadata_dir_.string());
    collector.startCompilationUnit((test_source_dir_ / "main.cpp").string(), "gcc", "11.2.0");
    collector.addSourceFile((test_source_dir_ / "main.cpp").string());
    collector.finishCompilationUnit();
    collector.writeMetadata();
    
    // Create enhanced gold adapter
    auto adapter = std::make_unique<EnhancedGoldAdapter>();
    
    // Set metadata directory
    adapter->setMetadataDirectory(test_metadata_dir_.string());
    
    // Test metadata loading
    EXPECT_TRUE(adapter->hasCompilerMetadata()) << "Expected compiler metadata to be loaded";
    EXPECT_GT(adapter->getSourceFileCount(), 0) << "Expected source files to be counted";
    
    // Test statistics
    adapter->printStatistics(); // Should not crash
    
    auto licenses = adapter->getUniqueLicenses();
    // MIT license should be detected from copyright notice
    EXPECT_FALSE(licenses.empty()) << "Expected at least one license to be detected";
}

TEST_F(CompilerPluginIntegrationTest, TestComponentHashesValidation)
{
    // Create a test file with known content
    std::ofstream test_file(test_source_dir_ / "hash_test.txt");
    test_file << "Hello World\n";
    test_file.close();
    
    // Calculate hashes using our metadata collector
    CompilerMetadataCollector collector;
    collector.setOutputDirectory(test_metadata_dir_.string());
    collector.startCompilationUnit((test_source_dir_ / "hash_test.txt").string(), "gcc", "11.2.0");
    collector.addSourceFile((test_source_dir_ / "hash_test.txt").string());
    collector.finishCompilationUnit();
    
    const auto& metadata = collector.getMetadata();
    ASSERT_FALSE(metadata.source_files.empty()) << "Expected source file to be added";
    
    const auto& file_component = metadata.source_files[0];
    EXPECT_TRUE(file_component.hashes.isValid()) << "Expected valid hashes";
    EXPECT_FALSE(file_component.hashes.sha256.empty()) << "Expected SHA-256 hash";
    EXPECT_FALSE(file_component.hashes.sha1.empty()) << "Expected SHA-1 hash";
    EXPECT_FALSE(file_component.hashes.md5.empty()) << "Expected MD5 hash";
    EXPECT_EQ(file_component.hashes.file_size, 12) << "Expected correct file size";
    
    // Verify hash lengths
    EXPECT_EQ(file_component.hashes.sha256.length(), 64) << "SHA-256 should be 64 hex chars";
    EXPECT_EQ(file_component.hashes.sha1.length(), 40) << "SHA-1 should be 40 hex chars";
    EXPECT_EQ(file_component.hashes.md5.length(), 32) << "MD5 should be 32 hex chars";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}