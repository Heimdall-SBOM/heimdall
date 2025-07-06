<img src="https://github.com/bakkertj/heimdall/blob/18e0ce7fc630923f15e6e29e94059e40b7d1af99/docs/images/logo_w_text.png" width="400">

A comprehensive Software Bill of Materials (SBOM) generation plugin that works with both LLVM LLD and GNU Gold linkers. Heimdall automatically generates accurate SBOMs during the linking process, capturing all components that actually make it into your final binaries, including comprehensive DWARF debug information.

[![Build Status](https://github.com/heimdall-sbom/heimdall/actions/workflows/ci.yml/badge.svg)](https://github.com/heimdall-sbom/heimdall/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Platform Support](https://img.shields.io/badge/platforms-macOS%20%7C%20Linux-green.svg)](#supported-platforms)
[![Tests](https://img.shields.io/badge/tests-166%20passed-brightgreen.svg)](#testing)
[![DWARF Support](https://img.shields.io/badge/DWARF-Enhanced-orange.svg)](#dwarf-debug-information-support)
[![SBOM Validation](https://img.shields.io/badge/SBOM-Validated-brightgreen.svg)](#sbom-validation)

## Features

- **Dual Linker Support**: Works seamlessly with both LLVM LLD and GNU Gold linkers
- **Multiple SBOM Formats**: Generates SPDX 2.3 and CycloneDX 1.6 compliant reports (1.6 is default, 1.4/1.5 can be selected)
- **Enhanced DWARF Integration**: Extracts and includes source files, functions, and compile units in SBOMs
- **Comprehensive Component Analysis**: Extracts versions, licenses, checksums, and dependencies
- **Package Manager Integration**: Recognizes Conan, vcpkg, and system packages
- **Cross-Platform**: Native support for macOS and Linux
- **Performance Optimized**: Minimal overhead during linking
- **CI/CD Ready**: Seamless integration with modern build systems
- **Security Focused**: Enables vulnerability scanning and compliance tracking
- **Comprehensive Testing**: Full unit test suite with real shared library testing
- **SBOM Validation**: Built-in validation tools for standards compliance
- **Shared Library SBOMs**: Automatic generation of SBOMs for all built shared libraries

## DWARF Debug Information Support

Heimdall now provides **comprehensive DWARF debug information integration** that enhances your SBOMs with detailed source code traceability:

### **Source File Tracking**
- **Automatic extraction** of source file paths from DWARF debug info
- **Separate SBOM components** for each source file with checksums and metadata
- **Parent-child relationships** between binaries and their source files
- **License detection** for source files

### **Function-Level Analysis**
- **Function names** extracted from DWARF and included in SBOM properties
- **Compile unit information** for build system integration
- **Debug information flags** indicating presence of DWARF data
- **Fallback extraction** using ELF symbol table when DWARF parsing fails

### **Standards Compliance**
- **SPDX 2.3**: Source files as separate `FileName` entries with `GENERATED_FROM` relationships
- **CycloneDX 1.6+**: DWARF data as component properties with `heimdall:` namespace (default is 1.6, can specify 1.4/1.5)
- **Full interoperability** with existing SBOM tools and workflows

### **Usage Examples**

```bash
# Generate SBOM with DWARF debug information
gcc -g -c main.c -o main.o

# LLD with enhanced DWARF support and CycloneDX 1.6 (default)
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:myapp.cyclonedx.json \
       --plugin-opt=format:cyclonedx \
       main.o -o myapp

# LLD with CycloneDX 1.4 explicitly
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:myapp.cyclonedx.json \
       --plugin-opt=format:cyclonedx \
       --plugin-opt=cyclonedx-version:1.4 \
       main.o -o myapp

# Gold with CycloneDX 1.5 explicitly
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.cyclonedx.json \
        --plugin-opt format=cyclonedx \
        --plugin-opt cyclonedx-version=1.5 \
        main.o -o myapp
```

### **Generated SBOM Content**

**CycloneDX with DWARF Properties:**
```json
{
  "properties": [
    {
      "name": "heimdall:source-files",
      "value": "main.c,utils.c,header.h"
    },
    {
      "name": "heimdall:functions", 
      "value": "main,calculate,process_data"
    },
    {
      "name": "heimdall:contains-debug-info",
      "value": "true"
    }
  ]
}
```

**SPDX with Source File Relationships:**
```
FileName: main.c
SPDXID: SPDXRef-main.c
FileChecksum: SHA256: abc123...
LicenseConcluded: MIT

Relationship: SPDXRef-myapp GENERATED_FROM SPDXRef-main.c
```

## SBOM Validation

Heimdall includes comprehensive validation tools to ensure your SBOMs are standards compliant:

### **Automated Validation**
```bash
# Basic validation (JSON syntax, SPDX structure)
./scripts/validate_sboms.sh build

# Advanced validation (schema validation, detailed reporting)
python3 scripts/validate_sboms_online.py build
```

### **Validation Features**
- **JSON syntax validation** for CycloneDX files
- **SPDX structure validation** (supports both package and file formats)
- **Required field checking** for both standards
- **Detailed validation logs** and summary reports
- **CI/CD integration** examples
- **Online validation** tool references

### **Validation Results**
✅ **All 16 Heimdall SBOMs pass validation:**
- 8 CycloneDX files (JSON syntax and schema compliant)
- 8 SPDX files (structure and format compliant)
- Full compliance with SPDX 2.3 and CycloneDX 1.6 standards

## Shared Library SBOM Generation

Heimdall automatically generates SBOMs for all built shared libraries:

### **Generated SBOMs**
- **Main binary**: 4 SBOMs (SPDX/CycloneDX × LLD/Gold plugins)
- **Shared libraries**: 10 SBOMs (3 libraries × 2 formats × 2 plugins)
- **Total**: 14 SBOM files with extended DWARF information

### **Shared Libraries Covered**
- `heimdall-gold.so` (Gold plugin)
- `heimdall-lld.so` (LLD plugin)
- `libheimdall-core.so.1.0.0` (Core library)

### **Usage**
```bash
# Build and generate all SBOMs
./build.sh
make build-test-sbom

# SBOMs are generated in build/sboms/
ls build/sboms/
# heimdall-build-*.{spdx,cyclonedx.json}
# heimdall-gold-*.{spdx,cyclonedx.json}
# heimdall-lld-*.{spdx,cyclonedx.json}
# libheimdall-core-*.{spdx,cyclonedx.json}
```

## Quick Start

### Installation

```bash
git clone https://github.com/heimdall-sbom/heimdall.git
cd heimdall
./build.sh
```

### Usage

```bash
# Using LLD (macOS/Linux) with DWARF support
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:myapp.json \
       main.o -o myapp

# Using Gold (Linux only) with DWARF support
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.json \
        main.o -o myapp
```

### Build System Integration

#### CMake
```cmake
find_library(HEIMDALL_LLD heimdall-lld)
target_link_options(myapp PRIVATE
    "LINKER:--plugin-opt=load:${HEIMDALL_LLD}"
    "LINKER:--plugin-opt=sbom-output:${CMAKE_BINARY_DIR}/myapp.json"
    "LINKER:--plugin-opt=format:cyclonedx"
)
```

#### Makefile
```makefile
LDFLAGS += -Wl,--plugin-opt=load:/usr/lib/heimdall-plugins/heimdall-lld.dylib
LDFLAGS += -Wl,--plugin-opt=sbom-output:$(TARGET).json
LDFLAGS += -Wl,--plugin-opt=format:cyclonedx
```

## Architecture

Heimdall consists of several key components:

### Core Components

- **ComponentInfo**: Represents individual software components with metadata including DWARF debug information
- **SBOMGenerator**: Generates SBOMs in multiple formats (SPDX, CycloneDX) with enhanced DWARF data
- **MetadataExtractor**: Extracts metadata from binary files (ELF, Mach-O, PE, archives) including DWARF debug info
- **DWARFExtractor**: Robust DWARF debug information extraction using LLVM libraries with fallback symbol table extraction
- **Utils**: Utility functions for file operations, path handling, and JSON formatting
- **PluginInterface**: Common interface for both LLD and Gold plugins

### Plugin Adapters

- **LLDAdapter**: Interfaces with LLVM LLD linker plugin system
- **GoldAdapter**: Interfaces with GNU Gold linker plugin system

### File Format Support

- **ELF**: Linux executables and libraries with full DWARF support
- **Mach-O**: macOS executables and libraries with DWARF support
- **PE**: Windows executables and libraries (basic support)
- **Archive**: Static libraries (.a, .lib) with member extraction

## Supported Platforms

| Platform | LLD | Gold | DWARF Support | Status |
|----------|-----|------|---------------|--------|
| macOS (ARM64) | ✅ | ❌ | ✅ Full | LLD Only - Gold not available on macOS |
| macOS (x86_64) | ✅ | ❌ | ✅ Full | LLD Only - Gold not available on macOS |
| Linux (x86_64) | ✅ | ✅ | ✅ Full | Fully Supported |
| Linux (ARM64) | ✅ | ✅ | ✅ Full | Fully Supported |

**Note**: GNU Gold linker is primarily designed for Linux systems and is not available on macOS. The Gold plugin will not be built on macOS systems.

## Building from Source

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- LLVM/LLD development libraries (LLVM 19 recommended for full DWARF support)
- GNU Gold linker (Linux only, optional for Gold plugin)
- OpenSSL development libraries

### macOS Setup

```bash
# Install LLVM/LLD and other dependencies
brew install llvm cmake ninja openssl
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build Heimdall (LLD plugin only)
./build.sh
```

### Linux Setup

#### Ubuntu/Debian
```bash
# Install all dependencies including Gold linker
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    binutils \
    binutils-dev \
    libssl-dev \
    libelf-dev \
    libgtest-dev \
    pkg-config

# Install LLVM 19 (recommended for full DWARF support)
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" | sudo tee /etc/apt/sources.list.d/llvm.list
sudo apt-get update
sudo apt-get install -y llvm-19-dev liblld-19-dev

# Build Heimdall with both LLD and Gold plugins
./build.sh
```

**Note**: 
- LLVM 19 is recommended for full DWARF debug info support. The build system will automatically detect and use LLVM 19 if available.
- `binutils` provides the Gold linker (`ld.gold`), while `binutils-dev` provides the BFD development files needed for the Gold plugin.
- `libgtest-dev` provides GoogleTest for unit testing. If not available, the build system will download it automatically.
- The build system now includes Ubuntu-specific library paths (`/usr/lib/x86_64-linux-gnu`) for proper system library detection.

**Troubleshooting BFD headers**: If you encounter "bfd.h not found" errors, try installing additional packages:
```bash
sudo apt-get install -y binutils-dev libbfd-dev
```

**Alternative LLVM versions**: If you prefer to use a different LLVM version, you can install it instead:
```bash
# For LLVM 14 (Ubuntu default)
sudo apt-get install -y llvm-dev liblld-14-dev

# For LLVM 15
sudo apt-get install -y llvm-15-dev liblld-15-dev

# For LLVM 16
sudo apt-get install -y llvm-16-dev liblld-16-dev
```

**Library Detection**: Heimdall now properly detects system libraries on Ubuntu including:
- OpenSSL libraries (`libssl.so.3`, `libcrypto.so.3`)
- System C library (`libc.so.6`)
- Pthread library (`libpthread.so.0`)
- Other system libraries in `/usr/lib/x86_64-linux-gnu/`

#### Fedora/RHEL/CentOS
```bash
# Install dependencies (required for build and tests)
sudo yum install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    llvm-devel \
    lld-devel \
    binutils-gold \
    openssl-devel \
    pkgconfig \
    zlib-devel \
    llvm-googletest \
    binutils-devel

# Build Heimdall
./build.sh
```

**Note**: The `binutils-devel` package provides the BFD headers required for the Gold plugin. Without this package, the Gold plugin will not be built, but the LLD plugin and core library will still work.

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S \
    base-devel \
    cmake \
    ninja \
    llvm \
    lld \
    binutils \
    openssl \
    pkgconf

# Build Heimdall
./build.sh
```

### Installing GNU Gold Linker

For detailed installation instructions, see [docs/gold-installation.md](docs/gold-installation.md).

#### Linux (Recommended)

GNU Gold is the default linker on most modern Linux distributions. If not available:

**Ubuntu/Debian:**
```bash
sudo apt-get install binutils-gold
```

**Fedora/RHEL/CentOS:**
```bash
sudo yum install binutils-gold
```

**Arch Linux:**
```bash
sudo pacman -S binutils
```

**Verify Gold installation:**
```bash
ld.gold --version
```

#### macOS (Not Supported)

GNU Gold is not available on macOS because:
- Gold is designed for ELF format binaries (Linux)
- macOS uses Mach-O format binaries
- Apple's linker ecosystem is different from Linux

**Alternative on macOS:**
- Use LLVM LLD linker (fully supported)
- LLD provides similar performance benefits to Gold
- LLD has better macOS integration

#### Building Gold from Source (Linux only)

If you need a specific version of Gold:

```bash
# Download and build binutils with Gold
wget https://ftp.gnu.org/gnu/binutils/binutils-2.44.tar.gz
tar xf binutils-2.44.tar.gz
cd binutils-2.44

# Configure with Gold enabled
./configure --enable-gold --enable-plugins --prefix=/usr/local
make -j$(nproc)
sudo make install

# Verify installation
/usr/local/bin/ld.gold --version
```

### Build Options

```bash
# Debug build with sanitizers
./build.sh --debug --sanitizers

# Build only LLD plugin (macOS default)
./build.sh --no-gold

# Build only Gold plugin (Linux only)
./build.sh --no-lld

# Custom build directory
./build.sh --build-dir mybuild --install-dir myinstall

# Force build both plugins (Linux only)
./build.sh --force-gold
```

### Cleaning Build Artifacts

For a complete clean checkout (removes all build artifacts):

```bash
# Comprehensive clean (recommended for check-in)
./clean.sh

# Or with custom directories
./clean.sh --build-dir mybuild --install-dir myinstall
```

For incremental clean (removes only object files and binaries):

```bash
cd build
make clean
```

**Note**: `make clean` only removes compiled objects and binaries but leaves CMake cache files, installed files, and build configuration. Use `./clean.sh` for a complete clean checkout.

## Testing

The project includes a comprehensive test suite using Google Test. To run the tests:

```bash
# Build and run tests
./build.sh
cd build
ctest --output-on-failure
```

For more detailed test information, see [tests/README.md](tests/README.md).

## Code Coverage

The project supports code coverage analysis using gcov and lcov. Coverage reports help identify untested code paths and ensure comprehensive testing.

### Enabling Coverage

To enable code coverage, build with the `ENABLE_COVERAGE` option:

```bash
# From project root
mkdir -p build
cd build
cmake -DENABLE_COVERAGE=ON ..
make -j$(nproc)
```

### Running Coverage Analysis

#### Using the Coverage Script (Recommended)

```bash
# From project root
./tests/coverage.sh
```

This script will:
- Automatically enable coverage if not already enabled
- Build the project with coverage instrumentation
- Run all tests to generate coverage data
- Generate both text and HTML coverage reports
- Display a summary of coverage results

#### Using the Simple Coverage Script

For basic coverage information:

```bash
# From project root
./tests/simple_coverage.sh
```

#### Using CMake Targets

```bash
# From build directory
make coverage        # Run tests and generate coverage
make coverage-clean  # Clean coverage data
```

### Coverage Reports

Coverage reports are generated in the `build/coverage/` directory:

- `coverage_summary.txt`: Text summary of coverage statistics
- `basic_coverage_report.txt`: Basic coverage information
- `coverage.info`: lcov coverage data file
- `html/`: HTML coverage report (if lcov is available)

### Current Coverage

The current test suite provides:
- **Line Coverage**: ~76.2%
- **Function Coverage**: ~80.1%

### Coverage Requirements

- **GCC/G++**: Coverage instrumentation is built into GCC
- **lcov** (optional): For HTML coverage reports
  - Ubuntu/Debian: `sudo apt-get install lcov`
  - CentOS/RHEL: `sudo yum install lcov`
  - macOS: `brew install lcov`

### Coverage Best Practices

1. **Regular Coverage Runs**: Run coverage analysis regularly during development
2. **Coverage Goals**: Aim for high coverage (>80%) on critical code paths
3. **Coverage Gaps**: Use coverage reports to identify untested code
4. **Coverage Cleanup**: Clean coverage data between runs for accurate results
5. **CI Integration**: Include coverage analysis in continuous integration

## Configuration

### Plugin Options

#### LLD Plugin Options
```bash
--plugin-opt=load:./heimdall-lld.dylib
--plugin-opt=sbom-output:output.json
--plugin-opt=format:spdx
--plugin-opt=verbose
--plugin-opt=no-debug-info
--plugin-opt=include-system-libs
```

#### Gold Plugin Options (Linux only)
```bash
--plugin ./heimdall-gold.so
--plugin-opt sbom-output=output.json
--plugin-opt format=cyclonedx
--plugin-opt verbose
--plugin-opt no-debug-info
--plugin-opt include-system-libs
```

### Configuration File

Create a `heimdall.conf` file:

```ini
output_path=myapp-sbom.json
format=spdx
verbose=true
extract_debug_info=true
include_system_libraries=false
```

## SBOM Formats

### SPDX 2.3

Heimdall generates SPDX 2.3 compliant documents with:

- Package information
- File-level details including source files
- License information
- Checksums (SHA256)
- Relationships between components including `GENERATED_FROM` for source files

### CycloneDX 1.6

Heimdall generates CycloneDX 1.6 compliant documents with:

- Component metadata
- Dependencies
- External references
- PURL identifiers
- Hash information
- Enhanced properties with DWARF debug information

## Examples

### Simple C Program with DWARF Support

```bash
# Compile with debug information
gcc -g -c main.c -o main.o

# Using LLD (macOS/Linux) with enhanced DWARF support
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:main-sbom.json \
       --plugin-opt=format:cyclonedx \
       main.o -o main

# Using Gold (Linux only) with enhanced DWARF support
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=main-sbom.json \
        --plugin-opt format=cyclonedx \
        main.o -o main

# View generated SBOM with DWARF data
cat main-sbom.json
```

### CMake Project with DWARF Integration

```cmake
# Find Heimdall
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

# Add SBOM generation to target with DWARF support
target_link_options(myapp PRIVATE
    "LINKER:--plugin-opt=load:${HEIMDALL_LLD}"
    "LINKER:--plugin-opt=sbom-output:${CMAKE_BINARY_DIR}/myapp-sbom.json"
    "LINKER:--plugin-opt=format:cyclonedx"
)

# Enable debug information for DWARF extraction
target_compile_options(myapp PRIVATE -g)
```

### Complex Project with Dependencies and DWARF

```bash
# Build with multiple libraries using LLD and DWARF support
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:complex-app-sbom.json \
       --plugin-opt=format:cyclonedx \
       --plugin-opt=verbose \
       main.o libmath.a libutils.so -o complex-app

# Build with multiple libraries using Gold and DWARF support (Linux only)
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=complex-app-sbom.json \
        --plugin-opt format=cyclonedx \
        --plugin-opt verbose \
        main.o libmath.a libutils.so -o complex-app
```

## API Reference

### ComponentInfo

```cpp
struct ComponentInfo {
    std::string name;
    std::string filePath;
    std::string version;
    std::string supplier;
    std::string license;
    std::string checksum;
    FileType fileType;
    std::vector<SymbolInfo> symbols;
    std::vector<std::string> dependencies;
    std::vector<std::string> sourceFiles;   // DWARF source files
    std::vector<std::string> functions;     // DWARF function names
    std::vector<std::string> compileUnits;  // DWARF compile units
    bool containsDebugInfo;                 // DWARF debug info flag
};
```

### SBOMGenerator

```cpp
class SBOMGenerator {
public:
    void processComponent(const ComponentInfo& component);
    void generateSBOM();
    void setOutputPath(const std::string& path);
    void setFormat(const std::string& format);
    size_t getComponentCount() const;
    void printStatistics() const;
};
```

### MetadataExtractor

```cpp
class MetadataExtractor {
public:
    bool extractMetadata(ComponentInfo& component);
    bool extractVersionInfo(ComponentInfo& component);
    bool extractLicenseInfo(ComponentInfo& component);
    bool extractSymbolInfo(ComponentInfo& component);
    bool extractDebugInfo(ComponentInfo& component);  // DWARF extraction
    void setExtractDebugInfo(bool extract);
    void setVerbose(bool verbose);
};
```

### DWARFExtractor

```cpp
class DWARFExtractor {
public:
    bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles);
    bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions);
    bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits);
    bool hasDWARFInfo(const std::string& filePath);
};
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
   Solution: Install LLVM development packages.

3. **Gold linker not found (Linux)**
   ```
   Warning: Gold linker not found
   ```
   Solution: Install binutils-gold package for your distribution. See [docs/gold-installation.md](docs/gold-installation.md) for detailed instructions.

4. **Gold not available on macOS**
   ```
   Warning: Gold linker not found
   ```
   Solution: This is expected on macOS. Use LLD linker instead.

5. **Permission denied**
   ```
   Error: Cannot write to output file
   ```
   Solution: Check write permissions for the output directory.

6. **OpenSSL deprecation warnings**
   ```
   Warning: 'SHA256_Init' is deprecated
   ```
   Solution: These warnings are harmless and the build will succeed. The code uses the legacy OpenSSL API for compatibility. Future versions will use the modern EVP interface.

7. **LLD/Gold plugins not built**
   ```
   Warning: LLD libraries not found
   Warning: Gold linker not found
   ```
   Solution: The core library will still build successfully. Install LLVM/LLD or Gold linker development packages if you need the respective plugins.

8. **SBOM consistency tests failing on Ubuntu**
   ```
   Error: OpenSSL libraries not found in SBOM
   Error: System C library not found in SBOM
   ```
   Solution: This issue has been fixed in the latest version. The plugins now properly detect Ubuntu-specific library paths (`/usr/lib/x86_64-linux-gnu`). Ensure you're using the latest version of Heimdall.

9. **DWARF extraction not working**
   ```
   Warning: No DWARF debug information found
   ```
   Solution: Ensure you're compiling with debug information (`-g` flag) and using LLVM 19+ for best DWARF support.

### Platform-Specific Issues

#### macOS
- **Gold plugin not built**: This is expected behavior. Gold is not available on macOS.
- **LLD path issues**: Ensure LLVM is properly installed via Homebrew and PATH is set correctly.
- **DWARF support**: Full DWARF support available with Homebrew LLVM.

#### Linux
- **Gold not found**: Install `binutils-gold` package for your distribution.
- **Plugin loading errors**: Ensure Gold was built with plugin support enabled.
- **DWARF support**: Full DWARF support available with LLVM 19+.

### Debug Mode

Enable verbose output to see detailed processing information:

```bash
# LLD
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:debug-sbom.json \
       --plugin-opt=verbose \
       main.o -o debug-app

# Gold (Linux only)
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=debug-sbom.json \
        --plugin-opt verbose \
        main.o -o debug-app
```

### Logging

Heimdall provides different log levels:

- **INFO**: General processing information
- **WARNING**: Non-critical issues
- **ERROR**: Critical errors
- **DEBUG**: Detailed debugging information (requires debug build)

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup and contribution guidelines.

### Documentation

For detailed information about LLD integration rationale, see [docs/lld-integration-rationale.md](docs/lld-integration-rationale.md).

For information about DWARF support and limitations, see [docs/heimdall-limitations.md](docs/heimdall-limitations.md).

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/heimdall-sbom/heimdall.git
cd heimdall

# Build in debug mode
./build.sh --debug

# Run tests
cd build && make test

# Clean for check-in (removes all build artifacts)
./clean.sh
```

### Code Style

- Follow the existing C++ style
- Use meaningful variable and function names
- Add comments for complex logic
- Include unit tests for new features

## License

Licensed under the Apache License 2.0 - see [LICENSE](LICENSE) for details.


