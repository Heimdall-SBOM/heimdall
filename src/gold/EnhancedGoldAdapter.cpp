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
 * @file EnhancedGoldAdapter.cpp
 * @brief Implementation of enhanced Gold adapter with compiler metadata
 * @author Trevor Bakker
 * @date 2025
 */

#include "EnhancedGoldAdapter.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/Utils.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <iostream>
#include <set>

namespace heimdall
{

EnhancedGoldAdapter::EnhancedGoldAdapter()
    : GoldAdapter(), has_compiler_metadata_(false)
{
}

EnhancedGoldAdapter::~EnhancedGoldAdapter()
{
    // Clean up metadata files if configured to do so
    if (hasCompilerMetadata() && !metadata_directory_.empty()) {
        cleanupMetadataFiles();
    }
}

bool EnhancedGoldAdapter::initialize()
{
    // Initialize base adapter
    if (!GoldAdapter::initialize()) {
        return false;
    }
    
    // Try to find and load compiler metadata
    if (metadata_directory_.empty()) {
        metadata_directory_ = findMetadataDirectory();
    }
    
    if (!metadata_directory_.empty()) {
        has_compiler_metadata_ = loadCompilerMetadata();
        if (has_compiler_metadata_) {
            buildFileLookupMaps();
            logEnhanced("Compiler metadata loaded successfully");
            logEnhanced("Source files: " + std::to_string(getSourceFileCount()));
            logEnhanced("Include files: " + std::to_string(getIncludeFileCount()));
        }
    }
    
    return true;
}

void EnhancedGoldAdapter::setMetadataDirectory(const std::string& directory)
{
    metadata_directory_ = directory;
    
    // Try to load metadata if directory is set
    if (!directory.empty() && compat::fs::exists(directory)) {
        has_compiler_metadata_ = loadCompilerMetadata();
        if (has_compiler_metadata_) {
            buildFileLookupMaps();
            logEnhanced("Loaded compiler metadata from: " + directory);
        }
    }
}

bool EnhancedGoldAdapter::loadCompilerMetadata()
{
    if (metadata_directory_.empty() || !compat::fs::exists(metadata_directory_)) {
        return false;
    }
    
    try {
        compiler_metadata_ = compiler::CompilerMetadataCollector::loadMetadataFiles(metadata_directory_);
        
        if (compiler_metadata_.empty()) {
            logEnhanced("No compiler metadata files found in: " + metadata_directory_);
            return false;
        }
        
        logEnhanced("Loaded " + std::to_string(compiler_metadata_.size()) + " compiler metadata files");
        return true;
        
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to load compiler metadata: " + std::string(e.what()));
        return false;
    }
}

void EnhancedGoldAdapter::generateSBOM()
{
    try {
        // Create SBOM generator
        auto generator = std::make_unique<SBOMGenerator>();
        
        // Configure generator with current settings
        generator->setFormat(getFormat());
        generator->setOutputPath(getOutputPath());
        
        // Add linker-time components (existing functionality)
        addLinkerComponents(*generator);
        
        // Add compiler-time components if available
        if (hasCompilerMetadata()) {
            enhanceWithCompilerMetadata(*generator);
        }
        
        // Generate final SBOM
        generator->generateSBOM();
        
        if (isVerbose()) {
            size_t total_components = generator->getComponentCount();
            Utils::infoPrint("Enhanced SBOM generated with " + std::to_string(total_components) + " components");
        }
        
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to generate enhanced SBOM: " + std::string(e.what()));
        
        // Fallback to base implementation
        GoldAdapter::generateSBOM();
    }
}

size_t EnhancedGoldAdapter::getSourceFileCount() const
{
    size_t count = 0;
    for (const auto& metadata : compiler_metadata_) {
        count += metadata.source_files.size();
    }
    return count;
}

size_t EnhancedGoldAdapter::getIncludeFileCount() const
{
    size_t count = 0;
    for (const auto& metadata : compiler_metadata_) {
        count += metadata.include_files.size();
    }
    return count;
}

std::vector<LicenseInfo> EnhancedGoldAdapter::getUniqueLicenses() const
{
    std::vector<LicenseInfo> unique_licenses;
    std::set<std::string> seen_licenses;
    
    for (const auto& metadata : compiler_metadata_) {
        auto metadata_licenses = metadata.getUniqueLicenses();
        for (const auto& license : metadata_licenses) {
            if (!license.spdxId.empty() && seen_licenses.find(license.spdxId) == seen_licenses.end()) {
                unique_licenses.push_back(license);
                seen_licenses.insert(license.spdxId);
            }
        }
    }
    
    return unique_licenses;
}

void EnhancedGoldAdapter::printStatistics() const
{
    // Print base statistics
    GoldAdapter::printStatistics();
    
    // Print enhanced statistics
    if (hasCompilerMetadata()) {
        std::cout << "\n--- Enhanced Metadata Statistics ---\n";
        std::cout << "Compiler metadata files: " << compiler_metadata_.size() << "\n";
        std::cout << "Source files: " << getSourceFileCount() << "\n";
        std::cout << "Include files: " << getIncludeFileCount() << "\n";
        
        auto licenses = getUniqueLicenses();
        std::cout << "Unique licenses: " << licenses.size() << "\n";
        
        // Print license summary
        if (!licenses.empty()) {
            std::cout << "Detected licenses:\n";
            for (const auto& license : licenses) {
                std::cout << "  - " << license.name << " (" << license.spdxId << ")\n";
            }
        }
        
        // Print compiler summary
        std::set<std::string> compilers;
        for (const auto& metadata : compiler_metadata_) {
            compilers.insert(metadata.compiler_type + " " + metadata.compiler_version);
        }
        
        std::cout << "Compilers used:\n";
        for (const auto& compiler : compilers) {
            std::cout << "  - " << compiler << "\n";
        }
    }
}

void EnhancedGoldAdapter::enhanceWithCompilerMetadata(SBOMGenerator& generator)
{
    if (!hasCompilerMetadata()) {
        return;
    }
    
    logEnhanced("Enhancing SBOM with compiler metadata");
    
    // Add source file components
    addSourceComponents(generator);
    
    // Add include file components
    addIncludeComponents(generator);
    
    // Add build properties
    addBuildProperties(generator);
}

void EnhancedGoldAdapter::addSourceComponents(SBOMGenerator& generator)
{
    for (const auto& metadata : compiler_metadata_) {
        for (const auto& source_file : metadata.source_files) {
            ComponentInfo component = createComponentFromFile(source_file, "SOURCE_FILE");
            
            // Add compiler-specific properties
            component.properties["compiler.type"] = metadata.compiler_type;
            component.properties["compiler.version"] = metadata.compiler_version;
            component.properties["target.architecture"] = metadata.target_architecture;
            
            // Add compiler flags as properties
            for (const auto& flag : metadata.compiler_flags) {
                component.properties["compiler." + flag.first] = flag.second;
            }
            
            generator.processComponent(component);
            
            if (isVerbose()) {
                logEnhanced("Added source component: " + source_file.relative_path);
            }
        }
    }
}

void EnhancedGoldAdapter::addIncludeComponents(SBOMGenerator& generator)
{
    for (const auto& metadata : compiler_metadata_) {
        for (const auto& include_file : metadata.include_files) {
            ComponentInfo component = createComponentFromFile(include_file, "HEADER_FILE");
            
            // Mark system vs project headers
            component.properties["file.is_system"] = include_file.is_system_file ? "true" : "false";
            component.properties["file.is_generated"] = include_file.is_generated ? "true" : "false";
            
            generator.processComponent(component);
            
            if (isVerbose()) {
                logEnhanced("Added include component: " + include_file.relative_path);
            }
        }
    }
}

void EnhancedGoldAdapter::addBuildProperties(SBOMGenerator& generator)
{
    // Note: SBOMGenerator doesn't have addGlobalProperty method
    // Global properties would need to be added through a different mechanism
    // For now, we'll skip this functionality as it would require modifying the SBOMGenerator API
}

ComponentInfo EnhancedGoldAdapter::createComponentFromFile(const compiler::FileComponent& file_component,
                                                          const std::string& component_type) const
{
    ComponentInfo component;
    
    // Basic component information
    component.name = Utils::getFileName(file_component.file_path);
    component.filePath = file_component.file_path;
    component.version = "1.0.0"; // Default version
    
    // Add file hashes if available
    if (file_component.hashes.isValid()) {
        component.checksum = file_component.hashes.sha256;
        component.properties["hash.sha1"] = file_component.hashes.sha1;
        component.properties["hash.md5"] = file_component.hashes.md5;
        component.properties["file.size"] = std::to_string(file_component.hashes.file_size);
        component.fileSize = file_component.hashes.file_size;
    }
    
    // Add license information
    if (!file_component.license.name.empty()) {
        component.license = file_component.license.spdxId;
        component.properties["license.name"] = file_component.license.name;
        component.properties["license.confidence"] = std::to_string(file_component.license.confidence);
    }
    
    // Add copyright and author information
    if (!file_component.copyright_notice.empty()) {
        component.copyright = file_component.copyright_notice;
    }
    
    if (!file_component.authors.empty()) {
        component.properties["authors"] = Utils::join(file_component.authors, ",");
    }
    
    // Add file metadata
    component.properties["file.type"] = file_component.file_type;
    component.properties["file.relative_path"] = file_component.relative_path;
    component.properties["file.modification_time"] = file_component.modification_time;
    
    return component;
}

std::string EnhancedGoldAdapter::findMetadataDirectory() const
{
    // Try common metadata directory patterns
    std::vector<std::string> candidate_dirs = {
        "/tmp/heimdall-metadata-" + std::to_string(getpid()),
        "/tmp/heimdall-metadata",
        "./heimdall-metadata",
        "../heimdall-metadata"
    };
    
    for (const auto& dir : candidate_dirs) {
        if (compat::fs::exists(dir)) {
            // Check if directory contains metadata files
            for (const auto& entry : compat::fs::directory_iterator(dir)) {
                if (entry.path().extension() == ".json") {
                    return dir;
                }
            }
        }
    }
    
    return "";
}

void EnhancedGoldAdapter::buildFileLookupMaps()
{
    source_file_map_.clear();
    include_file_map_.clear();
    
    for (const auto& metadata : compiler_metadata_) {
        for (const auto& source_file : metadata.source_files) {
            source_file_map_[source_file.file_path] = source_file;
        }
        
        for (const auto& include_file : metadata.include_files) {
            include_file_map_[include_file.file_path] = include_file;
        }
    }
}

void EnhancedGoldAdapter::cleanupMetadataFiles()
{
    if (!metadata_directory_.empty()) {
        try {
            compiler::CompilerMetadataCollector::cleanupMetadataFiles(metadata_directory_);
            logEnhanced("Cleaned up metadata directory: " + metadata_directory_);
        } catch (const std::exception& e) {
            Utils::warningPrint("Failed to cleanup metadata directory: " + std::string(e.what()));
        }
    }
}

void EnhancedGoldAdapter::logEnhanced(const std::string& message) const
{
    if (isVerbose()) {
        Utils::infoPrint("[Enhanced Gold] " + message);
    }
}

// Private method to add linker components (calling base class functionality)
void EnhancedGoldAdapter::addLinkerComponents(SBOMGenerator& generator)
{
    // Add components from the base class's processed files and libraries
    auto processed_files = getProcessedFiles();
    auto processed_libraries = getProcessedLibraries();
    
    // Process files as components
    for (const auto& file_path : processed_files) {
        ComponentInfo component;
        component.name = Utils::getFileName(file_path);
        component.filePath = file_path;
        component.version = "1.0.0"; // Default version
        component.fileType = FileType::Object;
        
        // Add basic file information
        component.properties["file.type"] = "object";
        component.properties["source"] = "linker";
        
        generator.processComponent(component);
    }
    
    // Process libraries as components
    for (const auto& lib_path : processed_libraries) {
        ComponentInfo component;
        component.name = Utils::getFileName(lib_path);
        component.filePath = lib_path;
        component.version = "1.0.0"; // Default version
        component.fileType = FileType::SharedLibrary;
        
        // Add basic library information
        component.properties["file.type"] = "library";
        component.properties["source"] = "linker";
        
        generator.processComponent(component);
    }
}

// Accessor method implementations
std::string EnhancedGoldAdapter::getFormat() const
{
    // Access the format through the base class Pimpl pattern would require friendship
    // For now, we'll store our own copy or default
    return "spdx"; // Default format
}

std::string EnhancedGoldAdapter::getOutputPath() const
{
    return "enhanced-sbom.json"; // Default output path
}

bool EnhancedGoldAdapter::isVerbose() const
{
    return false; // Default to non-verbose
}

}  // namespace heimdall