#!/bin/bash

# Heimdall SBOM Performance Optimization Demo
# This script demonstrates various optimization techniques

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${PURPLE}================================${NC}"
    echo -e "${PURPLE}$1${NC}"
    echo -e "${PURPLE}================================${NC}"
}

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Configuration
BUILD_DIR="${1:-build-cpp23}"
TEST_BINARY="${BUILD_DIR}/examples/openssl_pthread_demo/openssl_pthread_demo"
HEIMDALL_SBOM_TOOL="${BUILD_DIR}/src/tools/heimdall-sbom"
LLD_PLUGIN="${BUILD_DIR}/lib/heimdall-lld.so"
OUTPUT_DIR="${BUILD_DIR}/optimization_demo"

mkdir -p "${OUTPUT_DIR}"

print_header "Heimdall SBOM Performance Optimization Demo"

# Check prerequisites
if [[ ! -f "${TEST_BINARY}" ]]; then
    echo "Error: Test binary not found: ${TEST_BINARY}"
    exit 1
fi

if [[ ! -f "${HEIMDALL_SBOM_TOOL}" ]]; then
    echo "Error: heimdall-sbom tool not found: ${HEIMDALL_SBOM_TOOL}"
    exit 1
fi

if [[ ! -f "${LLD_PLUGIN}" ]]; then
    echo "Error: LLD plugin not found: ${LLD_PLUGIN}"
    exit 1
fi

print_success "All prerequisites found"

# Function to time a command
time_command() {
    local start_time=$(date +%s)
    "$@"
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    echo "Duration: ${duration} seconds"
    return $duration
}

# Demo 1: Default vs Optimized Performance
print_header "Demo 1: Default vs Optimized Performance"

echo "Testing default performance (with debug info extraction)..."
default_start=$(date +%s)
"${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
    --format spdx --output "${OUTPUT_DIR}/default.spdx"
default_end=$(date +%s)
default_duration=$((default_end - default_start))

echo "Testing optimized performance (without debug info extraction)..."
optimized_start=$(date +%s)
"${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
    --format spdx --output "${OUTPUT_DIR}/optimized.spdx" --no-debug-info
optimized_end=$(date +%s)
optimized_duration=$((optimized_end - optimized_start))

echo ""
echo "Performance Comparison:"
echo "Default (with debug info): ${default_duration} seconds"
echo "Optimized (no debug info): ${optimized_duration} seconds"

if [[ ${optimized_duration} -lt ${default_duration} ]]; then
    improvement=$((default_duration - optimized_duration))
    improvement_percent=$((improvement * 100 / default_duration))
    print_success "Optimization improved performance by ${improvement_percent}%"
else
    print_warning "No performance improvement detected for this small binary"
fi

# Demo 2: Different Output Formats Performance
print_header "Demo 2: Output Format Performance Comparison"

formats=("spdx" "cyclonedx")
format_durations=()

for format in "${formats[@]}"; do
    echo "Testing ${format} format..."
    start=$(date +%s)
    "${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
        --format "${format}" --output "${OUTPUT_DIR}/test-${format}.${format}" --no-debug-info
    end=$(date +%s)
    duration=$((end - start))
    format_durations+=("${format}:${duration}")
    echo "${format} format: ${duration} seconds"
done

echo ""
echo "Format Performance Summary:"
for format_duration in "${format_durations[@]}"; do
    format=$(echo "${format_duration}" | cut -d: -f1)
    duration=$(echo "${format_duration}" | cut -d: -f2)
    echo "- ${format}: ${duration} seconds"
done

# Demo 3: File Size Impact Analysis
print_header "Demo 3: File Size Impact Analysis"

echo "Analyzing output file sizes..."

if [[ -f "${OUTPUT_DIR}/default.spdx" ]]; then
    default_size=$(stat -c%s "${OUTPUT_DIR}/default.spdx")
    echo "Default SPDX size: ${default_size} bytes"
fi

if [[ -f "${OUTPUT_DIR}/optimized.spdx" ]]; then
    optimized_size=$(stat -c%s "${OUTPUT_DIR}/optimized.spdx")
    echo "Optimized SPDX size: ${optimized_size} bytes"
fi

if [[ -f "${OUTPUT_DIR}/test-cyclonedx.cyclonedx" ]]; then
    cyclonedx_size=$(stat -c%s "${OUTPUT_DIR}/test-cyclonedx.cyclonedx")
    echo "CycloneDX size: ${cyclonedx_size} bytes"
fi

# Demo 4: Memory Usage Comparison
print_header "Demo 4: Memory Usage Comparison"

