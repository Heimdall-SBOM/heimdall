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
    echo "  --build-dir <dir>       Build directory (default: build-<compiler>-cpp<standard>)"
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
echo "  $0 --standard 17 --compiler clang           # Build C++17 with Clang (build-clang-cpp17)"
echo "  $0 --standard 20 --compiler gcc --all       # Build C++20 with GCC, tests, and SBOMs (build-gcc-cpp20)"
echo "  $0 --standard 17 --clean --tests            # Clean build C++17 with tests (default: gcc, build-gcc-cpp17)"
echo "  $0 --standard 17 --examples                 # Build C++17 with examples (examples/*/build-gcc-cpp17/)"
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
    BUILD_DIR="build-${COMPILER}-cpp${STANDARD}"
fi

print_status "Building Heimdall with C++${STANDARD} using ${COMPILER}"
print_status "Build directory: $BUILD_DIR"

# Check LLVM compatibility
print_status "Checking LLVM compatibility for C++${STANDARD}..."

# Check if LLVM environment variables are already set (e.g., in CI)
if [ -n "$LLVM_CONFIG" ] && [ -n "$LLVM_VERSION" ]; then
    print_status "Using pre-configured LLVM environment:"
    print_status "  LLVM_CONFIG: $LLVM_CONFIG"
    print_status "  LLVM_VERSION: $LLVM_VERSION"
    
    # Verify the LLVM config is available
    if ! command -v "$LLVM_CONFIG" >/dev/null 2>&1 && [ ! -x "$LLVM_CONFIG" ]; then
        print_error "Pre-configured LLVM_CONFIG not found: $LLVM_CONFIG"
        exit 1
    fi
    
    # Get additional LLVM paths if not already set
    if [ -z "$LLVM_INCLUDE_DIRS" ]; then
        LLVM_INCLUDE_DIRS="$($LLVM_CONFIG --includedir)"
        export LLVM_INCLUDE_DIRS
    fi
    if [ -z "$LLVM_LIBRARY_DIRS" ]; then
        LLVM_LIBRARY_DIRS="$($LLVM_CONFIG --libdir)"
        export LLVM_LIBRARY_DIRS
    fi
    if [ -z "$LLVM_MAJOR_VERSION" ]; then
        LLVM_MAJOR_VERSION="$($LLVM_CONFIG --version | head -n1 | cut -d' ' -f3 | cut -d'.' -f1)"
        export LLVM_MAJOR_VERSION
    fi
else
    # Use version manager to detect and configure LLVM
    LLVM_ENV=$(./scripts/llvm_version_manager.sh --quiet "$STANDARD")
    if [ $? -ne 0 ] || [ -z "$LLVM_ENV" ]; then
        print_error "No compatible LLVM version found for C++${STANDARD}"
        print_error "Run './scripts/show_build_compatibility.sh' for installation guidance"
        exit 1
    fi
    eval "$LLVM_ENV"
fi

# Check compiler compatibility
print_status "Checking compiler compatibility for C++${STANDARD}..."

# Check if compiler environment variables are already set (e.g., in CI)
if [ -n "$CC" ] && [ -n "$CXX" ]; then
    print_status "Using pre-configured compiler environment:"
    print_status "  CC: $CC"
    print_status "  CXX: $CXX"
    
    # Verify the compilers are available
    if ! command -v "$CC" >/dev/null 2>&1; then
        print_error "Pre-configured CC not found: $CC"
        exit 1
    fi
    if ! command -v "$CXX" >/dev/null 2>&1; then
        print_error "Pre-configured CXX not found: $CXX"
        exit 1
    fi
    
    # Get compiler versions for display
    CC_VERSION="$($CC --version | head -n1)"
    CXX_VERSION="$($CXX --version | head -n1)"
    export CC_VERSION
    export CXX_VERSION
else
    # Use version manager to detect and configure compiler
    if [ "$COMPILER" = "clang" ]; then
        # Force Clang selection
        COMPILER_ENV=$(./scripts/compiler_version_manager.sh --quiet "$STANDARD" clang)
    else
        # Use default selection (prefers GCC)
        COMPILER_ENV=$(./scripts/compiler_version_manager.sh --quiet "$STANDARD")
    fi

    if [ $? -ne 0 ] || [ -z "$COMPILER_ENV" ]; then
        print_error "No compatible compiler found for C++${STANDARD}"
        print_error "Run './scripts/show_build_compatibility.sh' for installation guidance"
        exit 1
    fi

    eval "$COMPILER_ENV"
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

# Handle SCL compilers
if [ -n "$SCL_ENV" ]; then
    print_status "Using SCL environment: $SCL_ENV"
    # For SCL, we need to run cmake and make within the SCL environment
    SCL_CMD="scl enable $SCL_ENV --"
else
    SCL_CMD=""
fi

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

$SCL_CMD cmake .. $CMAKE_OPTS

# Build
print_status "Building Heimdall..."
# Cross-platform CPU count
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=$(sysctl -n hw.ncpu)
fi
$SCL_CMD make -j$JOBS

print_success "Build completed successfully!"

# Run tests if requested
if [ "$TESTS" = true ] || [ "$ALL" = true ]; then
    print_status "Running tests..."
    $SCL_CMD ctest --output-on-failure
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
    print_status "Building examples as standalone projects..."

    # Save the absolute path to the build directory
    BUILD_DIR_ABS="$(pwd)"

    # Go back to project root to find standalone examples
    cd ..

    # Define examples that are built as part of the main build (should be skipped)
    MAIN_BUILD_EXAMPLES=(
        "openssl_pthread_demo"
    )

    # Find all example directories with CMakeLists.txt
    for example_dir in examples/*/; do
        if [ -d "$example_dir" ] && [ -f "${example_dir}CMakeLists.txt" ]; then
            # Extract example name from directory path
            example_name=$(basename "$example_dir")
            
            # Skip examples that are built as part of the main build
            skip_example=false
            for main_example in "${MAIN_BUILD_EXAMPLES[@]}"; do
                if [ "$example_name" = "$main_example" ]; then
                    print_status "Skipping example built as part of main build: $example_dir"
                    skip_example=true
                    break
                fi
            done
            
            if [ "$skip_example" = true ]; then
                continue
            fi

            print_status "Building standalone example: $example_dir"
            cd "$example_dir"

            # Create build directory with new naming convention
            EXAMPLE_BUILD_DIR="build-${COMPILER}-cpp${STANDARD}"
            mkdir -p "$EXAMPLE_BUILD_DIR"
            cd "$EXAMPLE_BUILD_DIR"

            # Configure with CMake, passing the main build directory for finding Heimdall
            CMAKE_OPTS="-DCMAKE_CXX_STANDARD=${STANDARD} -DCMAKE_CXX_STANDARD_REQUIRED=ON"
            CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX"
            
            # Add Heimdall build directory and source directory to help find libraries and headers
            CMAKE_OPTS="$CMAKE_OPTS -DHEIMDALL_BUILD_DIR=${BUILD_DIR_ABS}"
            CMAKE_OPTS="$CMAKE_OPTS -DHEIMDALL_SOURCE_DIR=${BUILD_DIR_ABS}/.."
            
            # Configure and build
            cmake .. $CMAKE_OPTS
            make -j$JOBS

            print_success "Built standalone example: $example_dir (in $EXAMPLE_BUILD_DIR)"
            cd ../..
        fi
    done

    print_success "All standalone examples built successfully!"
    # Go back to original build directory
    cd "$BUILD_DIR_ABS"
fi

cd ..

print_success "Heimdall C++${STANDARD} build completed successfully!"
print_status "Build artifacts are in: $BUILD_DIR" 