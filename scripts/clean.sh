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
echo "  - C++ standard-specific build directories (build-cpp11, build-cpp14, etc.)"
echo "  - Install directory and all contents"
echo "  - CMake cache files"
echo "  - Compiled binaries and libraries"
echo "  - Test outputs"
echo "  - Example build artifacts (all heimdall-usage-* examples)"
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

# Clean C++ standard-specific build directories (both naming conventions)
CXX_BUILD_DIRS=(
    "build_cpp11" "build_cpp14" "build_cpp17" "build_cpp20" "build_cpp23"  # Old naming
    "build-cpp11" "build-cpp14" "build-cpp17" "build-cpp20" "build-cpp23"  # New naming
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

# Clean example build artifacts
print_status "Cleaning example build artifacts..."

# Clean openssl_pthread_demo example
if [[ -d "examples/openssl_pthread_demo/build" ]]; then
    print_status "Removing openssl_pthread_demo build directory"
    rm -rf "examples/openssl_pthread_demo/build"
    print_success "openssl_pthread_demo build directory removed"
fi

# Clean heimdall-usage-example artifacts
if [[ -d "examples/heimdall-usage-example" ]]; then
    print_status "Cleaning heimdall-usage-example artifacts..."
    cd examples/heimdall-usage-example
    rm -f *.o app-* *.json *.spdx *.cyclonedx.json 2>/dev/null || true
    cd ../..
    print_success "heimdall-usage-example artifacts cleaned"
fi

# Clean heimdall-usage-spdx-example artifacts
if [[ -d "examples/heimdall-usage-spdx-example" ]]; then
    print_status "Cleaning heimdall-usage-spdx-example artifacts..."
    cd examples/heimdall-usage-spdx-example
    rm -f *.o app-* *.spdx 2>/dev/null || true
    cd ../..
    print_success "heimdall-usage-spdx-example artifacts cleaned"
fi

# Clean heimdall-usage-cyclonedx-example artifacts
if [[ -d "examples/heimdall-usage-cyclonedx-example" ]]; then
    print_status "Cleaning heimdall-usage-cyclonedx-example artifacts..."
    cd examples/heimdall-usage-cyclonedx-example
    rm -f *.o app-* *.cyclonedx.json 2>/dev/null || true
    cd ../..
    print_success "heimdall-usage-cyclonedx-example artifacts cleaned"
fi

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

# Remove any Makefiles in subdirectories (except the root one)
find . -name "Makefile" -not -path "./Makefile" -delete 2>/dev/null || true

# Remove any CMakeFiles directories in subdirectories
find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true

# Clean any build directories that match our new naming pattern
print_status "Cleaning any remaining build directories..."
find . -maxdepth 1 -type d -name "build-cpp*" -exec rm -rf {} + 2>/dev/null || true
find . -maxdepth 1 -type d -name "build_cpp*" -exec rm -rf {} + 2>/dev/null || true

print_success "All build artifacts cleaned successfully!"

print_status "Cleanup summary:"
echo "  ✓ Build directory removed"
echo "  ✓ C++ standard-specific build directories removed"
echo "  ✓ Install directory removed"
echo "  ✓ All example build artifacts removed (heimdall-usage-*, openssl_pthread_demo)"
echo "  ✓ Test outputs removed"
echo "  ✓ Stray object files and libraries removed"
echo "  ✓ Generated SBOM files removed"
echo "  ✓ CMake cache files removed"

print_success "Project is now clean and ready for a fresh build!"
print_status "Run './build.sh' to rebuild the project." 
