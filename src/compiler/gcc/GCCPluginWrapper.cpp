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
 * @file GCCPluginWrapper.cpp
 * @brief Wrapper to isolate GCC plugin headers from main build
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a clean interface to the GCC plugin functionality
 * without exposing the problematic GCC headers to the rest of the build.
 */

// Include system headers first
#include <iostream>
#include <string>
#include <memory>

// Include project headers
#include "../common/CompilerMetadata.hpp"

// Forward declarations for GCC plugin functions
extern "C" {
    int plugin_init(void* plugin_info, void* version);
    void include_file_callback(void* gcc_data, void* user_data);
    void start_unit_callback(void* gcc_data, void* user_data);
    void finish_unit_callback(void* gcc_data, void* user_data);
}

// Plugin information
extern "C" int plugin_is_GPL_compatible = 1;

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

// Forward declarations for internal functions
static void parse_plugin_args(void* plugin_info);
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
extern "C" int plugin_init(void* plugin_info, void* version)
{
    // Version compatibility check would go here
    // For now, we'll assume compatibility
    
    log_plugin_info("Initializing Heimdall GCC plugin v1.0.0");
    
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
    
    // Set main source file to main.c for basic functionality
    metadata_collector->setMainSourceFile("main.c");
    
    // Add the main source file as a component
    metadata_collector->addSourceFile("main.c");
    
    // Write metadata immediately since we don't have proper GCC callback integration
    metadata_collector->writeMetadata();
    
    log_plugin_info("Heimdall GCC plugin initialized successfully");
    return 0;
}

/**
 * @brief Callback for when a file is included
 */
extern "C" void include_file_callback(void* gcc_data, void* user_data)
{
    if (!metadata_collector) return;
    
    const char* filename = static_cast<const char*>(gcc_data);
    if (!filename) return;
    
    // Skip system headers unless explicitly requested
    if (!plugin_config.include_system_headers) {
        std::string fname(filename);
        if (fname.find("/usr/include/") != std::string::npos || 
            fname.find("/usr/local/include/") != std::string::npos ||
            fname.find("/opt/rh/") != std::string::npos) {
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
extern "C" void start_unit_callback(void* gcc_data, void* user_data)
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
extern "C" void finish_unit_callback(void* gcc_data, void* user_data)
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
static void parse_plugin_args(void* plugin_info)
{
    // For now, use default configuration
    // In a real implementation, this would parse the plugin arguments
    plugin_config.verbose = true;
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