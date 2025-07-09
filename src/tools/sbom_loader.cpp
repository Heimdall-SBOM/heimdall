// Minimal C++ SBOM loader for Heimdall plugins
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

// LLVM symbols are now provided by llvm_symbols shared library

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_cyclonedx_version_func_t)(const char*);
typedef int (*set_spdx_version_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int generate_sbom(const char* plugin_path, const char* binary_path, const char* format,
                  const char* output_path, const char* cyclonedx_version, const char* spdx_version) {
    void* handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load plugin " << plugin_path << ": " << dlerror() << std::endl;
        return 1;
    }

    init_func_t onload = (init_func_t)dlsym(handle, "onload");
    set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    set_cyclonedx_version_func_t set_cyclonedx_version = (set_cyclonedx_version_func_t)dlsym(handle, "heimdall_set_cyclonedx_version");
    set_spdx_version_func_t set_spdx_version = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
    set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
    process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
    finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");

    if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
        std::cerr << "Failed to get function symbols: " << dlerror() << std::endl;
        dlclose(handle);
        return 1;
    }

    if (onload(nullptr) != 0) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        dlclose(handle);
        return 1;
    }

    if (set_format(format) != 0) {
        std::cerr << "Failed to set format" << std::endl;
        dlclose(handle);
        return 1;
    }

    // Handle CycloneDX versions
    if (strncmp(format, "cyclonedx", 9) == 0 && set_cyclonedx_version) {
        if (set_cyclonedx_version(cyclonedx_version) != 0) {
            std::cerr << "Failed to set CycloneDX version" << std::endl;
            dlclose(handle);
            return 1;
        }
    }
    
    // Handle SPDX versions
    if (strncmp(format, "spdx", 4) == 0 && set_spdx_version) {
        if (set_spdx_version(spdx_version) != 0) {
            std::cerr << "Failed to set SPDX version" << std::endl;
            dlclose(handle);
            return 1;
        }
    }
    
    if (set_output_path(output_path) != 0) {
        std::cerr << "Failed to set output path" << std::endl;
        dlclose(handle);
        return 1;
    }
    if (process_input_file(binary_path) != 0) {
        std::cerr << "Failed to process binary" << std::endl;
        dlclose(handle);
        return 1;
    }
    finalize();
    dlclose(handle);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <plugin_path> <binary_path> --format <format> --output <output_path> [--cyclonedx-version <version>] [--spdx-version <version>]" << std::endl;
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