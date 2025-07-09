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

# Standards to test
STANDARDS=("11" "14" "17" "20" "23")

print_status "Building and testing all C++ standards: ${STANDARDS[*]}"

for standard in "${STANDARDS[@]}"; do
    print_status "Building C++${standard}..."
    
    # Create build directory
    BUILD_DIR="build-cpp${standard}"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    
    # Configure and build
    cd "${BUILD_DIR}"
    
    if [[ "${standard}" == "11" || "${standard}" == "14" ]]; then
        # Use compatibility mode for C++11/14
        cmake .. -DCMAKE_CXX_STANDARD="${standard}" -DCMAKE_CXX_STANDARD_REQUIRED=ON -DHEIMDALL_CXX11_14_MODE=ON
    else
        # Standard build for C++17+
        cmake .. -DCMAKE_CXX_STANDARD="${standard}" -DCMAKE_CXX_STANDARD_REQUIRED=ON
    fi
    
    make -j$(nproc)
    
    # Run tests
    print_status "Running tests for C++${standard}..."
    ctest --output-on-failure
    
    # Generate SBOMs
    print_status "Generating SBOMs for C++${standard}..."
    ../scripts/generate_build_sboms.sh .
    
    cd ..
    
    print_success "C++${standard} build, test, and SBOM generation completed"
    echo ""
done

print_success "All C++ standards built and tested successfully!"
print_status "Build directories:"
for standard in "${STANDARDS[@]}"; do
    echo "  - build-cpp${standard}/"
done 