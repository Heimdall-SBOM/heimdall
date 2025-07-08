#!/usr/bin/env python3

import json
import uuid
import hashlib

def generate_perfect_hash(algorithm, content="test content"):
    """Generate properly formatted hash content for each algorithm"""
    if algorithm == "MD5":
        return hashlib.md5(content.encode()).hexdigest()
    elif algorithm == "SHA-1":
        return hashlib.sha1(content.encode()).hexdigest()
    elif algorithm == "SHA-256":
        return hashlib.sha256(content.encode()).hexdigest()
    elif algorithm == "SHA-384":
        return hashlib.sha384(content.encode()).hexdigest()
    elif algorithm == "SHA-512":
        return hashlib.sha512(content.encode()).hexdigest()
    elif algorithm == "SHA3-256":
        return hashlib.sha3_256(content.encode()).hexdigest()
    elif algorithm == "SHA3-384":
        return hashlib.sha3_384(content.encode()).hexdigest()
    elif algorithm == "SHA3-512":
        return hashlib.sha3_512(content.encode()).hexdigest()
    elif algorithm == "BLAKE2b-256":
        return hashlib.blake2b(content.encode(), digest_size=32).hexdigest()
    elif algorithm == "BLAKE2b-384":
        return hashlib.blake2b(content.encode(), digest_size=48).hexdigest()
    elif algorithm == "BLAKE2b-512":
        return hashlib.blake2b(content.encode(), digest_size=64).hexdigest()
    elif algorithm == "BLAKE3":
        # BLAKE3 typically 256-bit (64 hex chars)
        return hashlib.sha256(content.encode()).hexdigest()  # Fallback
    else:
        return "invalid"