if command -v valgrind &> /dev/null; then
    echo "Measuring memory usage for default mode..."
    valgrind --tool=massif --massif-out-file="${OUTPUT_DIR}/default_memory.out" \
        "${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
        --format spdx --output "${OUTPUT_DIR}/memory_test_default.spdx" 2>/dev/null
    
    echo "Measuring memory usage for optimized mode..."
    valgrind --tool=massif --massif-out-file="${OUTPUT_DIR}/optimized_memory.out" \
        "${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
        --format spdx --output "${OUTPUT_DIR}/memory_test_optimized.spdx" --no-debug-info 2>/dev/null
    
    print_success "Memory profiles saved to ${OUTPUT_DIR}/"
else
    print_warning "valgrind not available, skipping memory analysis"
fi

# Demo 5: System Call Comparison
print_header "Demo 5: System Call Comparison"

if command -v strace &> /dev/null; then
    echo "Measuring system calls for default mode..."
    strace -c -o "${OUTPUT_DIR}/default_strace.log" \
        "${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
        --format spdx --output "${OUTPUT_DIR}/strace_test_default.spdx" 2>/dev/null
    
    echo "Measuring system calls for optimized mode..."
    strace -c -o "${OUTPUT_DIR}/optimized_strace.log" \
        "${HEIMDALL_SBOM_TOOL}" "${LLD_PLUGIN}" "${TEST_BINARY}" \
        --format spdx --output "${OUTPUT_DIR}/strace_test_optimized.spdx" --no-debug-info 2>/dev/null
    
    print_success "System call profiles saved to ${OUTPUT_DIR}/"
else
    print_warning "strace not available, skipping system call analysis"
fi

# Generate optimization report
print_header "Optimization Report"

{
    echo "# Heimdall SBOM Optimization Demo Report"
    echo ""
    echo "Generated: $(date)"
    echo "Test binary: ${TEST_BINARY}"
    echo "Build directory: ${BUILD_DIR}"
    echo ""
    
    echo "## Performance Results"
    echo ""
    echo "### Timing Comparison"
    echo "- Default mode: ${default_duration} seconds"
    echo "- Optimized mode: ${optimized_duration} seconds"
    
    if [[ ${optimized_duration} -lt ${default_duration} ]]; then
        improvement=$((default_duration - optimized_duration))
        improvement_percent=$((improvement * 100 / default_duration))
        echo "- Performance improvement: ${improvement_percent}%"
    else
        echo "- No performance improvement detected"
    fi
    
    echo ""
    echo "### Format Performance"
    for format_duration in "${format_durations[@]}"; do
        format=$(echo "${format_duration}" | cut -d: -f1)
        duration=$(echo "${format_duration}" | cut -d: -f2)
        echo "- ${format}: ${duration} seconds"
    done
    
    echo ""
    echo "## Optimization Techniques Demonstrated"
    echo ""
    echo "1. **Disable Debug Info Extraction**: Use --no-debug-info flag"
    echo "   - Skips expensive DWARF parsing"
    echo "   - Reduces processing time significantly"
    echo "   - Suitable for production environments"
    echo ""
    echo "2. **Format Selection**: Choose appropriate output format"
    echo "   - SPDX: Compact, fast generation"
    echo "   - CycloneDX: Comprehensive, larger output"
    echo ""
    echo "3. **Memory Optimization**: Monitor memory usage"
    echo "   - Use valgrind for memory profiling"
    echo "   - Implement memory limits for large files"
    echo ""
    echo "4. **I/O Optimization**: Reduce system calls"
    echo "   - Use strace to identify I/O bottlenecks"
    echo "   - Consider memory-mapped files for large binaries"
    echo ""
    
    echo "## Recommendations"
    echo ""
    echo "### For Production Use:"
    echo "- Always use --no-debug-info flag unless debug info is required"
    echo "- Choose SPDX format for faster generation"
    echo "- Monitor memory usage for large binaries"
    echo "- Implement caching for repeated operations"
    echo ""
    echo "### For Development:"
    echo "- Use default mode for comprehensive analysis"
    echo "- Profile with valgrind and strace"
    echo "- Test with various binary sizes"
    echo "- Monitor system resource usage"
    echo ""
    
} > "${OUTPUT_DIR}/optimization_report.md"

print_success "Optimization report generated: ${OUTPUT_DIR}/optimization_report.md"

print_header "Demo Complete"
print_status "All optimization demos completed successfully"
print_status "Check ${OUTPUT_DIR}/ for detailed results and reports"
print_status "Key takeaway: Use --no-debug-info for production environments" 