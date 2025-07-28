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

# Build and test all supported C++ standards
# set -e  # Removed to allow manual error handling

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

# Standards to test
STANDARDS=("11" "14" "17" "20" "23")

# Log directory
LOG_DIR="logs"

# Create log directory if it doesn't exist
if [ ! -d "$LOG_DIR" ]; then
    mkdir -p "$LOG_DIR"
fi

print_status "Building and testing all C++ standards: ${STANDARDS[*]}"
print_status "NOTE: This will take awhile time, go grab a coffee"

declare -A build_results

for standard in "${STANDARDS[@]}"; do
    print_status "Building GCC C++${standard} and CLANG C++${standard}..."

    # Create log files for this build
    LOG_FILE_GCC="${LOG_DIR}/gcc_cpp${standard}.log"
    LOG_FILE_CLANG="${LOG_DIR}/clang_cpp${standard}.log"

    # Run the build scripts in parallel
    (
        if ./scripts/build.sh --standard $standard --compiler gcc --tests > "$LOG_FILE_GCC" 2>&1; then
            build_results["GCC C++${standard}"]="success"
            print_success "GCC C++${standard} build, test, and SBOM generation completed"
        else
            build_results["GCC C++${standard}"]="failure"
            print_error "GCC C++${standard} build failed"
        fi
    ) &
    (
       if ./scripts/build.sh --standard $standard --compiler clang --tests > "$LOG_FILE_CLANG" 2>&1; then
            build_results["CLANG C++${standard}"]="success"
            print_success "CLANG C++${standard} build, test, and SBOM generation completed"
        else
            build_results["CLANG C++${standard}"]="failure"
            print_error "CLANG C++${standard} build failed"
        fi
    ) &
    wait
done

print_status "Build summary:"
for standard in "${STANDARDS[@]}"; do
    
    if cat logs/gcc_cpp$standard.log | grep "Heimdall C++$standard build completed successfully!" > /dev/null; then
        print_success "GCC C++${standard}"
    else
        print_error "GCC C++${standard} build failed"
    fi

    if cat logs/clang_cpp$standard.log | grep "Heimdall C++$standard build completed successfully!" > /dev/null; then
        print_success "CLANG C++${standard}"
    else
        print_error "CLANG C++${standard} build failed"
    fi

done

print_status "Build directories:"
for standard in "${STANDARDS[@]}"; do
    echo "  - build-cpp${standard}/"
done

print_status "Log files can be found in the ${LOG_DIR} directory"
