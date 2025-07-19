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
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);
typedef int (*set_spdx_version_func_t)(const char*);

int main() {
    void* handle = dlopen("../../build/heimdall-lld.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin: %s\n", dlerror());
        return 1;
    }
    
    init_func_t onload = (init_func_t)dlsym(handle, "onload");
    set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
    process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
    finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");
    set_spdx_version_func_t set_spdx_version = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
    
    if (!onload || !set_format || !set_output_path || !process_input_file || !finalize || !set_spdx_version) {
        fprintf(stderr, "Failed to get function symbols: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    
    // Initialize the plugin
    if (onload(NULL) != 0) {
        fprintf(stderr, "Failed to initialize plugin\n");
        dlclose(handle);
        return 1;
    }
    
    if (set_format("spdx") != 0) {
        fprintf(stderr, "Failed to set output format\n");
        dlclose(handle);
        return 1;
    }
    // Set SPDX version to 3.0 (or change as needed)
    if (set_spdx_version("3.0") != 0) {
        fprintf(stderr, "Failed to set SPDX version\n");
        dlclose(handle);
        return 1;
    }
    if (set_output_path("openssl_pthread_demo_lld.spdx") != 0) {
        fprintf(stderr, "Failed to set output path\n");
        dlclose(handle);
        return 1;
    }
    if (process_input_file("build/openssl_pthread_demo") != 0) {
        fprintf(stderr, "Failed to process file\n");
        dlclose(handle);
        return 1;
    }
    
    // Generate the SBOM
    finalize();
    printf("spdx SBOM generated successfully: openssl_pthread_demo_lld.spdx\n");
    dlclose(handle);
    return 0;
}
