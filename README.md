# Heimdall

A comprehensive Software Bill of Materials (SBOM) generation plugin that works with both LLVM LLD and GNU Gold linkers. Heimdall automatically generates accurate SBOMs during the linking process, capturing all components that actually make it into your final binaries.

[![Build Status](https://github.com/your-org/heimdall/workflows/Build%20and%20Test/badge.svg)](https://github.com/your-org/heimdall/actions)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Platform Support](https://img.shields.io/badge/platforms-macOS%20%7C%20Linux-green.svg)](#supported-platforms)
[![Tests](https://img.shields.io/badge/tests-20%20passed-brightgreen.svg)](#testing)

## Features

- **Dual Linker Support**: Works seamlessly with both LLVM LLD and GNU Gold linkers
- **Multiple SBOM Formats**: Generates SPDX 2.3 and CycloneDX 1.4 compliant reports
- **Comprehensive Component Analysis**: Extracts versions, licenses, checksums, and dependencies
- **Package Manager Integration**: Recognizes Conan, vcpkg, and system packages
- **Cross-Platform**: Native support for macOS and Linux
- **Performance Optimized**: Minimal overhead during linking
- **CI/CD Ready**: Seamless integration with modern build systems
- **Security Focused**: Enables vulnerability scanning and compliance tracking
- **Comprehensive Testing**: Full unit test suite with real shared library testing

## Quick Start

### Installation

```bash
git clone https://github.com/your-org/heimdall.git
cd heimdall
./build.sh
```

### Usage

```bash
# Using LLD (macOS/Linux)
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:myapp.json \
       main.o -o myapp

# Using Gold (Linux only)
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
)
```

#### Makefile
```makefile
LDFLAGS += -Wl,--plugin-opt=load:/usr/lib/heimdall-plugins/heimdall-lld.dylib
LDFLAGS += -Wl,--plugin-opt=sbom-output:$(TARGET).json
```

## Architecture

Heimdall consists of several key components:

### Core Components

- **ComponentInfo**: Represents individual software components with metadata
- **SBOMGenerator**: Generates SBOMs in multiple formats (SPDX, CycloneDX)
- **MetadataExtractor**: Extracts metadata from binary files (ELF, Mach-O, PE, archives)
- **Utils**: Utility functions for file operations, path handling, and JSON formatting
- **PluginInterface**: Common interface for both LLD and Gold plugins

### Plugin Adapters

- **LLDAdapter**: Interfaces with LLVM LLD linker plugin system
- **GoldAdapter**: Interfaces with GNU Gold linker plugin system

### File Format Support

- **ELF**: Linux executables and libraries
- **Mach-O**: macOS executables and libraries  
- **PE**: Windows executables and libraries
- **Archive**: Static libraries (.a, .lib)

## Supported Platforms

| Platform | LLD | Gold | Status |
|----------|-----|------|--------|
| macOS (ARM64) | ✅ | ❌ | LLD Only - Gold not available on macOS |
| macOS (x86_64) | ✅ | ❌ | LLD Only - Gold not available on macOS |
| Linux (x86_64) | ✅ | ✅ | Fully Supported |
| Linux (ARM64) | ✅ | ✅ | Fully Supported |

**Note**: GNU Gold linker is primarily designed for Linux systems and is not available on macOS. The Gold plugin will not be built on macOS systems.

## Building from Source

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- LLVM/LLD development libraries
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
    llvm-dev \
    liblld-dev \
    binutils-gold \
    libssl-dev \
    pkg-config

# Build Heimdall with both LLD and Gold plugins
./build.sh
```

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

Heimdall includes a comprehensive test suite with 20 unit tests covering all major components:

```bash
# Run all tests
./build.sh

# Run tests only
cd build && make test

# Run specific test suites
./build/tests/heimdall_tests --gtest_filter="UtilsTest.*"
./build/tests/heimdall_tests --gtest_filter="MetadataExtractorTest.*"
```

### Test Coverage

- **Utils**: File operations, path handling, JSON formatting
- **ComponentInfo**: Component metadata management
- **SBOMGenerator**: SBOM generation in multiple formats
- **MetadataExtractor**: Binary file analysis with real shared libraries
- **Plugin Integration**: LLD and Gold plugin functionality

The test suite uses real shared libraries compiled during testing to ensure accurate validation of metadata extraction capabilities.

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
- File-level details
- License information
- Checksums (SHA256)
- Relationships between components

### CycloneDX 1.4

Heimdall generates CycloneDX 1.4 compliant documents with:

- Component metadata
- Dependencies
- External references
- PURL identifiers
- Hash information

## Examples

### Simple C Program

```bash
# Compile with SBOM generation
gcc -c main.c -o main.o

# Using LLD (macOS/Linux)
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:main-sbom.json \
       main.o -o main

# Using Gold (Linux only)
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=main-sbom.json \
        main.o -o main

# View generated SBOM
cat main-sbom.json
```

### CMake Project

```cmake
# Find Heimdall
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

# Add SBOM generation to target
target_link_options(myapp PRIVATE
    "LINKER:--plugin-opt=load:${HEIMDALL_LLD}"
    "LINKER:--plugin-opt=sbom-output:${CMAKE_BINARY_DIR}/myapp-sbom.json"
    "LINKER:--plugin-opt=format:cyclonedx"
)
```

### Complex Project with Dependencies

```bash
# Build with multiple libraries using LLD
ld.lld --plugin-opt=load:./heimdall-lld.dylib \
       --plugin-opt=sbom-output:complex-app-sbom.json \
       --plugin-opt=verbose \
       main.o libmath.a libutils.so -o complex-app

# Build with multiple libraries using Gold (Linux only)
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=complex-app-sbom.json \
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

### Platform-Specific Issues

#### macOS
- **Gold plugin not built**: This is expected behavior. Gold is not available on macOS.
- **LLD path issues**: Ensure LLVM is properly installed via Homebrew and PATH is set correctly.

#### Linux
- **Gold not found**: Install `binutils-gold` package for your distribution.
- **Plugin loading errors**: Ensure Gold was built with plugin support enabled.

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

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/your-org/heimdall.git
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

