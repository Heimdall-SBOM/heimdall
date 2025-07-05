#!/bin/bash

# Heimdall Test Coverage Script
# This script generates code coverage reports for the Heimdall project

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
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
    print_error "This script must be run from the project root directory"
    exit 1
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    print_error "Build directory not found. Please run './build.sh' first"
    exit 1
fi

cd build

# Check if coverage was enabled
if [ ! -f "CMakeCache.txt" ] || ! grep -q "ENABLE_COVERAGE:BOOL=ON" CMakeCache.txt; then
    print_warning "Coverage not enabled. Reconfiguring with coverage enabled..."
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

# Find all .gcda files
print_info "Finding coverage data files..."
GCDA_FILES=$(find . -name "*.gcda" 2>/dev/null || true)

if [ -z "$GCDA_FILES" ]; then
    print_error "No coverage data files found. Make sure tests were run successfully."
    exit 1
fi

print_info "Found coverage data files:"
echo "$GCDA_FILES"

# Generate coverage reports
print_info "Generating coverage reports..."

# Generate gcov reports for all source files
for gcda_file in $GCDA_FILES; do
    gcov_file="${gcda_file%.gcda}.gcov"
    print_info "Processing $gcda_file..."
    
    # Try different gcov approaches
    if gcov -r -b -s "$(pwd)/.." "$gcda_file" > /dev/null 2>&1; then
        print_info "Generated coverage for $gcda_file"
    elif gcov -r -b "$gcda_file" > /dev/null 2>&1; then
        print_info "Generated coverage for $gcda_file (without source path)"
    else
        print_warning "Failed to generate coverage for $gcda_file"
    fi
done

# Move all .gcov files to coverage directory
find . -name "*.gcov" -exec mv {} "$COVERAGE_DIR/" \; 2>/dev/null || true

# Alternative coverage generation for main source files
print_info "Generating coverage for main source files..."
MAIN_SOURCES=(
    "CMakeFiles/heimdall-core.dir/src/common/SBOMGenerator.cpp"
    "CMakeFiles/heimdall-core.dir/src/common/ComponentInfo.cpp"
    "CMakeFiles/heimdall-core.dir/src/common/MetadataExtractor.cpp"
    "CMakeFiles/heimdall-core.dir/src/common/Utils.cpp"
    "CMakeFiles/heimdall-core.dir/src/common/DWARFExtractor.cpp"
    "CMakeFiles/heimdall-core.dir/src/common/PluginInterface.cpp"
)

for source in "${MAIN_SOURCES[@]}"; do
    if [ -f "${source}.gcno" ]; then
        print_info "Processing $source..."
        gcov -r -b -s "$(pwd)/.." "${source}.gcno" > /dev/null 2>&1 || true
    fi
done

# Move any additional .gcov files
find . -name "*.gcov" -exec mv {} "$COVERAGE_DIR/" \; 2>/dev/null || true

# Generate summary report
print_info "Generating coverage summary..."
{
    echo "Heimdall Code Coverage Report"
    echo "============================="
    echo "Generated: $(date)"
    echo ""
    echo "Coverage Summary:"
    echo "================="
    
    # Process each .gcov file and extract coverage information
    total_lines=0
    covered_lines=0
    
    for gcov_file in "$COVERAGE_DIR"/*.gcov; do
        if [ -f "$gcov_file" ]; then
            filename=$(basename "$gcov_file" .gcov)
            echo ""
            echo "File: $filename"
            echo "----------------------------------------"
            
            # Count lines and coverage
            file_lines=0
            file_covered=0
            
            while IFS= read -r line; do
                if [[ $line =~ ^[[:space:]]*([0-9]+):[[:space:]]*([0-9]+): ]]; then
                    count="${BASH_REMATCH[1]}"
                    line_num="${BASH_REMATCH[2]}"
                    
                    if [ "$count" != "#####" ] && [ "$count" -gt 0 ]; then
                        ((file_covered++))
                    fi
                    ((file_lines++))
                fi
            done < "$gcov_file"
            
            if [ $file_lines -gt 0 ]; then
                coverage_percent=$((file_covered * 100 / file_lines))
                echo "Lines: $file_lines, Covered: $file_covered, Coverage: ${coverage_percent}%"
                ((total_lines += file_lines))
                ((covered_lines += file_covered))
            fi
        fi
    done
    
    echo ""
    echo "Overall Coverage:"
    echo "================="
    if [ $total_lines -gt 0 ]; then
        overall_coverage=$((covered_lines * 100 / total_lines))
        echo "Total Lines: $total_lines"
        echo "Covered Lines: $covered_lines"
        echo "Overall Coverage: ${overall_coverage}%"
    else
        echo "No coverage data available"
    fi
    
} > "$COVERAGE_DIR/coverage_summary.txt"

# Display summary
print_success "Coverage report generated in $COVERAGE_DIR/"
echo ""
echo "Coverage Summary:"
cat "$COVERAGE_DIR/coverage_summary.txt"

# Try to generate HTML report if lcov is available
if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
    print_info "Generating HTML coverage report..."
    
    # Create lcov info file
    lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" --quiet
    
    # Generate HTML report
    genhtml "$COVERAGE_DIR/coverage.info" --output-directory "$COVERAGE_DIR/html" --quiet
    
    print_success "HTML coverage report generated in $COVERAGE_DIR/html/"
    print_info "Open $COVERAGE_DIR/html/index.html in your browser to view the report"
else
    print_warning "lcov/genhtml not found. HTML report not generated."
    print_info "Install lcov to generate HTML coverage reports:"
    print_info "  Ubuntu/Debian: sudo apt-get install lcov"
    print_info "  CentOS/RHEL: sudo yum install lcov"
    print_info "  macOS: brew install lcov"
fi

print_success "Coverage analysis complete!"
print_info "Files generated:"
print_info "  - $COVERAGE_DIR/coverage_summary.txt (Text summary)"
if [ -d "$COVERAGE_DIR/html" ]; then
    print_info "  - $COVERAGE_DIR/html/ (HTML report)"
fi 