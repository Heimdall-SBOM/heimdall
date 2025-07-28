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
- **Signature Verification**: Cryptographic signature verification for CycloneDX SBOMs
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

### Signature Verification

```bash
# Verify signature using public key
heimdall-validate verify-signature myapp.signed.json --key public.key

# Verify signature using certificate
heimdall-validate verify-signature myapp.signed.json --cert certificate.pem
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

### `verify-signature` - SBOM Signature Verification

Verifies cryptographic signatures in CycloneDX SBOM files using the JSON Signature Format (JSF) specification.

#### Syntax
```bash
heimdall-validate verify-signature <file> [options]
```

#### Options
- `--key <key-file>` - Path to public key file (PEM format)
- `--cert <cert-file>` - Path to certificate file (PEM format)
- `--help` - Show help message

**Note:** Either `--key` or `--cert` must be specified for signature verification.

#### Examples
```bash
# Verify signature using public key
heimdall-validate verify-signature myapp.signed.json --key public.key

# Verify signature using certificate
heimdall-validate verify-signature myapp.signed.json --cert certificate.pem

# Verify signature with verbose output
heimdall-validate verify-signature myapp.signed.json --key public.key --verbose
```

#### Output
```
Loaded public key from: public.key
Verifying signature in myapp.signed.json...

Signature Verification Results:
==============================
✅ Signature is VALID

Signature Details:
  Algorithm: RS256
  Key ID: my-key-id
  Timestamp: 2025-07-28T00:03:13.600Z
  Excluded fields: signature, components[0].signature
```

#### Supported Algorithms
- **RSA**: RS256, RS384, RS512
- **ECDSA**: ES256, ES384, ES512  
- **Ed25519**: Ed25519

#### Signature Structure
The tool verifies signatures according to the JSON Signature Format (JSF) specification:

```json
{
  "signature": {
    "algorithm": "RS256",
    "value": "base64url-encoded-signature",
    "keyId": "optional-key-identifier",
    "timestamp": "2025-07-28T00:03:13.600Z",
    "excludes": ["signature", "components[0].signature"]
  }
}
```

#### Key and Certificate Formats
- **Public Keys**: PEM format RSA, ECDSA, or Ed25519 public keys
- **Certificates**: X.509 certificates in PEM format
- **Key Generation**: Use OpenSSL to generate compatible keys

#### Error Handling
- **Invalid Signature**: Returns error with verification failure details
- **Missing Signature**: Reports when no signature is found in SBOM
- **Invalid Key**: Reports key loading or format errors
- **Unsupported Algorithm**: Reports unsupported signature algorithms

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

### Signature Verification Examples

#### Verify RSA-Signed SBOM
```bash
heimdall-validate verify-signature myapp.rsa-signed.json --key rsa-public.key
```

**Expected Output:**
```
Loaded public key from: rsa-public.key
Verifying signature in myapp.rsa-signed.json...

Signature Verification Results:
==============================
✅ Signature is VALID

Signature Details:
  Algorithm: RS256
  Key ID: my-rsa-key
  Timestamp: 2025-07-28T10:30:00.000Z
  Excluded fields: signature
```

#### Verify ECDSA-Signed SBOM
```bash
heimdall-validate verify-signature myapp.ecdsa-signed.json --cert ecdsa-cert.pem
```

**Expected Output:**
```
Loaded public key from certificate: ecdsa-cert.pem
Verifying signature in myapp.ecdsa-signed.json...

Signature Verification Results:
==============================
✅ Signature is VALID

Signature Details:
  Algorithm: ES256
  Timestamp: 2025-07-28T10:30:00.000Z
  Excluded fields: signature, components[0].signature
```

#### Verify Ed25519-Signed SBOM
```bash
heimdall-validate verify-signature myapp.ed25519-signed.json --key ed25519-public.key
```

**Expected Output:**
```
Loaded public key from: ed25519-public.key
Verifying signature in myapp.ed25519-signed.json...

Signature Verification Results:
==============================
✅ Signature is VALID

Signature Details:
  Algorithm: Ed25519
  Timestamp: 2025-07-28T10:30:00.000Z
  Excluded fields: signature
```

#### Handle Invalid Signature
```bash
heimdall-validate verify-signature myapp.tampered.json --key public.key
```

**Expected Output:**
```
Loaded public key from: public.key
Verifying signature in myapp.tampered.json...

