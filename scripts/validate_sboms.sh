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

# Heimdall SBOM Validation Script
# This script validates generated SBOMs for standards compliance

set -e

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

# Configuration
BUILD_DIR="${1:-build}"
SBOM_DIR="${BUILD_DIR}/sboms"
VALIDATION_RESULTS_DIR="${BUILD_DIR}/validation_results"

# Create validation results directory
mkdir -p "${VALIDATION_RESULTS_DIR}"

print_status "Validating Heimdall SBOMs for standards compliance"
print_status "Build directory: ${BUILD_DIR}"
print_status "SBOM directory: ${SBOM_DIR}"
print_status "Validation results: ${VALIDATION_RESULTS_DIR}"

# Check if SBOM directory exists
if [[ ! -d "${SBOM_DIR}" ]]; then
    print_error "SBOM directory not found: ${SBOM_DIR}"
    print_error "Please run the SBOM generation script first"
    exit 1
fi

# Function to validate JSON syntax
validate_json() {
    local file="$1"
    local filename=$(basename "$file")
    
    print_status "Validating JSON syntax: ${filename}"
    
    if jq empty "$file" 2>/dev/null; then
        print_success "✓ JSON syntax valid: ${filename}"
        return 0
    else
        print_error "✗ JSON syntax invalid: ${filename}"
        return 1
    fi
}

