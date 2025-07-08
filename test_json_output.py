#!/usr/bin/env python3
"""
Test script to validate the JSON structure changes for CycloneDX 1.6 compliance.
This creates a mock output similar to what the fixed implementation would generate.
"""

import json
import uuid
import sys

def generate_mock_cyclonedx_1_6():
    """Generate a mock CycloneDX 1.6 SBOM with the structure from our fixes"""
    
    # Generate a proper UUID
    serial_uuid = str(uuid.uuid4())
    
    mock_sbom = {
        "$schema": "http://cyclonedx.org/schema/bom-1.6.schema.json",
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": f"urn:uuid:{serial_uuid}",
        "version": 1,
        "metadata": {
            "timestamp": "2025-01-01T00:00:00Z",
            "tools": {
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
            },
            "component": {
                "type": "application",
                "name": "test-app",
                "version": "1.0.0"
            },
            "lifecycles": [
                {
                    "phase": "build"
                }
            ]
        },
        "components": [
            {
                "type": "library",
                "bom-ref": "component-SPDXRef-test-library",
                "name": "test-library",
                "version": "1.2.3",
                "description": "library component",
                "supplier": {
                    "name": "Test Organization"
                },
                "hashes": [
                    {
                        "alg": "SHA-256",
                        "content": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
                    }
                ],
                "purl": "pkg:generic/test-library@1.2.3",
                "externalReferences": [
                    {
                        "type": "distribution",
                        "url": "NOASSERTION"
                    }
                ],
                "properties": [
                    {
                        "name": "heimdall:source-files",
                        "value": "/src/test.c"
                    },
                    {
                        "name": "heimdall:functions",
                        "value": "test_function"
                    },
                    {
                        "name": "heimdall:contains-debug-info",
                        "value": "true"
                    }
                ],
                "evidence": {
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
                            "location": "/usr/lib/libtest.so",
                            "line": 1
                        }
                    ],
                    "callstack": {
                        "frames": [
                            {
                                "function": "test_function",
                                "line": 1,
                                "column": 1
                            }
                        ]
                    }
                }
            }
        ]
    }
    
    return mock_sbom

def main():
    """Main test function"""
    print("🧪 Testing CycloneDX 1.6 JSON structure...")
    
    # Generate mock SBOM
    mock_sbom = generate_mock_cyclonedx_1_6()
    
    # Convert to JSON string
    json_output = json.dumps(mock_sbom, indent=2)
    
    # Save to file
    with open("mock_cyclonedx_1.6_output.json", "w") as f:
        f.write(json_output)
    
    print("📝 Generated mock CycloneDX 1.6 SBOM")
    print(f"📄 Saved to: mock_cyclonedx_1.6_output.json")
    
    # Validate using our validation script
    print("\n🔍 Running validation...")
    
    try:
        import subprocess
        result = subprocess.run([
            "python3", "validate_cyclonedx_fix.py", "mock_cyclonedx_1.6_output.json"
        ], capture_output=True, text=True)
        
        print(result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        
        return result.returncode
        
    except Exception as e:
        print(f"❌ Validation failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())