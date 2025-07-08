#!/usr/bin/env python3
"""
Validation script for CycloneDX 1.6 compliance fix.
This script validates the JSON structure against the key CycloneDX 1.6 schema requirements.
"""

import json
import re
import sys
import uuid
from typing import Dict, Any, List

class CycloneDX16Validator:
    """Validator for CycloneDX 1.6 schema compliance"""
    
    def __init__(self):
        self.errors = []
        self.warnings = []
    
    def validate_json_structure(self, sbom_content: str) -> bool:
        """Validate that the SBOM content is valid JSON and has correct structure"""
        try:
            sbom = json.loads(sbom_content)
        except json.JSONDecodeError as e:
            self.errors.append(f"Invalid JSON: {e}")
            return False
        
        return self.validate_sbom_structure(sbom)
    
    def validate_sbom_structure(self, sbom: Dict[Any, Any]) -> bool:
        """Validate the main SBOM structure against CycloneDX 1.6 requirements"""
        is_valid = True
        
        # Check required top-level fields
        required_fields = ["bomFormat", "specVersion"]
        for field in required_fields:
            if field not in sbom:
                self.errors.append(f"Missing required field: {field}")
                is_valid = False
        
        # Validate bomFormat
        if sbom.get("bomFormat") != "CycloneDX":
            self.errors.append(f"Invalid bomFormat: {sbom.get('bomFormat')}, expected 'CycloneDX'")
            is_valid = False
        
        # Validate specVersion
        if sbom.get("specVersion") != "1.6":
            self.errors.append(f"Invalid specVersion: {sbom.get('specVersion')}, expected '1.6'")
            is_valid = False
        
        # Check for schema reference (recommended)
        if "$schema" not in sbom:
            self.warnings.append("Missing $schema field (recommended)")
        elif not sbom["$schema"].endswith("bom-1.6.schema.json"):
            self.warnings.append(f"Schema reference may be incorrect: {sbom['$schema']}")
        
        # Validate serialNumber (SHOULD have one per spec)
        if "serialNumber" in sbom:
            if not self.validate_uuid(sbom["serialNumber"]):
                self.errors.append(f"Invalid serialNumber format: {sbom.get('serialNumber')}")
                is_valid = False
        else:
            self.warnings.append("Missing serialNumber (recommended)")
        
        # Validate version
        if "version" in sbom:
            if not isinstance(sbom["version"], int) or sbom["version"] < 1:
                self.errors.append(f"Invalid version: {sbom.get('version')}, must be integer >= 1")
                is_valid = False
        
        # Validate metadata
        if "metadata" in sbom:
            is_valid &= self.validate_metadata(sbom["metadata"])
        
        # Validate components
        if "components" in sbom:
            is_valid &= self.validate_components(sbom["components"])
        
        return is_valid
    
    def validate_uuid(self, uuid_str: str) -> bool:
        """Validate UUID format per RFC 4122"""
        if not uuid_str.startswith("urn:uuid:"):
            return False
        
        uuid_part = uuid_str[9:]  # Remove "urn:uuid:" prefix
        try:
            uuid.UUID(uuid_part, version=4)
            return True
        except (ValueError, TypeError):
            return False
    
    def validate_metadata(self, metadata: Dict[Any, Any]) -> bool:
        """Validate metadata structure for CycloneDX 1.6"""
        is_valid = True
        
        # Check tools structure (1.6 format)
        if "tools" in metadata:
            tools = metadata["tools"]
            if isinstance(tools, list):
                self.warnings.append("Tools array format is deprecated, use tools.components in 1.6")
            elif isinstance(tools, dict):
                if "components" in tools:
                    is_valid &= self.validate_tool_components(tools["components"])
                else:
                    self.warnings.append("Tools object should have 'components' array")
        
        # Check component (main subject)
        if "component" in metadata:
            is_valid &= self.validate_component(metadata["component"], is_main=True)
        
        return is_valid
    
    def validate_tool_components(self, tool_components: List) -> bool:
        """Validate tool components structure"""
        is_valid = True
        
        for tool in tool_components:
            if not isinstance(tool, dict):
                self.errors.append("Tool component must be an object")
                is_valid = False
                continue
            
            # Check required fields for tools
            if "name" not in tool:
                self.errors.append("Tool component missing required 'name' field")
                is_valid = False
            
            if "type" not in tool:
                self.errors.append("Tool component missing required 'type' field")
                is_valid = False
        
        return is_valid
    
    def validate_components(self, components: List) -> bool:
        """Validate components array"""
        is_valid = True
        
        for component in components:
            is_valid &= self.validate_component(component)
        
        return is_valid
    
    def validate_component(self, component: Dict[Any, Any], is_main: bool = False) -> bool:
        """Validate individual component structure"""
        is_valid = True
        
        # Check required fields
        required_fields = ["type", "name"]
        for field in required_fields:
            if field not in component:
                self.errors.append(f"Component missing required field: {field}")
                is_valid = False
        
        # Validate type field
        valid_types = [
            "application", "framework", "library", "container", "platform",
            "operating-system", "device", "device-driver", "firmware", "file",
            "machine-learning-model", "data", "cryptographic-asset"
        ]
        if "type" in component and component["type"] not in valid_types:
            self.errors.append(f"Invalid component type: {component['type']}")
            is_valid = False
        
        # Validate supplier structure (if present)
        if "supplier" in component:
            supplier = component["supplier"]
            if isinstance(supplier, str):
                self.errors.append("Supplier must be an organizational entity object, not a string")
                is_valid = False
            elif isinstance(supplier, dict):
                if "name" not in supplier:
                    self.errors.append("Supplier object missing required 'name' field")
                    is_valid = False
        
        # Validate hashes structure
        if "hashes" in component:
            is_valid &= self.validate_hashes(component["hashes"])
        
        # Validate evidence structure (1.6 feature)
        if "evidence" in component:
            is_valid &= self.validate_evidence(component["evidence"])
        
        return is_valid
    
    def validate_hashes(self, hashes: List) -> bool:
        """Validate hashes array structure"""
        is_valid = True
        
        for hash_obj in hashes:
            if not isinstance(hash_obj, dict):
                self.errors.append("Hash must be an object")
                is_valid = False
                continue
            
            if "alg" not in hash_obj:
                self.errors.append("Hash missing required 'alg' field")
                is_valid = False
            
            if "content" not in hash_obj:
                self.errors.append("Hash missing required 'content' field")
                is_valid = False
        
        return is_valid
    
    def validate_evidence(self, evidence: Dict[Any, Any]) -> bool:
        """Validate evidence structure for CycloneDX 1.6"""
        is_valid = True
        
        # Evidence structure validation - check for proper 1.6 fields
        if "occurrences" in evidence:
            if not isinstance(evidence["occurrences"], list):
                self.errors.append("Evidence occurrences must be an array")
                is_valid = False
        
        if "identity" in evidence:
            identity = evidence["identity"]
            if not isinstance(identity, dict):
                self.errors.append("Evidence identity must be an object")
                is_valid = False
        
        return is_valid
    
    def generate_sample_valid_cyclonedx_16(self) -> str:
        """Generate a sample valid CycloneDX 1.6 SBOM for comparison"""
        sample_uuid = str(uuid.uuid4())
        
        sample_sbom = {
            "$schema": "http://cyclonedx.org/schema/bom-1.6.schema.json",
            "bomFormat": "CycloneDX",
            "specVersion": "1.6",
            "serialNumber": f"urn:uuid:{sample_uuid}",
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
                    "name": "sample-app",
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
                    "bom-ref": "component-example-lib",
                    "name": "example-lib",
                    "version": "1.0.0",
                    "description": "library component",
                    "supplier": {
                        "name": "Example Organization"
                    },
                    "hashes": [
                        {
                            "alg": "SHA-256",
                            "content": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
                        }
                    ],
                    "purl": "pkg:generic/example-lib@1.0.0",
                    "externalReferences": [
                        {
                            "type": "distribution",
                            "url": "NOASSERTION"
                        }
                    ],
                    "evidence": {
                        "occurrences": [
                            {
                                "location": "/path/to/lib",
                                "line": 1
                            }
                        ]
                    }
                }
            ]
        }
        
        return json.dumps(sample_sbom, indent=2)
    
    def print_results(self) -> bool:
        """Print validation results"""
        if self.errors:
            print("❌ VALIDATION FAILED")
            print("\nErrors:")
            for error in self.errors:
                print(f"  • {error}")
        else:
            print("✅ VALIDATION PASSED")
        
        if self.warnings:
            print("\nWarnings:")
            for warning in self.warnings:
                print(f"  ⚠️  {warning}")
        
        return len(self.errors) == 0

def main():
    """Main validation function"""
    if len(sys.argv) < 2:
        print("Usage: python3 validate_cyclonedx_fix.py <sbom_file_or_json_content>")
        print("       python3 validate_cyclonedx_fix.py --sample")
        sys.exit(1)
    
    validator = CycloneDX16Validator()
    
    if sys.argv[1] == "--sample":
        print("Generating sample valid CycloneDX 1.6 SBOM:")
        print(validator.generate_sample_valid_cyclonedx_16())
        return
    
    # Try to read as file first, then as direct JSON content
    sbom_content = sys.argv[1]
    try:
        with open(sys.argv[1], 'r') as f:
            sbom_content = f.read()
    except FileNotFoundError:
        # Assume it's direct JSON content
        pass
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)
    
    print("Validating CycloneDX 1.6 compliance...")
    
    is_valid = validator.validate_json_structure(sbom_content)
    success = validator.print_results()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()