#!/bin/bash

# Heimdall Coverage Script
# This script builds the project with coverage enabled, runs tests, and generates coverage reports

set -e  # Exit on any error

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
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Check for required tools
check_requirements() {
    print_status "Checking requirements..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "cmake not found. Please install CMake."
        exit 1
    fi
    
    if ! command -v gcov &> /dev/null; then
        print_error "gcov not found. Please install GCC with coverage support."
        exit 1
    fi
    
    if ! command -v lcov &> /dev/null; then
        print_warning "lcov not found. Installing lcov..."
        if command -v yum &> /dev/null; then
            sudo yum install -y lcov
        elif command -v apt-get &> /dev/null; then
            sudo apt-get update && sudo apt-get install -y lcov
        else
            print_error "Could not install lcov automatically. Please install it manually."
            exit 1
        fi
    fi
    
    print_success "All requirements satisfied"
}

# Clean previous build artifacts
clean_build() {
    print_status "Cleaning previous build artifacts..."
    
    if [ -d "build" ]; then
        rm -rf build
        print_success "Removed build directory"
    fi
    
    # Clean any existing coverage files
    find . -name "*.gcda" -delete 2>/dev/null || true
    find . -name "*.gcno" -delete 2>/dev/null || true
    find . -name "*.gcov" -delete 2>/dev/null || true
    
    print_success "Clean completed"
}

# Configure and build with coverage
build_with_coverage() {
    print_status "Configuring and building with coverage enabled..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with coverage enabled
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DENABLE_COVERAGE=ON \
        -DBUILD_TESTS=ON \
        -DENABLE_DEBUG=ON
    
    if [ $? -ne 0 ]; then
        print_error "CMake configuration failed"
        exit 1
    fi
    
    # Build the project
    make -j$(nproc)
    
    if [ $? -ne 0 ]; then
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
    print_success "Build completed successfully"
}

# Run tests
run_tests() {
    print_status "Running tests..."
    
    cd build
    
    # Run the test executable
    if [ -f "tests/heimdall-tests" ]; then
        ./tests/heimdall-tests
        TEST_EXIT_CODE=$?
    else
        print_error "Test executable not found"
        exit 1
    fi
    
    cd ..
    
    if [ $TEST_EXIT_CODE -eq 0 ]; then
        print_success "Tests completed successfully"
    else
        print_warning "Some tests failed (exit code: $TEST_EXIT_CODE)"
    fi
}

# Generate coverage data
generate_coverage() {
    print_status "Generating coverage data..."
    
    cd build
    
    # Create coverage directory
    mkdir -p coverage
    
    # Use lcov to capture coverage data
    if command -v lcov &> /dev/null; then
        print_status "Using lcov for coverage generation..."
        
        # Capture coverage data
        lcov --capture --directory . --output-file coverage/coverage.info
        
        if [ $? -eq 0 ]; then
            # Remove system headers and external files
            lcov --remove coverage/coverage.info \
                '/usr/*' \
                '*/external/*' \
                '*/tests/*' \
                '*/testdata/*' \
                --output-file coverage/coverage_filtered.info
            
            # Generate HTML report
            genhtml coverage/coverage_filtered.info --output-directory coverage/html
            
            # Generate text summary
            lcov --summary coverage/coverage_filtered.info > coverage/coverage_summary.txt
            
            print_success "Coverage report generated in build/coverage/"
        else
            print_warning "lcov failed, falling back to gcov..."
            generate_gcov_coverage
        fi
    else
        print_warning "lcov not available, using gcov directly..."
        generate_gcov_coverage
    fi
    
    cd ..
}

# Fallback coverage generation using gcov directly
generate_gcov_coverage() {
    print_status "Generating coverage using gcov..."
    
    # Find all .gcda files
    find . -name "*.gcda" | while read gcda_file; do
        dir=$(dirname "$gcda_file")
        base=$(basename "$gcda_file" .gcda)
        
        # Find corresponding .gcno file
        gcno_file="$dir/$base.gcno"
        if [ -f "$gcno_file" ]; then
            print_status "Processing $gcda_file..."
            
            # Change to the directory and run gcov
            (cd "$dir" && gcov -o . "$base.gcda")
        fi
    done
    
    # Move all .gcov files to coverage directory
    find . -name "*.gcov" -exec mv {} coverage/ \; 2>/dev/null || true
    
    print_success "Gcov coverage files generated in build/coverage/"
}

# Display coverage summary
update_coverage_badge() {
    # Extract the line coverage percentage from the summary
    if [ -f "build/coverage/coverage_summary.txt" ]; then
        percent=$(grep -Po 'lines\.+: \K[0-9]+\.[0-9]+' build/coverage/coverage_summary.txt | head -1)
        if [ -n "$percent" ]; then
            badge="[![Coverage](https://img.shields.io/badge/coverage-${percent}%25-yellow.svg)](build/coverage/html/index.html)"
            # Replace the badge line in README.md
            sed -i -E "s|\[!\[Coverage\]\(https://img.shields.io/badge/coverage-[0-9]+\.[0-9]+%25-[a-z]+.svg\)\]\(build/coverage/html/index.html\)|$badge|g" README.md
        fi
    fi
}

# Display coverage summary
display_summary() {
    print_status "Coverage Summary:"
    
    if [ -f "build/coverage/coverage_summary.txt" ]; then
        echo "=== LCOV Coverage Summary ==="
        cat build/coverage/coverage_summary.txt
        echo ""
        echo "HTML report available at: build/coverage/html/index.html"
    fi
    
    if [ -d "build/coverage" ]; then
        echo "=== GCOV Files Generated ==="
        ls -la build/coverage/*.gcov 2>/dev/null || echo "No .gcov files found"
    fi
    
    # Show source files that should have coverage
    echo ""
    echo "=== Project Source Files ==="
    find src -name "*.cpp" -o -name "*.cxx" -o -name "*.cc" | sort
}

# Main execution
main() {
    print_status "Starting Heimdall coverage analysis..."
    
    check_requirements
    clean_build
    build_with_coverage
    run_tests
    generate_coverage
    display_summary
    update_coverage_badge
    print_success "Coverage analysis completed!"
    print_status "Check build/coverage/ for detailed reports"
}

# Run main function
main "$@" 