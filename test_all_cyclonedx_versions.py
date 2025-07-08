#!/usr/bin/env python3

import json
import sys
import os
import subprocess
import jsonschema
from jsonschema import validate, ValidationError

def run_validator(script_name, version):
    """Run a specific validator script and return its result"""
    print(f"\n{'='*50}")
    print(f"Testing CycloneDX {version} Compliance")
    print(f"{'='*50}")
    
    try:
        result = subprocess.run([sys.executable, script_name], 
                              capture_output=True, text=True, timeout=30)
        
        print(result.stdout)
        if result.stderr:
            print(f"Stderr: {result.stderr}")
        
        return result.returncode == 0
    except subprocess.TimeoutExpired:
        print(f"❌ Timeout running {script_name}")
        return False
    except Exception as e:
        print(f"❌ Error running {script_name}: {e}")
        return False

def load_schema(version):
    """Load the CycloneDX schema for a specific version"""
    schema_file = f'cyclonedx-{version}-schema.json'
    try:
        with open(schema_file, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Warning: Schema file {schema_file} not found")
        return None

def create_mock_data_for_version(version):
    """Create version-specific mock data"""
    base_data = {
        "bomFormat": "CycloneDX",
        "specVersion": version,
        "version": 1,
        "metadata": {
            "timestamp": "2024-01-20T10:00:00Z",
            "component": {
                "type": "application",
                "name": "test-app",
                "version": "1.0.0"
            }
        },
        "components": [
            {
                "type": "library",
                "bom-ref": "component-1",
                "name": "test-library",
                "description": "library component",
                "hashes": [
                    {
                        "alg": "SHA-256",
                        "content": "abc123def456"
                    }
                ],
                "purl": "pkg:generic/test-library@1.0.0",
                "externalReferences": [
                    {
                        "type": "distribution",
                        "url": "https://example.com/test-library"
                    }
                ]
            }
        ]
    }
    
    # Add serialNumber for 1.3+
    if version >= "1.3":
        base_data["serialNumber"] = "urn:uuid:12345678-1234-5678-9abc-123456789012"
    
    # Add $schema for 1.4+
    if version >= "1.4":
        base_data["$schema"] = f"http://cyclonedx.org/schema/bom-{version}.schema.json"
    
    # Configure tools structure based on version
    if version >= "1.5":
        # Tools.components structure for 1.5+
        base_data["metadata"]["tools"] = {
            "components": [
                {
                    "type": "application",
                    "bom-ref": "heimdall-sbom-generator",
                    "supplier": {
                        "name": "Heimdall Project"
                    },
                    "name": "Heimdall SBOM Generator",
                    "version": "2.0.0"
                }
            ]
        }
    else:
        # Simple tools array for 1.3/1.4
        base_data["metadata"]["tools"] = [
            {
                "vendor": "Heimdall Project",
                "name": "Heimdall SBOM Generator",
                "version": "2.0.0"
            }
        ]
    
    # Configure component fields based on version
    component = base_data["components"][0]
    
    # Version field required in 1.3, optional in 1.4+
    if version == "1.3":
        component["version"] = "1.0.0"
    elif version >= "1.4":
        component["version"] = "1.0.0"  # Optional but we'll include it
    
    # Supplier format: string in 1.3, object in 1.4+
    if version == "1.3":
        component["supplier"] = "Test Supplier"
    elif version >= "1.4":
        component["supplier"] = {
            "name": "Test Supplier"
        }
    
    # Evidence field only for 1.5+
    if version >= "1.5":
        component["evidence"] = {
            "identity": {
                "field": "hash",
                "confidence": 1.0,
                "methods": [
                    {
                        "technique": "binary-analysis",
                        "confidence": 1.0,
                        "value": "File hash verification"
                    }
                ]
            },
            "occurrences": [
                {
                    "location": "/path/to/test-library"
                }
            ]
        }
    
    # Lifecycles field only for 1.5+
    if version >= "1.5":
        base_data["metadata"]["lifecycles"] = [
            {
                "phase": "build"
            }
        ]
    
    return base_data

def validate_against_schema(data, schema, version):
    """Validate data against schema"""
    try:
        validate(instance=data, schema=schema)
        print(f"✅ Mock data validates against CycloneDX {version} schema")
        return True
    except ValidationError as e:
        print(f"❌ Mock data validation failed for {version}: {e.message}")
        return False

def validate_version_specific_structure(data, version):
    """Validate version-specific structure requirements"""
    issues = []
    
    # Check $schema field
    if version >= "1.4":
        if "$schema" not in data:
            issues.append(f"❌ $schema field missing (required in {version})")
        elif data["$schema"] != f"http://cyclonedx.org/schema/bom-{version}.schema.json":
            issues.append(f"❌ Incorrect $schema for {version}")
    else:
        if "$schema" in data:
            issues.append(f"❌ $schema field should not be present in {version}")
    
    # Check tools structure
    if "metadata" in data and "tools" in data["metadata"]:
        tools = data["metadata"]["tools"]
        if version >= "1.5":
            if not isinstance(tools, dict) or "components" not in tools:
                issues.append(f"❌ Tools should use components structure in {version}")
        else:
            if not isinstance(tools, list):
                issues.append(f"❌ Tools should be array in {version}")
    
    # Check component version requirement
    if "components" in data:
        for i, component in enumerate(data["components"]):
            if version == "1.3" and "version" not in component:
                issues.append(f"❌ Component {i} missing required version field in {version}")
    
    # Check evidence field
    if "components" in data:
        for i, component in enumerate(data["components"]):
            if version < "1.5" and "evidence" in component:
                issues.append(f"❌ Evidence field not available in {version}")
            elif version >= "1.5" and "evidence" not in component:
                # Evidence is optional even in 1.5+, so this is not an error
                pass
    
    # Check lifecycles
    if "metadata" in data and "lifecycles" in data["metadata"]:
        if version < "1.5":
            issues.append(f"❌ Lifecycles not available in {version}")
    
    return issues

def main():
    print("=== CycloneDX Multi-Version Compliance Test ===\n")
    
    versions = ["1.3", "1.4", "1.5", "1.6"]
    all_passed = True
    
    print("Testing individual validators...")
    
    # Test individual validators
    validator_results = {}
    for version in versions:
        script_name = f"validate_cyclonedx_{version}_fix.py"
        if os.path.exists(script_name):
            validator_results[version] = run_validator(script_name, version)
            if not validator_results[version]:
                all_passed = False
        else:
            print(f"⚠️  Validator script {script_name} not found, skipping...")
    
    print(f"\n{'='*60}")
    print("Testing cross-version mock data generation...")
    print(f"{'='*60}")
    
    # Test cross-version compatibility
    for version in versions:
        print(f"\nTesting CycloneDX {version} mock data generation:")
        
        # Load schema
        schema = load_schema(version)
        if not schema:
            print(f"⚠️  Schema for {version} not available, skipping...")
            continue
        
        # Create mock data
        mock_data = create_mock_data_for_version(version)
        
        # Validate against schema
        schema_valid = validate_against_schema(mock_data, schema, version)
        
        # Validate structure
        structure_issues = validate_version_specific_structure(mock_data, version)
        
        if structure_issues:
            print(f"❌ Structure issues for {version}:")
            for issue in structure_issues:
                print(f"  {issue}")
            all_passed = False
        else:
            print(f"✅ All structure requirements met for {version}")
        
        if not schema_valid:
            all_passed = False
    
    # Summary
    print(f"\n{'='*60}")
    print("TEST SUMMARY")
    print(f"{'='*60}")
    
    for version in versions:
        if version in validator_results:
            status = "✅ PASS" if validator_results[version] else "❌ FAIL"
            print(f"CycloneDX {version} individual validator: {status}")
        else:
            print(f"CycloneDX {version} individual validator: ⚠️ SKIPPED")
    
    print(f"\nOverall result: {'✅ ALL TESTS PASSED' if all_passed else '❌ SOME TESTS FAILED'}")
    
    return 0 if all_passed else 1

if __name__ == "__main__":
    sys.exit(main())