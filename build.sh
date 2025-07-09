#!/bin/bash

# Heimdall Build Script
# Supports C++11, C++14, C++17, C++20, and C++23

set -e

# Default values
BUILD_TYPE="Release"
CXX_STANDARD="17"
ENABLE_TESTS="ON"
ENABLE_COVERAGE="OFF"
ENABLE_CPP11_14="OFF"
USE_BOOST_FILESYSTEM="OFF"
BUILD_ALL_STANDARDS="OFF"

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

# Function to show help
show_help() {
    cat << EOF
Heimdall Build Script

Usage: $0 [OPTIONS]

Options:
    --build-type TYPE        Build type (Debug, Release, RelWithDebInfo, MinSizeRel) [default: Release]
    --cxx-standard VERSION   C++ standard version (11, 14, 17, 20, 23) [default: 17]
    --tests                  Enable tests [default: ON]
    --no-tests               Disable tests
    --coverage               Enable coverage reporting
    --no-boost                Disable Boost.Filesystem requirement
    --all-standards          Build all C++ standards (11, 14, 17, 20, 23)
    --help                   Show this help message

Examples:
    $0                                    # Build with C++17 (default)
    $0 --cxx-standard 14                  # Build with C++14 (auto-enables compatibility mode)
    $0 --cxx-standard 11                  # Build with C++11 (auto-enables compatibility mode)
    $0 --build-type Debug --coverage      # Debug build with coverage
    $0 --cxx-standard 23                  # Build with C++23
    $0 --all-standards                    # Build all C++ standards

C++ Standard Compatibility:
    C++11: Requires LLVM 7-18, Boost.Filesystem
    C++14: Requires LLVM 7-18, Boost.Filesystem  
    C++17: Requires LLVM 11+, standard library
    C++20: Requires LLVM 19+, standard library
    C++23: Requires LLVM 19+, standard library

EOF
}

# Function to check if Boost.Filesystem is available
check_boost_filesystem() {
    print_status "Checking for Boost.Filesystem..."
    
    # Try to find boost_filesystem using pkg-config
    if pkg-config --exists libboost_filesystem; then
        print_success "Boost.Filesystem found via pkg-config"
        return 0
    fi
    
    # Try to find boost_filesystem using find_library
    if [ -f "/usr/lib/x86_64-linux-gnu/libboost_filesystem.so" ] || \
       [ -f "/usr/lib/libboost_filesystem.so" ] || \
       [ -f "/usr/local/lib/libboost_filesystem.so" ]; then
        print_success "Boost.Filesystem library found"
        return 0
    fi
    
    # Check for Homebrew Boost on macOS
    if [ -f "/opt/homebrew/lib/libboost_filesystem.dylib" ] || \
       [ -f "/usr/local/lib/libboost_filesystem.dylib" ]; then
        print_success "Boost.Filesystem library found (Homebrew)"
        return 0
    fi
    
    print_error "Boost.Filesystem not found. Please install it:"
    echo "  Ubuntu/Debian: sudo apt-get install libboost-filesystem-dev"
    echo "  CentOS/RHEL: sudo yum install boost-devel"
    echo "  macOS: brew install boost"
    return 1
}

# Function to validate C++ standard compatibility
validate_cxx_standard() {
    local standard=$1
    
    case $standard in
        11|14)
            # C++11/14 automatically enable compatibility mode
            ENABLE_CPP11_14="ON"
            if [ "$USE_BOOST_FILESYSTEM" != "OFF" ]; then
                USE_BOOST_FILESYSTEM="ON"
            fi
            ;;
        17|20|23)
            # C++17+ use standard library features
            ENABLE_CPP11_14="OFF"
            USE_BOOST_FILESYSTEM="OFF"
            ;;
        *)
            print_error "Unsupported C++ standard: $standard"
            print_error "Supported standards: 11, 14, 17, 20, 23"
            exit 1
            ;;
    esac
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --cxx-standard)
            CXX_STANDARD="$2"
            shift 2
            ;;
        --tests)
            ENABLE_TESTS="ON"
            shift
            ;;
        --no-tests)
            ENABLE_TESTS="OFF"
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE="ON"
            shift
            ;;
        --no-boost)
            USE_BOOST_FILESYSTEM="OFF"
            shift
            ;;
        --all-standards)
            BUILD_ALL_STANDARDS="ON"
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Handle --all-standards option
if [ "$BUILD_ALL_STANDARDS" = "ON" ]; then
    print_status "Building all C++ standards..."
    if [ -f "./scripts/build_all_standards.sh" ]; then
        print_status "Calling scripts/build_all_standards.sh"
        exec ./scripts/build_all_standards.sh
    else
        print_error "scripts/build_all_standards.sh not found"
        exit 1
    fi
fi

# Validate C++ standard
validate_cxx_standard "$CXX_STANDARD"

# Check Boost.Filesystem if needed
if [ "$USE_BOOST_FILESYSTEM" = "ON" ]; then
    if ! check_boost_filesystem; then
        exit 1
    fi
fi

# Create build directory
BUILD_DIR="build-cpp${CXX_STANDARD}"
print_status "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
print_status "  Build Type: $BUILD_TYPE"
print_status "  C++ Standard: $CXX_STANDARD"
print_status "  Tests: $ENABLE_TESTS"
print_status "  Coverage: $ENABLE_COVERAGE"
print_status "  C++11/14 Mode: $ENABLE_CPP11_14"
print_status "  Use Boost.Filesystem: $USE_BOOST_FILESYSTEM"

cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_STANDARD="$CXX_STANDARD" \
    -DENABLE_TESTS="$ENABLE_TESTS" \
    -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
    -DENABLE_CPP11_14="$ENABLE_CPP11_14" \
    -DUSE_BOOST_FILESYSTEM="$USE_BOOST_FILESYSTEM"

if [ $? -eq 0 ]; then
    print_success "Configuration completed successfully"
else
    print_error "Configuration failed"
    exit 1
fi

# Build
print_status "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    print_success "Build completed successfully"
else
    print_error "Build failed"
    exit 1
fi

# Run tests if enabled
if [ "$ENABLE_TESTS" = "ON" ]; then
    print_status "Running tests..."
    make test
    
    if [ $? -eq 0 ]; then
        print_success "All tests passed"
    else
        print_error "Some tests failed"
        exit 1
    fi
fi

print_success "Build completed successfully!"
print_status "Build artifacts are in: $BUILD_DIR"
