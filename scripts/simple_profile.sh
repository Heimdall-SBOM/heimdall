#!/bin/bash

# Simple Heimdall SBOM Performance Profiling Script
# This script helps identify performance bottlenecks without external dependencies

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
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

print_header() {
    echo -e "${PURPLE}================================${NC}"
    echo -e "${PURPLE}$1${NC}"
    echo -e "${PURPLE}================================${NC}"
}

print_subheader() {
    echo -e "${CYAN}--- $1 ---${NC}"
}

# Configuration
BUILD_DIR="${1:-build-cpp23}"
TEST_BINARY="${BUILD_DIR}/examples/openssl_pthread_demo/openssl_pthread_demo"
PROFILE_DIR="${BUILD_DIR}/profiles"
SBOM_DIR="${BUILD_DIR}/sboms"
HEIMDALL_SBOM_TOOL="${BUILD_DIR}/src/tools/heimdall-sbom"
LLD_PLUGIN="${BUILD_DIR}/lib/heimdall-lld.so"
GOLD_PLUGIN="${BUILD_DIR}/lib/heimdall-gold.so"

# Create directories
mkdir -p "${PROFILE_DIR}"
mkdir -p "${SBOM_DIR}"

print_header "Heimdall SBOM Performance Profiling"
print_status "Build directory: ${BUILD_DIR}"
print_status "Test binary: ${TEST_BINARY}"
print_status "Profile directory: ${PROFILE_DIR}"
print_status "SBOM directory: ${SBOM_DIR}"

# Check prerequisites
check_prerequisites() {
    print_subheader "Checking Prerequisites"
    
    # Check if test binary exists
    if [[ ! -f "${TEST_BINARY}" ]]; then
        print_error "Test binary not found: ${TEST_BINARY}"
        print_error "Please ensure the build completed successfully"
        exit 1
    fi
    print_success "Found test binary: ${TEST_BINARY}"
    
    # Check if heimdall-sbom tool exists
    if [[ ! -f "${HEIMDALL_SBOM_TOOL}" ]]; then
        print_error "heimdall-sbom tool not found: ${HEIMDALL_SBOM_TOOL}"
        print_error "Please ensure the build completed successfully"
        exit 1
    fi
    print_success "Found heimdall-sbom tool: ${HEIMDALL_SBOM_TOOL}"
    
    # Check if plugins exist
    if [[ ! -f "${LLD_PLUGIN}" ]]; then
        print_warning "LLD plugin not found: ${LLD_PLUGIN}"
    else
        print_success "Found LLD plugin: ${LLD_PLUGIN}"
    fi
    
    if [[ ! -f "${GOLD_PLUGIN}" ]]; then
        print_warning "Gold plugin not found: ${GOLD_PLUGIN}"
    else
        print_success "Found Gold plugin: ${GOLD_PLUGIN}"
    fi
}

# Simple timing function using date
time_command() {
    local start_time=$(date +%s)
    "$@"
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    echo "Duration: ${duration} seconds"
}

# Profile basic timing
profile_basic_timing() {
    print_subheader "Basic Timing Profile"
    
    local plugin_path="$1"
    local plugin_name="$2"
    local format="$3"
    local output_file="$4"
    
    print_status "Profiling ${plugin_name} ${format} SBOM generation..."
    
    # Time the entire process
    local start_time=$(date +%s)
    
    # Run the command and capture output
    "${HEIMDALL_SBOM_TOOL}" "${plugin_path}" "${TEST_BINARY}" \
        --format "${format}" --output "${output_file}" 2>&1 | tee "${PROFILE_DIR}/${plugin_name}_${format}_timing.log"
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    echo "Total time: ${duration} seconds" >> "${PROFILE_DIR}/${plugin_name}_${format}_timing.log"
    
    if [[ -f "${output_file}" ]]; then
        print_success "Generated ${plugin_name} ${format} SBOM: ${output_file}"
        local file_size=$(stat -c%s "${output_file}")
        echo "Output file size: ${file_size} bytes" >> "${PROFILE_DIR}/${plugin_name}_${format}_timing.log"
        print_status "File size: ${file_size} bytes"
    else
        print_warning "${plugin_name} ${format} SBOM generation may have failed"
    fi
    
    print_status "Duration: ${duration} seconds"
    
    # Performance assessment
    if [[ ${duration} -gt 10 ]]; then
        print_warning "SLOW: Duration exceeds 10 seconds"
    elif [[ ${duration} -gt 5 ]]; then
        print_warning "MODERATE: Duration exceeds 5 seconds"
    elif [[ ${duration} -gt 2 ]]; then
        print_warning "SLIGHTLY SLOW: Duration exceeds 2 seconds"
    else
        print_success "FAST: Duration under 2 seconds"
    fi
}

