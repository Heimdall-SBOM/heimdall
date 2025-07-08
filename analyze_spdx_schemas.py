#!/usr/bin/env python3

import json
import sys
from collections import defaultdict

def analyze_spdx_schema(file_path, version):
    """Analyze an SPDX schema file and extract key information"""
    print(f"\n=== ANALYZING SPDX {version} SCHEMA ===")
    
    try:
        with open(file_path, 'r') as f:
            schema = json.load(f)
    except Exception as e:
        print(f"❌ Error loading {file_path}: {e}")
        return None
    
    analysis = {
        'version': version,
        'file_size': f"{len(json.dumps(schema)) / 1024:.1f} KB",
        'schema_type': schema.get('$schema', 'Unknown'),
        'title': schema.get('title', 'Unknown'),
        'description': schema.get('description', 'No description'),
        'properties': {},
        'required_fields': [],
        'definitions': {},
        'key_differences': []
    }
    
    # Analyze top-level properties
    if 'properties' in schema:
        analysis['properties'] = list(schema['properties'].keys())
        print(f"📋 Top-level properties ({len(analysis['properties'])}): {', '.join(analysis['properties'][:10])}{'...' if len(analysis['properties']) > 10 else ''}")
    
    # Analyze required fields
    if 'required' in schema:
        analysis['required_fields'] = schema['required']
        print(f"✅ Required fields: {', '.join(analysis['required_fields'])}")
    
    # Analyze definitions/components
    definitions_key = None
    for key in ['definitions', '$defs', 'components']:
        if key in schema:
            definitions_key = key
            break
    
    if definitions_key:
        definitions = schema[definitions_key]
        analysis['definitions'] = list(definitions.keys())
        print(f"🔧 Definitions ({len(analysis['definitions'])}): {', '.join(analysis['definitions'][:5])}{'...' if len(analysis['definitions']) > 5 else ''}")
        
        # Look for document/element types
        doc_types = []
        for def_name, def_content in definitions.items():
            if isinstance(def_content, dict):
                if 'type' in def_content or 'properties' in def_content:
                    if any(keyword in def_name.lower() for keyword in ['document', 'package', 'file', 'element', 'creation']):
                        doc_types.append(def_name)
        
        if doc_types:
            print(f"📄 Key document types: {', '.join(doc_types[:5])}")
    
    # Version-specific analysis
    if version == "2.3":
        analysis['key_differences'].extend([
            "Tag-value and RDF/XML format support",
            "Traditional SPDX v2 structure",
            "Package-centric model",
            "Basic relationship types"
        ])
    elif version == "3.0.0":
        analysis['key_differences'].extend([
            "JSON-LD based structure",
            "Element-based model (not package-centric)",
            "Enhanced relationship types",
            "Security and AI/ML extensions"
        ])
    elif version == "3.0.1":
        analysis['key_differences'].extend([
            "JSON-LD based structure (improved)",
            "Element-based model with refinements",
            "Enhanced relationship types",
            "Security and AI/ML extensions (updated)",
            "Bug fixes and clarifications from 3.0.0"
        ])
    
    return analysis

