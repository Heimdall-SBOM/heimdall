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
 * @file EnhancedSBOMGenerator.cpp
 * @brief Implementation of enhanced SBOM generator with compiler metadata
 * @author Trevor Bakker
 * @date 2025
 */

#include "EnhancedSBOMGenerator.hpp"
#include "../common/Utils.hpp"
#include "../compat/compatibility.hpp"
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <cstring>

namespace heimdall
{

// Function pointer types for plugin interface
using init_func_t = int (*)(void*);
using set_format_func_t = int (*)(const char*);
using set_output_path_func_t = int (*)(const char*);
using process_input_file_func_t = int (*)(const char*);
using finalize_func_t = void (*)();
using set_verbose_func_t = void (*)(bool);

EnhancedSBOMGenerator::EnhancedSBOMGenerator()
    : has_compiler_metadata_(false)
{
    sbom_generator_ = std::make_unique<SBOMGenerator>();
}

EnhancedSBOMGenerator::~EnhancedSBOMGenerator()
{
    if (config_.cleanup_metadata && !config_.metadata_directory.empty()) {
        cleanupMetadataFiles();
    }
}

void EnhancedSBOMGenerator::setConfig(const EnhancedSBOMConfig& config)
{
    config_ = config;
    
    if (sbom_generator_) {
        sbom_generator_->setFormat(config_.format);
        // Note: setVerbose not available in SBOMGenerator API
    }
}

bool EnhancedSBOMGenerator::loadCompilerMetadata(const std::string& metadata_dir)
{
    if (metadata_dir.empty() || !compat::fs::exists(metadata_dir)) {
        logEnhanced("Metadata directory not found: " + metadata_dir);
        return false;
    }
    
    try {
        compiler_metadata_ = compiler::CompilerMetadataCollector::loadMetadataFiles(metadata_dir);
        
        if (compiler_metadata_.empty()) {
            logEnhanced("No compiler metadata files found in: " + metadata_dir);
            return false;
        }
        
        has_compiler_metadata_ = true;
        logEnhanced("Loaded " + std::to_string(compiler_metadata_.size()) + " compiler metadata files");
        
        // Print summary if verbose
        if (config_.verbose) {
            size_t total_source_files = 0;
            size_t total_include_files = 0;
            
            for (const auto& metadata : compiler_metadata_) {
                total_source_files += metadata.source_files.size();
                total_include_files += metadata.include_files.size();
            }
            
            logEnhanced("Total source files: " + std::to_string(total_source_files));
            logEnhanced("Total include files: " + std::to_string(total_include_files));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to load compiler metadata: " + std::string(e.what()));
        return false;
    }
}

bool EnhancedSBOMGenerator::generateEnhancedSBOM()
{
    logEnhanced("Starting enhanced SBOM generation");
    
    // Try to load compiler metadata if directory is specified
    if (config_.include_compiler_metadata) {
        std::string metadata_dir = config_.metadata_directory;
        
        if (metadata_dir.empty()) {
            metadata_dir = findMetadataDirectory();
        }
        
        if (!metadata_dir.empty()) {
            loadCompilerMetadata(metadata_dir);
        }
    }
    
    // Generate base SBOM using plugin
    if (!generateWithPlugin()) {
        Utils::errorPrint("Failed to generate base SBOM using plugin");
        return false;
    }
    
    // Enhance with compiler metadata if available
    if (hasCompilerMetadata()) {
        if (!enhanceWithCompilerMetadata()) {
            Utils::warningPrint("Failed to enhance SBOM with compiler metadata");
            // Continue anyway - we have the base SBOM
        }
    }
    
    logEnhanced("Enhanced SBOM generation completed");
    return true;
}

size_t EnhancedSBOMGenerator::getComponentCount() const
{
    size_t count = 0;
    
    if (sbom_generator_) {
        count += sbom_generator_->getComponentCount();
    }
    
    if (hasCompilerMetadata()) {
        for (const auto& metadata : compiler_metadata_) {
            count += metadata.source_files.size();
            count += metadata.include_files.size();
        }
    }
    
    return count;
}

void EnhancedSBOMGenerator::printStatistics() const
{
    std::cout << "\n=== Enhanced SBOM Generation Statistics ===\n";
    
    if (sbom_generator_) {
        std::cout << "Linker components: " << sbom_generator_->getComponentCount() << "\n";
    }
    
    if (hasCompilerMetadata()) {
        size_t total_source = 0;
        size_t total_include = 0;
        std::set<std::string> compilers;
        std::set<std::string> licenses;
        
        for (const auto& metadata : compiler_metadata_) {
            total_source += metadata.source_files.size();
            total_include += metadata.include_files.size();
            compilers.insert(metadata.compiler_type + " " + metadata.compiler_version);
            
            auto metadata_licenses = metadata.getUniqueLicenses();
            for (const auto& license : metadata_licenses) {
                if (!license.spdxId.empty()) {
                    licenses.insert(license.spdxId);
                }
            }
        }
        
        std::cout << "Compiler metadata files: " << compiler_metadata_.size() << "\n";
        std::cout << "Source files: " << total_source << "\n";
        std::cout << "Include files: " << total_include << "\n";
        std::cout << "Unique licenses: " << licenses.size() << "\n";
        std::cout << "Compilers used: " << compilers.size() << "\n";
        
        if (config_.verbose) {
            std::cout << "\nCompilers:\n";
            for (const auto& compiler : compilers) {
                std::cout << "  - " << compiler << "\n";
            }
            
            if (!licenses.empty()) {
                std::cout << "\nLicenses:\n";
                for (const auto& license : licenses) {
                    std::cout << "  - " << license << "\n";
                }
            }
        }
    } else {
        std::cout << "No compiler metadata available\n";
    }
    
    std::cout << "Total components: " << getComponentCount() << "\n";
    std::cout << "Output format: " << config_.format << "\n";
    std::cout << "Output file: " << config_.output_path << "\n";
}

bool EnhancedSBOMGenerator::generateWithPlugin()
{
    // Load the plugin shared library
    void* handle = dlopen(config_.plugin_path.c_str(), RTLD_LAZY);
    if (!handle) {
        Utils::errorPrint("Failed to load plugin " + config_.plugin_path + ": " + dlerror());
        return false;
    }
    
    // Get function pointers from the plugin
    auto onload = (init_func_t)dlsym(handle, "onload");
    auto set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    auto set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
    auto process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
    auto finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");
    auto set_verbose = (set_verbose_func_t)dlsym(handle, "heimdall_set_verbose");
    
    if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
        Utils::errorPrint("Plugin does not provide required functions");
        dlclose(handle);
        return false;
    }
    
    // Initialize plugin
    if (onload(nullptr) != 0) {
        Utils::errorPrint("Plugin initialization failed");
        dlclose(handle);
        return false;
    }
    
    // Configure plugin
    if (set_format(config_.format.c_str()) != 0) {
        Utils::errorPrint("Failed to set format: " + config_.format);
        dlclose(handle);
        return false;
    }
    
    if (set_output_path(config_.output_path.c_str()) != 0) {
        Utils::errorPrint("Failed to set output path: " + config_.output_path);
        dlclose(handle);
        return false;
    }
    
    if (set_verbose) {
        set_verbose(config_.verbose);
    }
    
    // Process binary file
    if (process_input_file(config_.binary_path.c_str()) != 0) {
        Utils::errorPrint("Failed to process binary file: " + config_.binary_path);
        dlclose(handle);
        return false;
    }
    
    // Finalize and generate SBOM
    finalize();
    
    dlclose(handle);
    
    logEnhanced("Base SBOM generated using plugin");
    return true;
}

bool EnhancedSBOMGenerator::enhanceWithCompilerMetadata()
{
    if (!hasCompilerMetadata()) {
        return false;
    }
    
    logEnhanced("Enhancing SBOM with compiler metadata");
    
    try {
        // Read the existing SBOM file
        std::ifstream input_file(config_.output_path);
        if (!input_file.is_open()) {
            Utils::errorPrint("Failed to read generated SBOM file: " + config_.output_path);
            return false;
        }
        
        nlohmann::json sbom_json;
        input_file >> sbom_json;
        input_file.close();
        
        // Add compiler metadata components
        enhanceSBOMJson(sbom_json);
        
        // Write enhanced SBOM back
        std::ofstream output_file(config_.output_path);
        if (!output_file.is_open()) {
            Utils::errorPrint("Failed to write enhanced SBOM file: " + config_.output_path);
            return false;
        }
        
        output_file << sbom_json.dump(2);
        output_file.close();
        
        logEnhanced("SBOM enhanced with compiler metadata");
        return true;
        
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to enhance SBOM: " + std::string(e.what()));
        return false;
    }
}

void EnhancedSBOMGenerator::enhanceSBOMJson(nlohmann::json& sbom_json)
{
    // Add compiler metadata based on SBOM format
    if (config_.format.find("cyclonedx") != std::string::npos) {
        enhanceCycloneDXSBOM(sbom_json);
    } else if (config_.format.find("spdx") != std::string::npos) {
        enhanceSPDXSBOM(sbom_json);
    }
}

void EnhancedSBOMGenerator::enhanceCycloneDXSBOM(nlohmann::json& sbom_json)
{
    if (!sbom_json.contains("components")) {
        sbom_json["components"] = nlohmann::json::array();
    }
    
    auto& components = sbom_json["components"];
    
    // Add source file components
    for (const auto& metadata : compiler_metadata_) {
        for (const auto& source_file : metadata.source_files) {
            nlohmann::json component;
            component["type"] = "file";
            component["bom-ref"] = source_file.relative_path;
            component["name"] = Utils::getFileName(source_file.file_path);
            component["scope"] = "required";
            
            // Add hashes
            if (source_file.hashes.isValid()) {
                component["hashes"] = nlohmann::json::array();
                component["hashes"].push_back({
                    {"alg", "SHA-256"},
                    {"content", source_file.hashes.sha256}
                });
                if (!source_file.hashes.sha1.empty()) {
                    component["hashes"].push_back({
                        {"alg", "SHA-1"},
                        {"content", source_file.hashes.sha1}
                    });
                }
            }
            
            // Add license information
            if (!source_file.license.name.empty()) {
                component["licenses"] = nlohmann::json::array();
                component["licenses"].push_back({
                    {"license", {
                        {"id", source_file.license.spdxId},
                        {"name", source_file.license.name}
                    }}
                });
            }
            
            // Add copyright
            if (!source_file.copyright_notice.empty()) {
                component["copyright"] = source_file.copyright_notice;
            }
            
            // Add properties
            component["properties"] = nlohmann::json::array();
            component["properties"].push_back({
                {"name", "compiler.type"},
                {"value", metadata.compiler_type}
            });
            component["properties"].push_back({
                {"name", "compiler.version"},
                {"value", metadata.compiler_version}
            });
            component["properties"].push_back({
                {"name", "file.size"},
                {"value", std::to_string(source_file.hashes.file_size)}
            });
            
            components.push_back(component);
        }
        
        // Add include file components (system headers marked as optional)
        for (const auto& include_file : metadata.include_files) {
            nlohmann::json component;
            component["type"] = "file";
            component["bom-ref"] = include_file.relative_path;
            component["name"] = Utils::getFileName(include_file.file_path);
            component["scope"] = include_file.is_system_file ? "optional" : "required";
            
            // Add properties for include files
            component["properties"] = nlohmann::json::array();
            component["properties"].push_back({
                {"name", "file.type"},
                {"value", include_file.file_type}
            });
            component["properties"].push_back({
                {"name", "file.is_system"},
                {"value", include_file.is_system_file ? "true" : "false"}
            });
            
            if (include_file.hashes.isValid()) {
                component["properties"].push_back({
                    {"name", "file.size"},
                    {"value", std::to_string(include_file.hashes.file_size)}
                });
            }
            
            components.push_back(component);
        }
    }
}

void EnhancedSBOMGenerator::enhanceSPDXSBOM(nlohmann::json& sbom_json)
{
    // Add SPDX-specific enhancements
    if (!sbom_json.contains("files")) {
        sbom_json["files"] = nlohmann::json::array();
    }
    
    auto& files = sbom_json["files"];
    
    // Add source files
    for (const auto& metadata : compiler_metadata_) {
        for (const auto& source_file : metadata.source_files) {
            nlohmann::json file_entry;
            file_entry["fileName"] = source_file.relative_path;
            file_entry["SPDXID"] = "SPDXRef-" + Utils::replace(source_file.relative_path, "/", "-");
            
            // Add checksums
            if (source_file.hashes.isValid()) {
                file_entry["checksums"] = nlohmann::json::array();
                file_entry["checksums"].push_back({
                    {"algorithm", "SHA256"},
                    {"checksumValue", source_file.hashes.sha256}
                });
            }
            
            // Add license information
            if (!source_file.license.spdxId.empty()) {
                file_entry["licenseConcluded"] = source_file.license.spdxId;
            } else {
                file_entry["licenseConcluded"] = "NOASSERTION";
            }
            
            // Add copyright
            if (!source_file.copyright_notice.empty()) {
                file_entry["copyrightText"] = source_file.copyright_notice;
            } else {
                file_entry["copyrightText"] = "NOASSERTION";
            }
            
            files.push_back(file_entry);
        }
    }
}

ComponentInfo EnhancedSBOMGenerator::createComponentFromFile(const compiler::FileComponent& file_component,
                                                           const std::string& component_type) const
{
    ComponentInfo component;
    
    // Basic information
    component.name = Utils::getFileName(file_component.file_path);
    component.filePath = file_component.file_path;
    
    // Add hashes
    if (file_component.hashes.isValid()) {
        component.checksum = file_component.hashes.sha256;
        component.properties["hash.sha1"] = file_component.hashes.sha1;
        component.properties["hash.md5"] = file_component.hashes.md5;
        component.properties["file.size"] = std::to_string(file_component.hashes.file_size);
        component.fileSize = file_component.hashes.file_size;
    }
    
    // Add license
    if (!file_component.license.name.empty()) {
        component.license = file_component.license.spdxId;
    }
    
    // Add copyright
    if (!file_component.copyright_notice.empty()) {
        component.copyright = file_component.copyright_notice;
    }
    
    return component;
}

void EnhancedSBOMGenerator::cleanupMetadataFiles()
{
    if (!config_.metadata_directory.empty()) {
        try {
            compiler::CompilerMetadataCollector::cleanupMetadataFiles(config_.metadata_directory);
            logEnhanced("Cleaned up metadata directory: " + config_.metadata_directory);
        } catch (const std::exception& e) {
            Utils::warningPrint("Failed to cleanup metadata directory: " + std::string(e.what()));
        }
    }
}

std::string EnhancedSBOMGenerator::findMetadataDirectory() const
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
                    logEnhanced("Found metadata directory: " + dir);
                    return dir;
                }
            }
        }
    }
    
    return "";
}

