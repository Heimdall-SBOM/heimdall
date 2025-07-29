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

# Heimdall Signed SBOM Validation Script
# This script generates a signed SBOM of the heimdall-sbom tool and validates it with both Hoppr and heimdall-validate

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
BUILD_DIR="${1:-build-gcc-cpp23}"
PLUGIN_PATH="${BUILD_DIR}/lib/heimdall-lld.so"
BINARY_PATH="${BUILD_DIR}/src/tools/heimdall-sbom"
VALIDATE_PATH="${BUILD_DIR}/src/tools/heimdall-validate"
OUTPUT_FILE="heimdall-sbom-tool-signed.cdx.json"

# Create test keys directory
TEST_KEYS_DIR="test_keys"
mkdir -p "${TEST_KEYS_DIR}"

print_status "Starting signed SBOM generation and validation"
print_status "Build directory: ${BUILD_DIR}"
print_status "Plugin path: ${PLUGIN_PATH}"
print_status "Binary path: ${BINARY_PATH}"
print_status "Validate path: ${VALIDATE_PATH}"

# Check if required files exist
if [[ ! -f "${PLUGIN_PATH}" ]]; then
    print_error "LLD plugin not found: ${PLUGIN_PATH}"
    exit 1
fi

if [[ ! -f "${BINARY_PATH}" ]]; then
    print_error "heimdall-sbom binary not found: ${BINARY_PATH}"
    exit 1
fi

if [[ ! -f "${VALIDATE_PATH}" ]]; then
    print_error "heimdall-validate binary not found: ${VALIDATE_PATH}"
    exit 1
fi

print_success "All required binaries found"

# Generate Ed25519 test keys if they don't exist
PRIVATE_KEY_PATH="${TEST_KEYS_DIR}/heimdall_ed25519_private.pem"
PUBLIC_KEY_PATH="${TEST_KEYS_DIR}/heimdall_ed25519_public.pem"

if [[ ! -f "${PRIVATE_KEY_PATH}" ]] || [[ ! -f "${PUBLIC_KEY_PATH}" ]]; then
    print_status "Generating Ed25519 test keys..."
    
    # Generate private key
    openssl genpkey -algorithm Ed25519 -out "${PRIVATE_KEY_PATH}"
    
    # Extract public key
    openssl pkey -in "${PRIVATE_KEY_PATH}" -pubout -out "${PUBLIC_KEY_PATH}"
    
    print_success "Generated Ed25519 test keys"
else
    print_status "Using existing Ed25519 test keys"
fi

# Generate signed SBOM
print_status "Generating signed SBOM..."
"${BINARY_PATH}" "${PLUGIN_PATH}" "${BINARY_PATH}" \
    --format cyclonedx \
    --output "${OUTPUT_FILE}" \
    --sign-key "${PRIVATE_KEY_PATH}" \
    --sign-algorithm Ed25519 \
    --sign-key-id heimdall-ed25519-key-2025

if [[ ! -f "${OUTPUT_FILE}" ]]; then
    print_error "Failed to generate signed SBOM"
    exit 1
fi

print_success "Generated signed SBOM: ${OUTPUT_FILE}"

# Validate with heimdall-validate
print_status "Validating with heimdall-validate..."
if "${VALIDATE_PATH}" verify-signature "${OUTPUT_FILE}" --key "${PUBLIC_KEY_PATH}"; then
    print_success "heimdall-validate verification PASSED"
else
    print_error "heimdall-validate verification FAILED"
    exit 1
fi

# Validate with Hoppr
print_status "Validating with Hoppr..."
if hopctl validate sbom -s "${OUTPUT_FILE}"; then
    print_success "Hoppr validation PASSED"
else
    print_error "Hoppr validation FAILED"
    exit 1
fi

print_success "All validations passed!"
print_status "Signed SBOM file: ${OUTPUT_FILE}"
print_status "Private key: ${PRIVATE_KEY_PATH}"
print_status "Public key: ${PUBLIC_KEY_PATH}" 