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

# Shared libraries to generate SBOMs for
SHARED_LIBS=(
    "${BUILD_DIR}/heimdall-gold.so"
    "${BUILD_DIR}/heimdall-lld.so"
    "${BUILD_DIR}/libheimdall-core.so.1.0.0"
)

# Create SBOM output directory
mkdir -p "${SBOM_DIR}"

print_status "Generating build SBOMs using Heimdall plugins"
print_status "Build directory: ${BUILD_DIR}"
print_status "SBOM output directory: ${SBOM_DIR}"
print_status "Test binary: ${TEST_BINARY}"
print_status "Shared libraries: ${#SHARED_LIBS[@]} libraries"

# Check if test binary exists
if [[ ! -f "${TEST_BINARY}" ]]; then
    print_error "Test binary not found: ${TEST_BINARY}"
    print_error "Please ensure the build completed successfully"
    exit 1
fi

# Check if shared libraries exist
for lib in "${SHARED_LIBS[@]}"; do
    if [[ ! -f "${lib}" ]]; then
        print_warning "Shared library not found: ${lib}"
    else
        print_success "Found shared library: ${lib}"
    fi
done

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
typedef int (*set_cyclonedx_version_func_t)(const char*);
typedef int (*set_spdx_version_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int generate_sbom(const char* plugin_path, const char* format, 
                  const char* output_path, const char* binary_path, const char* cyclonedx_version, const char* spdx_version) {
    void* handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin %s: %s\n", plugin_path, dlerror());
        return 1;
    }

    // Get function pointers
    init_func_t onload = (init_func_t)dlsym(handle, "onload");
    set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
    set_cyclonedx_version_func_t set_cyclonedx_version = (set_cyclonedx_version_func_t)dlsym(handle, "heimdall_set_cyclonedx_version");
    set_spdx_version_func_t set_spdx_version = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
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

    // Set CycloneDX version if format is cyclonedx and function is available
    if (strcmp(format, "cyclonedx") == 0 && set_cyclonedx_version) {
        if (set_cyclonedx_version(cyclonedx_version) != 0) {
            fprintf(stderr, "Failed to set CycloneDX version\n");
            dlclose(handle);
            return 1;
        }
    }

    // Set SPDX version if format is spdx and function is available
    if (strcmp(format, "spdx") == 0 && set_spdx_version) {
        if (set_spdx_version(spdx_version) != 0) {
            fprintf(stderr, "Failed to set SPDX version\n");
            dlclose(handle);
            return 1;
        }
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
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <plugin_path> <format> <output_path> <binary_path> <cyclonedx_version> <spdx_version>\n", argv[0]);
        return 1;
    }

    return generate_sbom(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
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
    local cyclonedx_version="${6:-1.6}"
    local spdx_version="${7:-3.0}"
    
    print_status "Generating ${plugin_name} ${format} SBOM: ${output_file}"
    
    # Use the compiled SBOM generator
    "${BUILD_DIR}/generate_sbom" "${plugin_path}" "${format}" "${output_file}" "${binary_path}" "${cyclonedx_version}" "${spdx_version}"
    
    if [[ $? -eq 0 ]] && [[ -f "${output_file}" ]]; then
        print_success "Generated ${plugin_name} ${format} SBOM: ${output_file}"
        return 0
    else
        print_warning "${plugin_name} ${format} SBOM generation may have failed"
        return 1
    fi
}

# Function to generate multiple CycloneDX versions
generate_cyclonedx_versions() {
    local plugin_path="$1"
    local output_base="$2"
    local binary_path="$3"
    local plugin_name="$4"
    
    # Generate CycloneDX 1.4, 1.5, and 1.6
    local versions=("1.4" "1.5" "1.6")
    
    for version in "${versions[@]}"; do
        local output_file="${output_base}-v${version}.cyclonedx.json"
        generate_sbom_with_plugin "${plugin_path}" "cyclonedx" "${output_file}" "${binary_path}" "${plugin_name}" "${version}"
    done
}

# Create and compile the SBOM generator
create_sbom_generator
compile_sbom_generator

# Generate all SBOMs
print_status "Starting SBOM generation..."

# LLD Plugin SBOMs
if [[ "${LLD_AVAILABLE}" == "true" ]]; then
    generate_sbom_with_plugin "${LLD_PLUGIN}" "spdx" "${SBOM_DIR}/heimdall-build-lld.spdx" "${TEST_BINARY}" "LLD"
    generate_cyclonedx_versions "${LLD_PLUGIN}" "${SBOM_DIR}/heimdall-build-lld" "${TEST_BINARY}" "LLD"
else
    print_warning "Skipping LLD SBOM generation (plugin not available)"
fi

# Gold Plugin SBOMs
if [[ "${GOLD_AVAILABLE}" == "true" ]]; then
    generate_sbom_with_plugin "${GOLD_PLUGIN}" "spdx" "${SBOM_DIR}/heimdall-build-gold.spdx" "${TEST_BINARY}" "Gold"
    generate_cyclonedx_versions "${GOLD_PLUGIN}" "${SBOM_DIR}/heimdall-build-gold" "${TEST_BINARY}" "Gold"
else
    print_warning "Skipping Gold SBOM generation (plugin not available)"
fi

# Generate SBOMs for shared libraries
print_status ""
print_status "Generating SBOMs for shared libraries..."

for lib in "${SHARED_LIBS[@]}"; do
    if [[ ! -f "${lib}" ]]; then
        print_warning "Skipping SBOM generation for missing library: ${lib}"
        continue
    fi
    
    # Extract library name for file naming
    lib_name=$(basename "${lib}" | sed 's/\.so.*$//')
    print_status "Processing shared library: ${lib_name}"
    
    # LLD Plugin SBOMs for shared library
    if [[ "${LLD_AVAILABLE}" == "true" ]]; then
        generate_sbom_with_plugin "${LLD_PLUGIN}" "spdx" "${SBOM_DIR}/${lib_name}-lld.spdx" "${lib}" "LLD"
        generate_cyclonedx_versions "${LLD_PLUGIN}" "${SBOM_DIR}/${lib_name}-lld" "${lib}" "LLD"
    else
        print_warning "Skipping LLD SBOM generation for ${lib_name} (plugin not available)"
    fi
    
    # Gold Plugin SBOMs for shared library
    if [[ "${GOLD_AVAILABLE}" == "true" ]]; then
        generate_sbom_with_plugin "${GOLD_PLUGIN}" "spdx" "${SBOM_DIR}/${lib_name}-gold.spdx" "${lib}" "Gold"
        generate_cyclonedx_versions "${GOLD_PLUGIN}" "${SBOM_DIR}/${lib_name}-gold" "${lib}" "Gold"
    else
        print_warning "Skipping Gold SBOM generation for ${lib_name} (plugin not available)"
    fi
done

# Summary
print_status ""
print_status "=== SBOM Generation Summary ==="
print_status "SBOM files generated in: ${SBOM_DIR}"

# Main binary SBOMs
if [[ -f "${SBOM_DIR}/heimdall-build-lld.spdx" ]]; then
    print_success "✓ LLD SPDX SBOM: ${SBOM_DIR}/heimdall-build-lld.spdx"
else
    print_warning "✗ LLD SPDX SBOM: Not generated"
fi

# Check for CycloneDX versions
for version in "1.4" "1.5" "1.6"; do
    if [[ -f "${SBOM_DIR}/heimdall-build-lld-v${version}.cyclonedx.json" ]]; then
        print_success "✓ LLD CycloneDX ${version} SBOM: ${SBOM_DIR}/heimdall-build-lld-v${version}.cyclonedx.json"
    else
        print_warning "✗ LLD CycloneDX ${version} SBOM: Not generated"
    fi
done

if [[ -f "${SBOM_DIR}/heimdall-build-gold.spdx" ]]; then
    print_success "✓ Gold SPDX SBOM: ${SBOM_DIR}/heimdall-build-gold.spdx"
else
    print_warning "✗ Gold SPDX SBOM: Not generated"
fi

# Check for CycloneDX versions
for version in "1.4" "1.5" "1.6"; do
    if [[ -f "${SBOM_DIR}/heimdall-build-gold-v${version}.cyclonedx.json" ]]; then
        print_success "✓ Gold CycloneDX ${version} SBOM: ${SBOM_DIR}/heimdall-build-gold-v${version}.cyclonedx.json"
    else
        print_warning "✗ Gold CycloneDX ${version} SBOM: Not generated"
    fi
done

# Shared library SBOMs
print_status ""
print_status "=== Shared Library SBOMs ==="

for lib in "${SHARED_LIBS[@]}"; do
    if [[ ! -f "${lib}" ]]; then
        continue
    fi
    
    lib_name=$(basename "${lib}" | sed 's/\.so.*$//')
    
    # LLD SBOMs for this library
    if [[ -f "${SBOM_DIR}/${lib_name}-lld.spdx" ]]; then
        print_success "✓ ${lib_name} LLD SPDX SBOM: ${SBOM_DIR}/${lib_name}-lld.spdx"
    else
        print_warning "✗ ${lib_name} LLD SPDX SBOM: Not generated"
    fi
    
    # Check for CycloneDX versions
    for version in "1.4" "1.5" "1.6"; do
        if [[ -f "${SBOM_DIR}/${lib_name}-lld-v${version}.cyclonedx.json" ]]; then
            print_success "✓ ${lib_name} LLD CycloneDX ${version} SBOM: ${SBOM_DIR}/${lib_name}-lld-v${version}.cyclonedx.json"
        else
            print_warning "✗ ${lib_name} LLD CycloneDX ${version} SBOM: Not generated"
        fi
    done
    
    # Gold SBOMs for this library
    if [[ -f "${SBOM_DIR}/${lib_name}-gold.spdx" ]]; then
        print_success "✓ ${lib_name} Gold SPDX SBOM: ${SBOM_DIR}/${lib_name}-gold.spdx"
    else
        print_warning "✗ ${lib_name} Gold SPDX SBOM: Not generated"
    fi
    
    # Check for CycloneDX versions
    for version in "1.4" "1.5" "1.6"; do
        if [[ -f "${SBOM_DIR}/${lib_name}-gold-v${version}.cyclonedx.json" ]]; then
            print_success "✓ ${lib_name} Gold CycloneDX ${version} SBOM: ${SBOM_DIR}/${lib_name}-gold-v${version}.cyclonedx.json"
        else
            print_warning "✗ ${lib_name} Gold CycloneDX ${version} SBOM: Not generated"
        fi
    done
done

print_status ""
print_success "SBOM generation completed!"
print_status "All SBOM files are preserved as build artifacts in: ${SBOM_DIR}" 