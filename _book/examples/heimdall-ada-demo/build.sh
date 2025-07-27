#!/bin/bash

# Heimdall Ada Demo Build Script
# This script builds the Ada demo application

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
    echo "  --static    Build static version only"
    echo "  --dynamic   Build dynamic version only"
    echo "  --test      Build and test both versions"
    echo "  --clean     Clean build artifacts"
    echo "  --help      Show this help message"
    echo ""
    echo "Default: Build both static and dynamic versions"
}

# Parse command line arguments
BUILD_STATIC=true
BUILD_DYNAMIC=true
RUN_TESTS=false
CLEAN_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --static)
            BUILD_DYNAMIC=false
            shift
            ;;
        --dynamic)
            BUILD_STATIC=false
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --clean)
            CLEAN_ONLY=true
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

# Check if we're in the right directory
if [[ ! -f "Makefile" ]]; then
    print_error "Makefile not found. Please run this script from the heimdall-ada-demo directory."
    exit 1
fi

# Check for GNAT
if ! command -v gnatmake >/dev/null 2>&1; then
    print_error "GNAT (GNU Ada compiler) not found."
    print_error "Please install GNAT:"
    print_error "  RHEL/Fedora: sudo dnf install gcc-gnat"
    print_error "  Ubuntu/Debian: sudo apt-get install gnat"
    exit 1
fi

print_status "Using GNAT: $(gnatmake --version | head -n1)"

# Clean if requested
if [ "$CLEAN_ONLY" = true ]; then
    print_status "Cleaning build artifacts..."
    make clean
    print_success "Clean complete"
    exit 0
fi

# Build static version
if [ "$BUILD_STATIC" = true ]; then
    print_status "Building static version..."
    make static
    print_success "Static build complete"
fi

# Build dynamic version
if [ "$BUILD_DYNAMIC" = true ]; then
    print_status "Building dynamic version..."
    make dynamic
    print_success "Dynamic build complete"
fi

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    print_status "Running tests..."
    make test
    print_success "Tests completed"
fi

print_success "Ada demo build completed successfully!"
print_status "Built executables:"
if [ "$BUILD_STATIC" = true ]; then
    echo "  - bin/main_static (static version)"
fi
if [ "$BUILD_DYNAMIC" = true ]; then
    echo "  - bin/main_dynamic (dynamic version)"
    echo "  - lib/libstring_utils.so (shared library)"
fi 