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

# Shared libraries to generate SBOMs for
SHARED_LIBS=(
    "${BUILD_DIR}/lib/heimdall-gold.so"
    "${BUILD_DIR}/lib/heimdall-lld.so"
    "${BUILD_DIR}/lib/libheimdall-core.so.1.0.0"
)

# Create SBOM output directory
mkdir -p "${SBOM_DIR}"

print_status "Generating build SBOMs using Heimdall plugins"
print_status "Build directory: ${BUILD_DIR}"
print_status "SBOM output directory: ${SBOM_DIR}"
print_status "Shared libraries: ${#SHARED_LIBS[@]} libraries"


# Check if shared libraries exist
for lib in "${SHARED_LIBS[@]}"; do
    if [[ ! -f "${lib}" ]]; then
        print_warning "Shared library not found: ${lib}"
    else
        print_success "Found shared library: ${lib}"
    fi
done

# Check if plugins exist
LLD_PLUGIN="${BUILD_DIR}/lib/heimdall-lld.so"
GOLD_PLUGIN="${BUILD_DIR}/lib/heimdall-gold.so"

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

# Check if the C++ SBOM loader is available
check_heimdall_sbom() {
    local loader_path="${BUILD_DIR}/src/tools/heimdall-sbom"
    if [[ ! -f "${loader_path}" ]]; then
        print_error "SBOM loader not found: ${loader_path}"
        print_error "Please ensure the build completed successfully"
        exit 1
    fi
    print_success "Found SBOM loader: ${loader_path}"
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
    
    # Build the command with correct argument order and flags
    local cmd=("${BUILD_DIR}/src/tools/heimdall-sbom" "${plugin_path}" "${binary_path}" --format "${format}" --output "${output_file}")
    if [[ "${format}" == cyclonedx* ]]; then
        cmd+=(--cyclonedx-version "${cyclonedx_version}")
    fi
    if [[ "${format}" == spdx* ]]; then
        cmd+=(--spdx-version "${spdx_version}")
    fi

    "${cmd[@]}"
    if [[ $? -eq 0 ]] && [[ -f "${output_file}" ]]; then
        print_success "Generated ${plugin_name} ${format} SBOM: ${output_file}"
        return 0
    else
        print_warning "${plugin_name} ${format} SBOM generation may have failed"
        return 1
    fi
}

# Function to generate multiple SPDX versions
generate_spdx_versions() {
    local plugin_path="$1"
    local output_base="$2"
    local binary_path="$3"
    local plugin_name="$4"
    
    # Generate SPDX 2.3, 3.0, 3.0.1 
    local versions=("2.3" "3.0" "3.0.1")
    
    for version in "${versions[@]}"; do
        local output_file="${output_base}-v${version}.spdx.json"
        # Always use --format spdx, version is set by --spdx-version
        generate_sbom_with_plugin "${plugin_path}" "spdx" "${output_file}" "${binary_path}" "${plugin_name}" "1.6" "${version}"
    done
}

# Check if the SBOM loader is available
check_heimdall_sbom

# Generate all SBOMs
print_status "Starting SBOM generation..."

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
        generate_spdx_versions "${LLD_PLUGIN}" "${SBOM_DIR}/${lib_name}-lld" "${lib}" "LLD"
    else
        print_warning "Skipping LLD SBOM generation for ${lib_name} (plugin not available)"
    fi
    
    # Gold Plugin SBOMs for shared library
    if [[ "${GOLD_AVAILABLE}" == "true" ]]; then
        generate_sbom_with_plugin "${GOLD_PLUGIN}" "spdx" "${SBOM_DIR}/${lib_name}-gold.spdx" "${lib}" "Gold"
        generate_spdx_versions "${GOLD_PLUGIN}" "${SBOM_DIR}/${lib_name}-gold" "${lib}" "Gold"
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

# Check for SPDX versions
for version in "2.3" "3.0" "3.0.1"; do
    if [[ -f "${SBOM_DIR}/heimdall-build-lld-v${version}.spdx.json" ]]; then
        print_success "✓ LLD SPDX ${version} SBOM: ${SBOM_DIR}/heimdall-build-lld-v${version}.spdx.json"
    else
        print_warning "✗ LLD SPDX ${version} SBOM: Not generated"
    fi
done

# Check for CycloneDX versions
for version in "2.3" "3.0" "3.0.1"; do
    if [[ -f "${SBOM_DIR}/heimdall-build-gold-v${version}.spdx.json" ]]; then
        print_success "✓ Gold SPDX ${version} SBOM: ${SBOM_DIR}/heimdall-build-gold-v${version}.spdx.json"
    else
        print_warning "✗ Gold SPDX ${version} SBOM: Not generated"
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
    
    # Check for SPDX versions
    for version in "2.3" "3.0" "3.0.1"; do
        if [[ -f "${SBOM_DIR}/${lib_name}-lld-v${version}.spdx.json" ]]; then
            print_success "✓ ${lib_name} LLD SPDX ${version} SBOM: ${SBOM_DIR}/${lib_name}-lld-v${version}.spdx.json"
        else
            print_warning "✗ ${lib_name} LLD SPDX ${version} SBOM: Not generated"
        fi
    done
    
    # Gold SBOMs for this library
    if [[ -f "${SBOM_DIR}/${lib_name}-gold.spdx" ]]; then
        print_success "✓ ${lib_name} Gold SPDX SBOM: ${SBOM_DIR}/${lib_name}-gold.spdx"
    else
        print_warning "✗ ${lib_name} Gold SPDX SBOM: Not generated"
    fi
    
    # Check for SPDX versions
    for version in "2.3" "3.0" "3.0.1"; do
        if [[ -f "${SBOM_DIR}/${lib_name}-gold-v${version}.spdx.json" ]]; then
            print_success "✓ ${lib_name} Gold SPDX ${version} SBOM: ${SBOM_DIR}/${lib_name}-gold-v${version}.spdx.json"
        else
            print_warning "✗ ${lib_name} Gold SPDX ${version} SBOM: Not generated"
        fi
    done
done

print_status ""
print_success "SBOM generation completed!"
print_status "All SBOM files are preserved as build artifacts in: ${SBOM_DIR}" 
