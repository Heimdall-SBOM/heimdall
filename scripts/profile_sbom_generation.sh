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

# Heimdall SBOM Generation Profiler
# This script profiles all SBOM generation processes with detailed metrics

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

# Configuration
BUILD_DIR="${1:-build}"
PROFILE_DIR="${BUILD_DIR}/sbom_profiles"
SBOM_DIR="${BUILD_DIR}/sboms"
TEST_BINARY="${BUILD_DIR}/examples/openssl_pthread_demo/openssl_pthread_demo"

# Shared libraries to generate SBOMs for
SHARED_LIBS=(
    "${BUILD_DIR}/lib/heimdall-gold.so"
    "${BUILD_DIR}/lib/heimdall-lld.so"
    "${BUILD_DIR}/lib/libheimdall-core.so.1.0.0"
)

# Create profile output directory
mkdir -p "${PROFILE_DIR}"

print_status "Starting comprehensive SBOM generation profiling"
print_status "Build directory: ${BUILD_DIR}"
print_status "Profile output directory: ${PROFILE_DIR}"
print_status "SBOM output directory: ${SBOM_DIR}"

# Check if profiling tools are available
check_profiling_tools() {
    local profiling_example="${BUILD_DIR}/examples/profiling_example"
    local benchmark_tool="${BUILD_DIR}/src/tools/heimdall-benchmark"
    
    if [[ ! -f "${profiling_example}" ]]; then
        print_error "Profiling example not found: ${profiling_example}"
        print_error "Please build with --profiling flag"
        exit 1
    fi
    
    if [[ ! -f "${benchmark_tool}" ]]; then
        print_error "Benchmark tool not found: ${benchmark_tool}"
        print_error "Please build with --benchmarks flag"
        exit 1
    fi
    
    print_success "Profiling tools available"
}

# Function to profile SBOM generation with detailed metrics
profile_sbom_generation() {
    local plugin_path="$1"
    local format="$2"
    local output_file="$3"
    local binary_path="$4"
    local plugin_name="$5"
    local profile_name="$6"
    local cyclonedx_version="${7:-1.6}"
    local spdx_version="${8:-3.0}"
    
    print_status "Profiling ${plugin_name} ${format} SBOM generation: ${profile_name}"
    
    # Create profile session name
    local session_name="sbom_${plugin_name}_${format}_${profile_name}"
    
    # Start profiling session
    local start_time=$(date +%s.%N)
    local start_memory=$(free -b | grep Mem | awk '{print $3}')
    
    # Build the command
    local cmd=("${BUILD_DIR}/src/tools/heimdall-sbom" "${plugin_path}" "${binary_path}" --format "${format}" --output "${output_file}")
    if [[ "${format}" == cyclonedx* ]]; then
        cmd+=(--cyclonedx-version "${cyclonedx_version}")
    fi
    if [[ "${format}" == spdx* ]]; then
        cmd+=(--spdx-version "${spdx_version}")
    fi
    
    # Run the command and capture output
    local output
    local exit_code
    output=$("${cmd[@]}" 2>&1)
    exit_code=$?
    
    # End profiling session
    local end_time=$(date +%s.%N)
    local end_memory=$(free -b | grep Mem | awk '{print $3}')
    
    # Calculate metrics
    local elapsed_time=$(echo "$end_time - $start_time" | bc -l)
    local memory_used=$(echo "$end_memory - $start_memory" | bc -l)
    local output_size=0
    if [[ -f "${output_file}" ]]; then
        output_size=$(stat -c%s "${output_file}" 2>/dev/null || echo "0")
    fi
    
    # Create profile record
    local profile_record=$(cat <<EOF
{
  "session_name": "${session_name}",
  "plugin_name": "${plugin_name}",
  "format": "${format}",
  "binary_path": "${binary_path}",
  "output_file": "${output_file}",
  "elapsed_seconds": ${elapsed_time},
  "elapsed_milliseconds": $(echo "$elapsed_time * 1000" | bc -l),
  "memory_used_bytes": ${memory_used},
  "memory_used_mb": $(echo "$memory_used / 1024 / 1024" | bc -l),
  "output_size_bytes": ${output_size},
  "output_size_kb": $(echo "$output_size / 1024" | bc -l),
  "exit_code": ${exit_code},
  "success": $([[ $exit_code -eq 0 ]] && echo "true" || echo "false"),
  "command": "${cmd[*]}",
  "output": $(echo "$output" | jq -R .),
  "timestamp": "$(date -Iseconds)"
}
EOF
)
    
    # Save profile record
    echo "$profile_record" > "${PROFILE_DIR}/${session_name}.json"
    
    # Print summary
    if [[ $exit_code -eq 0 ]] && [[ -f "${output_file}" ]]; then
        print_success "✓ ${session_name}: ${elapsed_time}s, ${memory_used} bytes, ${output_size} bytes output"
    else
        print_warning "✗ ${session_name}: ${elapsed_time}s, ${memory_used} bytes (failed)"
    fi
    
    return $exit_code
}

