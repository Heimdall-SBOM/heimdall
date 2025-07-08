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

# Heimdall SBOM Generator Build Script
# This script builds the Heimdall project with support for both LLD and Gold plugins

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

# Default configuration
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_DIR="install"
ENABLE_DEBUG=false
ENABLE_SANITIZERS=false
BUILD_LLD_PLUGIN=true
BUILD_GOLD_PLUGIN=true
BUILD_SHARED_CORE=true
BUILD_TESTS=true
BUILD_EXAMPLES=true
CXX_STANDARD=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            ENABLE_DEBUG=true
            shift
            ;;
        --sanitizers)
            ENABLE_SANITIZERS=true
            shift
            ;;
        --no-lld)
            BUILD_LLD_PLUGIN=false
            shift
            ;;
        --no-gold)
            BUILD_GOLD_PLUGIN=false
            shift
            ;;
        --no-shared-core)
            BUILD_SHARED_CORE=false
            shift
            ;;
        --no-tests)
            BUILD_TESTS=false
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=false
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        --cxx-standard)
            CXX_STANDARD="$2"
            shift 2
            ;;
        --help|-h)
            echo "Heimdall SBOM Generator Build Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug              Build in debug mode"
            echo "  --sanitizers         Enable AddressSanitizer and UBSan"
            echo "  --no-lld             Disable LLD plugin build"
            echo "  --no-gold            Disable Gold plugin build"
            echo "  --no-shared-core     Disable shared core library"
            echo "  --no-tests           Disable test suite"
            echo "  --no-examples        Disable example projects"
            echo "  --build-dir DIR      Set build directory (default: build)"
            echo "  --install-dir DIR    Set install directory (default: install)"
            echo "  --cxx-standard VER   Set C++ standard version (e.g., 17, 20)"
            echo "  --help, -h           Show this help message"
            echo ""
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

print_status "Building Heimdall SBOM Generator"
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"
print_status "Install directory: $INSTALL_DIR"

# Check for required tools
print_status "Checking for required tools..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake 3.16 or later."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
print_success "Found CMake $CMAKE_VERSION"

# Check for C++ compiler
if command -v g++ &> /dev/null; then
    COMPILER="g++"
    COMPILER_VERSION=$(g++ --version | head -n1 | cut -d' ' -f4)
    print_success "Found GCC $COMPILER_VERSION"
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++"
    COMPILER_VERSION=$(clang++ --version | head -n1 | cut -d' ' -f3)
    print_success "Found Clang $COMPILER_VERSION"
else
    print_error "No C++ compiler found. Please install GCC or Clang."
    exit 1
fi

# Check for LLVM/LLD (for LLD plugin)
if [[ "$BUILD_LLD_PLUGIN" == true ]]; then
    if command -v llvm-config &> /dev/null; then
        LLVM_VERSION=$(llvm-config --version)
        print_success "Found LLVM $LLVM_VERSION"
    else
        print_warning "LLVM not found. LLD plugin will not be built."
        BUILD_LLD_PLUGIN=false
    fi
fi

# Check for Gold linker (for Gold plugin)
if [[ "$BUILD_GOLD_PLUGIN" == true ]]; then
    if command -v ld.gold &> /dev/null; then
        print_success "Found Gold linker"
    else
        print_warning "Gold linker not found. Gold plugin will not be built."
        BUILD_GOLD_PLUGIN=false
    fi
fi

# Check for OpenSSL (for checksums)
if ! pkg-config --exists openssl; then
    print_warning "OpenSSL not found via pkg-config. Checksum generation may not work."
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
print_status "Configuring CMake..."

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    "-DBUILD_LLD_PLUGIN=$BUILD_LLD_PLUGIN"
    "-DBUILD_GOLD_PLUGIN=$BUILD_GOLD_PLUGIN"
    "-DBUILD_SHARED_CORE=$BUILD_SHARED_CORE"
    "-DBUILD_TESTS=$BUILD_TESTS"
    "-DBUILD_EXAMPLES=$BUILD_EXAMPLES"
    "-DENABLE_DEBUG=$ENABLE_DEBUG"
    "-DENABLE_SANITIZERS=$ENABLE_SANITIZERS"
)

