#!/bin/bash

# Simple Coverage Script for Heimdall
# This script provides basic coverage functionality

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "This script must be run from the project root directory"
    exit 1
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Please run './build.sh' first"
    exit 1
fi

cd build

# Check if coverage was enabled
if [ ! -f "CMakeCache.txt" ] || ! grep -q "ENABLE_COVERAGE:BOOL=ON" CMakeCache.txt; then
    print_info "Coverage not enabled. Reconfiguring with coverage enabled..."
    cmake -DENABLE_COVERAGE=ON ..
fi

# Build with coverage
print_info "Building with coverage enabled..."
cmake --build . -- -j$(nproc)

# Run tests
print_info "Running tests to generate coverage data..."
ctest --output-on-failure

# Create coverage directory
COVERAGE_DIR="coverage"
mkdir -p "$COVERAGE_DIR"

# Generate basic coverage report
print_info "Generating basic coverage report..."

{
    echo "Heimdall Basic Coverage Report"
    echo "=============================="
    echo "Generated: $(date)"
    echo ""
    echo "Coverage Data Files Found:"
    echo "=========================="
    
    # List all .gcda files
    print_info "Searching for .gcda files..."
    find . -name "*.gcda" 2>/dev/null | sort | while read -r gcda_file; do
        echo "  - $gcda_file"
    done
    
    echo ""
    echo "Coverage Notes Files Found:"
    echo "==========================="
    
    # List all .gcno files
    print_info "Searching for .gcno files..."
    find . -name "*.gcno" 2>/dev/null | sort | while read -r gcno_file; do
        echo "  - $gcno_file"
    done
    
    echo ""
    echo "Summary:"
    echo "========"
    echo "Coverage data files: $(find . -name "*.gcda" 2>/dev/null | wc -l)"
    echo "Coverage notes files: $(find . -name "*.gcno" 2>/dev/null | wc -l)"
    echo ""
    echo "Coverage is enabled and data is being collected."
    echo "To view detailed coverage information, install lcov:"
    echo "  sudo yum install lcov"
    echo ""
    echo "Then run:"
    echo "  lcov --capture --directory . --output-file coverage.info"
    echo "  genhtml coverage.info --output-directory coverage/html"
    
} > "$COVERAGE_DIR/basic_coverage_report.txt"

# Display the report
print_success "Basic coverage report generated!"
echo ""
cat "$COVERAGE_DIR/basic_coverage_report.txt"

print_success "Coverage analysis complete!"
print_info "Files generated:"
print_info "  - $COVERAGE_DIR/basic_coverage_report.txt" 