# Function to generate comprehensive SBOM profiles
generate_comprehensive_profiles() {
    print_status "Generating comprehensive SBOM profiles..."
    
    # Profile main binary SBOMs
    print_status "Profiling main binary SBOM generation..."
    
    # LLD Plugin profiles
    if [[ -f "${BUILD_DIR}/lib/heimdall-lld.so" ]]; then
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "spdx" "${SBOM_DIR}/heimdall-build-lld.spdx" "${TEST_BINARY}" "LLD" "main_binary" "3.0"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-lld-v1.4.cyclonedx.json" "${TEST_BINARY}" "LLD" "main_binary_v1.4" "1.4"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-lld-v1.5.cyclonedx.json" "${TEST_BINARY}" "LLD" "main_binary_v1.5" "1.5"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-lld-v1.6.cyclonedx.json" "${TEST_BINARY}" "LLD" "main_binary_v1.6" "1.6"
    else
        print_warning "LLD plugin not available, skipping LLD profiles"
    fi
    
    # Gold Plugin profiles
    if [[ -f "${BUILD_DIR}/lib/heimdall-gold.so" ]]; then
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "spdx" "${SBOM_DIR}/heimdall-build-gold.spdx" "${TEST_BINARY}" "Gold" "main_binary" "3.0"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-gold-v1.4.cyclonedx.json" "${TEST_BINARY}" "Gold" "main_binary_v1.4" "1.4"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-gold-v1.5.cyclonedx.json" "${TEST_BINARY}" "Gold" "main_binary_v1.5" "1.5"
        profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/heimdall-build-gold-v1.6.cyclonedx.json" "${TEST_BINARY}" "Gold" "main_binary_v1.6" "1.6"
    else
        print_warning "Gold plugin not available, skipping Gold profiles"
    fi
    
    # Profile shared library SBOMs
    print_status "Profiling shared library SBOM generation..."
    
    for lib in "${SHARED_LIBS[@]}"; do
        if [[ ! -f "${lib}" ]]; then
            print_warning "Skipping profiling for missing library: ${lib}"
            continue
        fi
        
        # Extract library name for file naming
        lib_name=$(basename "${lib}" | sed 's/\.so.*$//')
        print_status "Profiling shared library: ${lib_name}"
        
        # LLD Plugin profiles for shared library
        if [[ -f "${BUILD_DIR}/lib/heimdall-lld.so" ]]; then
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "spdx" "${SBOM_DIR}/${lib_name}-lld.spdx" "${lib}" "LLD" "${lib_name}" "3.0"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-lld-v1.4.cyclonedx.json" "${lib}" "LLD" "${lib_name}_v1.4" "1.4"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-lld-v1.5.cyclonedx.json" "${lib}" "LLD" "${lib_name}_v1.5" "1.5"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-lld.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-lld-v1.6.cyclonedx.json" "${lib}" "LLD" "${lib_name}_v1.6" "1.6"
        fi
        
        # Gold Plugin profiles for shared library
        if [[ -f "${BUILD_DIR}/lib/heimdall-gold.so" ]]; then
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "spdx" "${SBOM_DIR}/${lib_name}-gold.spdx" "${lib}" "Gold" "${lib_name}" "3.0"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-gold-v1.4.cyclonedx.json" "${lib}" "Gold" "${lib_name}_v1.4" "1.4"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-gold-v1.5.cyclonedx.json" "${lib}" "Gold" "${lib_name}_v1.5" "1.5"
            profile_sbom_generation "${BUILD_DIR}/lib/heimdall-gold.so" "cyclonedx" "${SBOM_DIR}/${lib_name}-gold-v1.6.cyclonedx.json" "${lib}" "Gold" "${lib_name}_v1.6" "1.6"
        fi
    done
}

