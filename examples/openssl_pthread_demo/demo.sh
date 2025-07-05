#!/bin/bash

set -e

echo "🔧 Heimdall OpenSSL + Pthreads Demo"
echo "===================================="
echo "This demo shows how OpenSSL and pthreads dependencies appear in the SBOM."
echo ""

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

# Check if we're in the right directory
if [ ! -f "main.c" ]; then
    print_error "Please run this script from the openssl_pthread_demo directory"
    exit 1
fi

# Create build directory
print_status "Creating build directory..."
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

# Generate SBOM using the plugin
print_status "Generating SBOM for the executable..."

# Get the path to the Heimdall plugin
HEIMDALL_PLUGIN=""
print_status "Looking for Heimdall plugin..."
if [ -f "../../build/heimdall-lld.dylib" ]; then
    HEIMDALL_PLUGIN="../../build/heimdall-lld.dylib"
    print_status "Found plugin: $HEIMDALL_PLUGIN"
elif [ -f "../../build/heimdall-lld.so" ]; then
    HEIMDALL_PLUGIN="../../build/heimdall-lld.so"
    print_status "Found plugin: $HEIMDALL_PLUGIN"
else
    print_error "Heimdall plugin not found. Please build the main project first."
    print_status "Checked paths:"
    print_status "  - ../../build/heimdall-lld.dylib"
    print_status "  - ../../build/heimdall-lld.so"
    exit 1
fi

# Get absolute path to the demo binary
ABS_BIN_PATH="$(cd build && pwd)/openssl_pthread_demo"

# Create a simple test program to load the plugin and generate SBOM
cat > generate_sbom.c << EOF
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int main() {
    void* handle = dlopen("./heimdall-lld.dylib", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("./heimdall-lld.so", RTLD_LAZY);
    }
    
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
    if (set_output_path("./openssl_pthread_demo.spdx") != 0) {
        fprintf(stderr, "Failed to set output path\n");
        dlclose(handle);
        return 1;
    }
    if (process_input_file("$ABS_BIN_PATH") != 0) {
        fprintf(stderr, "Failed to process file\n");
        dlclose(handle);
        return 1;
    }
    
    // Generate the SBOM
    finalize();
    printf("SBOM generated successfully: openssl_pthread_demo.spdx\n");
    dlclose(handle);
    return 0;
}
EOF

# Compile the SBOM generator
gcc -o generate_sbom generate_sbom.c -ldl

# Copy the plugin to current directory
cp "$HEIMDALL_PLUGIN" .

# Generate SBOM
print_status "Generating SPDX SBOM..."
./generate_sbom

if [ $? -eq 0 ]; then
    print_success "SPDX SBOM generated successfully!"
    
    # Show a preview of the SBOM
    print_status "SBOM Preview (first 50 lines):"
    echo "======================================"
    head -50 openssl_pthread_demo.spdx
    echo ""
    
    # Count components
    COMPONENT_COUNT=$(grep -c "^SPDXID:" openssl_pthread_demo.spdx 2>/dev/null || echo "0")
    print_status "Found $COMPONENT_COUNT components in the SBOM"
    
    # Show OpenSSL-related components
    print_status "OpenSSL-related components:"
    grep -i "openssl\|ssl\|crypto" openssl_pthread_demo.spdx | head -10
    
    # Show pthread-related components
    print_status "Pthread-related components:"
    grep -i "pthread\|thread" openssl_pthread_demo.spdx | head -10
    
else
    print_error "Failed to generate SBOM"
    exit 1
fi

# Generate CycloneDX SBOM as well
print_status "Generating CycloneDX SBOM..."
cat > generate_cyclonedx_sbom.c << EOF
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int main() {
    void* handle = dlopen("./heimdall-lld.dylib", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("./heimdall-lld.so", RTLD_LAZY);
    }
    
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
    
    // Initialize the plugin
    if (onload(NULL) != 0) {
        fprintf(stderr, "Failed to initialize plugin\n");
        dlclose(handle);
        return 1;
    }
    
    if (set_format("cyclonedx") != 0) {
        fprintf(stderr, "Failed to set output format\n");
        dlclose(handle);
        return 1;
    }
    if (set_output_path("./openssl_pthread_demo.cyclonedx.json") != 0) {
        fprintf(stderr, "Failed to set output path\n");
        dlclose(handle);
        return 1;
    }
    if (process_input_file("$ABS_BIN_PATH") != 0) {
        fprintf(stderr, "Failed to process file\n");
        dlclose(handle);
        return 1;
    }
    
    // Generate the SBOM
    finalize();
    printf("CycloneDX SBOM generated successfully: openssl_pthread_demo.cyclonedx.json\n");
    dlclose(handle);
    return 0;
}
EOF

# Compile and run CycloneDX generator
gcc -o generate_cyclonedx_sbom generate_cyclonedx_sbom.c -ldl
./generate_cyclonedx_sbom

if [ $? -eq 0 ]; then
    print_success "CycloneDX SBOM generated successfully!"
    
    # Show a preview of the CycloneDX SBOM
    print_status "CycloneDX SBOM Preview (first 30 lines):"
    echo "==============================================="
    head -30 openssl_pthread_demo.cyclonedx.json
    echo ""
    
    # Count components in CycloneDX
    COMPONENT_COUNT=$(grep -c '"name"' openssl_pthread_demo.cyclonedx.json 2>/dev/null || echo "0")
    print_status "Found $COMPONENT_COUNT components in the CycloneDX SBOM"
    
else
    print_error "Failed to generate CycloneDX SBOM"
fi

echo ""
print_success "Demo completed successfully!"
echo ""
echo "📋 Generated files:"
echo "   - openssl_pthread_demo.spdx (SPDX format)"
echo "   - openssl_pthread_demo.cyclonedx.json (CycloneDX format)"
echo ""
echo "🔍 These SBOMs document all the dependencies including:"
echo "   - OpenSSL libraries and their versions"
echo "   - Pthread libraries"
echo "   - System libraries (libc, libm, etc.)"
echo "   - Dynamic dependencies and their relationships"
echo ""
echo "💡 You can examine the full SBOM files to see detailed dependency information." 