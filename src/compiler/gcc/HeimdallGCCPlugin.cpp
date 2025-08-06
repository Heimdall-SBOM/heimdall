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
 * @file HeimdallGCCPlugin.cpp
 * @brief GCC compiler plugin for Heimdall SBOM generation
 * @author Trevor Bakker
 * @date 2025
 *
 * This plugin hooks into GCC compilation phases to collect metadata
 * for enhanced SBOM generation, including source files, includes,
 * hashes, and license information.
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "c-family/c-common.h"
#include "diagnostic.h"
#include "plugin.h"
#include "../common/CompilerMetadata.hpp"
#include <iostream>
#include <string>
#include <cstring>

// GPL compatibility assertion (required by GCC)
int plugin_is_GPL_compatible;

// Plugin information
static struct plugin_info heimdall_plugin_info = {
    "1.0.0",
    "Heimdall SBOM Compiler Plugin for GCC"
};

// Global metadata collector
static std::unique_ptr<heimdall::compiler::CompilerMetadataCollector> metadata_collector;

// Plugin configuration
struct PluginConfig {
    std::string output_dir;
    std::string format;
    bool verbose;
    bool include_system_headers;
    
    PluginConfig() : format("json"), verbose(false), include_system_headers(false) {}
};

static PluginConfig plugin_config;

// Forward declarations
static void include_file_callback(void* gcc_data, void* user_data);
static void start_unit_callback(void* gcc_data, void* user_data);
static void finish_unit_callback(void* gcc_data, void* user_data);
static void parse_plugin_args(struct plugin_name_args* plugin_info);
static void capture_compiler_flags();
static void capture_build_environment();
static std::string get_current_source_file();
static void log_plugin_info(const std::string& message);
static void log_plugin_error(const std::string& message);

/**
 * @brief Main plugin entry point called by GCC
 * @param plugin_info Plugin name and arguments
 * @param version GCC version information
 * @return 0 on success, 1 on failure
 */
int plugin_init(struct plugin_name_args* plugin_info, 
                struct plugin_gcc_version* version)
{
    // Version compatibility check
    if (!plugin_default_version_check(version, &gcc_version)) {
        log_plugin_error("Heimdall plugin: incompatible GCC version");
        return 1;
    }
    
    log_plugin_info("Initializing Heimdall GCC plugin v1.0.0");
    
    // Register plugin information
    register_callback(plugin_info->base_name, PLUGIN_INFO, 
                     NULL, &heimdall_plugin_info);
    
    // Parse plugin arguments
    parse_plugin_args(plugin_info);
    
    // Initialize metadata collector
    try {
        metadata_collector = std::make_unique<heimdall::compiler::CompilerMetadataCollector>();
        metadata_collector->setVerbose(plugin_config.verbose);
        
        if (!plugin_config.output_dir.empty()) {
            metadata_collector->setOutputDirectory(plugin_config.output_dir);
        }
        
        // Set compiler information
        metadata_collector->setCompilerType("gcc");
        metadata_collector->setCompilerVersion(version->basever);
        
        log_plugin_info("Metadata collector initialized");
    } catch (const std::exception& e) {
        log_plugin_error("Failed to initialize metadata collector: " + std::string(e.what()));
        return 1;
    }
    
    // Register callbacks for compilation phases
    register_callback(plugin_info->base_name, PLUGIN_INCLUDE_FILE,
                     include_file_callback, NULL);
    
    register_callback(plugin_info->base_name, PLUGIN_START_UNIT,
                     start_unit_callback, NULL);
    
    register_callback(plugin_info->base_name, PLUGIN_FINISH_UNIT,
                     finish_unit_callback, NULL);
    
    log_plugin_info("GCC callbacks registered successfully");
    
    return 0;
}

/**
 * @brief Callback for tracking included files
 * @param gcc_data File path being included
 * @param user_data User-defined data (unused)
 */
static void include_file_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector || !gcc_data) {
        return;
    }
    
    const char* file_path = static_cast<const char*>(gcc_data);
    
    // Skip empty or invalid paths
    if (!file_path || strlen(file_path) == 0) {
        return;
    }
    
    try {
        // Determine if this is a system header
        std::string file_type = "header";
        if (metadata_collector->isSystemFile(file_path)) {
            file_type = "system_header";
            
            // Skip system headers if not configured to include them
            if (!plugin_config.include_system_headers) {
                return;
            }
        }
        
        // Process the included file with enhanced metadata collection
        metadata_collector->processFileComponent(file_path, file_type);
        
        if (plugin_config.verbose) {
            log_plugin_info("Processed include: " + std::string(file_path));
        }
    } catch (const std::exception& e) {
        log_plugin_error("Error processing include file " + std::string(file_path) + 
                        ": " + e.what());
    }
}

/**
 * @brief Callback for start of compilation unit
 * @param gcc_data GCC data (unused)
 * @param user_data User-defined data (unused)
 */
static void start_unit_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) {
        return;
    }
    
    try {
        // Capture the main source file being compiled
        std::string source_file = get_current_source_file();
        if (!source_file.empty()) {
            metadata_collector->setMainSourceFile(source_file);
            
            // Process main source file with enhanced metadata
            metadata_collector->processFileComponent(source_file, "source");
            
            if (plugin_config.verbose) {
                log_plugin_info("Processing main source file: " + source_file);
            }
        }
        
        // Capture compiler flags and build environment
        capture_compiler_flags();
        capture_build_environment();
        
    } catch (const std::exception& e) {
        log_plugin_error("Error in start_unit_callback: " + std::string(e.what()));
    }
}

