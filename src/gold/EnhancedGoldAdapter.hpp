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
 * @file EnhancedGoldAdapter.hpp
 * @brief Enhanced Gold adapter with compiler metadata integration
 * @author Trevor Bakker
 * @date 2025
 *
 * This enhanced adapter extends the basic Gold adapter to include
 * compiler metadata from the Heimdall compiler plugins, providing
 * comprehensive SBOM generation with both compile-time and link-time data.
 */

#pragma once

#include "GoldAdapter.hpp"
#include "../compiler/common/CompilerMetadata.hpp"
#include "../common/ComponentInfo.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace heimdall
{

/**
 * @brief Enhanced Gold adapter with compiler metadata integration
 */
class EnhancedGoldAdapter : public GoldAdapter
{
private:
    std::vector<compiler::CompilerMetadata> compiler_metadata_;
    std::string metadata_directory_;
    bool has_compiler_metadata_;
    std::map<std::string, compiler::FileComponent> source_file_map_;
    std::map<std::string, compiler::FileComponent> include_file_map_;
    
public:
    /**
     * @brief Constructor
     */
    EnhancedGoldAdapter();
    
    /**
     * @brief Destructor
     */
    ~EnhancedGoldAdapter();
    
    /**
     * @brief Initialize the enhanced adapter
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * @brief Set the metadata directory where compiler plugins store data
     * @param directory Path to metadata directory
     */
    void setMetadataDirectory(const std::string& directory);
    
    /**
     * @brief Load compiler metadata from plugin output files
     * @return true if metadata was loaded successfully
     */
    bool loadCompilerMetadata();
    
    /**
     * @brief Generate enhanced SBOM with compiler and linker metadata
     */
    void generateSBOM();
    
    /**
     * @brief Get the number of source files processed by compiler plugins
     * @return Number of source files
     */
    size_t getSourceFileCount() const;
    
    /**
     * @brief Get the number of include files processed by compiler plugins
     * @return Number of include files
     */
    size_t getIncludeFileCount() const;
    
    /**
     * @brief Get unique licenses from all processed files
     * @return Vector of unique licenses
     */
    std::vector<LicenseInfo> getUniqueLicenses() const;
    
    /**
     * @brief Print enhanced statistics including compiler metadata
     */
    void printStatistics() const;
    
    /**
     * @brief Check if compiler metadata is available
     * @return true if compiler metadata was loaded
     */
    bool hasCompilerMetadata() const { return has_compiler_metadata_; }
    
protected:
    /**
     * @brief Enhance linker components with compiler metadata
     * @param generator SBOM generator to enhance
     */
    void enhanceWithCompilerMetadata(SBOMGenerator& generator);
    
    /**
     * @brief Add source file components to SBOM
     * @param generator SBOM generator to add components to
     */
    void addSourceComponents(SBOMGenerator& generator);
    
    /**
     * @brief Add include file components to SBOM
     * @param generator SBOM generator to add components to
     */
    void addIncludeComponents(SBOMGenerator& generator);
    
    /**
     * @brief Add compiler build information as properties
     * @param generator SBOM generator to add properties to
     */
    void addBuildProperties(SBOMGenerator& generator);
    
    /**
     * @brief Create component info from file component
     * @param file_component Compiler file component
     * @param component_type Type of component to create
     * @return ComponentInfo object
     */
    ComponentInfo createComponentFromFile(const compiler::FileComponent& file_component,
                                        const std::string& component_type) const;
    
    /**
     * @brief Find metadata directory automatically
     * @return Path to metadata directory or empty string if not found
     */
    std::string findMetadataDirectory() const;
    
    /**
     * @brief Build file lookup maps for efficient access
     */
    void buildFileLookupMaps();
    
    /**
     * @brief Clean up temporary metadata files
     */
    void cleanupMetadataFiles();
    
    /**
     * @brief Log enhanced adapter messages
     * @param message Message to log
     */
    void logEnhanced(const std::string& message) const;
    
    /**
     * @brief Add linker components from base adapter
     * @param generator SBOM generator to add components to
     */
    void addLinkerComponents(SBOMGenerator& generator);
    
    // Accessor methods that delegate to base class
    std::string getFormat() const;
    std::string getOutputPath() const;
    bool isVerbose() const;
};

}  // namespace heimdall