void EnhancedSBOMGenerator::logEnhanced(const std::string& message) const
{
    if (config_.verbose) {
        Utils::infoPrint("[Enhanced SBOM] " + message);
    }
}

// Global functions
int generate_enhanced_sbom(const EnhancedSBOMConfig& config)
{
    try {
        EnhancedSBOMGenerator generator;
        generator.setConfig(config);
        
        if (!generator.generateEnhancedSBOM()) {
            return 1;
        }
        
        if (config.verbose) {
            generator.printStatistics();
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        Utils::errorPrint("Enhanced SBOM generation failed: " + std::string(e.what()));
        return 1;
    }
}

bool parse_enhanced_arguments(int argc, char* argv[], EnhancedSBOMConfig& config)
{
    // Check for help
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_enhanced_usage();
            return false;
        }
    }
    
    // Require minimum arguments
    if (argc < 7) {
        Utils::errorPrint("Insufficient arguments");
        print_enhanced_usage();
        return false;
    }
    
    // Parse positional arguments
    config.plugin_path = argv[1];
    config.binary_path = argv[2];
    
    // Parse named arguments
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            config.format = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config.output_path = argv[++i];
        } else if (strcmp(argv[i], "--metadata-dir") == 0 && i + 1 < argc) {
            config.metadata_directory = argv[++i];
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config.verbose = true;
        } else if (strcmp(argv[i], "--no-compiler-metadata") == 0) {
            config.include_compiler_metadata = false;
        } else if (strcmp(argv[i], "--no-cleanup") == 0) {
            config.cleanup_metadata = false;
        } else if (strcmp(argv[i], "--cyclonedx-version") == 0 && i + 1 < argc) {
            config.cyclonedx_version = argv[++i];
        } else if (strcmp(argv[i], "--spdx-version") == 0 && i + 1 < argc) {
            config.spdx_version = argv[++i];
        }
    }
    
    // Validate required arguments
    if (config.output_path.empty()) {
        Utils::errorPrint("Output path is required (--output)");
        return false;
    }
    
    return true;
}

