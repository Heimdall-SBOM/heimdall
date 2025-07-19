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
 * @file sbom_loader.cpp
 * @brief Dynamic SBOM generator loader for Heimdall plugins
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides a minimal C++ SBOM generator that dynamically loads
 * Heimdall plugins and generates SBOMs from binary files. It supports:
 * 
 * - Dynamic loading of LLD and Gold linker plugins
 * - SBOM generation in SPDX and CycloneDX formats
 * - Configurable output formats and versions
 * - Command-line interface for batch processing
 * 
 * The loader uses dynamic linking (dlopen/dlsym) to load plugin functions
 * and provides a simple interface for SBOM generation without requiring
 * the full Heimdall library to be linked.
 * 
 * Supported Formats:
 * - SPDX 2.3, 3.0, 3.0.0, 3.0.1
 * - CycloneDX 1.4, 1.5, 1.6
 */

// Minimal C++ SBOM generator for Heimdall plugins (heimdall-sbom)
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

// LLVM symbols are now provided by llvm_symbols shared library

/**
 * @brief Function pointer types for plugin interface
 */
typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_cyclonedx_version_func_t)(const char*);
typedef int (*set_spdx_version_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

/**
 * @brief Generate SBOM from binary file using a dynamic plugin
 * 
 * This function dynamically loads a Heimdall plugin and uses it to generate
 * an SBOM from a binary file. It handles plugin initialization, configuration,
 * processing, and cleanup.
 * 
 * @param plugin_path Path to the plugin shared library
 * @param binary_path Path to the binary file to analyze
 * @param format SBOM format ("spdx", "cyclonedx", etc.)
 * @param output_path Path for the output SBOM file
 * @param cyclonedx_version CycloneDX specification version
 * @param spdx_version SPDX specification version
 * @return 0 on success, 1 on failure
 */
int generate_sbom(const char* plugin_path, const char* binary_path, const char* format,
                  const char* output_path, const char* cyclonedx_version, const char* spdx_version) {
    // Load the plugin shared library
    void* handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load plugin " << plugin_path << ": " << dlerror() << std::endl;
        return 1;
    }

    // Get function pointers from the plugin
    init_func_t onload = (init_func_t)dlsym(handle, "onload");
    set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    set_cyclonedx_version_func_t set_cyclonedx_version = (set_cyclonedx_version_func_t)dlsym(handle, "heimdall_set_cyclonedx_version");
    set_spdx_version_func_t set_spdx_version = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
    set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
    process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
    finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");

    // Check that all required functions are available
    if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
        std::cerr << "Failed to get function symbols: " << dlerror() << std::endl;
        dlclose(handle);
        return 1;
    }

    // Initialize the plugin
    if (onload(nullptr) != 0) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        dlclose(handle);
        return 1;
    }

    // Set the output format
    if (set_format(format) != 0) {
        std::cerr << "Failed to set format" << std::endl;
        dlclose(handle);
        return 1;
    }

    // Handle CycloneDX version configuration
    if (strncmp(format, "cyclonedx", 9) == 0 && set_cyclonedx_version) {
        if (set_cyclonedx_version(cyclonedx_version) != 0) {
            std::cerr << "Failed to set CycloneDX version" << std::endl;
            dlclose(handle);
            return 1;
        }
    }
    
    // Handle SPDX version configuration
    if (strncmp(format, "spdx", 4) == 0 && set_spdx_version) {
        if (set_spdx_version(spdx_version) != 0) {
            std::cerr << "Failed to set SPDX version" << std::endl;
            dlclose(handle);
            return 1;
        }
    }
    
    // Set output path
    if (set_output_path(output_path) != 0) {
        std::cerr << "Failed to set output path" << std::endl;
        dlclose(handle);
        return 1;
    }
    
    // Process the binary file
    if (process_input_file(binary_path) != 0) {
        std::cerr << "Failed to process binary" << std::endl;
        dlclose(handle);
        return 1;
    }
    
    // Finalize and generate the SBOM
    finalize();
    dlclose(handle);
    return 0;
}

/**
 * @brief Main function for the heimdall-sbom tool
 * 
 * Parses command-line arguments and calls generate_sbom() to create
 * an SBOM from a binary file using a dynamic plugin.
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: heimdall-sbom <plugin_path> <binary_path> --format <format> --output <output_path> [--cyclonedx-version <version>] [--spdx-version <version>]" << std::endl;
        std::cerr << "  Supported formats: spdx, spdx-2.3, spdx-3.0, spdx-3.0.0, spdx-3.0.1, cyclonedx, cyclonedx-1.4, cyclonedx-1.6" << std::endl;
        std::cerr << "  Default versions: cyclonedx-1.6, spdx-2.3" << std::endl;
        return 1;
    }
    
    const char* plugin_path = argv[1];
    const char* binary_path = argv[2];
    const char* format = "spdx";
    const char* output_path = "sbom.json";
    const char* cyclonedx_version = "1.6";
    const char* spdx_version = "2.3";
    
    // Parse command line arguments
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            format = argv[++i];
            // Extract version from format string for SPDX
            if (strncmp(format, "spdx-", 5) == 0) {
                spdx_version = format + 5; // Skip "spdx-" prefix
            }
            // Extract version from format string for CycloneDX
            if (strncmp(format, "cyclonedx-", 10) == 0) {
                cyclonedx_version = format + 10; // Skip "cyclonedx-" prefix
            }
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else if (strcmp(argv[i], "--cyclonedx-version") == 0 && i + 1 < argc) {
            cyclonedx_version = argv[++i];
        } else if (strcmp(argv[i], "--spdx-version") == 0 && i + 1 < argc) {
            spdx_version = argv[++i];
        }
    }
    
    return generate_sbom(plugin_path, binary_path, format, output_path, cyclonedx_version, spdx_version);
} 