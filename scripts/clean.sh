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

# Heimdall Clean Script
# This script removes all build artifacts for a clean checkout

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

# Default directories to clean
BUILD_DIR="build"
INSTALL_DIR="install"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        --help|-h)
            echo "Heimdall Clean Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR      Set build directory (default: build)"
            echo "  --install-dir DIR    Set install directory (default: install)"
            echo "  --help, -h           Show this help message"
            echo ""
            echo "This script removes all build artifacts including:"
echo "  - Build directory and all contents"
echo "  - C++ standard-specific build directories:"
echo "    * Old naming: build-cpp11, build-cpp14, etc."
echo "    * New naming: build-gcc-cpp11, build-clang-cpp17, etc."
echo "  - Install directory and all contents"
echo "  - CMake cache files"
echo "  - Compiled binaries and libraries"
echo "  - Test outputs"
echo "  - Example build artifacts:"
echo "    * Old naming: examples/*/build/"
echo "    * New naming: examples/*/build-{compiler}-cpp{standard}/"
            echo ""
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_status "Cleaning Heimdall build artifacts..."

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Clean build directory
if [[ -d "$BUILD_DIR" ]]; then
    print_status "Removing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    print_success "Build directory removed"
else
    print_warning "Build directory not found: $BUILD_DIR"
fi

# Clean C++ standard-specific build directories (all naming conventions)
CXX_BUILD_DIRS=(
    "build_cpp11" "build_cpp14" "build_cpp17" "build_cpp20" "build_cpp23"  # Old naming
    "build-cpp11" "build-cpp14" "build-cpp17" "build-cpp20" "build-cpp23"  # New naming (without compiler)
    "build-gcc-cpp11" "build-gcc-cpp14" "build-gcc-cpp17" "build-gcc-cpp20" "build-gcc-cpp23"  # GCC naming
    "build-clang-cpp11" "build-clang-cpp14" "build-clang-cpp17" "build-clang-cpp20" "build-clang-cpp23"  # Clang naming
)
for dir in "${CXX_BUILD_DIRS[@]}"; do
    if [[ -d "$dir" ]]; then
        print_status "Removing C++ build directory: $dir"
        rm -rf "$dir"
        print_success "C++ build directory removed: $dir"
    fi
done

# Clean install directory
if [[ -d "$INSTALL_DIR" ]]; then
    print_status "Removing install directory: $INSTALL_DIR"
    rm -rf "$INSTALL_DIR"
    print_success "Install directory removed"
else
    print_warning "Install directory not found: $INSTALL_DIR"
fi