# Function to validate CycloneDX schema
validate_cyclonedx_schema() {
    local file="$1"
    local filename=$(basename "$file")
    
    print_status "Validating CycloneDX schema: ${filename}"
    
    # Extract specVersion from the file
    local spec_version=$(jq -r '.specVersion // empty' "$file" 2>/dev/null)
    
    if [[ -z "$spec_version" ]]; then
        print_error "✗ No specVersion found in: ${filename}"
        return 1
    fi
    
    print_success "✓ CycloneDX specVersion: ${spec_version}"
    
    # Validate required CycloneDX fields
    local required_fields=("bomFormat" "specVersion" "version" "metadata" "components")
    local missing_fields=()
    
    for field in "${required_fields[@]}"; do
        if ! jq -e ".$field" "$file" >/dev/null 2>&1; then
            missing_fields+=("$field")
        fi
    done
    
    if [[ ${#missing_fields[@]} -eq 0 ]]; then
        print_success "✓ All required CycloneDX fields present: ${filename}"
    else
        print_error "✗ Missing required fields in ${filename}: ${missing_fields[*]}"
        return 1
    fi
    
    # Validate metadata structure
    if ! jq -e '.metadata.timestamp' "$file" >/dev/null 2>&1; then
        print_warning "⚠ Missing timestamp in metadata: ${filename}"
    else
        print_success "✓ Metadata timestamp present: ${filename}"
    fi
    
    # Validate components array
    local component_count=$(jq -r '.components | length' "$file" 2>/dev/null)
    if [[ "$component_count" =~ ^[0-9]+$ ]] && [[ "$component_count" -gt 0 ]]; then
        print_success "✓ Components array valid (${component_count} components): ${filename}"
    else
        print_warning "⚠ No components found or invalid components array: ${filename}"
    fi
    
    return 0
}

# Function to validate SPDX structure
validate_spdx_structure() {
    local file="$1"
    local filename=$(basename "$file")
    
    print_status "Validating SPDX structure: ${filename}"
    
    # Check if it's a valid SPDX document
    if grep -q "SPDXVersion:" "$file" && grep -q "DataLicense:" "$file"; then
        print_success "✓ SPDX header fields present: ${filename}"
    else
        print_error "✗ Missing SPDX header fields: ${filename}"
        return 1
    fi
    
    # Check for required SPDX fields
    local required_fields=("SPDXVersion" "DataLicense" "DocumentName" "DocumentNamespace")
    local missing_fields=()
    
    for field in "${required_fields[@]}"; do
        if ! grep -q "^${field}:" "$file"; then
            missing_fields+=("$field")
        fi
    done
    
    if [[ ${#missing_fields[@]} -eq 0 ]]; then
        print_success "✓ All required SPDX fields present: ${filename}"
    else
        print_error "✗ Missing required SPDX fields in ${filename}: ${missing_fields[*]}"
        return 1
    fi
    
    # Check for SPDX version
    local spdx_version=$(grep "^SPDXVersion:" "$file" | cut -d: -f2 | tr -d ' ')
    if [[ -n "$spdx_version" ]]; then
        print_success "✓ SPDX version: ${spdx_version}"
    else
        print_warning "⚠ Could not determine SPDX version: ${filename}"
    fi
    
    # Check for package or file information (both are valid SPDX formats)
    if grep -q "^PackageName:" "$file"; then
        local package_count=$(grep -c "^PackageName:" "$file")
        print_success "✓ Package information present (${package_count} packages): ${filename}"
    elif grep -q "^FileName:" "$file"; then
        local file_count=$(grep -c "^FileName:" "$file")
        print_success "✓ File information present (${file_count} files): ${filename}"
    else
        print_warning "⚠ No package or file information found: ${filename}"
    fi
    
    return 0
}

# Function to validate online using CycloneDX validator
validate_cyclonedx_online() {
    local file="$1"
    local filename=$(basename "$file")
    
    print_status "Validating CycloneDX online: ${filename}"
    
    # Note: This is a placeholder for online validation
    # In a real implementation, you would POST to a CycloneDX validator API
    # For now, we'll just note that online validation is available
    
    print_warning "⚠ Online CycloneDX validation not implemented (requires API endpoint)"
    print_status "  You can validate manually at: https://cyclonedx.org/tool-center/"
    return 0
}

# Function to validate online using SPDX validator
validate_spdx_online() {
    local file="$1"
    local filename=$(basename "$file")
    
    print_status "Validating SPDX online: ${filename}"
    
    # Note: This is a placeholder for online validation
    # In a real implementation, you would POST to an SPDX validator API
    
    print_warning "⚠ Online SPDX validation not implemented (requires API endpoint)"
    print_status "  You can validate manually at: https://tools.spdx.org/app/validate/"
    return 0
}

# Function to generate validation report
generate_validation_report() {
    local report_file="${VALIDATION_RESULTS_DIR}/validation_report.txt"
    
    print_status "Generating validation report: ${report_file}"
    
    {
        echo "Heimdall SBOM Validation Report"
        echo "Generated: $(date)"
        echo "Build directory: ${BUILD_DIR}"
        echo "SBOM directory: ${SBOM_DIR}"
        echo ""
        echo "=== Validation Summary ==="
        echo "Total SBOMs validated: $1"
        echo "Valid SBOMs: $2"
        echo "Invalid SBOMs: $3"
        echo "Warnings: $4"
        echo ""
        echo "=== Validation Details ==="
        echo "See individual validation logs for details."
        echo ""
        echo "=== Manual Validation Resources ==="
        echo "CycloneDX Validator: https://cyclonedx.org/tool-center/"
        echo "SPDX Validator: https://tools.spdx.org/app/validate/"
        echo "CycloneDX Schema: https://cyclonedx.org/schema/"
        echo "SPDX Specification: https://spdx.github.io/spdx-spec/"
    } > "$report_file"
    
    print_success "Validation report generated: ${report_file}"
}

# Main validation logic
print_status "Starting SBOM validation..."

total_sboms=0
valid_sboms=0
invalid_sboms=0
warnings=0

# Process all SBOM files
for sbom_file in "${SBOM_DIR}"/*.{spdx,json}; do
    if [[ ! -f "$sbom_file" ]]; then
        continue
    fi
    
    total_sboms=$((total_sboms + 1))
    filename=$(basename "$sbom_file")
    validation_log="${VALIDATION_RESULTS_DIR}/${filename%.*}_validation.log"
    
    print_status ""
    print_status "=== Validating: ${filename} ==="
    
    # Redirect output to validation log
    {
        echo "Validation log for: ${filename}"
        echo "Generated: $(date)"
        echo ""
        
        # Validate based on file type
        if [[ "$sbom_file" == *.json ]]; then
            # CycloneDX JSON validation
            echo "=== JSON Syntax Validation ==="
            if validate_json "$sbom_file"; then
                echo "JSON syntax: VALID"
            else
                echo "JSON syntax: INVALID"
                invalid_sboms=$((invalid_sboms + 1))
                continue
            fi
            
            echo ""
            echo "=== CycloneDX Schema Validation ==="
            if validate_cyclonedx_schema "$sbom_file"; then
                echo "CycloneDX schema: VALID"
                valid_sboms=$((valid_sboms + 1))
            else
                echo "CycloneDX schema: INVALID"
                invalid_sboms=$((invalid_sboms + 1))
            fi
            
            echo ""
            echo "=== Online Validation ==="
            validate_cyclonedx_online "$sbom_file"
            
        elif [[ "$sbom_file" == *.spdx ]]; then
            # SPDX validation
            echo "=== SPDX Structure Validation ==="
            if validate_spdx_structure "$sbom_file"; then
                echo "SPDX structure: VALID"
                valid_sboms=$((valid_sboms + 1))
            else
                echo "SPDX structure: INVALID"
                invalid_sboms=$((invalid_sboms + 1))
            fi
            
            echo ""
            echo "=== Online Validation ==="
            validate_spdx_online "$sbom_file"
        fi
        
        echo ""
        echo "=== Validation Complete ==="
        
    } > "$validation_log" 2>&1
    
    # Display summary for this file
    if [[ -f "$sbom_file" ]]; then
        if [[ "$sbom_file" == *.json ]]; then
            if jq empty "$sbom_file" 2>/dev/null; then
                print_success "✓ ${filename}: JSON valid"
            else
                print_error "✗ ${filename}: JSON invalid"
            fi
        elif [[ "$sbom_file" == *.spdx ]]; then
            if grep -q "SPDXVersion:" "$sbom_file"; then
                print_success "✓ ${filename}: SPDX structure valid"
            else
                print_error "✗ ${filename}: SPDX structure invalid"
            fi
        fi
    fi
done

# Generate final report
generate_validation_report "$total_sboms" "$valid_sboms" "$invalid_sboms" "$warnings"

# Summary
print_status ""
print_status "=== Validation Summary ==="
print_status "Total SBOMs validated: ${total_sboms}"
print_status "Valid SBOMs: ${valid_sboms}"
print_status "Invalid SBOMs: ${invalid_sboms}"
print_status "Warnings: ${warnings}"

if [[ $invalid_sboms -eq 0 ]]; then
    print_success "All SBOMs passed validation!"
else
    print_warning "Some SBOMs failed validation. Check individual logs for details."
fi

print_status ""
print_status "=== Manual Validation Resources ==="
print_status "CycloneDX Validator: https://cyclonedx.org/tool-center/"
print_status "SPDX Validator: https://tools.spdx.org/app/validate/"
print_status "CycloneDX Schema: https://cyclonedx.org/schema/"
print_status "SPDX Specification: https://spdx.github.io/spdx-spec/"

print_status ""
print_success "SBOM validation completed!"
print_status "Detailed logs available in: ${VALIDATION_RESULTS_DIR}" 