# Profile with strace (system calls)
profile_system_calls() {
    print_subheader "System Call Profile"
    
    local plugin_path="$1"
    local plugin_name="$2"
    local format="$3"
    local output_file="$4"
    
    print_status "Profiling system calls for ${plugin_name} ${format}..."
    
    # Use strace to track system calls
    strace -c -o "${PROFILE_DIR}/${plugin_name}_${format}_strace.log" \
        "${HEIMDALL_SBOM_TOOL}" "${plugin_path}" "${TEST_BINARY}" \
        --format "${format}" --output "${output_file}"
    
    print_success "System call profile saved: ${PROFILE_DIR}/${plugin_name}_${format}_strace.log"
}

# Profile with valgrind (memory usage)
profile_memory_usage() {
    print_subheader "Memory Usage Profile"
    
    local plugin_path="$1"
    local plugin_name="$2"
    local format="$3"
    local output_file="$4"
    
    print_status "Profiling memory usage for ${plugin_name} ${format}..."
    
    # Use valgrind to track memory usage
    valgrind --tool=massif --massif-out-file="${PROFILE_DIR}/${plugin_name}_${format}_massif.out" \
        "${HEIMDALL_SBOM_TOOL}" "${plugin_path}" "${TEST_BINARY}" \
        --format "${format}" --output "${output_file}"
    
    # Convert to text format
    ms_print "${PROFILE_DIR}/${plugin_name}_${format}_massif.out" > "${PROFILE_DIR}/${plugin_name}_${format}_memory.log"
    
    print_success "Memory profile saved: ${PROFILE_DIR}/${plugin_name}_${format}_memory.log"
}

# Profile individual components
profile_components() {
    print_subheader "Component-Level Profile"
    
    local plugin_path="$1"
    local plugin_name="$2"
    local format="$3"
    local output_file="$4"
    
    print_status "Profiling individual components for ${plugin_name} ${format}..."
    
    # Profile with detailed timing for each step
    {
        echo "=== Component-Level Profile for ${plugin_name} ${format} ==="
        echo "Timestamp: $(date)"
        echo "Test binary: ${TEST_BINARY}"
        echo "Plugin: ${plugin_path}"
        echo "Format: ${format}"
        echo "Output: ${output_file}"
        echo ""
        
        # Time the actual processing
        echo "=== Processing Time ==="
        local start=$(date +%s)
        "${HEIMDALL_SBOM_TOOL}" "${plugin_path}" "${TEST_BINARY}" \
            --format "${format}" --output "${output_file}"
        local end=$(date +%s)
        local duration=$((end - start))
        echo "Total processing time: ${duration} seconds"
        echo ""
        
        # Analyze output file
        if [[ -f "${output_file}" ]]; then
            echo "=== Output Analysis ==="
            local file_size=$(stat -c%s "${output_file}")
            echo "Output file size: ${file_size} bytes"
            
            # Count components if it's JSON
            if [[ "${format}" == *"cyclonedx"* ]] || [[ "${format}" == *"spdx"* ]]; then
                local component_count=$(grep -c '"name"' "${output_file}" 2>/dev/null || echo "0")
                echo "Component count: ${component_count}"
            fi
        fi
        
    } > "${PROFILE_DIR}/${plugin_name}_${format}_components.log"
    
    print_success "Component profile saved: ${PROFILE_DIR}/${plugin_name}_${format}_components.log"
}

