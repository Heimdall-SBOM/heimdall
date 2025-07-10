#!/usr/bin/env python3

"""
Heimdall SBOM Online Validation Script
This script validates SBOMs using online validation services and APIs.
"""

import json
import sys
import os
import requests
from pathlib import Path
from typing import Dict, List, Optional, Tuple

def validate_cyclonedx_json_schema(sbom_content: str) -> Tuple[bool, List[str]]:
    """
    Validate CycloneDX JSON against the official schema.
    This is a basic validation - for full validation, use the official tools.
    """
    errors = []
    
    try:
        data = json.loads(sbom_content)
    except json.JSONDecodeError as e:
        errors.append(f"Invalid JSON: {e}")
        return False, errors
    
    # Check required top-level fields
    required_fields = ['bomFormat', 'specVersion', 'version', 'metadata', 'components']
    for field in required_fields:
        if field not in data:
            errors.append(f"Missing required field: {field}")
    
    # Validate bomFormat
    if 'bomFormat' in data and data['bomFormat'] != 'CycloneDX':
        errors.append("bomFormat must be 'CycloneDX'")
    
    # Validate specVersion
    if 'specVersion' in data:
        spec_version = data['specVersion']
        valid_versions = ['1.0', '1.1', '1.2', '1.3', '1.4', '1.5']
        if spec_version not in valid_versions:
            errors.append(f"Invalid specVersion: {spec_version}. Must be one of {valid_versions}")
    
    # Validate metadata structure
    if 'metadata' in data:
        metadata = data['metadata']
        if 'timestamp' not in metadata:
            errors.append("Missing timestamp in metadata")
        if 'tools' not in metadata:
            errors.append("Missing tools in metadata")
    
    # Validate components array
    if 'components' in data:
        components = data['components']
        if not isinstance(components, list):
            errors.append("components must be an array")
        else:
            for i, component in enumerate(components):
                if not isinstance(component, dict):
                    errors.append(f"Component {i} must be an object")
                elif 'type' not in component:
                    errors.append(f"Component {i} missing required 'type' field")
                elif 'name' not in component:
                    errors.append(f"Component {i} missing required 'name' field")
    
    return len(errors) == 0, errors

def validate_spdx_structure(spdx_content: str) -> Tuple[bool, List[str]]:
    """
    Validate SPDX structure and required fields.
    """
    errors = []
    lines = spdx_content.split('\n')
    
    # Check for required header fields
    required_fields = ['SPDXVersion', 'DataLicense', 'DocumentName', 'DocumentNamespace']
    found_fields = set()
    
    for line in lines:
        line = line.strip()
        if ':' in line:
            field = line.split(':', 1)[0].strip()
            found_fields.add(field)
    
    for field in required_fields:
        if field not in found_fields:
            errors.append(f"Missing required SPDX field: {field}")
    
    # Check SPDX version
    for line in lines:
        if line.startswith('SPDXVersion:'):
            version = line.split(':', 1)[1].strip()
            if not version.startswith('SPDX-'):
                errors.append(f"Invalid SPDX version format: {version}")
            break
    
    # Check for package or file information (both are valid SPDX formats)
    has_packages = any('PackageName:' in line for line in lines)
    has_files = any('FileName:' in line for line in lines)
    if not has_packages and not has_files:
        errors.append("No package or file information found")
    
    return len(errors) == 0, errors

def validate_with_cyclonedx_tool_center(sbom_file: Path) -> Dict:
    """
    Simulate validation with CycloneDX Tool Center.
    In a real implementation, this would use their API.
    """
    print(f"  Validating with CycloneDX Tool Center: {sbom_file.name}")
    print(f"    Manual validation URL: https://cyclonedx.org/tool-center/")
    print(f"    Upload your file: {sbom_file} to the tool center")
    
    return {
        'validated': False,
        'message': 'Manual validation required',
        'url': 'https://cyclonedx.org/tool-center/'
    }

def validate_with_spdx_tools(sbom_file: Path) -> Dict:
    """
    Simulate validation with SPDX Tools.
    In a real implementation, this would use their API.
    """
    print(f"  Validating with SPDX Tools: {sbom_file.name}")
    print(f"    Manual validation URL: https://tools.spdx.org/app/validate/")
    print(f"    Upload your file: {sbom_file} to the SPDX validator")
    
    return {
        'validated': False,
        'message': 'Manual validation required',
        'url': 'https://tools.spdx.org/app/validate/'
    }