void print_enhanced_usage()
{
    std::cout << "Heimdall Enhanced SBOM Generator - Comprehensive SBOM Generation with Compiler Metadata\n\n";
    std::cout << "Usage: heimdall-enhanced-sbom <plugin_path> <binary_path> --format <format> --output <output_path> [options]\n\n";
    
    std::cout << "Required Arguments:\n";
    std::cout << "  <plugin_path>           Path to Heimdall linker plugin (.so file)\n";
    std::cout << "  <binary_path>           Path to binary file to analyze\n";
    std::cout << "  --format <format>       Output format (spdx, cyclonedx, etc.)\n";
    std::cout << "  --output <output_path>  Output file path for the generated SBOM\n\n";
    
    std::cout << "Enhanced Options:\n";
    std::cout << "  --metadata-dir <dir>    Directory containing compiler metadata files\n";
    std::cout << "  --no-compiler-metadata  Disable compiler metadata integration\n";
    std::cout << "  --no-cleanup           Don't cleanup temporary metadata files\n";
    std::cout << "  --verbose              Enable verbose output\n\n";
    
    std::cout << "Format Options:\n";
    std::cout << "  --format spdx          Generate SPDX format (default: 2.3)\n";
    std::cout << "  --format cyclonedx     Generate CycloneDX format (default: 1.6)\n";
    std::cout << "  --cyclonedx-version    Specify CycloneDX version (1.4, 1.6)\n";
    std::cout << "  --spdx-version         Specify SPDX version (2.3, 3.0)\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  # Generate enhanced SPDX SBOM with compiler metadata\n";
    std::cout << "  heimdall-enhanced-sbom ./lib/heimdall-lld.so ./myapp --format spdx --output enhanced.spdx\n\n";
    
    std::cout << "  # Generate enhanced CycloneDX SBOM with verbose output\n";
    std::cout << "  heimdall-enhanced-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output enhanced.cdx.json --verbose\n\n";
    
    std::cout << "  # Use specific metadata directory\n";
    std::cout << "  heimdall-enhanced-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output enhanced.cdx.json \\\n";
    std::cout << "    --metadata-dir ./build/heimdall-metadata\n\n";
    
    std::cout << "Features:\n";
    std::cout << "  - Combines linker analysis with compiler metadata\n";
    std::cout << "  - File integrity verification with SHA-256, SHA-1, MD5 hashes\n";
    std::cout << "  - Automatic license detection and SPDX compliance\n";
    std::cout << "  - Source file and header file tracking\n";
    std::cout << "  - Build environment and compiler flag recording\n";
    std::cout << "  - Copyright and author information extraction\n";
}

}  // namespace heimdall