/**
 * @brief Callback for end of compilation unit
 * @param gcc_data GCC data (unused)
 * @param user_data User-defined data (unused)
 */
static void finish_unit_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) {
        return;
    }
    
    try {
        // Set output object file path if available
        if (asm_file_name) {
            metadata_collector->setObjectFile(asm_file_name);
        }
        
        // Write metadata to intermediate file
        metadata_collector->writeMetadata();
        
        if (plugin_config.verbose) {
            size_t file_count = metadata_collector->getProcessedFileCount();
            log_plugin_info("Compilation unit finished. Processed " + 
                           std::to_string(file_count) + " files");
            log_plugin_info("Metadata written to: " + metadata_collector->getMetadataFilePath());
        }
        
    } catch (const std::exception& e) {
        log_plugin_error("Error in finish_unit_callback: " + std::string(e.what()));
    }
}

/**
 * @brief Parse plugin command line arguments
 * @param plugin_info Plugin information with arguments
 */
static void parse_plugin_args(struct plugin_name_args* plugin_info)
{
    for (int i = 0; i < plugin_info->argc; i++) {
        const char* key = plugin_info->argv[i].key;
        const char* value = plugin_info->argv[i].value;
        
        if (strcmp(key, "output-dir") == 0 && value) {
            plugin_config.output_dir = value;
        } else if (strcmp(key, "format") == 0 && value) {
            plugin_config.format = value;
        } else if (strcmp(key, "verbose") == 0) {
            plugin_config.verbose = true;
        } else if (strcmp(key, "include-system-headers") == 0) {
            plugin_config.include_system_headers = true;
        }
        
        if (plugin_config.verbose) {
            std::string arg_info = "Plugin arg: " + std::string(key);
            if (value) {
                arg_info += "=" + std::string(value);
            }
            log_plugin_info(arg_info);
        }
    }
}

/**
 * @brief Capture compiler flags and optimization settings
 */
static void capture_compiler_flags()
{
    if (!metadata_collector) {
        return;
    }
    
    try {
        // Capture optimization level
        metadata_collector->addCompilerFlag("optimization_level", 
                                           std::to_string(optimize));
        
        // Capture debug information level  
        metadata_collector->addCompilerFlag("debug_level",
                                           std::to_string(debug_info_level));
        
        // Capture target information
        if (target_cpu) {
            metadata_collector->addCompilerFlag("target_cpu", target_cpu);
        }
        metadata_collector->addCompilerFlag("target_arch", TARGET_MACHINE);
        
        // Capture additional build information
        metadata_collector->addCompilerFlag("pic_mode", flag_pic ? "true" : "false");
        metadata_collector->addCompilerFlag("exceptions_enabled", flag_exceptions ? "true" : "false");
        metadata_collector->addCompilerFlag("rtti_enabled", flag_rtti ? "true" : "false");
        metadata_collector->addCompilerFlag("stack_protector", flag_stack_protect ? "true" : "false");
        
        // Capture warning flags
        metadata_collector->addCompilerFlag("warnings_as_errors", werror_flag ? "true" : "false");
        metadata_collector->addCompilerFlag("extra_warnings", extra_warnings ? "true" : "false");
        
        // Capture language standard
        if (flag_isoc99) {
            metadata_collector->addCompilerFlag("c_standard", "c99");
        } else if (flag_isoc11) {
            metadata_collector->addCompilerFlag("c_standard", "c11");
        }
        
        // Capture C++ specific flags
        if (flag_cpp_version) {
            metadata_collector->addCompilerFlag("cpp_version", std::to_string(flag_cpp_version));
        }
        
        if (plugin_config.verbose) {
            log_plugin_info("Captured compiler flags and settings");
        }
        
    } catch (const std::exception& e) {
        log_plugin_error("Error capturing compiler flags: " + std::string(e.what()));
    }
}

/**
 * @brief Capture build environment information
 */
static void capture_build_environment()
{
    if (!metadata_collector) {
        return;
    }
    
    try {
        // Set target architecture
        metadata_collector->setTargetArchitecture(TARGET_MACHINE);
        
        // Try to determine project root from current working directory
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            metadata_collector->setProjectRoot(cwd);
            free(cwd);
        }
        
        // Additional environment information could be captured here
        // such as environment variables, build system info, etc.
        
        if (plugin_config.verbose) {
            log_plugin_info("Captured build environment information");
        }
        
    } catch (const std::exception& e) {
        log_plugin_error("Error capturing build environment: " + std::string(e.what()));
    }
}

/**
 * @brief Get the current source file being compiled
 * @return Source file path
 */
static std::string get_current_source_file()
{
    if (main_input_filename) {
        return std::string(main_input_filename);
    }
    
    // Try to get from input file list
    if (in_fnames && num_in_fnames > 0) {
        return std::string(in_fnames[0]);
    }
    
    return "";
}

/**
 * @brief Log informational message
 * @param message Message to log
 */
static void log_plugin_info(const std::string& message)
{
    if (plugin_config.verbose) {
        std::cout << "[Heimdall-GCC] " << message << std::endl;
    }
}

/**
 * @brief Log error message
 * @param message Error message to log
 */
static void log_plugin_error(const std::string& message)
{
    std::cerr << "[Heimdall-GCC Error] " << message << std::endl;
}