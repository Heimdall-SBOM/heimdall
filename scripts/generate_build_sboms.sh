#!/bin/bash

# Copyright 2025 The Heimdall Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Heimdall Build SBOM Generator
# This script generates SBOMs using both LLD and Gold plugins after a successful build

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Configuration
BUILD_DIR="${1:-build}"
SBOM_DIR="${BUILD_DIR}/sboms"
TEST_BINARY="${BUILD_DIR}/examples/openssl_pthread_demo/openssl_pthread_demo"

# Create SBOM output directory
mkdir -p "${SBOM_DIR}"

print_status "Generating build SBOMs using Heimdall plugins"
print_status "Build directory: ${BUILD_DIR}"
print_status "SBOM output directory: ${SBOM_DIR}"
print_status "Test binary: ${TEST_BINARY}"

# Check if test binary exists
if [[ ! -f "${TEST_BINARY}" ]]; then
    print_error "Test binary not found: ${TEST_BINARY}"
    print_error "Please ensure the build completed successfully"
    exit 1
fi

# Check if plugins exist
LLD_PLUGIN="${BUILD_DIR}/heimdall-lld.so"
GOLD_PLUGIN="${BUILD_DIR}/heimdall-gold.so"

if [[ ! -f "${LLD_PLUGIN}" ]]; then
    print_warning "LLD plugin not found: ${LLD_PLUGIN}"
    LLD_AVAILABLE=false
else
    LLD_AVAILABLE=true
    print_success "Found LLD plugin: ${LLD_PLUGIN}"
fi

if [[ ! -f "${GOLD_PLUGIN}" ]]; then
    print_warning "Gold plugin not found: ${GOLD_PLUGIN}"
    GOLD_AVAILABLE=false
else
    GOLD_AVAILABLE=true
    print_success "Found Gold plugin: ${GOLD_PLUGIN}"
fi

# Create a simple C program to generate SBOMs using dlopen
create_sbom_generator() {
    cat > "${BUILD_DIR}/generate_sbom.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int generate_sbom(const char* plugin_path, const char* format, 
                  const char* output_path, const char* binary_path) {
    void* handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin %s: %s\n", plugin_path, dlerror());
        return 1;
    }

    // Get function pointers
    init_func_t onload = (init_func_t)dlsym(handle, "onload");
    set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
    process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
    finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");

    if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
        fprintf(stderr, "Failed to get function symbols: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    // Initialize plugin
    if (onload(NULL) != 0) {
        fprintf(stderr, "Failed to initialize plugin\n");
        dlclose(handle);
        return 1;
    }

    // Set format
    if (set_format(format) != 0) {
        fprintf(stderr, "Failed to set format\n");
        dlclose(handle);
        return 1;
    }

    // Set output path
    if (set_output_path(output_path) != 0) {
        fprintf(stderr, "Failed to set output path\n");
        dlclose(handle);
        return 1;
    }

    // Process binary
    if (process_input_file(binary_path) != 0) {
        fprintf(stderr, "Failed to process binary\n");
        dlclose(handle);
        return 1;
    }

    // Generate SBOM
    finalize();
    dlclose(handle);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <plugin_path> <format> <output_path> <binary_path>\n", argv[0]);
        return 1;
    }

    return generate_sbom(argv[1], argv[2], argv[3], argv[4]);
}
EOF
}

# Compile the SBOM generator
compile_sbom_generator() {
    print_status "Compiling SBOM generator..."
    gcc -o "${BUILD_DIR}/generate_sbom" "${BUILD_DIR}/generate_sbom.c" -ldl
    if [[ $? -eq 0 ]]; then
        print_success "SBOM generator compiled successfully"
    else
        print_error "Failed to compile SBOM generator"
        exit 1
    fi
}

# Function to generate SBOM with a plugin
generate_sbom_with_plugin() {
    local plugin_path="$1"
    local format="$2"
    local output_file="$3"
    local binary_path="$4"
    local plugin_name="$5"
    
    print_status "Generating ${plugin_name} ${format} SBOM: ${output_file}"
    
    # Use the compiled SBOM generator
    "${BUILD_DIR}/generate_sbom" "${plugin_path}" "${format}" "${output_file}" "${binary_path}"
    
    if [[ $? -eq 0 ]] && [[ -f "${output_file}" ]]; then
        print_success "Generated ${plugin_name} ${format} SBOM: ${output_file}"
        return 0
    else
        print_warning "${plugin_name} ${format} SBOM generation may have failed"
        return 1
    fi
}

# Create and compile the SBOM generator
create_sbom_generator
compile_sbom_generator

# Generate all SBOMs
print_status "Starting SBOM generation..."

# LLD Plugin SBOMs
if [[ "${LLD_AVAILABLE}" == "true" ]]; then
    generate_sbom_with_plugin "${LLD_PLUGIN}" "spdx" "${SBOM_DIR}/heimdall-build-lld.spdx" "${TEST_BINARY}" "LLD"
    generate_sbom_with_plugin "${LLD_PLUGIN}" "cyclonedx" "${SBOM_DIR}/heimdall-build-lld.cyclonedx.json" "${TEST_BINARY}" "LLD"
else
    print_warning "Skipping LLD SBOM generation (plugin not available)"
fi

# Gold Plugin SBOMs
if [[ "${GOLD_AVAILABLE}" == "true" ]]; then
    generate_sbom_with_plugin "${GOLD_PLUGIN}" "spdx" "${SBOM_DIR}/heimdall-build-gold.spdx" "${TEST_BINARY}" "Gold"
    generate_sbom_with_plugin "${GOLD_PLUGIN}" "cyclonedx" "${SBOM_DIR}/heimdall-build-gold.cyclonedx.json" "${TEST_BINARY}" "Gold"
else
    print_warning "Skipping Gold SBOM generation (plugin not available)"
fi

# Summary
print_status ""
print_status "=== SBOM Generation Summary ==="
print_status "SBOM files generated in: ${SBOM_DIR}"

if [[ -f "${SBOM_DIR}/heimdall-build-lld.spdx" ]]; then
    print_success "✓ LLD SPDX SBOM: ${SBOM_DIR}/heimdall-build-lld.spdx"
else
    print_warning "✗ LLD SPDX SBOM: Not generated"
fi

if [[ -f "${SBOM_DIR}/heimdall-build-lld.cyclonedx.json" ]]; then
    print_success "✓ LLD CycloneDX SBOM: ${SBOM_DIR}/heimdall-build-lld.cyclonedx.json"
else
    print_warning "✗ LLD CycloneDX SBOM: Not generated"
fi

if [[ -f "${SBOM_DIR}/heimdall-build-gold.spdx" ]]; then
    print_success "✓ Gold SPDX SBOM: ${SBOM_DIR}/heimdall-build-gold.spdx"
else
    print_warning "✗ Gold SPDX SBOM: Not generated"
fi

if [[ -f "${SBOM_DIR}/heimdall-build-gold.cyclonedx.json" ]]; then
    print_success "✓ Gold CycloneDX SBOM: ${SBOM_DIR}/heimdall-build-gold.cyclonedx.json"
else
    print_warning "✗ Gold CycloneDX SBOM: Not generated"
fi

print_status ""
print_success "SBOM generation completed!"
print_status "All SBOM files are preserved as build artifacts in: ${SBOM_DIR}" 