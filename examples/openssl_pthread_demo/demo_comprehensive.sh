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

#!/bin/bash

set -e

echo "🔧 Heimdall Comprehensive Demo: LLD + Gold + SPDX + CycloneDX"
echo "============================================================="
echo "This demo shows how to use both LLD and Gold plugins with both SPDX and CycloneDX formats."
echo "It demonstrates compilation with both GCC and LLVM/Clang."
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
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

print_header() {
    echo -e "${PURPLE}[HEADER]${NC} $1"
}

print_subheader() {
    echo -e "${CYAN}[SUBHEADER]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "main.c" ]; then
    print_error "Please run this script from the openssl_pthread_demo directory"
    exit 1
fi

# Check for required tools
check_tool() {
    if ! command -v $1 &> /dev/null; then
        print_error "$1 is not installed or not in PATH"
        return 1
    fi
    print_success "Found $1: $(command -v $1)"
    return 0
}

print_header "Checking required tools..."
check_tool "gcc" || exit 1
check_tool "clang" || print_warning "clang not found, will skip LLVM/Clang builds"
check_tool "ld.lld" || print_warning "ld.lld not found, will skip LLD linking"
check_tool "ld.gold" || print_warning "ld.gold not found, will skip Gold linking"
check_tool "cmake" || exit 1
check_tool "make" || exit 1

# Get the path to the Heimdall plugins
HEIMDALL_LLD_PLUGIN=""
HEIMDALL_GOLD_PLUGIN=""

print_header "Looking for Heimdall plugins..."
if [ -f "../../build/heimdall-lld.so" ]; then
    HEIMDALL_LLD_PLUGIN="../../build/heimdall-lld.so"
    print_success "Found LLD plugin: $HEIMDALL_LLD_PLUGIN"
elif [ -f "../../build/heimdall-lld.dylib" ]; then
    HEIMDALL_LLD_PLUGIN="../../build/heimdall-lld.dylib"
    print_success "Found LLD plugin: $HEIMDALL_LLD_PLUGIN"
else
    print_error "Heimdall LLD plugin not found"
    exit 1
fi

if [ -f "../../build/heimdall-gold.so" ]; then
    HEIMDALL_GOLD_PLUGIN="../../build/heimdall-gold.so"
    print_success "Found Gold plugin: $HEIMDALL_GOLD_PLUGIN"
elif [ -f "../../build/heimdall-gold.dylib" ]; then
    HEIMDALL_GOLD_PLUGIN="../../build/heimdall-gold.dylib"
    print_success "Found Gold plugin: $HEIMDALL_GOLD_PLUGIN"
else
    print_error "Heimdall Gold plugin not found"
    exit 1
fi

# Create build directories
print_header "Creating build directories..."
mkdir -p build_gcc_lld
mkdir -p build_gcc_gold
mkdir -p build_clang_lld
mkdir -p build_clang_gold

# Function to build with specific compiler and linker
build_with_compiler_linker() {
    local compiler=$1
    local linker=$2
    local build_dir=$3
    local binary_name=$4
    
    print_subheader "Building with $compiler + $linker in $build_dir"
    
    cd $build_dir
    
    # Configure with CMake
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_C_COMPILER=$compiler \
             -DCMAKE_CXX_COMPILER=${compiler/clang/clang++} \
             -DCMAKE_C_COMPILER_LAUNCHER="" \
             -DCMAKE_CXX_COMPILER_LAUNCHER="" \
             -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=$linker"
    
    # Build the project
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Rename the binary
    if [ -f "openssl_pthread_demo" ]; then
        mv openssl_pthread_demo $binary_name
        print_success "Built $binary_name with $compiler + $linker"
    else
        print_error "Build failed for $compiler + $linker"
        return 1
    fi
    
    # Test the binary
    print_status "Testing $binary_name..."
    ./$binary_name > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        print_success "$binary_name runs successfully"
    else
        print_error "$binary_name failed to run"
        return 1
    fi
    
    cd ..
}

# Function to generate SBOM with specific plugin and format
# Now runs each generation in a separate process for serialization
run_sbom_generator() {
    local plugin_path=$1
    local format=$2
    local output_file=$3
    local binary_path=$4
    
    cat > run_sbom_generator.c << EOF
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int main() {
    void* handle = dlopen("$plugin_path", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin: %s\n", dlerror());
        return 1;
    }
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
    if (onload(NULL) != 0) {
        fprintf(stderr, "Failed to initialize plugin\n");
        dlclose(handle);
        return 1;
    }
    if (set_format("$format") != 0) {
        fprintf(stderr, "Failed to set output format\n");
        dlclose(handle);
        return 1;
    }
    if (set_output_path("$output_file") != 0) {
        fprintf(stderr, "Failed to set output path\n");
        dlclose(handle);
        return 1;
    }
    if (process_input_file("$binary_path") != 0) {
        fprintf(stderr, "Failed to process file\n");
        dlclose(handle);
        return 1;
    }
    finalize();
    dlclose(handle);
    return 0;
}
EOF
    gcc -o run_sbom_generator run_sbom_generator.c -ldl
    ./run_sbom_generator
    local result=$?
    rm -f run_sbom_generator run_sbom_generator.c
    return $result
}

generate_sbom() {
    local plugin_path=$1
    local format=$2
    local binary_path=$3
    local output_file=$4
    print_subheader "Generating $format SBOM for $binary_path using $plugin_path"
    run_sbom_generator "$plugin_path" "$format" "$output_file" "$binary_path"
    if [ $? -eq 0 ]; then
        print_success "$format SBOM generated: $output_file"
        if [ "$format" = "spdx" ]; then
            COMPONENT_COUNT=$(grep -c "^SPDXID:" $output_file 2>/dev/null || echo "0")
        else
            COMPONENT_COUNT=$(grep -c '"name"' $output_file 2>/dev/null || echo "0")
        fi
        print_status "Found $COMPONENT_COUNT components in the $format SBOM"
        print_status "OpenSSL-related components:"
        grep -i "openssl\|ssl\|crypto" $output_file | head -5
    else
        print_error "Failed to generate $format SBOM"
        return 1
    fi
}

# Build with different compiler/linker combinations
print_header "Building with different compiler/linker combinations..."

# GCC + LLD
if command -v gcc &> /dev/null && command -v ld.lld &> /dev/null; then
    build_with_compiler_linker "gcc" "lld" "build_gcc_lld" "openssl_pthread_demo_gcc_lld"
else
    print_warning "Skipping GCC + LLD build (missing tools)"
fi

# GCC + Gold
if command -v gcc &> /dev/null && command -v ld.gold &> /dev/null; then
    build_with_compiler_linker "gcc" "gold" "build_gcc_gold" "openssl_pthread_demo_gcc_gold"
else
    print_warning "Skipping GCC + Gold build (missing tools)"
fi

# Clang + LLD
if command -v clang &> /dev/null && command -v ld.lld &> /dev/null; then
    build_with_compiler_linker "clang" "lld" "build_clang_lld" "openssl_pthread_demo_clang_lld"
else
    print_warning "Skipping Clang + LLD build (missing tools)"
fi

# Clang + Gold
if command -v clang &> /dev/null && command -v ld.gold &> /dev/null; then
    build_with_compiler_linker "clang" "gold" "build_clang_gold" "openssl_pthread_demo_clang_gold"
else
    print_warning "Skipping Clang + Gold build (missing tools)"
fi

# Generate SBOMs for each binary
print_header "Generating SBOMs for each binary..."

# Function to generate all SBOMs for a binary
generate_all_sboms_for_binary() {
    local binary_dir=$1
    local binary_name=$2
    local binary_path="$binary_dir/$binary_name"
    
    if [ ! -f "$binary_path" ]; then
        print_warning "Binary $binary_path not found, skipping"
        return
    fi
    
    print_subheader "Generating SBOMs for $binary_name"
    
    # Generate SPDX with LLD plugin
    generate_sbom "$HEIMDALL_LLD_PLUGIN" "spdx" "$binary_path" "${binary_name}_lld.spdx"
    
    # Generate CycloneDX with LLD plugin
    generate_sbom "$HEIMDALL_LLD_PLUGIN" "cyclonedx" "$binary_path" "${binary_name}_lld.cyclonedx.json"
    
    # Generate SPDX with Gold plugin
    generate_sbom "$HEIMDALL_GOLD_PLUGIN" "spdx" "$binary_path" "${binary_name}_gold.spdx"
    
    # Generate CycloneDX with Gold plugin
    generate_sbom "$HEIMDALL_GOLD_PLUGIN" "cyclonedx" "$binary_path" "${binary_name}_gold.cyclonedx.json"
}

# Generate SBOMs for each binary
generate_all_sboms_for_binary "build_gcc_lld" "openssl_pthread_demo_gcc_lld"
generate_all_sboms_for_binary "build_gcc_gold" "openssl_pthread_demo_gcc_gold"
generate_all_sboms_for_binary "build_clang_lld" "openssl_pthread_demo_clang_lld"
generate_all_sboms_for_binary "build_clang_gold" "openssl_pthread_demo_clang_gold"

# Summary
print_header "Demo Summary"
echo "================"

echo ""
print_success "Demo completed successfully!"
echo ""
echo "📋 Generated binaries:"
ls -la build_*/openssl_pthread_demo_* 2>/dev/null || echo "   No binaries found"

echo ""
echo "📋 Generated SBOM files:"
ls -la *.spdx *.cyclonedx.json 2>/dev/null || echo "   No SBOM files found"

echo ""
echo "🔍 SBOM Comparison:"
echo "   Each binary has been analyzed with both LLD and Gold plugins"
echo "   Each analysis generated both SPDX and CycloneDX formats"
echo "   This allows comparison of:"
echo "     - Different linkers (LLD vs Gold)"
echo "     - Different formats (SPDX vs CycloneDX)"
echo "     - Different compilers (GCC vs Clang)"
echo ""
echo "💡 You can compare the SBOM files to see how different tools and formats"
echo "   represent the same dependencies differently."
echo ""
echo "📊 Files generated per binary:"
echo "   - Binary executable"
echo "   - SPDX SBOM (LLD plugin)"
echo "   - CycloneDX SBOM (LLD plugin)"
echo "   - SPDX SBOM (Gold plugin)"
echo "   - CycloneDX SBOM (Gold plugin)"
echo ""
echo "🎯 Total: Up to 4 binaries × 4 SBOMs = 16 SBOM files" 