# Heimdall

<center><img src="https://github.com/Heimdall-SBOM/heimdall/blob/main/docs/images/logo_w_text.png?raw=true" alt="Heimdall Logo" width="50%"></center>

## Overview
Heimdall is designed for both embedded software development and regular applications on Linux and macOS. It provides comprehensive SBOM generation capabilities with support for digital signatures, Mach-O and ELF binary analysis, and DWARF debug information extraction.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Platform Support](https://img.shields.io/badge/platforms-macOS%20%7C%20Linux-green.svg)](#supported-platforms)
![Tests](https://img.shields.io/badge/tests-546%20passed-brightgreen.svg)
[![C++ Standards](https://img.shields.io/badge/C%2B%2B-11%2F14%2F17%2F20%2F23-blue.svg)](#c-standard-support)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=bugs)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
![Coverage](https://img.shields.io/badge/coverage-44.4%25-yellow.svg)](build/coverage/html/index.html)
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FHeimdall-SBOM%2Fheimdall.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2FHeimdall-SBOM%2Fheimdall?ref=badge_shield)

## Features

### Core Capabilities
- **Dual Linker Support**: Seamless integration with LLVM LLD and GNU Gold linkers
- **Compiler Plugin Integration**: Enhanced SBOM generation with GCC and Clang compiler plugins
- **Multiple SBOM Formats**: Generates SPDX 2.3, 3.0, and 3.0.1 and CycloneDX 1.4, 1.5, and 1.6 compliant reports
- **Comprehensive Component Analysis**: Extracts versions, licenses, checksums, and dependencies
- **Package Manager Integration**: Recognizes Conan, vcpkg, and system packages
- **Cross-Platform**: Native support for macOS and Linux
- **Digital Signatures**: Generate digtially signed SBOM with embedded signature using all algorithms permitted by CycloneDX 1.5+ 

### Binary Analysis
- **ELF Support**: Complete analysis of Linux executables and libraries
- **Mach-O Support**: Full analysis of macOS executables and libraries
- **DWARF Integration**: Extracts debug information for detailed component analysis
- **Archive Analysis**: Processes static libraries (.a, .lib)

### Embedded Software Support
- **Cross-Compilation**: Supports cross-compiled binaries for embedded targets
- **Static Analysis**: Works with statically linked embedded applications

### Security and Compliance
- **Supply Chain Security**: Enables vulnerability scanning and compliance tracking
- **Digital Signature Verification**: Validates signed components
- **License Compliance**: Comprehensive license information extraction
- **Vulnerability Assessment**: Integrates with security scanning tools
- **SBOM Validation**: Validates both SDPX and CycloneDX SBOM

### Performance and Integration
- **Performance Optimized**: Minimal overhead during linking process
- **CI/CD Ready**: Seamless integration with modern build systems using a provided CMake module
- **Multi-Standard C++ Support**: Robust compatibility with C++11, C++14, C++17, C++20, and C++23
- **Multi-Compiler Support**: Automatic detection and selection of GCC and Clang versions
- **Enhanced Source Analysis**: Compiler plugins extract metadata during compilation for richer SBOMs

## Getting Started

### Prerequisites

- **Linux**: GCC 4.8+ or Clang 3.3+, CMake 3.16+, LLVM development libraries
- **macOS**: Xcode Command Line Tools, CMake 3.16+, LLVM (via Homebrew)
- **Git**: For cloning the repository with submodules

### Installation

1. **Clone the repository with submodules**:
   ```bash
   git clone --recurse-submodules https://github.com/Heimdall-SBOM/heimdall.git
   cd heimdall
   ```

2. **Install dependencies using the setup script**:
   ```bash
   # Run the interactive setup script (recommended)
   sudo ./scripts/setup.sh
   
   # Or auto-detect OS and install
   sudo ./scripts/setup.sh --auto-detect
   
   # Or run OS-specific script directly
   sudo ./scripts/setup-ubuntu.sh    # For Ubuntu
   sudo ./scripts/setup-macos.sh     # For macOS
   ```

3. **Build Heimdall**:
   ```bash
   # Build with specific settings (GCC, C++17) - compiler plugins enabled by default
   ./scripts/build.sh --standard 17 --compiler gcc --tests
   
   # Or build all compatible standards
   ./scripts/build_all_standards.sh
   ```

### Supported Distributions

The setup script automatically supports:
- **Ubuntu 22.04+**: GCC 11, 13 + LLVM 18
- **Debian Bookworm**: GCC 11, 12 + LLVM 18
- **CentOS Stream 9**: GCC 11, 13, 14 + LLVM 20
- **Fedora Latest**: GCC 15 + LLVM 18
- **Arch Linux**: GCC 14, 15 + LLVM 18
- **OpenSUSE Tumbleweed**: GCC 11, 13 + LLVM 18
- **Rocky Linux 9**: GCC 11, 13 + LLVM 16

## Building

### Build Scripts

Heimdall provides comprehensive build scripts for different scenarios:

#### Basic Build
```bash
# Build with specific C++ standard and compiler
./scripts/build.sh --standard 17 --compiler gcc --tests

# Build with Clang
./scripts/build.sh --standard 17 --compiler clang --tests
```

#### Advanced Build Options
```bash
# Clean build directory before building
./scripts/build.sh --standard 17 --compiler gcc --clean --tests

# Custom build directory
./scripts/build.sh --standard 17 --compiler gcc --build-dir mybuild --tests

# Build all compatible standards
./scripts/build_all_standards.sh
```

#### Clean Build Artifacts
```bash
# Clean all build artifacts
./scripts/clean.sh

# Clean specific build directory
./scripts/clean.sh --build-dir mybuild
```

### C++ Standard Support

Heimdall supports multiple C++ standards with automatic compiler and LLVM version selection:

| C++ Standard | LLVM Version | GCC Version | Clang Version | Status |
|--------------|--------------|-------------|---------------|--------|
| C++11        | 7+           | 4.8+        | 3.3+          | ✅ Working |
| C++14        | 7+           | 6+          | 3.4+          | ✅ Working |
| C++17        | 11+          | 7+          | 5+            | ✅ Working |
| C++20        | 19+          | 13+         | 14+           | ✅ Working |
| C++23        | 19+          | 13+         | 14+           | ✅ Working |

## Tools

### heimdall-sbom

The `heimdall-sbom` tool generates SBOMs from binary files and provides comprehensive analysis capabilities.

#### Basic Usage
```bash
# Generate SPDX format SBOM
heimdall-sbom /path/to/plugin.so /path/to/binary --format spdx --output binary.spdx

# Generate CycloneDX format SBOM
heimdall-sbom /path/to/plugin.so /path/to/binary --format cyclonedx --output binary.cdx

# Extract debug information
heimdall-sbom /path/to/plugin.so /path/to/binary --debug-info --output binary.spdx
```

#### Advanced Options
```bash
# Include system libraries
heimdall-sbom /path/to/plugin.so /path/to/binary --include-system-libs --output binary.spdx

# Verbose output
heimdall-sbom /path/to/plugin.so /path/to/binary --verbose --output binary.spdx

# Custom configuration file
heimdall-sbom /path/to/plugin.so /path/to/binary --config heimdall.conf --output binary.spdx
```

#### Configuration File
Create a `heimdall.conf` file for custom settings:
```ini
output_path=myapp-sbom.json
format=spdx
verbose=true
extract_debug_info=true
include_system_libraries=false
```

### heimdall-validate

The `heimdall-validate` tool validates SBOM files for compliance, correctness, and signature verification.

#### Basic Validation
```bash
# Validate SPDX file
heimdall-validate --format spdx --input file.spdx

# Validate CycloneDX file
heimdall-validate --format cyclonedx --input file.cdx

# Validate with detailed output
heimdall-validate --format spdx --input file.spdx --verbose
```

#### Advanced Validation
```bash
# Validate against specific schema version
heimdall-validate --format spdx --input file.spdx --schema-version 2.3

# Validate with custom rules
heimdall-validate --format spdx --input file.spdx --rules custom-rules.json

# Generate validation report
heimdall-validate --format spdx --input file.spdx --report validation-report.json
```

## Usage Examples

### Using LLD (macOS/Linux) - Wrapper Approach
```bash
# Step 1: Link normally with LLD
g++ -fuse-ld=lld main.o utils.o math.o -o myapp

# Step 2: Generate SBOM using wrapper tool
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so myapp --format spdx --output myapp.spdx
```

### Using Gold (Linux only) - Plugin Interface
```bash
# Direct plugin integration
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=myapp.spdx \
    main.o utils.o math.o -o myapp

# Or use wrapper approach if plugin fails
g++ -fuse-ld=gold main.o utils.o math.o -o myapp
heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so myapp --format spdx --output myapp.spdx
```

### CMake Integration
```cmake
# Find Heimdall
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

# Add SBOM generation to target
add_custom_command(TARGET myapp POST_BUILD
    COMMAND heimdall-sbom ${HEIMDALL_LLD} $<TARGET_FILE:myapp> 
            --format spdx --output ${CMAKE_BINARY_DIR}/myapp.spdx
    COMMENT "Generating SBOM for myapp"
)
```

### Complex Project with Dependencies
```bash
# Build with multiple libraries using LLD
g++ -fuse-ld=lld main.o libmath.a libutils.so -o complex-app
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so complex-app --format spdx --output complex-app.spdx

# Build with multiple libraries using Gold
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=complex-app.spdx \
    -Wl,--plugin-opt=verbose \
    main.o libmath.a libutils.so -o complex-app
```

## SBOM Formats

### SPDX 2.3/3.0/3.0.1

Heimdall generates SPDX compliant documents with:
- Package information and metadata
- File-level details and relationships
- License information and compliance data
- Checksums (SHA256, SHA1, MD5)
- Component relationships and dependencies
- External references and PURL identifiers

### CycloneDX 1.4/1.5/1.6

Heimdall generates CycloneDX compliant documents with:
- Component metadata and version information
- Dependency relationships and graphs
- External references and PURL identifiers
- Hash information and integrity verification
- License and copyright information
- Vulnerability and security metadata

## Supported Platforms

| Platform | LLD | Gold | Status |
|----------|-----|------|--------|
| macOS (ARM64) | ✅ | ❌ | LLD Only - Gold not available on macOS |
| macOS (x86_64) | ✅ | ❌ | LLD Only - Gold not available on macOS |
| Linux (x86_64) | ✅ | ✅ | Fully Supported |
| Linux (ARM64) | ✅ | ✅ | Fully Supported |

**Note**: GNU Gold linker is primarily designed for Linux systems and is not available on macOS. The Gold plugin will not be built on macOS systems.

## Testing

The project includes a comprehensive test suite with **546 passing tests** across **30 test suites** and **44.4% code coverage**. The test suite validates compatibility across all supported C++ standards and provides extensive error handling and edge case testing.

### Running Tests
```bash
# Test specific C++ standard with gcc
./scripts/build.sh --standard 17 --compiler gcc --tests

# Test specific C++ standard with Clang
./scripts/build.sh --standard 20 --compiler clang --tests

# Run tests with coverage analysis
./tests/coverage.sh
```

## Troubleshooting

### Common Issues

1. **Plugin not found**
   ```
   Error: Could not load plugin
   ```
   Solution: Ensure the plugin path is correct and the plugin is built for your platform.

2. **Missing dependencies**
   ```
   Error: LLVM libraries not found
   ```
   Solution: Install LLVM development packages using the setup script.

3. **Gold linker not found (Linux)**
   ```
   Warning: Gold linker not found
   ```
   Solution: Install `binutils-gold` package for your distribution.

4. **Permission denied**
   ```
   Error: Cannot write to output file
   ```
   Solution: Check write permissions for the output directory.

5. **C++ Standard compatibility issues**
   ```
   Error: 'std::filesystem' is not a member of 'std'
   ```
   Solution: Use the compatibility layer with `heimdall::compat::fs` or build with `--cpp11-14 --no-boost` for older standards.

### Platform-Specific Issues

#### macOS
- **Gold plugin not built**: This is expected behavior. Gold is not available on macOS.
- **LLD path issues**: Ensure LLVM is properly installed via Homebrew and PATH is set correctly.

#### Linux
- **Gold not found**: Install `binutils-gold` package for your distribution.
- **Plugin loading errors**: Ensure Gold was built with plugin support enabled.
- **SCL toolsets**: For C++20/23, you may need to activate SCL toolsets on RHEL/Rocky/CentOS.

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup and contribution guidelines.

### Development Setup
```bash
# Clone with submodules
git clone --recursive https://github.com/Heimdall-SBOM/heimdall.git
cd heimdall

# Build in debug mode
./scripts/build.sh --standard 17 --compiler gcc --tests

# Test all C++ standards
for std in 11 14 17 20 23; do
    echo "Testing C++$std..."
    ./scripts/build.sh --standard $std --compiler gcc --tests
done
```

## Documentation

For detailed information about:
- **Compiler Plugins**: [docs/compiler_plugins.md](docs/compiler_plugins.md)
- **Multi-standard C++ support**: [docs/multi-standard-support.md](docs/multi-standard-support.md)
- **Usage examples**: [docs/usage.md](docs/usage.md)
- **DevContainer updates**: [docs/devcontainer-update-process.md](docs/devcontainer-update-process.md)
- **Container digest updates**: [.github/CONTAINER_DIGEST_UPDATES.md](.github/CONTAINER_DIGEST_UPDATES.md)

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.



[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FHeimdall-SBOM%2Fheimdall.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2FHeimdall-SBOM%2Fheimdall?ref=badge_large)