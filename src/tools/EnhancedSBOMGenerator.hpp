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
 * @file EnhancedSBOMGenerator.hpp
 * @brief Enhanced SBOM generator with compiler metadata integration
 * @author Trevor Bakker
 * @date 2025
 *
 * This enhanced SBOM generator extends the basic heimdall-sbom functionality
 * to include compiler metadata from Heimdall compiler plugins, providing
 * comprehensive SBOM generation with both compile-time and link-time data.
 */

#pragma once

#include "../compiler/common/CompilerMetadata.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/ComponentInfo.hpp"
#include <string>
#include <vector>
#include <memory>

namespace heimdall
{

/**
 * @brief Configuration for enhanced SBOM generation
 */
struct EnhancedSBOMConfig
{
    // Basic configuration
    std::string plugin_path;
    std::string binary_path;
    std::string output_path;
    std::string format = "spdx";
    std::string cyclonedx_version = "1.6";
    std::string spdx_version = "2.3";
    
    // Enhanced features
    std::string metadata_directory;
    bool include_compiler_metadata = true;
    bool cleanup_metadata = true;
    bool verbose = false;
    
    // Existing features
    bool include_system_libraries = false;
    bool transitive_dependencies = true;
    std::string ali_file_path;
    
    // Signing configuration
    std::string sign_key_path;
    std::string sign_cert_path;
    std::string sign_algorithm;
    std::string sign_key_id;
};

/**
 * @brief Enhanced SBOM generator class
 */
class EnhancedSBOMGenerator
{
private:
    EnhancedSBOMConfig config_;
    std::vector<compiler::CompilerMetadata> compiler_metadata_;
    std::unique_ptr<SBOMGenerator> sbom_generator_;
    bool has_compiler_metadata_;
    
public:
    /**
     * @brief Constructor
     */
    EnhancedSBOMGenerator();
    
    /**
     * @brief Destructor
     */
    ~EnhancedSBOMGenerator();
    
    /**
     * @brief Set configuration
     * @param config Configuration to use
     */
    void setConfig(const EnhancedSBOMConfig& config);
    
    /**
     * @brief Load compiler metadata from specified directory
     * @param metadata_dir Directory containing compiler metadata files
     * @return true if metadata was loaded successfully
     */
    bool loadCompilerMetadata(const std::string& metadata_dir);
    
    /**
     * @brief Generate enhanced SBOM with both linker and compiler data
     * @return true if generation was successful
     */
    bool generateEnhancedSBOM();
    
    /**
     * @brief Check if compiler metadata is available
     * @return true if compiler metadata was loaded
     */
    bool hasCompilerMetadata() const { return has_compiler_metadata_; }
    
    /**
     * @brief Get the number of components that will be included
     * @return Total component count
     */
    size_t getComponentCount() const;
    
    /**
     * @brief Print generation statistics
     */
    void printStatistics() const;
    
private:
    /**
     * @brief Generate SBOM using plugin (existing functionality)
     * @return true if successful
     */
    bool generateWithPlugin();
    
    /**
     * @brief Enhance the generated SBOM with compiler metadata
     * @return true if successful
     */
    bool enhanceWithCompilerMetadata();
    
    /**
     * @brief Add source file components from compiler metadata
     */
    void addSourceComponents();
    
    /**
     * @brief Add include file components from compiler metadata
     */
    void addIncludeComponents();
    
    /**
     * @brief Add build information as SBOM properties
     */
    void addBuildProperties();
    
    /**
     * @brief Create component info from file component
     * @param file_component Compiler file component
     * @param component_type Type of component
     * @return ComponentInfo object
     */
    ComponentInfo createComponentFromFile(const compiler::FileComponent& file_component,
                                        const std::string& component_type) const;
    
    /**
     * @brief Clean up temporary metadata files
     */
    void cleanupMetadataFiles();
    
    /**
     * @brief Find metadata directory automatically
     * @return Path to metadata directory or empty if not found
     */
    std::string findMetadataDirectory() const;
    
    /**
     * @brief Log enhanced generator messages
     * @param message Message to log
     */
    void logEnhanced(const std::string& message) const;
    
    /**
     * @brief Enhance SBOM JSON with compiler metadata
     * @param sbom_json JSON object to enhance
     */
    void enhanceSBOMJson(nlohmann::json& sbom_json);
    
    /**
     * @brief Enhance CycloneDX SBOM with compiler metadata
     * @param sbom_json CycloneDX SBOM JSON to enhance
     */
    void enhanceCycloneDXSBOM(nlohmann::json& sbom_json);
    
    /**
     * @brief Enhance SPDX SBOM with compiler metadata
     * @param sbom_json SPDX SBOM JSON to enhance
     */
    void enhanceSPDXSBOM(nlohmann::json& sbom_json);
};

/**
 * @brief Enhanced SBOM generation function with compiler metadata support
 * @param config Enhanced SBOM configuration
 * @return 0 on success, 1 on failure
 */
int generate_enhanced_sbom(const EnhancedSBOMConfig& config);

/**
 * @brief Parse enhanced command line arguments
 * @param argc Number of arguments
 * @param argv Argument array
 * @param config Configuration to populate
 * @return true if parsing successful
 */
bool parse_enhanced_arguments(int argc, char* argv[], EnhancedSBOMConfig& config);

/**
 * @brief Print enhanced usage information
 */
void print_enhanced_usage();

}  // namespace heimdall