Signature Verification Results:
==============================
❌ Signature is INVALID
Error: Signature verification failed
```

#### Handle Missing Signature
```bash
heimdall-validate verify-signature myapp.unsigned.json --key public.key
```

**Expected Output:**
```
Loaded public key from: public.key
Verifying signature in myapp.unsigned.json...

Signature Verification Results:
==============================
❌ Signature is INVALID
Error: No signature found in SBOM
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

### Signature Verification

#### Key Management

**Generating RSA Keys:**
```bash
# Generate RSA private key
openssl genrsa -out private.key 2048

# Extract public key
openssl rsa -in private.key -pubout -out public.key

# Generate self-signed certificate
openssl req -new -x509 -key private.key -out certificate.pem -days 365 -subj '/CN=My Organization'
```

**Generating ECDSA Keys:**
```bash
# Generate ECDSA private key (P-256)
openssl ecparam -genkey -name prime256v1 -out ecdsa_private.key

# Extract public key
openssl ec -in ecdsa_private.key -pubout -out ecdsa_public.key

# Generate certificate
openssl req -new -x509 -key ecdsa_private.key -out ecdsa_cert.pem -days 365
```

**Generating Ed25519 Keys:**
```bash
# Generate Ed25519 private key
openssl genpkey -algorithm ED25519 -out ed25519_private.key

# Extract public key
openssl pkey -in ed25519_private.key -pubout -out ed25519_public.key
```

#### Batch Signature Verification

**Verify Multiple Signed SBOMs:**
```bash
#!/bin/bash
# verify-signed-sboms.sh

PUBLIC_KEY="$1"
if [ -z "$PUBLIC_KEY" ]; then
    echo "Usage: $0 <public-key-file>"
    exit 1
fi

for file in *.signed.json; do
    echo "Verifying signature in $file..."
    if heimdall-validate verify-signature "$file" --key "$PUBLIC_KEY"; then
        echo "✅ $file: Signature VALID"
    else
        echo "❌ $file: Signature INVALID"
    fi
    echo ""
done
```

**Verify with Certificate:**
```bash
#!/bin/bash
# verify-with-cert.sh

CERT_FILE="$1"
if [ -z "$CERT_FILE" ]; then
    echo "Usage: $0 <certificate-file>"
    exit 1
fi

for file in *.signed.json; do
    echo "Verifying signature in $file..."
    if heimdall-validate verify-signature "$file" --cert "$CERT_FILE"; then
        echo "✅ $file: Signature VALID"
    else
        echo "❌ $file: Signature INVALID"
    fi
    echo ""
done
```

#### CI/CD Integration for Signature Verification

**GitHub Actions Example:**
```yaml
name: SBOM Signature Verification
on: [push, pull_request]

jobs:
  verify-signatures:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Build Heimdall
        run: |
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
      
      - name: Verify SBOM Signatures
        run: |
          # Verify using organization's public key
          ./build/heimdall-validate verify-signature myapp.signed.json --key org-public.key
          
          # Verify using certificate
          ./build/heimdall-validate verify-signature myapp.signed.json --cert org-cert.pem
      
      - name: Verify All Signed SBOMs
        run: |
          for file in *.signed.json; do
            echo "Verifying $file..."
            ./build/heimdall-validate verify-signature "$file" --key org-public.key
          done
```

**Jenkins Pipeline Example:**
```groovy
pipeline {
    agent any
    
    environment {
        PUBLIC_KEY = credentials('org-public-key')
        CERT_FILE = credentials('org-certificate')
    }
    
    stages {
        stage('Verify Signatures') {
            steps {
                sh '''
                    # Verify individual SBOMs
                    ./build/heimdall-validate verify-signature app.signed.json --key $PUBLIC_KEY
                    ./build/heimdall-validate verify-signature libs.signed.json --cert $CERT_FILE
                    
                    # Verify all signed SBOMs
                    for file in *.signed.json; do
                        echo "Verifying $file..."
                        ./build/heimdall-validate verify-signature "$file" --key $PUBLIC_KEY
                    done
                '''
            }
        }
    }
}
```

#### Advanced Signature Verification Scripts

