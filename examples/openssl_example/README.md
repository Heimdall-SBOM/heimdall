# Heimdall OpenSSL Example

This example demonstrates how to use Heimdall plugins to generate Software Bill of Materials (SBOMs) for applications that use OpenSSL.

## Overview

The example includes:
- **C Example** (`main.c`): Simple C program using OpenSSL SHA256
- **C++ Example** (`main.cpp`): C++ program using OpenSSL SHA256
- **Build Scripts**: Automated build and SBOM generation for both LLD and Gold linkers
- **Multiple SBOM Formats**: SPDX 2.3, SPDX 3.0, and CycloneDX 1.6 support

## Prerequisites

- Heimdall project built (see main README for build instructions)
- OpenSSL development libraries
- GCC/G++ with LLD and Gold linker support

## Quick Start

### Build and Generate SBOMs

```bash
# Build C example with SBOM generation
./build_c_example.sh

# Build C++ example with SBOM generation  
./build_cpp_example.sh
```

### Manual SBOM Generation

You can also generate SBOMs manually using the `heimdall-sbom` tool:

```bash
# Generate SPDX 2.3 SBOM
../../build/src/tools/heimdall-sbom ../../build/lib/heimdall-lld.so openssl_c_example_lld \
  --format spdx-2.3 --output openssl_c_example.spdx.json

# Generate SPDX 3.0 SBOM
../../build/src/tools/heimdall-sbom ../../build/lib/heimdall-lld.so openssl_c_example_lld \
  --format spdx-3.0 --output openssl_c_example.spdx3.json

# Generate CycloneDX 1.6 SBOM
../../build/src/tools/heimdall-sbom ../../build/lib/heimdall-lld.so openssl_c_example_lld \
  --format cyclonedx-1.6 --output openssl_c_example.cyclonedx.json
```

## Supported SBOM Formats

### SPDX 2.3 (Tag-Value Format)
- **Format**: `spdx-2.3`
- **Output**: Tag-value text format
- **Features**: Full SPDX 2.3 compliance with proper relationships and license validation

### SPDX 3.0 (JSON Format)
- **Format**: `spdx-3.0` or `spdx-3.0.0`
- **Output**: JSON format
- **Features**: Modern SPDX 3.0 JSON structure with enhanced metadata

### CycloneDX 1.6 (JSON Format)
- **Format**: `cyclonedx-1.6`
- **Output**: JSON format
- **Features**: Evidence fields, debug properties, and comprehensive component metadata

## Generated Files

After running the build scripts, you'll get:

### C Example
- `openssl_c_example_lld` - Binary built with LLD
- `openssl_c_example_gold` - Binary built with Gold
- `openssl_c_example.spdx.json` - SPDX 2.3 SBOM
- `openssl_c_example.spdx3.json` - SPDX 3.0 SBOM
- `openssl_c_example.cyclonedx.json` - CycloneDX 1.6 SBOM
- `openssl_c_example_gold.json` - Gold plugin SBOM

### C++ Example
- `openssl_cpp_example_lld` - Binary built with LLD
- `openssl_cpp_example_gold` - Binary built with Gold
- `openssl_cpp_example.spdx.json` - SPDX 2.3 SBOM
- `openssl_cpp_example.spdx3.json` - SPDX 3.0 SBOM
- `openssl_cpp_example.cyclonedx.json` - CycloneDX 1.6 SBOM
- `openssl_cpp_example_gold.json` - Gold plugin SBOM

## SBOM Validation

All generated SBOMs are validated using Heimdall's built-in validator:

```bash
# Validate SPDX SBOM
../../build/src/tools/debug_validator openssl_c_example.spdx.json

# Validate CycloneDX SBOM
../../build/src/tools/debug_validator openssl_c_example.cyclonedx.json
```

## SBOM Content

The generated SBOMs include:

### Components
- **Executable**: The main application binary
- **System Libraries**: libc, libcrypto, libssl
- **Metadata**: File sizes, checksums, versions, licenses

### Relationships
- Package contains all components
- Dependencies between components
- Source file relationships (when debug info available)

### License Information
- **libc**: GPL-2.0-only
- **libcrypto/libssl**: Apache-2.0
- **Application**: Apache-2.0 (inherited from OpenSSL)

## Advanced Usage

### Custom SBOM Generation

You can customize SBOM generation with additional options:

```bash
# Generate with custom SPDX version
../../build/src/tools/heimdall-sbom ../../build/lib/heimdall-lld.so binary \
  --format spdx-3.0.1 --spdx-version 3.0.1 --output custom.spdx.json

# Generate with custom CycloneDX version
../../build/src/tools/heimdall-sbom ../../build/lib/heimdall-lld.so binary \
  --format cyclonedx-1.4 --cyclonedx-version 1.4 --output custom.cyclonedx.json
```

### Plugin Configuration

The heimdall-sbom tool supports various plugin configuration options:

- `--format`: SBOM format (spdx, cyclonedx, etc.)
- `--output`: Output file path
- `--spdx-version`: SPDX specification version
- `--cyclonedx-version`: CycloneDX specification version

## Troubleshooting

### Common Issues

1. **Plugin not found**: Ensure Heimdall is built and plugins are in `../../build/lib/`
2. **Linking errors**: Check that OpenSSL development libraries are installed
3. **SBOM validation fails**: Verify the SBOM format matches the validator expectations

### Debug Information

Enable verbose output by setting the `HEIMDALL_VERBOSE` environment variable:

```bash
export HEIMDALL_VERBOSE=1
./build_c_example.sh
```

## Integration with CI/CD

The build scripts can be easily integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions step
- name: Generate SBOMs
  run: |
    cd examples/openssl_example
    ./build_c_example.sh
    ./build_cpp_example.sh
```

## License

This example is part of the Heimdall project and is licensed under the Apache License 2.0. 