#!/bin/bash

echo "=== SBOM Output Comparison Script ==="
echo "===================================="

# Function to compare two JSON files
compare_json_files() {
    local file1="$1"
    local file2="$2"
    local description="$3"
    
    echo "Comparing $description:"
    echo "  File 1: $file1"
    echo "  File 2: $file2"
    
    if [ ! -f "$file1" ] || [ ! -f "$file2" ]; then
        echo "  ERROR: One or both files do not exist"
        return 1
    fi
    
    # Check file sizes
    size1=$(wc -c < "$file1")
    size2=$(wc -c < "$file2")
    echo "  File sizes: $size1 bytes vs $size2 bytes"
    
    if [ "$size1" -eq "$size2" ]; then
        echo "  ✓ File sizes match"
    else
        echo "  ✗ File sizes differ"
    fi
    
    # Compare content (ignoring timestamps and other variable fields)
    if diff <(jq -S 'del(.metadata.timestamp)' "$file1") <(jq -S 'del(.metadata.timestamp)' "$file2") > /dev/null 2>&1; then
        echo "  ✓ Content matches (ignoring timestamps)"
        return 0
    else
        echo "  ✗ Content differs"
        echo "  Differences (ignoring timestamps):"
        diff <(jq -S 'del(.metadata.timestamp)' "$file1") <(jq -S 'del(.metadata.timestamp)' "$file2") | head -20
        return 1
    fi
}

# Function to extract key metrics from SBOM
extract_metrics() {
    local file="$1"
    local description="$2"
    
    echo "Metrics for $description ($file):"
    
    if [ ! -f "$file" ]; then
        echo "  ERROR: File does not exist"
        return 1
    fi
    
    # Count components
    component_count=$(jq '.components | length' "$file" 2>/dev/null || echo "0")
    echo "  Components: $component_count"
    
    # Count dependencies
    dependency_count=$(jq '[.components[].dependencies // empty] | length' "$file" 2>/dev/null || echo "0")
    echo "  Dependencies: $dependency_count"
    
    # Count symbols (if available)
    symbol_count=$(jq '[.components[].symbols // empty] | length' "$file" 2>/dev/null || echo "0")
    echo "  Symbols: $symbol_count"
    
    # Count source files (if available)
    source_count=$(jq '[.components[].sourceFiles // empty] | length' "$file" 2>/dev/null || echo "0")
    echo "  Source files: $source_count"
    
    echo ""
}

# Main comparison logic
echo "Baseline files:"
ls -la baseline_*.json
echo ""

# Extract metrics from baseline files
extract_metrics "baseline_spdx.json" "Baseline SPDX"
extract_metrics "baseline_cyclonedx.json" "Baseline CycloneDX"

echo "=== Comparison Results ==="

# Compare baseline files (they should be identical to themselves)
echo "Self-comparison (sanity check):"
compare_json_files "baseline_spdx.json" "baseline_spdx.json" "SPDX with itself"
compare_json_files "baseline_cyclonedx.json" "baseline_cyclonedx.json" "CycloneDX with itself"

echo ""
echo "=== Instructions for After Optimization ==="
echo "After implementing optimizations, run:"
echo "1. ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format spdx --output=optimized_spdx.json"
echo "2. ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format cyclonedx --output=optimized_cyclonedx.json"
echo "3. ./compare_sbom_outputs.sh"
echo ""
echo "Then compare:"
echo "- baseline_spdx.json vs optimized_spdx.json"
echo "- baseline_cyclonedx.json vs optimized_cyclonedx.json"
echo ""
echo "Expected result: Files should be identical (except for timestamps)" 