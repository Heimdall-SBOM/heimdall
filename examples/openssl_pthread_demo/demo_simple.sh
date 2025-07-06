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

echo "🔧 Heimdall Simple Demo: LLD + Gold + SPDX + CycloneDX"
echo "======================================================"
echo "This demo shows how to use both LLD and Gold plugins with both SPDX and CycloneDX formats."
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
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

# Check if we're in the right directory
if [ ! -f "main.c" ]; then
    print_error "Please run this script from the openssl_pthread_demo directory"
    exit 1
fi

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

# Build the demo using CMake
print_header "Building the demo..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
print_status "Building the OpenSSL pthread demo..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

print_success "Build completed successfully!"

# Run the program
print_status "Running the OpenSSL pthread demo..."
echo ""
./openssl_pthread_demo
echo ""

# Check if the program ran successfully
if [ $? -eq 0 ]; then
    print_success "Program executed successfully!"
else
    print_error "Program execution failed!"
    exit 1
fi

# Go back to the script directory for SBOM generation
cd ..

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
    local output_file=$3
    print_header "Generating $format SBOM using $plugin_path"
    run_sbom_generator "$plugin_path" "$format" "$output_file" "build/openssl_pthread_demo"
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

# Generate SBOMs with both plugins and both formats
print_header "Generating SBOMs with different plugins and formats..."

# Generate SPDX with LLD plugin
generate_sbom "$HEIMDALL_LLD_PLUGIN" "spdx" "openssl_pthread_demo_lld.spdx"

# Generate CycloneDX with LLD plugin
generate_sbom "$HEIMDALL_LLD_PLUGIN" "cyclonedx" "openssl_pthread_demo_lld.cyclonedx.json"

# Generate SPDX with Gold plugin
generate_sbom "$HEIMDALL_GOLD_PLUGIN" "spdx" "openssl_pthread_demo_gold.spdx"

# Generate CycloneDX with Gold plugin
generate_sbom "$HEIMDALL_GOLD_PLUGIN" "cyclonedx" "openssl_pthread_demo_gold.cyclonedx.json"

# Summary
print_header "Demo Summary"
echo "================"

echo ""
print_success "Demo completed successfully!"
echo ""
echo "📋 Generated SBOM files:"
ls -la *.spdx *.cyclonedx.json 2>/dev/null || echo "   No SBOM files found"

echo ""
echo "🔍 SBOM Comparison:"
echo "   The same binary has been analyzed with both LLD and Gold plugins"
echo "   Each analysis generated both SPDX and CycloneDX formats"
echo "   This allows comparison of:"
echo "     - Different linkers (LLD vs Gold)"
echo "     - Different formats (SPDX vs CycloneDX)"
echo ""
echo "💡 You can compare the SBOM files to see how different tools and formats"
echo "   represent the same dependencies differently."
echo ""
echo "📊 Files generated:"
echo "   - openssl_pthread_demo_lld.spdx (SPDX format, LLD plugin)"
echo "   - openssl_pthread_demo_lld.cyclonedx.json (CycloneDX format, LLD plugin)"
echo "   - openssl_pthread_demo_gold.spdx (SPDX format, Gold plugin)"
echo "   - openssl_pthread_demo_gold.cyclonedx.json (CycloneDX format, Gold plugin)"
echo ""
echo "🎯 Total: 4 SBOM files for comprehensive comparison" 