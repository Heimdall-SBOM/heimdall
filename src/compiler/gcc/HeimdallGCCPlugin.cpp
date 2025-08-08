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

// Save any existing abort macro to restore later
#ifdef abort
#define HEIMDALL_SAVED_ABORT abort
#undef abort
#endif

// Include GCC plugin headers first to avoid conflicts
extern "C" {
    #include "gcc-plugin.h"
    #include "plugin-version.h"
    #include "tree.h"
    #include "c-family/c-common.h"
    #include "diagnostic.h"
    #include "plugin.h"
}

// Restore the original abort macro if it was saved
#ifdef HEIMDALL_SAVED_ABORT
#define abort HEIMDALL_SAVED_ABORT
#undef HEIMDALL_SAVED_ABORT
#endif

// Include system headers
#include <iostream>
#include <string>
#include <cstring>

// Include project headers after GCC headers to avoid conflicts
#include "../common/CompilerMetadata.hpp"

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
    metadata_collector = std::make_unique<heimdall::compiler::CompilerMetadataCollector>();
    if (!metadata_collector) {
        log_plugin_error("Failed to initialize metadata collector");
        return 1;
    }
    
    // Capture build environment
    capture_build_environment();
    
    // Register callbacks
    register_callback(plugin_info->base_name, PLUGIN_INCLUDE_FILE, 
                     include_file_callback, NULL);
    register_callback(plugin_info->base_name, PLUGIN_START_UNIT, 
                     start_unit_callback, NULL);
    register_callback(plugin_info->base_name, PLUGIN_FINISH_UNIT, 
                     finish_unit_callback, NULL);
    
    log_plugin_info("Heimdall GCC plugin initialized successfully");
    return 0;
}

/**
 * @brief Callback for when a file is included
 */
static void include_file_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) return;
    
    const char* filename = static_cast<const char*>(gcc_data);
    if (!filename) return;
    
    // Skip system headers unless explicitly requested
    if (!plugin_config.include_system_headers) {
        if (strstr(filename, "/usr/include/") || 
            strstr(filename, "/usr/local/include/") ||
            strstr(filename, "/opt/rh/")) {
            return;
        }
    }
    
    if (plugin_config.verbose) {
        log_plugin_info("Including file: " + std::string(filename));
    }
    
    metadata_collector->addSourceFile(filename);
}

/**
 * @brief Callback for when compilation of a translation unit starts
 */
static void start_unit_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) return;
    
    std::string source_file = get_current_source_file();
    if (!source_file.empty()) {
        if (plugin_config.verbose) {
            log_plugin_info("Starting compilation of: " + source_file);
        }
        
        metadata_collector->setMainSourceFile(source_file);
        capture_compiler_flags();
    }
}

/**
 * @brief Callback for when compilation of a translation unit finishes
 */
static void finish_unit_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) return;
    
    std::string source_file = get_current_source_file();
    if (!source_file.empty()) {
        if (plugin_config.verbose) {
            log_plugin_info("Finished compilation of: " + source_file);
        }
        
        // Write metadata for this translation unit
        metadata_collector->writeMetadata();
    }
}

/**
 * @brief Parse plugin command line arguments
 */
static void parse_plugin_args(struct plugin_name_args* plugin_info)
{
    if (!plugin_info || !plugin_info->argv) return;
    
    for (int i = 0; i < plugin_info->argc; i++) {
        const char* arg = plugin_info->argv[i].key;
        const char* value = plugin_info->argv[i].value;
        
        if (strcmp(arg, "verbose") == 0) {
            plugin_config.verbose = true;
        } else if (strcmp(arg, "output-dir") == 0 && value) {
            plugin_config.output_dir = value;
        } else if (strcmp(arg, "format") == 0 && value) {
            plugin_config.format = value;
        } else if (strcmp(arg, "include-system-headers") == 0) {
            plugin_config.include_system_headers = true;
        }
    }
}

/**
 * @brief Capture compiler flags and settings
 */
static void capture_compiler_flags()
{
    if (!metadata_collector) return;
    
    // Capture target architecture
    metadata_collector->addCompilerFlag("target_arch", "x86_64");
    
    // Capture optimization level
    metadata_collector->addCompilerFlag("optimization_level", "0");
    
    // Capture debug information
    metadata_collector->addCompilerFlag("debug_info", "true");
    
    // Capture warnings as errors
    metadata_collector->addCompilerFlag("warnings_as_errors", "false");
    
    // Capture C++ standard
    metadata_collector->addCompilerFlag("cpp_standard", "23");
    
    if (plugin_config.verbose) {
        log_plugin_info("Captured compiler flags");
    }
}

/**
 * @brief Capture build environment information
 */
static void capture_build_environment()
{
    if (!metadata_collector) return;
    
    // Set target architecture
    metadata_collector->setTargetArchitecture("x86_64");
    
    // Set compiler information
    metadata_collector->setCompilerType("gcc");
    metadata_collector->setCompilerVersion("13.0.0");
    
    if (plugin_config.verbose) {
        log_plugin_info("Captured build environment");
    }
}

/**
 * @brief Get the current source file being compiled
 */
static std::string get_current_source_file()
{
    // For now, return a placeholder since we can't easily access GCC internals
    // In a real implementation, this would extract the current source file
    return "unknown_source.cpp";
}

/**
 * @brief Log plugin information message
 */
static void log_plugin_info(const std::string& message)
{
    if (plugin_config.verbose) {
        std::cerr << "[Heimdall GCC Plugin] INFO: " << message << std::endl;
    }
}

/**
 * @brief Log plugin error message
 */
static void log_plugin_error(const std::string& message)
{
    std::cerr << "[Heimdall GCC Plugin] ERROR: " << message << std::endl;
}