**Comprehensive Verification Script:**
```bash
#!/bin/bash
# comprehensive-verify.sh

SBOM_FILE="$1"
KEY_FILE="$2"
CERT_FILE="$3"

if [ -z "$SBOM_FILE" ]; then
    echo "Usage: $0 <sbom-file> [key-file] [cert-file]"
    exit 1
fi

echo "=== SBOM Signature Verification Report ==="
echo "File: $SBOM_FILE"
echo "Key: ${KEY_FILE:-Not specified}"
echo "Certificate: ${CERT_FILE:-Not specified}"
echo ""

# Check if file exists
if [ ! -f "$SBOM_FILE" ]; then
    echo "❌ Error: SBOM file not found: $SBOM_FILE"
    exit 1
fi

# Verify signature
if [ -n "$KEY_FILE" ]; then
    echo "Verifying with public key: $KEY_FILE"
    if heimdall-validate verify-signature "$SBOM_FILE" --key "$KEY_FILE"; then
        echo "✅ Signature verification PASSED with key"
        exit 0
    else
        echo "❌ Signature verification FAILED with key"
        exit 1
    fi
elif [ -n "$CERT_FILE" ]; then
    echo "Verifying with certificate: $CERT_FILE"
    if heimdall-validate verify-signature "$SBOM_FILE" --cert "$CERT_FILE"; then
        echo "✅ Signature verification PASSED with certificate"
        exit 0
    else
        echo "❌ Signature verification FAILED with certificate"
        exit 1
    fi
else
    echo "❌ Error: Must specify either --key or --cert"
    exit 1
fi
```

**Multi-Key Verification Script:**
```bash
#!/bin/bash
# multi-key-verify.sh

SBOM_FILE="$1"
shift

if [ -z "$SBOM_FILE" ] || [ $# -eq 0 ]; then
    echo "Usage: $0 <sbom-file> <key1> [key2] [key3] ..."
    exit 1
fi

echo "Verifying signature in $SBOM_FILE with multiple keys..."

for key in "$@"; do
    echo "Trying key: $key"
    if heimdall-validate verify-signature "$SBOM_FILE" --key "$key"; then
        echo "✅ Signature verified with key: $key"
        exit 0
    else
        echo "❌ Failed with key: $key"
    fi
done

echo "❌ Signature verification failed with all keys"
exit 1
```

#### Security Best Practices

**Key Management:**
- Store public keys and certificates securely
- Use environment variables or secrets management for sensitive keys
- Rotate keys regularly
- Use different keys for different environments (dev, staging, production)

**Verification Workflow:**
- Always verify signatures before processing SBOMs
- Use multiple verification methods when possible
- Log verification results for audit trails
- Fail builds/processes on signature verification failures

**Error Handling:**
- Handle missing signatures gracefully
- Provide clear error messages for verification failures
- Implement retry logic for temporary failures
- Validate key and certificate formats before use

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

#### 2. "Must specify either --key or --cert for signature verification"
**Cause:** No key or certificate specified for signature verification
**Solution:** Provide either a public key file or certificate file
```bash
heimdall-validate verify-signature myfile.json --key public.key
# or
heimdall-validate verify-signature myfile.json --cert certificate.pem
```

#### 3. "Failed to load public key"
**Cause:** Invalid key file format or file doesn't exist
**Solution:** Check key file format and path
```bash
# Verify key file exists and is readable
ls -la public.key

# Check key format (should be PEM)
head -1 public.key  # Should start with "-----BEGIN PUBLIC KEY-----"

# Generate new key if needed
openssl genrsa -out private.key 2048
openssl rsa -in private.key -pubout -out public.key
```

#### 4. "Failed to load certificate"
**Cause:** Invalid certificate file format or file doesn't exist
**Solution:** Check certificate file format and path
```bash
# Verify certificate file exists and is readable
ls -la certificate.pem

# Check certificate format (should be PEM)
head -1 certificate.pem  # Should start with "-----BEGIN CERTIFICATE-----"

# Validate certificate
openssl x509 -in certificate.pem -text -noout
```

#### 5. "No signature found in SBOM"
**Cause:** SBOM file is not signed or signature field is missing
**Solution:** Verify the SBOM was signed using heimdall-sbom
```bash
# Check if SBOM contains signature field
grep -i "signature" myfile.json

# Sign the SBOM if needed
heimdall-sbom plugin.so binary --format cyclonedx --output myfile.json --sign-key private.key
```

#### 6. "Signature verification failed"
**Cause:** Signature is invalid, tampered, or wrong key used
**Solution:** Check signature integrity and key matching
```bash
# Verify you're using the correct public key
# Check if SBOM was modified after signing
# Ensure the private key used for signing matches the public key used for verification
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
| 3 | Signature verification failed |
| 4 | Key or certificate loading failed |

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
- [JSON Signature Format (JSF) Specification](https://cyberphone.github.io/doc/security/jsf.html)
