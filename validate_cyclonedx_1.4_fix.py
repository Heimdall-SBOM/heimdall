#!/usr/bin/env python3

import json
import sys
import jsonschema
from jsonschema import validate, ValidationError, Draft7Validator

def load_schema():
    """Load the CycloneDX 1.4 schema"""
    try:
        with open('cyclonedx-1.4-schema.json', 'r') as f:
            schema = json.load(f)
        return schema
    except FileNotFoundError:
        print("Error: CycloneDX 1.4 schema file not found")
        return None
    except json.JSONDecodeError as e:
        print(f"Error parsing schema: {e}")
        return None

def create_mock_cyclonedx_1_4():
    """Create a mock CycloneDX 1.4 SBOM following the exact schema requirements"""
    return {
        "$schema": "http://cyclonedx.org/schema/bom-1.4.schema.json",
        "bomFormat": "CycloneDX",
        "specVersion": "1.4",
        "serialNumber": "urn:uuid:12345678-1234-5678-9abc-123456789012",
        "version": 1,
        "metadata": {
            "timestamp": "2024-01-20T10:00:00Z",
            "tools": [
                {
                    "vendor": "Heimdall Project",
                    "name": "Heimdall SBOM Generator",
                    "version": "2.0.0"
                }
            ],
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
                "version": "1.0.0",
                "description": "library component",
                "supplier": {
                    "name": "Test Supplier"
                },
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

def validate_cyclonedx_1_4_structure(data):
    """Validate CycloneDX 1.4 specific structure requirements"""
    issues = []
    
    # Check that $schema is present and correct for 1.4
    if "$schema" in data:
        if data["$schema"] == "http://cyclonedx.org/schema/bom-1.4.schema.json":
            print("✅ $schema field correct for CycloneDX 1.4")
        else:
            issues.append(f"❌ Incorrect $schema: {data['$schema']}")
    else:
        issues.append("❌ $schema field missing (required in CycloneDX 1.4)")
    
    # Check tools structure (should be simple array, not tools.components)
    if "metadata" in data and "tools" in data["metadata"]:
        tools = data["metadata"]["tools"]
        if isinstance(tools, list):
            print("✅ Tools structure is simple array (correct for 1.4)")
            # Check tool structure
            for tool in tools:
                if "vendor" in tool and "name" in tool and "version" in tool:
                    print("✅ Tool has required fields (vendor, name, version)")
                else:
                    issues.append("❌ Tool missing required fields")
        else:
            issues.append("❌ Tools should be array in CycloneDX 1.4, not object")
    
    # Check component version requirement (optional in 1.4)
    if "components" in data:
        for i, component in enumerate(data["components"]):
            if "version" in component:
                print(f"✅ Component {i} has version field (optional in 1.4)")
            
            # Check supplier format (object in 1.4, not string)
            if "supplier" in component:
                if isinstance(component["supplier"], dict) and "name" in component["supplier"]:
                    print(f"✅ Component {i} supplier is organizational entity object (correct for 1.4)")
                else:
                    issues.append(f"❌ Component {i} supplier should be organizational entity object in CycloneDX 1.4")
    
    # Check that evidence field is not present (not available in 1.4)
    if "components" in data:
        for i, component in enumerate(data["components"]):
            if "evidence" in component:
                issues.append(f"❌ Component {i} has evidence field (not available in 1.4)")
            else:
                print(f"✅ Component {i} correctly omits evidence field")
    
    # Check that lifecycles are not present (not available in 1.4)
    if "metadata" in data and "lifecycles" in data["metadata"]:
        issues.append("❌ Lifecycles field not available in CycloneDX 1.4")
    else:
        print("✅ Lifecycles correctly omitted for CycloneDX 1.4")
    
    return issues

def main():
    print("=== CycloneDX 1.4 Compliance Validation ===\n")
    
    # Load schema
    schema = load_schema()
    if not schema:
        return 1
    
    print("✅ Loaded CycloneDX 1.4 schema\n")
    
    # Create and validate mock data
    print("Testing mock CycloneDX 1.4 SBOM:")
    mock_data = create_mock_cyclonedx_1_4()
    
    try:
        validate(instance=mock_data, schema=schema)
        print("✅ Mock data validates against CycloneDX 1.4 schema")
    except ValidationError as e:
        print(f"❌ Mock data validation failed: {e.message}")
        return 1
    
    # Check structure requirements
    print("\nChecking CycloneDX 1.4 structure requirements:")
    issues = validate_cyclonedx_1_4_structure(mock_data)
    
    if issues:
        print("\nIssues found:")
        for issue in issues:
            print(f"  {issue}")
        return 1
    else:
        print("\n✅ All CycloneDX 1.4 structure requirements met")
    
    print("\n=== CycloneDX 1.4 Validation Successful ===")
    return 0

if __name__ == "__main__":
    sys.exit(main())