# Generate summary report
generate_summary_report() {
    print_subheader "Generating Summary Report"
    
    local summary_file="${PROFILE_DIR}/performance_summary.md"
    
    {
        echo "# Heimdall SBOM Performance Profile Summary"
        echo ""
        echo "Generated: $(date)"
        echo "Test binary: ${TEST_BINARY}"
        echo "Build directory: ${BUILD_DIR}"
        echo ""
        
        echo "## Available Plugins"
        if [[ -f "${LLD_PLUGIN}" ]]; then
            echo "- ✅ LLD Plugin: ${LLD_PLUGIN}"
        else
            echo "- ❌ LLD Plugin: Not found"
        fi
        
        if [[ -f "${GOLD_PLUGIN}" ]]; then
            echo "- ✅ Gold Plugin: ${GOLD_PLUGIN}"
        else
            echo "- ❌ Gold Plugin: Not found"
        fi
        echo ""
        
        echo "## Profile Files Generated"
        echo ""
        for file in "${PROFILE_DIR}"/*.log; do
            if [[ -f "${file}" ]]; then
                local basename=$(basename "${file}")
                echo "- \`${basename}\`"
            fi
        done
        echo ""
        
        echo "## SBOM Files Generated"
        echo ""
        for file in "${SBOM_DIR}"/*.json "${SBOM_DIR}"/*.spdx; do
            if [[ -f "${file}" ]]; then
                local basename=$(basename "${file}")
                local file_size=$(stat -c%s "${file}")
                echo "- \`${basename}\` (${file_size} bytes)"
            fi
        done
        echo ""
        
        echo "## Performance Analysis"
        echo ""
        echo "### Common Bottlenecks"
        echo ""
        echo "1. **DWARF Debug Information Extraction**: The most time-consuming operation"
        echo "   - LLVM DWARF parsing can be slow for large binaries"
        echo "   - Heuristic fallback parsing is also expensive"
        echo "   - Consider disabling debug info extraction with \`--no-debug-info\`"
        echo ""
        echo "2. **Symbol Table Processing**: Can be slow for binaries with many symbols"
        echo "   - ELF symbol table parsing"
        echo "   - Archive member extraction"
        echo ""
        echo "3. **File I/O Operations**: Multiple file reads during metadata extraction"
        echo "   - Binary file reading"
        echo "   - Section table parsing"
        echo "   - Dependency resolution"
        echo ""
        echo "### Optimization Suggestions"
        echo ""
        echo "1. **Disable Debug Info**: Use \`--no-debug-info\` flag"
        echo "2. **Limit Symbol Extraction**: Process only essential symbols"
        echo "3. **Cache Results**: Implement caching for repeated operations"
        echo "4. **Parallel Processing**: Process multiple files in parallel (with caution)"
        echo "5. **Selective Extraction**: Extract only required metadata types"
        echo ""
        
    } > "${summary_file}"
    
    print_success "Summary report generated: ${summary_file}"
}

# Quick profiling for immediate feedback
run_quick_profile() {
    print_subheader "Quick Performance Profile"
    
    local plugin_path="$1"
    local plugin_name="$2"
    
    if [[ ! -f "${plugin_path}" ]]; then
        print_warning "${plugin_name} plugin not found, skipping quick profile"
        return
    fi
    
    print_status "Running quick profile for ${plugin_name}..."
    
    # Simple timing test
    local start_time=$(date +%s)
    
    "${HEIMDALL_SBOM_TOOL}" "${plugin_path}" "${TEST_BINARY}" \
        --format "spdx" --output "${SBOM_DIR}/quick-${plugin_name}.spdx"
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    print_success "${plugin_name} quick profile completed in ${duration} seconds"
    
    # Check output file
    if [[ -f "${SBOM_DIR}/quick-${plugin_name}.spdx" ]]; then
        local file_size=$(stat -c%s "${SBOM_DIR}/quick-${plugin_name}.spdx")
        print_success "Generated SBOM: ${file_size} bytes"
    else
        print_warning "SBOM generation may have failed"
    fi
}

# Main profiling function
run_comprehensive_profiling() {
    print_subheader "Running Comprehensive Profiling"
    
    # Profile LLD plugin if available
    if [[ -f "${LLD_PLUGIN}" ]]; then
        print_status "Profiling LLD plugin..."
        
        # Basic timing
        profile_basic_timing "${LLD_PLUGIN}" "LLD" "spdx" "${SBOM_DIR}/heimdall-lld-profile.spdx"
        profile_basic_timing "${LLD_PLUGIN}" "LLD" "cyclonedx" "${SBOM_DIR}/heimdall-lld-profile.cyclonedx.json"
        
        # System call profiling
        if command -v strace &> /dev/null; then
            profile_system_calls "${LLD_PLUGIN}" "LLD" "spdx" "${SBOM_DIR}/heimdall-lld-strace.spdx"
        fi
        
        # Memory profiling
        if command -v valgrind &> /dev/null; then
            profile_memory_usage "${LLD_PLUGIN}" "LLD" "cyclonedx" "${SBOM_DIR}/heimdall-lld-memory.cyclonedx.json"
        fi
        
        # Component-level profiling
        profile_components "${LLD_PLUGIN}" "LLD" "spdx" "${SBOM_DIR}/heimdall-lld-components.spdx"
    fi
    
    # Profile Gold plugin if available
    if [[ -f "${GOLD_PLUGIN}" ]]; then
        print_status "Profiling Gold plugin..."
        
        # Basic timing
        profile_basic_timing "${GOLD_PLUGIN}" "Gold" "spdx" "${SBOM_DIR}/heimdall-gold-profile.spdx"
        profile_basic_timing "${GOLD_PLUGIN}" "Gold" "cyclonedx" "${SBOM_DIR}/heimdall-gold-profile.cyclonedx.json"
        
        # System call profiling
        if command -v strace &> /dev/null; then
            profile_system_calls "${GOLD_PLUGIN}" "Gold" "spdx" "${SBOM_DIR}/heimdall-gold-strace.spdx"
        fi
        
        # Memory profiling
        if command -v valgrind &> /dev/null; then
            profile_memory_usage "${GOLD_PLUGIN}" "Gold" "cyclonedx" "${SBOM_DIR}/heimdall-gold-memory.cyclonedx.json"
        fi
        
        # Component-level profiling
        profile_components "${GOLD_PLUGIN}" "Gold" "spdx" "${SBOM_DIR}/heimdall-gold-components.spdx"
    fi
}

# Main execution
main() {
    print_header "Starting Heimdall SBOM Performance Profiling"
    
    # Check prerequisites
    check_prerequisites
    
    # Run quick profiles first for immediate feedback
    print_subheader "Quick Profiles"
    run_quick_profile "${LLD_PLUGIN}" "LLD"
    run_quick_profile "${GOLD_PLUGIN}" "Gold"
    
    # Ask user if they want comprehensive profiling
    echo ""
    read -p "Run comprehensive profiling? This will take several minutes. (y/N): " -n 1 -r
    echo ""
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        run_comprehensive_profiling
        generate_summary_report
    fi
    
    print_header "Profiling Complete"
    print_status "Profile data: ${PROFILE_DIR}"
    print_status "SBOM files: ${SBOM_DIR}"
    print_status "Check the generated logs for detailed performance analysis"
}

# Run main function
main "$@" 