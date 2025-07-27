# OpenSSL + Pthreads Demo

This example demonstrates how Heimdall captures and documents external dependencies like OpenSSL and pthreads in Software Bill of Materials (SBOMs).

## Overview

The demo program performs the following operations:
- **OpenSSL Operations**: Initializes SSL/TLS contexts, performs cryptographic hashing (SHA256), and demonstrates various OpenSSL API usage
- **Pthread Operations**: Creates multiple threads, uses mutexes for synchronization, and demonstrates multi-threading capabilities
- **SBOM Generation**: Shows how all dependencies are captured and documented in both SPDX and CycloneDX formats

## What This Demonstrates

### 1. External Library Detection
Heimdall automatically detects and documents:
- **OpenSSL libraries**: `libssl.dylib`, `libcrypto.dylib` with their versions
- **Pthread libraries**: System threading libraries
- **System dependencies**: Standard C libraries, math libraries, etc.
- **Dynamic dependencies**: All libraries loaded at runtime

### 2. Dependency Relationships
The SBOM shows:
- Direct dependencies (what the executable links against)
- Indirect dependencies (what those libraries depend on)
- Version information for each component
- Checksums for integrity verification

### 3. Multiple Output Formats
The demo generates SBOMs in:
- **SPDX format**: Industry-standard format with detailed component information
- **CycloneDX format**: JSON-based format suitable for integration with security tools

## Files

- `main.c`: The demo program using OpenSSL and pthreads
- `CMakeLists.txt`: Build configuration with proper linking
- `demo.sh`: Original demo script (updated to use both LLD and Gold plugins)
- `demo_simple.sh`: Simple demo showing LLD + Gold with SPDX + CycloneDX
- `demo_comprehensive.sh`: Comprehensive demo with multiple compiler/linker combinations
- `README.md`: This documentation

## Prerequisites

- CMake 3.16 or later
- OpenSSL development libraries
- Heimdall project built (see main project README)
- macOS or Linux with pthreads support

## Quick Start

1. **Build the main Heimdall project first**:
   ```bash
   cd /path/to/heimdall
   mkdir build && cd build
   cmake ..
   make
   ```

2. **Choose your demo**:
   
   **Simple Demo** (recommended for first-time users):
   ```bash
   cd examples/openssl_pthread_demo
   ./demo_simple.sh
   ```
   
   **Original Demo** (updated to use both plugins):
   ```bash
   cd examples/openssl_pthread_demo
   ./demo.sh
   ```
   
   **Comprehensive Demo** (advanced users - multiple compiler/linker combinations):
   ```bash
   cd examples/openssl_pthread_demo
   ./demo_comprehensive.sh
   ```

## Manual Build

If you prefer to build manually:

```bash
cd examples/openssl_pthread_demo
mkdir build && cd build
cmake ..
make
./openssl_pthread_demo
```

## Expected Output

The demo will:
1. Build the OpenSSL pthread demo program
2. Run the program showing OpenSSL version and thread operations
3. Generate SBOMs using both LLD and Gold plugins:
   - `openssl_pthread_demo_lld.spdx` (SPDX format, LLD plugin)
   - `openssl_pthread_demo_lld.cyclonedx.json` (CycloneDX format, LLD plugin)
   - `openssl_pthread_demo_gold.spdx` (SPDX format, Gold plugin)
   - `openssl_pthread_demo_gold.cyclonedx.json` (CycloneDX format, Gold plugin)
4. Show previews of the SBOMs highlighting OpenSSL and pthread components
5. Allow comparison between different plugins and formats

## SBOM Contents

The generated SBOMs will include:

### OpenSSL Components
- `libssl.dylib` / `libssl.so` with version information
- `libcrypto.dylib` / `libcrypto.so` with version information
- OpenSSL build information and compiler details
- Cryptographic algorithm implementations

### Pthread Components
- System threading libraries
- Mutex and synchronization primitives
- Thread management functions

### System Components
- Standard C library (`libc`)
- Math library (`libm`)
- Dynamic linker (`libdyld` / `ld-linux`)
- System frameworks and libraries

## Understanding the SBOM

### SPDX Format
```spdx
SPDXID: SPDXRef-libssl-1.1.1w
PackageName: libssl
PackageVersion: 1.1.1w
PackageDownloadLocation: NOASSERTION
FilesAnalyzed: false
PackageChecksum: SHA256: a1b2c3d4e5f6...
PackageLicenseConcluded: OpenSSL
PackageLicenseDeclared: OpenSSL
```

### CycloneDX Format
```json
{
  "bomFormat": "CycloneDX",
  "specVersion": "1.4",
  "components": [
    {
      "type": "library",
      "name": "libssl",
      "version": "1.1.1w",
      "purl": "pkg:generic/libssl@1.1.1w",
      "hashes": [
        {
          "alg": "SHA-256",
          "content": "a1b2c3d4e5f6..."
        }
      ]
    }
  ]
}
```

## Use Cases

This demo is useful for:
- **Security Auditing**: Understanding what cryptographic libraries are used
- **Compliance**: Documenting dependencies for regulatory requirements
- **Vulnerability Assessment**: Identifying components that need security updates
- **License Compliance**: Tracking licenses of all dependencies
- **Supply Chain Security**: Verifying the integrity of all components

## Troubleshooting

### OpenSSL Not Found
```bash
# On macOS with Homebrew
brew install openssl

# On Ubuntu/Debian
sudo apt-get install libssl-dev

# On CentOS/RHEL
sudo yum install openssl-devel
```

### Build Errors
- Ensure Heimdall main project is built first
- Check that OpenSSL development headers are installed
- Verify CMake can find OpenSSL: `cmake --find-package OpenSSL`

### Plugin Loading Issues
- Ensure the Heimdall plugin is built and accessible
- Check file permissions on the plugin
- Verify the plugin path in the demo script

## Next Steps

After running this demo, you can:
1. Examine the full SBOM files to understand dependency structure
2. Use the SBOMs with security scanning tools
3. Integrate SBOM generation into your build process
4. Explore other examples in the `examples/` directory 