#!/usr/bin/env python3

import json
import sys
import os

def create_mock_cyclonedx_data(version):
    """Create version-specific mock CycloneDX data"""
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
        component["version"] = "1.0.0"  # Optional but include it
    
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

def validate_cyclonedx_structure(data, version):
    """Validate CycloneDX structure for specific version"""
    issues = []
    
    print(f"\n=== Validating CycloneDX {version} Structure ===")
    
    # Check required fields
    required_fields = ["bomFormat", "specVersion", "version"]
    for field in required_fields:
        if field not in data:
            issues.append(f"❌ Missing required field: {field}")
        else:
            print(f"✅ Required field present: {field}")
    
    # Check bomFormat
    if data.get("bomFormat") != "CycloneDX":
        issues.append(f"❌ bomFormat should be 'CycloneDX', got: {data.get('bomFormat')}")
    else:
        print(f"✅ bomFormat is correct: {data.get('bomFormat')}")
    
    # Check specVersion
    if data.get("specVersion") != version:
        issues.append(f"❌ specVersion should be '{version}', got: {data.get('specVersion')}")
    else:
        print(f"✅ specVersion is correct: {data.get('specVersion')}")
    
    # Check $schema field based on version
    if version >= "1.4":
        if "$schema" not in data:
            issues.append(f"❌ $schema field missing (required for {version}+)")
        elif data["$schema"] != f"http://cyclonedx.org/schema/bom-{version}.schema.json":
            issues.append(f"❌ Incorrect $schema for {version}")
        else:
            print(f"✅ $schema field correct for {version}")
    else:
        if "$schema" in data:
            issues.append(f"❌ $schema field should not be present for {version}")
        else:
            print(f"✅ $schema field correctly omitted for {version}")
    
    # Check serialNumber for 1.3+
    if version >= "1.3":
        if "serialNumber" not in data:
            issues.append(f"❌ serialNumber missing (should be present for {version}+)")
        else:
            serial = data["serialNumber"]
            if not serial.startswith("urn:uuid:"):
                issues.append(f"❌ serialNumber should be in UUID format")
            else:
                print(f"✅ serialNumber in correct UUID format")
    
    # Check tools structure
    if "metadata" in data and "tools" in data["metadata"]:
        tools = data["metadata"]["tools"]
        if version >= "1.5":
            if not isinstance(tools, dict) or "components" not in tools:
                issues.append(f"❌ Tools should use components structure for {version}+")
            else:
                print(f"✅ Tools structure correct for {version} (components)")
        else:
            if not isinstance(tools, list):
                issues.append(f"❌ Tools should be array for {version}")
            else:
                print(f"✅ Tools structure correct for {version} (array)")
    
    # Check component version requirement
    if "components" in data:
        for i, component in enumerate(data["components"]):
            if version == "1.3" and "version" not in component:
                issues.append(f"❌ Component {i} missing required version field (required in 1.3)")
            elif "version" in component:
                print(f"✅ Component {i} has version field")
            
            # Check supplier format
            if "supplier" in component:
                supplier = component["supplier"]
                if version == "1.3":
                    if not isinstance(supplier, str):
                        issues.append(f"❌ Component {i} supplier should be string for {version}")
                    else:
                        print(f"✅ Component {i} supplier is string (correct for {version})")
                elif version >= "1.4":
                    if not isinstance(supplier, dict) or "name" not in supplier:
                        issues.append(f"❌ Component {i} supplier should be object for {version}+")
                    else:
                        print(f"✅ Component {i} supplier is object (correct for {version}+)")
            
            # Check evidence field
            if "evidence" in component:
                if version < "1.5":
                    issues.append(f"❌ Component {i} evidence field not available in {version}")
                else:
                    print(f"✅ Component {i} evidence field available in {version}")
            elif version >= "1.5":
                print(f"✅ Component {i} evidence field optional in {version}")
    
    # Check lifecycles
    if "metadata" in data and "lifecycles" in data["metadata"]:
        if version < "1.5":
            issues.append(f"❌ Lifecycles field not available in {version}")
        else:
            print(f"✅ Lifecycles field available in {version}")
    
    return issues

def test_json_serialization(data, version):
    """Test that the data can be properly serialized to JSON"""
    try:
        json_str = json.dumps(data, indent=2)
        parsed = json.loads(json_str)
        print(f"✅ JSON serialization/deserialization successful for {version}")
        return True
    except Exception as e:
        print(f"❌ JSON serialization failed for {version}: {e}")
        return False

def main():
    print("=== CycloneDX Multi-Version Structure Validation ===\n")
    
    versions = ["1.3", "1.4", "1.5", "1.6"]
    all_passed = True
    
    for version in versions:
        print(f"\n{'='*60}")
        print(f"Testing CycloneDX {version}")
        print(f"{'='*60}")
        
        # Create mock data
        mock_data = create_mock_cyclonedx_data(version)
        
        # Test JSON serialization
        json_ok = test_json_serialization(mock_data, version)
        
        # Validate structure
        issues = validate_cyclonedx_structure(mock_data, version)
        
        # Report results
        if issues:
            print(f"\n❌ Issues found for CycloneDX {version}:")
            for issue in issues:
                print(f"  {issue}")
            all_passed = False
        else:
            print(f"\n✅ All structure validations passed for CycloneDX {version}")
        
        if not json_ok:
            all_passed = False
        
        # Save mock data to file for manual inspection
        output_file = f"mock_cyclonedx_{version.replace('.', '_')}.json"
        try:
            with open(output_file, 'w') as f:
                json.dump(mock_data, f, indent=2)
            print(f"✅ Mock data saved to {output_file}")
        except Exception as e:
            print(f"⚠️  Could not save mock data: {e}")
    
    # Summary
    print(f"\n{'='*60}")
    print("FINAL SUMMARY")
    print(f"{'='*60}")
    
    for version in versions:
        print(f"CycloneDX {version}: {'✅ PASS' if all_passed else '❓ CHECK DETAILS'}")
    
    print(f"\nOverall result: {'✅ ALL VERSIONS VALIDATED' if all_passed else '❌ ISSUES FOUND'}")
    
    return 0 if all_passed else 1

if __name__ == "__main__":
    sys.exit(main())