# Add C++ standard if specified
if [[ -n "$CXX_STANDARD" ]]; then
    print_status "Using C++ standard: $CXX_STANDARD"
    CMAKE_ARGS+=("-DCMAKE_CXX_STANDARD=$CXX_STANDARD")
fi

# Add platform-specific options
if [[ "$OSTYPE" == "darwin"* ]]; then
    print_status "Detected macOS"
    CMAKE_ARGS+=("-DPLATFORM_MACOS=TRUE")
    # Homebrew LLVM detection (Apple Silicon and Intel)
    if [[ -d "/opt/homebrew/opt/llvm" ]]; then
        print_status "Using Homebrew LLVM from /opt/homebrew/opt/llvm"
        CMAKE_ARGS+=(
            "-DCMAKE_PREFIX_PATH=/opt/homebrew/opt/llvm"
            "-DCMAKE_INCLUDE_PATH=/opt/homebrew/opt/llvm/include"
            "-DCMAKE_LIBRARY_PATH=/opt/homebrew/opt/llvm/lib"
        )
    elif [[ -d "/usr/local/opt/llvm" ]]; then
        print_status "Using Homebrew LLVM from /usr/local/opt/llvm"
        CMAKE_ARGS+=(
            "-DCMAKE_PREFIX_PATH=/usr/local/opt/llvm"
            "-DCMAKE_INCLUDE_PATH=/usr/local/opt/llvm/include"
            "-DCMAKE_LIBRARY_PATH=/usr/local/opt/llvm/lib"
        )
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    print_status "Detected Linux"
    CMAKE_ARGS+=("-DPLATFORM_LINUX=TRUE")
else
    print_warning "Unknown platform: $OSTYPE"
fi

cmake "${CMAKE_ARGS[@]}" ..

if [[ $? -ne 0 ]]; then
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_status "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [[ $? -ne 0 ]]; then
    print_error "Build failed"
    exit 1
fi

# Run tests if enabled
if [[ "$BUILD_TESTS" == true ]]; then
    print_status "Running tests..."
    make test
fi

# Install if requested
print_status "Installing..."
make install

# Show build results
print_success "Build completed successfully!"

print_status "Build artifacts:"
if [[ "$BUILD_SHARED_CORE" == true ]]; then
    if [[ -f "libheimdall-core.so" ]]; then
        print_success "  - libheimdall-core.so (Linux)"
    elif [[ -f "libheimdall-core.dylib" ]]; then
        print_success "  - libheimdall-core.dylib (macOS)"
    fi
fi

if [[ "$BUILD_LLD_PLUGIN" == true ]]; then
    if [[ -f "heimdall-lld.so" ]]; then
        print_success "  - heimdall-lld.so (Linux)"
    elif [[ -f "heimdall-lld.dylib" ]]; then
        print_success "  - heimdall-lld.dylib (macOS)"
    fi
fi

if [[ "$BUILD_GOLD_PLUGIN" == true ]]; then
    if [[ -f "heimdall-gold.so" ]]; then
        print_success "  - heimdall-gold.so"
    fi
fi

print_status "Installation directory: $INSTALL_DIR"
print_status ""
print_status "Usage examples:"
print_status "  # Using LLD plugin:"
print_status "  ld.lld --plugin-opt=load:./heimdall-lld.dylib \\"
print_status "         --plugin-opt=sbom-output:myapp.json \\"
print_status "         main.o -o myapp"
print_status ""
print_status "  # Using Gold plugin:"
print_status "  ld.gold --plugin ./heimdall-gold.so \\"
print_status "          --plugin-opt sbom-output=myapp.json \\"
print_status "          main.o -o myapp"
