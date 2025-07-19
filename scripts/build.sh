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

# Heimdall Build Script
# Builds Heimdall with a specific C++ standard and compiler

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --standard <version>    C++ standard to build (11, 14, 17, 20, 23)"
    echo "  --compiler <name>       Compiler to use (gcc, clang). Default: gcc"
    echo "  --build-dir <dir>       Build directory (default: build-cpp<standard>)"
    echo "  --clean                 Clean build directory before building"
    echo "  --tests                 Run tests after building"
    echo "  --sboms                 Generate SBOMs after building"
    echo "  --profiling             Enable performance profiling"
    echo "  --benchmarks            Enable performance benchmarks"
    echo "  --examples              Build examples after main build"
    echo "  --all                   Build, test, and generate SBOMs"
    echo "  --help                  Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --standard 17 --compiler clang           # Build C++17 with Clang"
    echo "  $0 --standard 20 --compiler gcc --all       # Build C++20 with GCC, tests, and SBOMs"
    echo "  $0 --standard 17 --clean --tests            # Clean build C++17 with tests (default: gcc)"
    echo "  $0 --standard 17 --examples                 # Build C++17 with examples"
    echo ""
    echo "Available standards: 11, 14, 17, 20, 23"
}

# Parse command line arguments
STANDARD=""
COMPILER="gcc"
BUILD_DIR=""
CLEAN=false
TESTS=false
SBOMS=false
PROFILING=false
BENCHMARKS=false
ALL=false
EXAMPLES=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --standard)
            STANDARD="$2"
            shift 2
            ;;
        --compiler)
            COMPILER="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --tests)
            TESTS=true
            shift
            ;;
        --sboms)
            SBOMS=true
            shift
            ;;
        --profiling)
            PROFILING=true
            shift
            ;;
        --benchmarks)
            BENCHMARKS=true
            shift
            ;;
        --examples)
            EXAMPLES=true
            shift
            ;;
        --all)
            ALL=true
            shift
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate standard
if [ -z "$STANDARD" ]; then
    print_error "C++ standard not specified"
    show_usage
    exit 1
fi

case $STANDARD in
    11|14|17|20|23)
        ;;
    *)
        print_error "Invalid C++ standard: $STANDARD"
        print_error "Supported standards: 11, 14, 17, 20, 23"
        exit 1
        ;;
esac

# Set default build directory if not specified
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="build-cpp${STANDARD}"
fi

print_status "Building Heimdall with C++${STANDARD} using ${COMPILER}"
print_status "Build directory: $BUILD_DIR"

# Check LLVM compatibility
print_status "Checking LLVM compatibility for C++${STANDARD}..."
LLVM_ENV=$(./scripts/llvm_version_manager.sh --quiet "$STANDARD")
if [ $? -ne 0 ] || [ -z "$LLVM_ENV" ]; then
    print_error "No compatible LLVM version found for C++${STANDARD}"
    print_error "Run './scripts/show_build_compatibility.sh' for installation guidance"
    exit 1
fi

eval "$LLVM_ENV"

# Compiler selection logic
if [ "$COMPILER" = "clang" ]; then
    if ! command -v clang >/dev/null 2>&1 || ! command -v clang++ >/dev/null 2>&1; then
        print_error "Clang not found in PATH. Please install clang and clang++."
        exit 1
    fi
    export CC=clang
    export CXX=clang++
    print_status "Using Clang: $(clang --version | head -n1)"
else
    # For GCC, check what's available and guide the user
    print_status "Checking GCC compatibility for C++${STANDARD}..."
    
    # Check if we have a compatible GCC version in PATH
    gcc_version=$(gcc --version | head -n1 | cut -d' ' -f3)
    gcc_major=$(echo "$gcc_version" | cut -d'.' -f1)
    
    # Check if we need SCL for this C++ standard
    needs_scl=false
    required_gcc_major=0
    
    case $STANDARD in
        11) required_gcc_major=4 ;;
        14) required_gcc_major=6 ;;
        17) required_gcc_major=7 ;;
        20|23) required_gcc_major=13 ;;
    esac
    
    if [ "$gcc_major" -lt "$required_gcc_major" ]; then
        needs_scl=true
    fi
    
    if [ "$needs_scl" = true ]; then
        print_error "Your system GCC version ($gcc_version) is not compatible with C++${STANDARD}."
        print_error "C++${STANDARD} requires GCC ${required_gcc_major}+."
        print_error ""
        print_error "To build with GCC, please activate an SCL toolset first:"
        print_error "  scl enable gcc-toolset-14 bash"
        print_error "  ./scripts/build.sh --standard ${STANDARD} --compiler gcc --tests"
        print_error ""
        print_error "Or use Clang instead:"
        print_error "  ./scripts/build.sh --standard ${STANDARD} --compiler clang --tests"
        exit 1
    else
        export CC=gcc
        export CXX=g++
        print_status "Using system GCC: $gcc_version"
    fi
fi

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    print_status "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_status "Using LLVM: $LLVM_VERSION"
print_status "Using compiler: $CXX_VERSION"

# Configure with CMake
print_status "Configuring with CMake..."

# Build CMake options
CMAKE_OPTS="-DCMAKE_CXX_STANDARD=${STANDARD} -DCMAKE_CXX_STANDARD_REQUIRED=ON -DLLVM_CONFIG=$LLVM_CONFIG -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX"

# Add compatibility mode for C++11/14
if [[ "${STANDARD}" == "11" || "${STANDARD}" == "14" ]]; then
    CMAKE_OPTS="$CMAKE_OPTS -DHEIMDALL_CXX11_14_MODE=ON"
fi

# Add profiling options
if [ "$PROFILING" = true ]; then
    CMAKE_OPTS="$CMAKE_OPTS -DENABLE_PROFILING=ON"
    print_status "Profiling enabled"
fi

if [ "$BENCHMARKS" = true ]; then
    CMAKE_OPTS="$CMAKE_OPTS -DENABLE_BENCHMARKS=ON"
    print_status "Benchmarks enabled"
fi

cmake .. $CMAKE_OPTS

# Build
print_status "Building Heimdall..."
# Cross-platform CPU count
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=$(sysctl -n hw.ncpu)
fi
make -j$JOBS

print_success "Build completed successfully!"

# Run tests if requested
if [ "$TESTS" = true ] || [ "$ALL" = true ]; then
    print_status "Running tests..."
    ctest --output-on-failure
    print_success "Tests completed successfully!"
fi

# Generate SBOMs if requested
if [ "$SBOMS" = true ] || [ "$ALL" = true ]; then
    print_status "Generating SBOMs..."
    ../scripts/generate_build_sboms.sh .
    print_success "SBOM generation completed!"
fi

# Build examples if requested
if [ "$EXAMPLES" = true ]; then
    print_status "Building examples..."

    # Save the absolute path to the build directory
    BUILD_DIR_ABS="$(pwd)"

    # Go back to project root to find standalone examples
    cd ../..

    # Find all example directories with CMakeLists.txt
    for example_dir in examples/*/; do
        if [ -d "$example_dir" ] && [ -f "${example_dir}CMakeLists.txt" ]; then
            print_status "Building example: $example_dir"
            cd "$example_dir"

            # Create build directory
            mkdir -p build
            cd build

            # Configure and build
            cmake ..
            make -j$JOBS

            print_success "Built example: $example_dir"
            cd ../..
        fi
    done

    print_success "All examples built successfully!"
    # Go back to original build directory
    cd "$BUILD_DIR_ABS"
fi

cd ..

print_success "Heimdall C++${STANDARD} build completed successfully!"
print_status "Build artifacts are in: $BUILD_DIR" 