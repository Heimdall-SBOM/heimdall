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

# Heimdall SBOM Generation Profiler (Simplified)
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

# Function to calculate time difference using Python
calculate_time_diff() {
    local start_time="$1"
    local end_time="$2"
    python3 -c "print($end_time - $start_time)"
}

# Function to calculate memory difference using Python
calculate_memory_diff() {
    local start_memory="$1"
    local end_memory="$2"
    python3 -c "print($end_memory - $start_memory)"
}

# Function to convert bytes to MB using Python
bytes_to_mb() {
    local bytes="$1"
    python3 -c "print($bytes / 1024 / 1024)"
}

# Function to convert bytes to KB using Python
bytes_to_kb() {
    local bytes="$1"
    python3 -c "print($bytes / 1024)"
}

# Function to convert seconds to milliseconds using Python
seconds_to_ms() {
    local seconds="$1"
    python3 -c "print($seconds * 1000)"
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
    
    # Calculate metrics using Python
    local elapsed_time=$(calculate_time_diff "$start_time" "$end_time")
    local memory_used=$(calculate_memory_diff "$start_memory" "$end_memory")
    local memory_mb=$(bytes_to_mb "$memory_used")
    local output_size=0
    if [[ -f "${output_file}" ]]; then
        output_size=$(stat -c%s "${output_file}" 2>/dev/null || echo "0")
    fi
    local output_kb=$(bytes_to_kb "$output_size")
    local elapsed_ms=$(seconds_to_ms "$elapsed_time")
    
    # Create profile record
    local profile_record=$(cat <<EOF
{
  "session_name": "${session_name}",
  "plugin_name": "${plugin_name}",
  "format": "${format}",
  "binary_path": "${binary_path}",
  "output_file": "${output_file}",
  "elapsed_seconds": ${elapsed_time},
  "elapsed_milliseconds": ${elapsed_ms},
  "memory_used_bytes": ${memory_used},
  "memory_used_mb": ${memory_mb},
  "output_size_bytes": ${output_size},
  "output_size_kb": ${output_kb},
  "exit_code": ${exit_code},
  "success": $([[ $exit_code -eq 0 ]] && echo "true" || echo "false"),
  "command": "${cmd[*]}",
  "output": $(echo "$output" | python3 -c "import sys, json; print(json.dumps(sys.stdin.read().strip()))"),
  "timestamp": "$(date -Iseconds)"
}
EOF
)
    
    # Save profile record
    echo "$profile_record" > "${PROFILE_DIR}/${session_name}.json"
    
    # Print summary
    if [[ $exit_code -eq 0 ]] && [[ -f "${output_file}" ]]; then
        print_success "✓ ${session_name}: ${elapsed_time}s, ${memory_mb}MB, ${output_kb}KB output"
    else
        print_warning "✗ ${session_name}: ${elapsed_time}s, ${memory_mb}MB (failed)"
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

# Function to generate summary report using Python
generate_summary_report() {
    print_status "Generating comprehensive summary report..."
    
    local summary_file="${PROFILE_DIR}/sbom_generation_summary.json"
    
    # Use Python to process all profile files and generate summary
    python3 -c "
import json
import os
import glob

# Collect all profile data
profiles = []
total_sessions = 0
successful_sessions = 0
total_time = 0.0
total_memory = 0.0
total_output_size = 0

# Read all profile files
for profile_file in glob.glob('${PROFILE_DIR}/*.json'):
    if os.path.basename(profile_file) != 'sbom_generation_summary.json':
        try:
            with open(profile_file, 'r') as f:
                profile_data = json.load(f)
                profiles.append(profile_data)
                
                # Extract metrics
                elapsed = profile_data.get('elapsed_seconds', 0)
                memory = profile_data.get('memory_used_bytes', 0)
                output_size = profile_data.get('output_size_bytes', 0)
                success = profile_data.get('success', False)
                
                total_sessions += 1
                total_time += elapsed
                total_memory += memory
                total_output_size += output_size
                
                if success:
                    successful_sessions += 1
        except Exception as e:
            print(f'Error reading {profile_file}: {e}')

# Calculate summary statistics
failed_sessions = total_sessions - successful_sessions
success_rate = (successful_sessions * 100 / total_sessions) if total_sessions > 0 else 0
total_time_minutes = total_time / 60
total_memory_mb = total_memory / 1024 / 1024
total_output_size_mb = total_output_size / 1024 / 1024
avg_time_per_session = total_time / total_sessions if total_sessions > 0 else 0
avg_memory_per_session = total_memory / total_sessions if total_sessions > 0 else 0
avg_output_size_per_session = total_output_size / total_sessions if total_sessions > 0 else 0

# Create summary report
summary_report = {
    'summary': {
        'total_sessions': total_sessions,
        'successful_sessions': successful_sessions,
        'failed_sessions': failed_sessions,
        'success_rate': success_rate,
        'total_execution_time_seconds': total_time,
        'total_execution_time_minutes': total_time_minutes,
        'total_memory_used_bytes': total_memory,
        'total_memory_used_mb': total_memory_mb,
        'total_output_size_bytes': total_output_size,
        'total_output_size_mb': total_output_size_mb,
        'average_time_per_session': avg_time_per_session,
        'average_memory_per_session': avg_memory_per_session,
        'average_output_size_per_session': avg_output_size_per_session
    },
    'profiles': profiles,
    'timestamp': '$(date -Iseconds)',
    'build_directory': '${BUILD_DIR}',
    'profile_directory': '${PROFILE_DIR}'
}

# Save summary report
with open('${summary_file}', 'w') as f:
    json.dump(summary_report, f, indent=2)

print(f'Summary report saved to: {summary_file}')

# Print summary to console
print('')
print('=== SBOM Generation Profile Summary ===')
print(f'Total sessions: {total_sessions}')
print(f'Successful sessions: {successful_sessions}')
print(f'Failed sessions: {failed_sessions}')
print(f'Success rate: {success_rate:.1f}%')
print(f'Total execution time: {total_time:.6f} seconds ({total_time_minutes:.2f} minutes)')
print(f'Total memory used: {total_memory_mb:.2f} MB')
print(f'Total output size: {total_output_size_mb:.2f} MB')
print(f'Average time per session: {avg_time_per_session:.6f} seconds')
print(f'Average memory per session: {avg_memory_per_session / 1024 / 1024:.2f} MB')
print(f'Profile files: {len(profiles)} files')
print(f'Summary report: {summary_file}')
"
    
    print_success "Summary report saved to: ${summary_file}"
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
        # Use Python to convert summary to visualizer format
        python3 -c "
import json
import os

# Read summary file
with open('${summary_file}', 'r') as f:
    summary_data = json.load(f)

# Convert to visualizer format
visualizer_data = {
    'profiler_results': {
        'total_sessions': summary_data['summary']['total_sessions'],
        'sessions': []
    }
}

# Convert profiles to visualizer format
for profile in summary_data['profiles']:
    session = {
        'name': profile['session_name'],
        'elapsed_seconds': profile['elapsed_seconds'],
        'elapsed_milliseconds': profile['elapsed_milliseconds'],
        'peak_memory': profile['memory_used_bytes'],
        'metrics': {
            'output_size_bytes': profile['output_size_bytes'],
            'output_size_kb': profile['output_size_kb'],
            'success': profile['success'],
            'plugin_name': profile['plugin_name'],
            'format': profile['format']
        }
    }
    visualizer_data['profiler_results']['sessions'].append(session)

# Save combined data
with open('${combined_json}', 'w') as f:
    json.dump(visualizer_data, f, indent=2)

print(f'Combined data saved to: {combined_json}')
"
        
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