# Function to generate summary report
generate_summary_report() {
    print_status "Generating comprehensive summary report..."
    
    local summary_file="${PROFILE_DIR}/sbom_generation_summary.json"
    local total_sessions=0
    local successful_sessions=0
    local total_time=0
    local total_memory=0
    local total_output_size=0
    
    # Collect all profile data
    local all_profiles=()
    for profile_file in "${PROFILE_DIR}"/*.json; do
        if [[ -f "${profile_file}" ]]; then
            local profile_data=$(cat "${profile_file}")
            all_profiles+=("$profile_data")
            
            # Extract metrics
            local elapsed=$(echo "$profile_data" | jq -r '.elapsed_seconds // 0')
            local memory=$(echo "$profile_data" | jq -r '.memory_used_bytes // 0')
            local output_size=$(echo "$profile_data" | jq -r '.output_size_bytes // 0')
            local success=$(echo "$profile_data" | jq -r '.success // false')
            
            total_sessions=$((total_sessions + 1))
            total_time=$(echo "$total_time + $elapsed" | bc -l)
            total_memory=$(echo "$total_memory + $memory" | bc -l)
            total_output_size=$(echo "$total_output_size + $output_size" | bc -l)
            
            if [[ "$success" == "true" ]]; then
                successful_sessions=$((successful_sessions + 1))
            fi
        fi
    done
    
    # Create summary report
    local summary_report=$(cat <<EOF
{
  "summary": {
    "total_sessions": ${total_sessions},
    "successful_sessions": ${successful_sessions},
    "failed_sessions": $((total_sessions - successful_sessions)),
    "success_rate": $(echo "scale=2; $successful_sessions * 100 / $total_sessions" | bc -l),
    "total_execution_time_seconds": ${total_time},
    "total_execution_time_minutes": $(echo "scale=2; $total_time / 60" | bc -l),
    "total_memory_used_bytes": ${total_memory},
    "total_memory_used_mb": $(echo "scale=2; $total_memory / 1024 / 1024" | bc -l),
    "total_output_size_bytes": ${total_output_size},
    "total_output_size_mb": $(echo "scale=2; $total_output_size / 1024 / 1024" | bc -l),
    "average_time_per_session": $(echo "scale=6; $total_time / $total_sessions" | bc -l),
    "average_memory_per_session": $(echo "scale=2; $total_memory / $total_sessions" | bc -l),
    "average_output_size_per_session": $(echo "scale=2; $total_output_size / $total_sessions" | bc -l)
  },
  "profiles": [$(IFS=,; echo "${all_profiles[*]}")],
  "timestamp": "$(date -Iseconds)",
  "build_directory": "${BUILD_DIR}",
  "profile_directory": "${PROFILE_DIR}"
}
EOF
)
    
    echo "$summary_report" > "${summary_file}"
    print_success "Summary report saved to: ${summary_file}"
    
    # Print summary to console
    print_status ""
    print_status "=== SBOM Generation Profile Summary ==="
    print_status "Total sessions: ${total_sessions}"
    print_status "Successful sessions: ${successful_sessions}"
    print_status "Failed sessions: $((total_sessions - successful_sessions))"
    print_status "Success rate: $(echo "scale=1; $successful_sessions * 100 / $total_sessions" | bc -l)%"
    print_status "Total execution time: ${total_time} seconds ($(echo "scale=2; $total_time / 60" | bc -l) minutes)"
    print_status "Total memory used: $(echo "scale=2; $total_memory / 1024 / 1024" | bc -l) MB"
    print_status "Total output size: $(echo "scale=2; $total_output_size / 1024 / 1024" | bc -l) MB"
    print_status "Average time per session: $(echo "scale=6; $total_time / $total_sessions" | bc -l) seconds"
    print_status "Average memory per session: $(echo "scale-2; $total_memory / $total_sessions / 1024 / 1024" | bc -l) MB"
    print_status "Profile files: ${PROFILE_DIR}/*.json"
    print_status "Summary report: ${summary_file}"
}

# Function to generate visualizations
generate_visualizations() {
    print_status "Generating visualizations..."
    
    # Check if Python visualizer is available
    local visualizer="../tools/profiling_visualizer.py"
    if [[ ! -f "${visualizer}" ]]; then
        print_warning "Visualizer not found: ${visualizer}"
        print_warning "Skipping visualization generation"
        return
    fi
    
    # Create combined JSON for visualization
    local combined_json="${PROFILE_DIR}/combined_profiles.json"
    local summary_file="${PROFILE_DIR}/sbom_generation_summary.json"
    
    if [[ -f "${summary_file}" ]]; then
        # Convert summary to visualizer format
        local visualizer_data=$(cat <<EOF
{
  "profiler_results": {
    "total_sessions": $(cat "${summary_file}" | jq -r '.summary.total_sessions'),
    "sessions": [
      $(for profile_file in "${PROFILE_DIR}"/*.json; do
          if [[ -f "${profile_file}" ]] && [[ "$(basename "${profile_file}")" != "sbom_generation_summary.json" ]]; then
            local profile_data=$(cat "${profile_file}")
            local session_name=$(echo "$profile_data" | jq -r '.session_name')
            local elapsed=$(echo "$profile_data" | jq -r '.elapsed_seconds')
            local memory=$(echo "$profile_data" | jq -r '.memory_used_bytes')
            local output_size=$(echo "$profile_data" | jq -r '.output_size_bytes')
            local success=$(echo "$profile_data" | jq -r '.success')
            
            cat <<INNEREOF
      {
        "name": "${session_name}",
        "elapsed_seconds": ${elapsed},
        "elapsed_milliseconds": $(echo "$elapsed * 1000" | bc -l),
        "peak_memory": ${memory},
        "metrics": {
          "output_size_bytes": ${output_size},
          "output_size_kb": $(echo "$output_size / 1024" | bc -l),
          "success": ${success},
          "plugin_name": $(echo "$profile_data" | jq -r '.plugin_name'),
          "format": $(echo "$profile_data" | jq -r '.format')
        }
      }
INNEREOF
          fi
        done | paste -sd ',' -)
    ]
  }
}
EOF
)
        
        echo "$visualizer_data" > "${combined_json}"
        
        # Generate visualizations
        python3 "${visualizer}" "${combined_json}" --output-dir "${PROFILE_DIR}/charts"
        print_success "Visualizations generated in: ${PROFILE_DIR}/charts"
    else
        print_warning "Summary file not found, skipping visualization"
    fi
}

# Main execution
main() {
    print_status "Starting SBOM generation profiling..."
    
    # Check prerequisites
    check_profiling_tools
    
    # Check if test binary exists
    if [[ ! -f "${TEST_BINARY}" ]]; then
        print_error "Test binary not found: ${TEST_BINARY}"
        print_error "Please ensure the build completed successfully"
        exit 1
    fi
    
    # Check if SBOM loader exists
    local sbom_loader="${BUILD_DIR}/src/tools/heimdall-sbom"
    if [[ ! -f "${sbom_loader}" ]]; then
        print_error "SBOM loader not found: ${sbom_loader}"
        print_error "Please ensure the build completed successfully"
        exit 1
    fi
    
    # Create SBOM directory
    mkdir -p "${SBOM_DIR}"
    
    # Generate comprehensive profiles
    generate_comprehensive_profiles
    
    # Generate summary report
    generate_summary_report
    
    # Generate visualizations
    generate_visualizations
    
    print_success "SBOM generation profiling completed!"
    print_status "Profile data: ${PROFILE_DIR}"
    print_status "SBOM files: ${SBOM_DIR}"
    print_status "Summary report: ${PROFILE_DIR}/sbom_generation_summary.json"
}

# Run main function
main "$@" 