def compare_spdx_versions(analyses):
    """Compare different SPDX versions"""
    print(f"\n{'='*70}")
    print("SPDX VERSION COMPARISON")
    print(f"{'='*70}")
    
    # Create comparison table
    print(f"{'Aspect':<25} {'SPDX 2.3':<20} {'SPDX 3.0.0':<20} {'SPDX 3.0.1':<20}")
    print("-" * 85)
    
    versions = ['2.3', '3.0.0', '3.0.1']
    version_data = {analysis['version']: analysis for analysis in analyses if analysis}
    
    # Compare schema types
    schema_types = []
    for v in versions:
        if v in version_data:
            schema_types.append(version_data[v]['schema_type'].split('/')[-1] if '/' in version_data[v]['schema_type'] else version_data[v]['schema_type'])
        else:
            schema_types.append('N/A')
    
    print(f"{'Schema Type':<25} {schema_types[0]:<20} {schema_types[1]:<20} {schema_types[2]:<20}")
    
    # Compare property counts
    prop_counts = []
    for v in versions:
        if v in version_data:
            prop_counts.append(str(len(version_data[v]['properties'])))
        else:
            prop_counts.append('N/A')
    
    print(f"{'Properties Count':<25} {prop_counts[0]:<20} {prop_counts[1]:<20} {prop_counts[2]:<20}")
    
    # Compare definition counts
    def_counts = []
    for v in versions:
        if v in version_data:
            def_counts.append(str(len(version_data[v]['definitions'])))
        else:
            def_counts.append('N/A')
    
    print(f"{'Definitions Count':<25} {def_counts[0]:<20} {def_counts[1]:<20} {def_counts[2]:<20}")
    
    # Compare file sizes
    file_sizes = []
    for v in versions:
        if v in version_data:
            file_sizes.append(version_data[v]['file_size'])
        else:
            file_sizes.append('N/A')
    
    print(f"{'Schema Size':<25} {file_sizes[0]:<20} {file_sizes[1]:<20} {file_sizes[2]:<20}")
    
    print("\nKEY ARCHITECTURAL DIFFERENCES:")
    print("=" * 50)
    
    for v in versions:
        if v in version_data:
            print(f"\nSPDX {v}:")
            for diff in version_data[v]['key_differences']:
                print(f"  • {diff}")

def identify_implementation_requirements():
    """Identify what needs to be implemented for multi-version SPDX support"""
    print(f"\n{'='*70}")
    print("IMPLEMENTATION REQUIREMENTS")
    print(f"{'='*70}")
    
    requirements = {
        'SPDX 2.3': [
            "Tag-value format support (SPDXVersion, DataLicense, etc.)",
            "Package-centric document structure",
            "CreationInfo with creators array",
            "Package verification codes",
            "File-level checksums and license info",
            "Traditional relationship types (CONTAINS, DESCRIBES, etc.)",
            "Annotation support"
        ],
        'SPDX 3.0.0/3.0.1': [
            "JSON-LD format with @context",
            "Element-based architecture (not package-centric)",
            "Enhanced CreationInfo structure",
            "Security profiles and vulnerability info",
            "AI/ML and dataset profiles", 
            "Enhanced relationship types",
            "Bundle and collection support",
            "Licensing profiles with enhanced expressions"
        ],
        'Common Requirements': [
            "Version-specific document generation",
            "License identifier validation",
            "SPDX namespace/ID generation",
            "Checksum/hash validation",
            "Creator/tool information handling",
            "Cross-reference validation"
        ]
    }
    
    for category, items in requirements.items():
        print(f"\n{category}:")
        for item in items:
            print(f"  ✓ {item}")
    
    print(f"\n{'='*70}")
    print("NEXT STEPS")
    print(f"{'='*70}")
    print("1. ✓ Schema analysis complete")
    print("2. → Enhance SBOMGenerator.cpp for SPDX multi-version support")
    print("3. → Create version-specific SPDX generation functions")
    print("4. → Implement comprehensive SPDX validation")
    print("5. → Create perfect SPDX test data for all versions")
    print("6. → Build comprehensive test suite")

def main():
    print("=== SPDX MULTI-VERSION SCHEMA ANALYSIS ===")
    
    schema_files = [
        ('spdx-2.3-schema.json', '2.3'),
        ('spdx-3.0.0-schema.json', '3.0.0'),
        ('spdx-3.0.1-schema.json', '3.0.1')
    ]
    
    analyses = []
    for file_path, version in schema_files:
        analysis = analyze_spdx_schema(file_path, version)
        if analysis:
            analyses.append(analysis)
    
    if analyses:
        compare_spdx_versions(analyses)
        identify_implementation_requirements()
    
    print(f"\n✅ SPDX Schema Analysis Complete!")
    print(f"Ready to proceed with multi-version SPDX implementation.")

if __name__ == "__main__":
    main()