def validate_sbom_file(sbom_file: Path) -> Dict:
    """
    Validate a single SBOM file.
    """
    print(f"\n=== Validating: {sbom_file.name} ===")
    
    try:
        with open(sbom_file, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        return {
            'file': str(sbom_file),
            'valid': False,
            'errors': [f"Failed to read file: {e}"]
        }
    
    result = {
        'file': str(sbom_file),
        'valid': True,
        'errors': [],
        'warnings': []
    }
    
    # Validate based on file type
    if sbom_file.suffix == '.json':
        # CycloneDX JSON validation
        valid, errors = validate_cyclonedx_json_schema(content)
        result['valid'] = valid
        result['errors'].extend(errors)
        
        if valid:
            print(f"  ✓ CycloneDX JSON schema validation: PASSED")
        else:
            print(f"  ✗ CycloneDX JSON schema validation: FAILED")
            for error in errors:
                print(f"    Error: {error}")
        
        # Online validation
        validate_with_cyclonedx_tool_center(sbom_file)
        
    elif sbom_file.suffix == '.spdx':
        # SPDX validation
        valid, errors = validate_spdx_structure(content)
        result['valid'] = valid
        result['errors'].extend(errors)
        
        if valid:
            print(f"  ✓ SPDX structure validation: PASSED")
        else:
            print(f"  ✗ SPDX structure validation: FAILED")
            for error in errors:
                print(f"    Error: {error}")
        
        # Online validation
        validate_with_spdx_tools(sbom_file)
    
    return result

def main():
    """
    Main validation function.
    """
    if len(sys.argv) != 2:
        print("Usage: python3 validate_sboms_online.py <build_directory>")
        sys.exit(1)
    
    build_dir = Path(sys.argv[1])
    sbom_dir = build_dir / 'sboms'
    
    if not sbom_dir.exists():
        print(f"Error: SBOM directory not found: {sbom_dir}")
        print("Please run the SBOM generation script first.")
        sys.exit(1)
    
    print("Heimdall SBOM Online Validation")
    print("=" * 40)
    print(f"Build directory: {build_dir}")
    print(f"SBOM directory: {sbom_dir}")
    print()
    
    # Find all SBOM files
    sbom_files = list(sbom_dir.glob('*.json')) + list(sbom_dir.glob('*.spdx'))
    
    if not sbom_files:
        print("No SBOM files found.")
        sys.exit(1)
    
    print(f"Found {len(sbom_files)} SBOM files to validate.")
    
    # Validate each file
    results = []
    total_files = len(sbom_files)
    valid_files = 0
    
    for sbom_file in sbom_files:
        result = validate_sbom_file(sbom_file)
        results.append(result)
        
        if result['valid']:
            valid_files += 1
    
    # Summary
    print("\n" + "=" * 40)
    print("VALIDATION SUMMARY")
    print("=" * 40)
    print(f"Total files: {total_files}")
    print(f"Valid files: {valid_files}")
    print(f"Invalid files: {total_files - valid_files}")
    
    if valid_files == total_files:
        print("\n✓ All SBOMs passed validation!")
    else:
        print(f"\n⚠ {total_files - valid_files} SBOMs failed validation.")
        print("\nFailed files:")
        for result in results:
            if not result['valid']:
                print(f"  - {result['file']}")
                for error in result['errors']:
                    print(f"    Error: {error}")
    
    print("\n" + "=" * 40)
    print("ONLINE VALIDATION RESOURCES")
    print("=" * 40)
    print("For complete validation, use these online tools:")
    print()
    print("CycloneDX Validation:")
    print("  - Tool Center: https://cyclonedx.org/tool-center/")
    print("  - Schema: https://cyclonedx.org/schema/")
    print("  - Specification: https://cyclonedx.org/specification/")
    print()
    print("SPDX Validation:")
    print("  - Validator: https://tools.spdx.org/app/validate/")
    print("  - Specification: https://spdx.github.io/spdx-spec/")
    print("  - Tools: https://github.com/spdx/tools")
    print()
    print("Additional Tools:")
    print("  - OWASP Dependency Check: https://owasp.org/www-project-dependency-check/")
    print("  - Snyk: https://snyk.io/")
    print("  - FOSSA: https://fossa.com/")

if __name__ == '__main__':
    main() 