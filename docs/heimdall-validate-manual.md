# Heimdall Validate - SBOM Validation and Comparison Tool

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Commands Reference](#commands-reference)
5. [Supported Formats](#supported-formats)
6. [Examples](#examples)
7. [Advanced Usage](#advanced-usage)
8. [Troubleshooting](#troubleshooting)
9. [Integration](#integration)

## Overview

`heimdall-validate` is a comprehensive command-line tool for validating, comparing, merging, and diffing Software Bill of Materials (SBOM) files. It supports both SPDX and CycloneDX formats and provides detailed validation reports, comparison analysis, and merge capabilities.

### Key Features

- **Multi-format Support**: SPDX (2.3, 3.0) and CycloneDX (1.4, 1.5, 1.6)
- **Auto-detection**: Automatically detects SBOM format from file content
- **Validation**: Comprehensive schema and structure validation
- **Comparison**: Detailed component-by-component comparison
- **Merging**: Combine multiple SBOMs into a single file
- **Diff Reports**: Generate structured diff reports in multiple formats
- **Version-specific Validation**: Support for different SBOM format versions

## Installation

### Prerequisites

- C++17 or later compiler
- CMake 3.15 or later
- Git

### Building from Source

```bash
# Clone the repository
git clone https://github.com/your-org/heimdall.git
cd heimdall

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)

# The heimdall-validate tool will be available at:
# build/heimdall-validate
```

### Verifying Installation

```bash
./build/heimdall-validate --help
```

## Quick Start

### Basic Validation

```bash
# Validate a CycloneDX SBOM
heimdall-validate validate myapp.cyclonedx.json

# Validate an SPDX SBOM
heimdall-validate validate myapp.spdx

# Auto-detect format
heimdall-validate validate myapp.sbom
```

### Compare Two SBOMs

```bash
# Compare two SBOM files
heimdall-validate compare old.sbom new.sbom

# Generate a diff report
heimdall-validate diff old.sbom new.sbom --report-format json
```

### Merge Multiple SBOMs

```bash
# Merge multiple SBOMs into one
heimdall-validate merge lib1.json lib2.json lib3.json --output merged.json
```

## Commands Reference

### `validate` - SBOM Validation

Validates SBOM files for format compliance and structural integrity.

#### Syntax
```bash
heimdall-validate validate <file> [options]
```

#### Options
- `--format <format>` - Specify SBOM format (spdx, cyclonedx)
- `--version <version>` - Specify version for validation
- `--help` - Show help message

#### Examples
```bash
# Basic validation with auto-detection
heimdall-validate validate myapp.json

# Specify format explicitly
heimdall-validate validate myapp.json --format cyclonedx

# Validate specific version
heimdall-validate validate myapp.json --format cyclonedx --version 1.6

# Validate SPDX with specific version
heimdall-validate validate myapp.spdx --format spdx --version 3.0
```

#### Output
```
Validating myapp.json (cyclonedx format)...
Validation Results:
==================
Valid: Yes
Format: CycloneDX
Version: 1.6

Warnings:
  ⚠️  No components found in SBOM
```

### `compare` - SBOM Comparison

Compares two SBOM files and shows differences.

#### Syntax
```bash
heimdall-validate compare <old-file> <new-file> [options]
```

#### Options
- `--format <format>` - Specify SBOM format (spdx, cyclonedx)
- `--help` - Show help message

#### Examples
```bash
# Compare two SBOMs
heimdall-validate compare old.json new.json

# Compare with specific format
heimdall-validate compare old.spdx new.spdx --format spdx
```

#### Output
```
Comparing old.json and new.json (cyclonedx format)...
Comparison Results:
==================
Total differences: 3
Added: 1
Removed: 0
Modified: 2
Unchanged: 15

➕ Added: libnew.so (libnew-1.2.0)
🔄 Modified: libssl.so (libssl-1.1.1) -> libssl.so (libssl-1.1.1f)
🔄 Modified: libcrypto.so (libcrypto-1.1.1) -> libcrypto.so (libcrypto-1.1.1f)
```

### `diff` - Generate Diff Reports

Generates structured diff reports between two SBOM files.

#### Syntax
```bash
heimdall-validate diff <old-file> <new-file> [options]
```

#### Options
- `--format <format>` - Specify SBOM format (spdx, cyclonedx)
- `--report-format <format>` - Output format (text, json, csv) [default: text]
- `--help` - Show help message

#### Examples
```bash
# Generate text diff report
heimdall-validate diff old.json new.json

# Generate JSON diff report
heimdall-validate diff old.json new.json --report-format json

# Generate CSV diff report
heimdall-validate diff old.json new.json --report-format csv
```

#### Output Formats

**Text Format:**
```
SBOM Diff Report
================
Generated: 2025-07-09T10:30:00Z
Old File: old.json
New File: new.json
Format: CycloneDX

Summary:
- Added: 1 component
- Removed: 0 components
- Modified: 2 components
- Unchanged: 15 components

Details:
[ADDED] libnew.so (libnew-1.2.0)
[MODIFIED] libssl.so: 1.1.1 -> 1.1.1f
[MODIFIED] libcrypto.so: 1.1.1 -> 1.1.1f
```

**JSON Format:**
```json
{
  "metadata": {
    "generated": "2025-07-09T10:30:00Z",
    "oldFile": "old.json",
    "newFile": "new.json",
    "format": "cyclonedx"
  },
  "summary": {
    "added": 1,
    "removed": 0,
    "modified": 2,
    "unchanged": 15
  },
  "differences": [
    {
      "type": "added",
      "component": {
        "name": "libnew.so",
        "version": "1.2.0",
        "id": "libnew-1.2.0"
      }
    }
  ]
}
```

### `merge` - SBOM Merging

Merges multiple SBOM files into a single SBOM.

#### Syntax
```bash
heimdall-validate merge <files...> --output <output-file> [options]
```

#### Options
- `--output <file>` - Output file path (required)
- `--format <format>` - Output format (spdx, cyclonedx) [default: cyclonedx]
- `--version <version>` - Output version [default: 1.6 for CycloneDX, 3.0 for SPDX]
- `--help` - Show help message

#### Examples
```bash
# Merge multiple CycloneDX files
heimdall-validate merge lib1.json lib2.json lib3.json --output merged.json

# Merge with specific format and version
heimdall-validate merge lib1.json lib2.json --output merged.spdx --format spdx --version 2.3

# Merge SPDX files
heimdall-validate merge app1.spdx app2.spdx --output merged.spdx --format spdx
```

#### Output
```
Merging 3 SBOM files...
Merged SBOM written to: merged.json
```

## Supported Formats

### CycloneDX Support

| Version | Status | Features |
|---------|--------|----------|
| 1.4 | ✅ Full Support | Basic validation, component structure |
| 1.5 | ✅ Full Support | Enhanced validation, additional fields |
| 1.6 | ✅ Full Support | Latest features, complete validation |

**Validation Features:**
- Required field validation (`bomFormat`, `specVersion`, `version`, `metadata`)
- Component structure validation
- Hash format validation
- UUID format validation
- Metadata timestamp validation

### SPDX Support

| Version | Status | Format | Features |
|---------|--------|--------|----------|
| 2.3 | ✅ Full Support | Tag-value | Complete validation |
| 3.0 | ✅ Full Support | JSON | Complete validation |
| 3.0.0 | ✅ Full Support | JSON | Complete validation |
| 3.0.1 | ✅ Full Support | JSON | Complete validation |

**Validation Features:**
- Required field validation (`SPDXVersion`, `DataLicense`, `DocumentName`, etc.)
- License expression validation
- SPDX identifier validation
- Package and file information validation

## Examples

### Validation Examples

#### Validate a CycloneDX 1.6 SBOM
```bash
heimdall-validate validate myapp-v1.6.json --format cyclonedx --version 1.6
```

**Expected Output:**
```
Validating myapp-v1.6.json (cyclonedx, version 1.6 format)...
Validation Results:
==================
Valid: Yes
Format: CycloneDX
Version: 1.6
```

#### Validate an SPDX 2.3 SBOM
```bash
heimdall-validate validate myapp.spdx --format spdx --version 2.3
```

**Expected Output:**
```
Validating myapp.spdx (spdx, version 2.3 format)...
Validation Results:
==================
Valid: Yes
Format: SPDX 2.3
Version: 2.3
```

### Comparison Examples

#### Compare Application Versions
```bash
heimdall-validate compare app-v1.0.json app-v1.1.json
```

**Expected Output:**
```
Comparing app-v1.0.json and app-v1.1.json (cyclonedx format)...
Comparison Results:
==================
Total differences: 2
Added: 1
Removed: 0
Modified: 1
Unchanged: 12

➕ Added: libnewfeature.so (libnewfeature-1.0.0)
🔄 Modified: myapp (1.0.0) -> myapp (1.1.0)
```

#### Compare Different Builds
```bash
heimdall-validate compare debug-build.json release-build.json
```

### Diff Report Examples

#### Generate JSON Diff Report
```bash
heimdall-validate diff old.json new.json --report-format json > diff-report.json
```

#### Generate CSV Diff Report
```bash
heimdall-validate diff old.json new.json --report-format csv > diff-report.csv
```

### Merge Examples

#### Merge Library SBOMs
```bash
heimdall-validate merge libssl.json libcrypto.json libzlib.json --output all-libs.json
```

#### Merge with Specific Format
```bash
heimdall-validate merge app.json libs.json --output complete.spdx --format spdx --version 2.3
```

## Advanced Usage

### Batch Processing

#### Validate Multiple Files
```bash
#!/bin/bash
for file in *.json; do
    echo "Validating $file..."
    heimdall-validate validate "$file"
    if [ $? -eq 0 ]; then
        echo "✅ $file is valid"
    else
        echo "❌ $file has validation errors"
    fi
done
```

#### Compare All Versions
```bash
#!/bin/bash
for version in v1.0 v1.1 v1.2; do
    echo "Comparing $version with previous..."
    if [ "$version" != "v1.0" ]; then
        prev_version=$(echo $version | sed 's/v1.\([0-9]\)/v1.$((\1-1))/')
        heimdall-validate compare "app-$prev_version.json" "app-$version.json"
    fi
done
```

### Integration with CI/CD

#### GitHub Actions Example
```yaml
name: SBOM Validation
on: [push, pull_request]

jobs:
  validate-sbom:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Build Heimdall
        run: |
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
      
      - name: Validate SBOMs
        run: |
          ./build/heimdall-validate validate myapp.cyclonedx.json
          ./build/heimdall-validate validate myapp.spdx
      
      - name: Compare with Previous
        if: github.event_name == 'push'
        run: |
          ./build/heimdall-validate compare previous.json current.json
```

#### Jenkins Pipeline Example
```groovy
pipeline {
    agent any
    
    stages {
        stage('Generate SBOM') {
            steps {
                sh 'make sbom'
            }
        }
        
        stage('Validate SBOM') {
            steps {
                sh './build/heimdall-validate validate myapp.cyclonedx.json'
                sh './build/heimdall-validate validate myapp.spdx'
            }
        }
        
        stage('Compare SBOM') {
            when {
                changeSet()
            }
            steps {
                sh './build/heimdall-validate compare old.json new.json'
            }
        }
    }
}
```

### Custom Validation Scripts

#### Comprehensive Validation Script
```bash
#!/bin/bash
# validate-sbom.sh

SBOM_FILE="$1"
FORMAT="$2"
VERSION="$3"

if [ -z "$SBOM_FILE" ]; then
    echo "Usage: $0 <sbom-file> [format] [version]"
    exit 1
fi

echo "=== SBOM Validation Report ==="
echo "File: $SBOM_FILE"
echo "Format: ${FORMAT:-auto-detected}"
echo "Version: ${VERSION:-auto-detected}"
echo ""

# Run validation
if heimdall-validate validate "$SBOM_FILE" ${FORMAT:+--format $FORMAT} ${VERSION:+--version $VERSION}; then
    echo "✅ Validation PASSED"
    exit 0
else
    echo "❌ Validation FAILED"
    exit 1
fi
```

## Troubleshooting

### Common Issues

#### 1. "Cannot auto-detect SBOM format"
**Cause:** File doesn't contain recognizable SPDX or CycloneDX headers
**Solution:** Specify format explicitly
```bash
heimdall-validate validate myfile --format cyclonedx
```

#### 2. "Unsupported CycloneDX version"
**Cause:** File uses CycloneDX version not supported (1.0-1.3, 2.0+)
**Solution:** Use supported versions (1.4, 1.5, 1.6)
```bash
# Convert to supported version or use different tool
```

#### 3. "File does not exist"
**Cause:** Invalid file path or permissions issue
**Solution:** Check file path and permissions
```bash
ls -la myfile.json
heimdall-validate validate ./myfile.json
```

#### 4. "Cannot open file"
**Cause:** File permissions or file system issues
**Solution:** Check file permissions and disk space
```bash
chmod 644 myfile.json
df -h .
```

### Debug Mode

Enable verbose output for debugging:
```bash
heimdall-validate validate myfile.json --verbose
```

### Error Codes

| Exit Code | Meaning |
|-----------|---------|
| 0 | Success |
| 1 | Validation failed or error |
| 2 | Invalid command line arguments |

### Performance Tips

1. **Use specific format detection** when possible to avoid auto-detection overhead
2. **Process large files** in smaller batches for memory efficiency
3. **Use JSON output** for programmatic processing
4. **Cache validation results** for repeated operations

## Integration

### With Other Tools

#### Integration with CycloneDX Tools
```bash
# Validate with multiple tools
heimdall-validate validate myapp.json
cyclonedx-cli validate myapp.json
```

#### Integration with SPDX Tools
```bash
# Validate with multiple tools
heimdall-validate validate myapp.spdx
spdx-tools-validate myapp.spdx
```

### API Integration

The tool can be integrated into custom scripts and applications:

```python
import subprocess
import json

def validate_sbom(file_path, format=None, version=None):
    cmd = ['heimdall-validate', 'validate', file_path]
    if format:
        cmd.extend(['--format', format])
    if version:
        cmd.extend(['--version', version])
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0, result.stdout, result.stderr

# Usage
is_valid, output, errors = validate_sbom('myapp.json', 'cyclonedx', '1.6')
```

### Configuration Files

Create configuration files for repeated operations:

```json
{
  "validation": {
    "default_format": "cyclonedx",
    "default_version": "1.6",
    "strict_mode": true
  },
  "comparison": {
    "ignore_versions": false,
    "ignore_hashes": false
  },
  "merge": {
    "default_output_format": "cyclonedx",
    "deduplicate": true
  }
}
```

## Conclusion

`heimdall-validate` provides comprehensive SBOM validation, comparison, and manipulation capabilities. It supports the latest SBOM standards and integrates well with existing toolchains and CI/CD pipelines.

For more information, see:
- [Heimdall User Guide](heimdall-users-guide.md)
- [SBOM Validation Guide](sbom_validation_guide.md)
- [CycloneDX Specification](https://cyclonedx.org/specification/)