def create_perfect_test_data(version):
    """Create perfect test data with all correct formats"""
    
    data = {
        "bomFormat": "CycloneDX",
        "specVersion": version,
        "version": 1
    }
    
    # Add $schema for 1.4+
    if version >= "1.4":
        data["$schema"] = f"http://cyclonedx.org/schema/bom-{version}.schema.json"
    
    # Add serialNumber for 1.3+
    if version >= "1.3":
        data["serialNumber"] = f"urn:uuid:{uuid.uuid4()}"
    
    # Perfect metadata
    metadata = {
        "timestamp": "2024-01-20T10:00:00Z",
        "component": {
            "type": "application",
            "bom-ref": "main-application",
            "name": "test-application",
            "version": "1.0.0",
            "description": "Test application for comprehensive validation",
            "scope": "required"
        },
        "authors": [
            {
                "name": "John Doe",
                "email": "john.doe@example.com",
                "phone": "555-1234"
            }
        ],
        "manufacture": {
            "name": "Example Manufacturing Corp",
            "url": ["https://manufacturing.example.com"],
            "contact": [
                {
                    "name": "Manufacturing Contact",
                    "email": "contact@manufacturing.example.com"
                }
            ]
        },
        "supplier": {
            "name": "Example Supplier Inc",
            "url": ["https://supplier.example.com"]
        },
        "licenses": [
            {
                "license": {
                    "id": "Apache-2.0"
                }
            },
            {
                "expression": "Apache-2.0 AND MIT"
            }
        ],
        "properties": [
            {
                "name": "build.system",
                "value": "gradle"
            }
        ]
    }
    
    # Tools structure based on version
    if version >= "1.5":
        metadata["tools"] = {
            "components": [
                {
                    "type": "application",
                    "bom-ref": "heimdall-sbom-generator",
                    "supplier": {
                        "name": "Heimdall Project"
                    },
                    "name": "Heimdall SBOM Generator",
                    "version": "2.0.0",
                    "hashes": [
                        {
                            "alg": "SHA-256",
                            "content": generate_perfect_hash("SHA-256", "heimdall-tool")
                        }
                    ]
                }
            ]
        }
        
        # Valid lifecycle phases
        metadata["lifecycles"] = [
            {"phase": "design"},
            {"phase": "build"},
            {"phase": "operations"}
        ]
    else:
        metadata["tools"] = [
            {
                "vendor": "Heimdall Project",
                "name": "Heimdall SBOM Generator",
                "version": "2.0.0",
                "hashes": [
                    {
                        "alg": "SHA-256",
                        "content": generate_perfect_hash("SHA-256", "heimdall-tool")
                    }
                ]
            }
        ]
    
    data["metadata"] = metadata
    
    # Perfect components
    component1 = {
        "type": "library",
        "bom-ref": "library-component-1",
        "name": "test-library",
        "group": "com.example",
        "version": "1.2.3",
        "description": "A comprehensive test library",
        "scope": "required",
        "author": "Library Author Team",
        "publisher": "Example Publishers",
        "mime-type": "application/java-archive",
        "hashes": [
            {
                "alg": "SHA-256",
                "content": generate_perfect_hash("SHA-256", "test-library")
            },
            {
                "alg": "SHA-1",
                "content": generate_perfect_hash("SHA-1", "test-library")
            },
            {
                "alg": "MD5",
                "content": generate_perfect_hash("MD5", "test-library")
            }
        ],
        "licenses": [
            {
                "license": {
                    "id": "MIT",
                    "url": "https://opensource.org/licenses/MIT"
                }
            }
        ],
        "copyright": "Copyright (c) 2024 Example Corp",
        "cpe": "cpe:2.3:a:example:test_library:1.2.3:*:*:*:*:*:*:*",
        "purl": "pkg:maven/com.example/test-library@1.2.3?type=jar",
        "swid": {
            "tagId": "example.com/test-library-1.2.3",
            "name": "Test Library",
            "version": "1.2.3",
            "tagVersion": 1,
            "patch": False
        },
        "externalReferences": [
            {
                "type": "website",
                "url": "https://example.com/test-library"
            },
            {
                "type": "vcs",
                "url": "https://github.com/example/test-library"
            },
            {
                "type": "distribution",
                "url": "https://repo1.maven.org/maven2/com/example/test-library/1.2.3/",
                "hashes": [
                    {
                        "alg": "SHA-256",
                        "content": generate_perfect_hash("SHA-256", "distribution-test-library")
                    }
                ]
            }
        ],
        "properties": [
            {
                "name": "package.manager",
                "value": "maven"
            }
        ],
        "pedigree": {
            "ancestors": [
                {
                    "type": "library",
                    "name": "original-library",
                    "version": "1.0.0"
                }
            ],
            "commits": [
                {
                    "uid": "abc123def456",
                    "url": "https://github.com/example/test-library/commit/abc123def456",
                    "author": {
                        "timestamp": "2024-01-10T10:00:00Z",
                        "name": "Developer Name",
                        "email": "dev@example.com"
                    },
                    "message": "Initial commit"
                }
            ],
            "patches": [
                {
                    "type": "backport",
                    "diff": {
                        "url": "https://example.com/patch.diff"
                    }
                }
            ]
        }
    }
    
    # Version-specific supplier format
    if version == "1.3":
        component1["supplier"] = "Example Supplier Corp"
    else:
        component1["supplier"] = {
            "name": "Example Supplier Corp",
            "url": ["https://supplier.example.com"]
        }
    
    # Add evidence for 1.5+
    if version >= "1.5":
        evidence = {
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
                    "location": "/usr/local/lib/test-library-1.2.3.jar"
                }
            ],
            "callstack": {
                "frames": [
                    {
                        "function": "main",
                        "line": 1,
                        "column": 1
                    }
                ]
            }
        }
        
        # Add module field for 1.5
        if version == "1.5":
            for frame in evidence["callstack"]["frames"]:
                frame["module"] = "test-library"
        
        component1["evidence"] = evidence
    
    # Component 2: Framework
    component2 = {
        "type": "framework",
        "bom-ref": "framework-component-2",
        "name": "test-framework",
        "version": "2.1.0",
        "scope": "optional",
        "hashes": [
            {
                "alg": "BLAKE2b-256",
                "content": generate_perfect_hash("BLAKE2b-256", "test-framework")
            }
        ],
        "purl": "pkg:npm/test-framework@2.1.0"
    }
    
    if version >= "1.4":
        component2["supplier"] = {"name": "Framework Supplier"}
    else:
        component2["supplier"] = "Framework Supplier"
    
    # Component 3: Container
    component3 = {
        "type": "container",
        "bom-ref": "container-component-3", 
        "name": "test-container",
        "version": "3.0.0",
        "scope": "excluded",
        "hashes": [
            {
                "alg": "SHA3-512",
                "content": generate_perfect_hash("SHA3-512", "test-container")
            }
        ],
        "purl": "pkg:docker/test-container@3.0.0"
    }
    
    if version >= "1.4":
        component3["supplier"] = {"name": "Container Supplier"}
    else:
        component3["supplier"] = "Container Supplier"
    
    data["components"] = [component1, component2, component3]
    
    # Services
    data["services"] = [
        {
            "bom-ref": "api-service-1",
            "name": "test-api-service",
            "version": "1.0.0",
            "description": "REST API service",
            "endpoints": [
                "https://api.example.com/v1"
            ],
            "authenticated": True,
            "x-trust-boundary": False,
            "data": [
                {
                    "flow": "inbound",
                    "classification": "PII"
                }
            ],
            "provider": {
                "name": "API Provider Corp",
                "url": ["https://provider.example.com"]
            },
            "group": "com.example.services"
        }
    ]
    
    # Dependencies
    data["dependencies"] = [
        {
            "ref": "main-application",
            "dependsOn": ["library-component-1", "framework-component-2"]
        },
        {
            "ref": "library-component-1",
            "dependsOn": []
        },
        {
            "ref": "framework-component-2",
            "dependsOn": ["library-component-1"]
        },
        {
            "ref": "container-component-3",
            "dependsOn": []
        }
    ]
    
    # Compositions
    data["compositions"] = [
        {
            "aggregate": "complete",
            "assemblies": ["main-application", "library-component-1"],
            "dependencies": ["library-component-1"]
        }
    ]
    
    # External references
    data["externalReferences"] = [
        {
            "type": "website",
            "url": "https://example.com/project"
        }
    ]
    
    return data

def main():
    print("=== Creating Perfect CycloneDX Test Data ===\n")
    
    versions = ["1.3", "1.4", "1.5", "1.6"]
    
    for version in versions:
        print(f"Creating perfect test data for CycloneDX {version}...")
        
        data = create_perfect_test_data(version)
        
        # Save to file
        filename = f"perfect_cyclonedx_{version.replace('.', '_')}.json"
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)
        
        print(f"✅ Saved to {filename}")
    
    print("\n✅ All perfect test data files created!")

if __name__ == "__main__":
    main()