# Clean example build artifacts (generic, all subdirectories)
print_status "Cleaning all example build artifacts generically..."
for exdir in examples/*/; do
    if [[ -d "$exdir" ]]; then
        print_status "Cleaning $exdir..."
        
        # Clean old build directories (backward compatibility)
        rm -rf "$exdir/build" "$exdir/CMakeFiles" 2>/dev/null || true
        
        # Clean new build directories with naming convention
        for compiler in gcc clang; do
            for standard in 11 14 17 20 23; do
                build_dir="$exdir/build-${compiler}-cpp${standard}"
                if [[ -d "$build_dir" ]]; then
                    print_status "Removing example build directory: $build_dir"
                    rm -rf "$build_dir"
                fi
            done
        done
        
        rm -f "$exdir/CMakeCache.txt" 2>/dev/null || true
        # Don't remove Makefile for Ada demo
        if [[ "$exdir" != *"heimdall-ada-demo"* ]]; then
            rm -f "$exdir/Makefile" 2>/dev/null || true
        fi
        find "$exdir" -maxdepth 1 -type f \( -name "*.o" -o -name "*.so" -o -name "*.a" -o -name "*.dylib" -o -name "*.spdx" -o -name "*.cyclonedx.json" -o -name "*sbom*.json" -o -name "*.json" \) -delete 2>/dev/null || true
        # Remove executables (files with execute permission, not directories)
        # Don't remove build.sh for Ada demo
        if [[ "$exdir" != *"heimdall-ada-demo"* ]]; then
            find "$exdir" -maxdepth 1 -type f -executable -exec rm -f {} + 2>/dev/null || true
        else
            find "$exdir" -maxdepth 1 -type f -executable ! -name "build.sh" -exec rm -f {} + 2>/dev/null || true
        fi
        # Clean Ada-specific build artifacts
        rm -rf "$exdir/bin" "$exdir/lib" 2>/dev/null || true
        find "$exdir" -name "*.ali" -delete 2>/dev/null || true
        find "$exdir" -name "b~*.c" -delete 2>/dev/null || true
        find "$exdir" -name "b~*.o" -delete 2>/dev/null || true
        find "$exdir" -name "s~*.c" -delete 2>/dev/null || true
        find "$exdir" -name "s~*.o" -delete 2>/dev/null || true
    fi
    print_success "Cleaned $exdir"
done

# Clean test outputs
if [[ -d "tests/temp" ]]; then
    print_status "Removing test output directory"
    rm -rf "tests/temp"
    print_success "Test output directory removed"
fi

# Clean any stray build artifacts
print_status "Cleaning stray build artifacts..."

# Remove any .o files in src directories
find src -name "*.o" -delete 2>/dev/null || true
find src -name "*.so" -delete 2>/dev/null || true
find src -name "*.dylib" -delete 2>/dev/null || true
find src -name "*.a" -delete 2>/dev/null || true

# Remove any .o files in examples directories
find examples -name "*.o" -delete 2>/dev/null || true
find examples -name "*.so" -delete 2>/dev/null || true
find examples -name "*.dylib" -delete 2>/dev/null || true
find examples -name "*.a" -delete 2>/dev/null || true

# Remove any executable files in examples
find examples -type f -executable -name "openssl_pthread_demo" -delete 2>/dev/null || true

# Remove any SBOM files generated by examples
find examples -name "*.spdx" -delete 2>/dev/null || true
find examples -name "*.cyclonedx.json" -delete 2>/dev/null || true
find examples -name "*.json" -name "*sbom*" -delete 2>/dev/null || true

# Remove any CMake cache files in subdirectories
find . -name "CMakeCache.txt" -delete 2>/dev/null || true
find . -name "cmake_install.cmake" -delete 2>/dev/null || true
find . -name "CTestTestfile.cmake" -delete 2>/dev/null || true

# Remove any Makefiles in subdirectories (except the root one and Ada demo)
find . -name "Makefile" -not -path "./Makefile" -not -path "./examples/heimdall-ada-demo/Makefile" -delete 2>/dev/null || true

# Remove any CMakeFiles directories in subdirectories
find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true

# Clean any build directories that match our naming patterns
print_status "Cleaning any remaining build directories..."
find . -maxdepth 1 -type d -name "build-cpp*" -exec rm -rf {} + 2>/dev/null || true
find . -maxdepth 1 -type d -name "build_cpp*" -exec rm -rf {} + 2>/dev/null || true
find . -maxdepth 1 -type d -name "build-gcc-cpp*" -exec rm -rf {} + 2>/dev/null || true
find . -maxdepth 1 -type d -name "build-clang-cpp*" -exec rm -rf {} + 2>/dev/null || true

print_success "All build artifacts cleaned successfully!"

print_status "Cleanup summary:"
echo "  ✓ Build directory removed"
echo "  ✓ C++ standard-specific build directories removed"
echo "  ✓ Install directory removed"
echo "  ✓ All example build artifacts removed (old and new naming conventions)"
echo "  ✓ Test outputs removed"
echo "  ✓ Stray object files and libraries removed"
echo "  ✓ Generated SBOM files removed"
echo "  ✓ CMake cache files removed"

print_success "Project is now clean and ready for a fresh build!"
print_status "Run './build.sh' to rebuild the project." 
