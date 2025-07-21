# Heimdall-SBOM

<img src="https://github.com/bakkertj/heimdall/blob/18e0ce7fc630923f15e6e29e94059e40b7d1af99/docs/images/logo_w_text.png" width="400">

A comprehensive Software Bill of Materials (SBOM) generation plugin that works with both LLVM LLD and GNU Gold linkers. Heimdall automatically generates accurate SBOMs during the linking process, capturing all components that actually make it into your final binaries.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Platform Support](https://img.shields.io/badge/platforms-macOS%20%7C%20Linux-green.svg)](#supported-platforms)
[![Tests](https://img.shields.io/badge/tests-336%20passed-brightgreen.svg)](#testing)
[![C++ Standards](https://img.shields.io/badge/C%2B%2B-11%2F14%2F17%2F20%2F23-blue.svg)](#c-standard-support)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=bugs)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=Heimdall-SBOM_heimdall&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=Heimdall-SBOM_heimdall)
[![Coverage](https://img.shields.io/badge/coverage-47.3%25-yellow.svg)](build/coverage/html/index.html)

## Features

- **Dual Linker Support**: Works seamlessly with both LLVM LLD and GNU Gold linkers
- **Multiple SBOM Formats**: Generates SPDX 2.3, 3.0, and 3.0.1 and CycloneDX 1.4, 1.5, and 1.6 compliant reports
- **Comprehensive Component Analysis**: Extracts versions, licenses, checksums, and dependencies
- **Package Manager Integration**: Recognizes Conan, vcpkg, and system packages
- **Cross-Platform**: Native support for macOS and Linux
- **Performance Optimized**: Minimal overhead during linking
- **CI/CD Ready**: Seamless integration with modern build systems
- **Security Focused**: Enables vulnerability scanning and compliance tracking
- **Comprehensive Testing**: 257 passing tests across 19 test suites with 45.4% code coverage
- **Multi-Standard C++ Support**: Robust compatibility layer supporting C++11, C++14, C++17, C++20, and C++23
- **Enhanced Compatibility**: Automatic feature detection and standard library compatibility
- **Multi-Compiler Support**: Automatic detection and selection of GCC and Clang versions
- **SCL Integration**: Support for Software Collections (SCL) on RHEL/Rocky/CentOS
- **Enhanced Test Coverage**: Recent improvements added 151 new test cases for SBOM validation, comparison, and plugin functionality

## Code Coverage

- **Current coverage:** ![Coverage](https://img.shields.io/badge/coverage-47.3%25-yellow.svg)
- Coverage is generated using `tests/coverage.sh` and includes all source files, including `src/tools/*.cpp`.
- To update the badge and report, run:
  ```bash
  ./tests/coverage.sh
  ```
- The full HTML report is available at `build/coverage/html/index.html` after running the script.

## Build System and Compatibility

Heimdall features a robust build system that automatically selects the correct compiler and LLVM version for each C++ standard. The build scripts in `scripts/` ensure seamless compatibility and provide clear diagnostics if your system is missing required tools.

### Key Features

- **Automatic Compiler/LLVM Selection**: Detects all available GCC and Clang versions, as well as all installed LLVM versions
- **Multi-Compiler Support**: Choose between GCC and Clang with the `--compiler` option
- **SCL Integration**: Automatic detection and guidance for SCL toolsets on RHEL/Rocky/CentOS
- **Compatibility Checker**: Run `./scripts/show_build_compatibility.sh` to see which C++ standards you can build
- **Clear Error Messages**: Specific instructions when SCL activation is needed

### Build Commands

```bash
# Build all compatible standards
./scripts/build_all_standards.sh

# Build a specific standard with GCC (default)
./scripts/build.sh --standard 17 --compiler gcc --tests

# Build a specific standard with Clang
./scripts/build.sh --standard 17 --compiler clang --tests

# Build with tests and SBOM generation
./scripts/build.sh --standard 20 --all

# Clean all build artifacts
./scripts/clean.sh

# Check compatibility
./scripts/show_build_compatibility.sh
```

## C++ Standard Support

Heimdall supports multiple C++ standards with automatic compiler and LLVM version selection:

| C++ Standard | LLVM Version | GCC Version | Clang Version | Features | Status |
|--------------|--------------|-------------|---------------|----------|--------|
| C++11        | 7+           | 4.8+        | 3.3+          | Basic + Compatibility | ✅ Working |
| C++14        | 7+           | 6+          | 3.4+          | Enhanced + Compatibility | ✅ Working |
| C++17        | 11+          | 7+          | 5+            | Full Standard Library | ✅ Working |
| C++20        | 19+          | 13+         | 14+           | Full + std::format    | ✅ Working |
| C++23        | 19+          | 13+         | 14+           | Full + std::format    | ✅ Working |

> **Note:** C++20/23 require GCC 13+ or Clang 14+ for `<format>` support. The build system will provide clear guidance if your compiler is too old.

## Quick Start

### Prerequisites

- **Linux**: GCC 4.8+ or Clang 3.3+, CMake 3.16+, LLVM development libraries
- **macOS**: Xcode Command Line Tools, CMake 3.16+, LLVM (via Homebrew)
- **SCL Support**: On RHEL/Rocky/CentOS, SCL toolsets are automatically detected

### Automatic Setup (Recommended)

We provide an automated setup script that detects your Linux distribution and installs all necessary dependencies:

```bash
git clone --recurse-submodules https://github.com/your-org/heimdall.git

cd heimdall

# Run the automated setup script
sudo ./setup.sh

# Build with default settings (GCC, C++17)
./scripts/build.sh --standard 17 --compiler gcc --tests

# Or build all compatible standards
./scripts/build_all_standards.sh
```

#### Setup Script Options

```bash
# Show what would be installed without actually installing
sudo ./setup.sh --dry-run

# Install with verbose output
sudo ./setup.sh --verbose

# Skip LLVM installation (use system LLVM if available)
sudo ./setup.sh --skip-llvm

# Skip GCC installation (use system GCC)
sudo ./setup.sh --skip-gcc

# Show help
./setup.sh --help
```

#### Supported Distributions

The setup script automatically supports:
- **Ubuntu 22.04+**: GCC 11, 13 + LLVM 18
- **Debian Bookworm**: GCC 11, 12 + LLVM 18
- **Debian Testing**: GCC 12, 13, 14 + LLVM 18
- **CentOS Stream 9**: GCC 11, 13, 14 + LLVM 20
- **Fedora Latest**: GCC 15 + LLVM 18
- **Arch Linux**: GCC 14, 15 + LLVM 18
- **OpenSUSE Tumbleweed**: GCC 11, 13 + LLVM 18
- **Rocky Linux 9**: GCC 11, 13 + LLVM 16

### Manual Installation

If you prefer to install dependencies manually or the setup script doesn't support your distribution:

### Usage Examples

```bash
# Build C++11 with system GCC
./scripts/build.sh --standard 11 --compiler gcc --tests

# Build C++17 with Clang
./scripts/build.sh --standard 17 --compiler clang --tests

# Build C++23 with GCC (requires SCL activation on RHEL/Rocky/CentOS)
scl enable gcc-toolset-14 bash
./scripts/build.sh --standard 23 --compiler gcc --tests

# Build with tests and SBOM generation
./scripts/build.sh --standard 20 --all

# Check what you can build on your system
./scripts/show_build_compatibility.sh
```

### Using Heimdall as a Linker Plugin

Once built, you can use Heimdall to generate SBOMs during compilation. See the complete usage guide in [`docs/usage.md`](docs/usage.md) and try the examples:

**Format-Specific Examples:**
```bash
# SPDX-only example
cd examples/heimdall-usage-spdx-example
./run_example.sh

# CycloneDX-only example  
cd examples/heimdall-usage-cyclonedx-example
./run_example.sh

# General example with multiple formats
cd examples/heimdall-usage-example
./run_example.sh
./advanced_example.sh
```

**Quick Example:**
```bash
# Navigate to the example directory
cd examples/heimdall-usage-example

# Run the automated example
./run_example.sh

# Or try the advanced example with different SBOM formats
./advanced_example.sh
```

## Building from Source

### Linux Setup

**Recommended**: Use the automated setup script for easy installation:
```bash
sudo ./setup.sh
```

#### Ubuntu/Debian
```bash
# Install dependencies
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

# Build Heimdall
./scripts/build.sh --standard 17 --compiler gcc --tests
```

#### Fedora/RHEL/CentOS
```bash
# Install dependencies
sudo yum install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    llvm-devel \
    lld-devel \
    binutils-gold \
    openssl-devel \
    pkgconfig

# For C++20/23, you may need SCL toolsets
sudo yum install -y gcc-toolset-14

# Build with system GCC
./scripts/build.sh --standard 17 --compiler gcc --tests

# Or activate SCL for C++20/23
scl enable gcc-toolset-14 bash
./scripts/build.sh --standard 23 --compiler gcc --tests
```

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
./scripts/build.sh --standard 17 --compiler gcc --tests
```

### macOS Setup

```bash
# Install LLVM and other dependencies
brew install llvm cmake ninja openssl llvm@18
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build Heimdall (LLD plugin only)
./scripts/build.sh --standard 17 --compiler gcc --tests

# Or use Clang
./scripts/build.sh --standard 17 --compiler clang --tests
```

### Build Options

```bash
# Build with specific C++ standard
./scripts/build.sh --standard 20 --compiler gcc --tests

# Build with Clang
./scripts/build.sh --standard 17 --compiler clang --tests

# Build with tests and SBOM generation
./scripts/build.sh --standard 23 --all

# Clean build directory before building
./scripts/build.sh --standard 17 --compiler gcc --clean --tests

# Custom build directory
./scripts/build.sh --standard 17 --compiler gcc --build-dir mybuild --tests
```

### SCL Integration (RHEL/Rocky/CentOS)

For C++20/23 on RHEL/Rocky/CentOS systems:

```bash
# Check available SCL toolsets
scl --list

# Activate GCC 14 toolset
scl enable gcc-toolset-14 bash

# Build C++23
./scripts/build.sh --standard 23 --compiler gcc --tests
```

The build system will automatically detect if you need SCL and provide clear instructions.

## Testing

The project includes a comprehensive test suite with **257 passing tests** across **19 test suites** and **45.4% code coverage**. The test suite validates compatibility across all supported C++ standards and provides extensive error handling and edge case testing.

### Test Statistics

| Metric | Count | Status |
|--------|-------|--------|
| **Total Tests** | 266 | ✅ All passing |
| **Passed Tests** | 257 | ✅ Stable |
| **Skipped Tests** | 9 | ⏭️ Expected (missing dependencies) |
| **Failed Tests** | 0 | ✅ No failures |
| **Test Suites** | 19 | ✅ Comprehensive coverage |
| **Execution Time** | ~32 seconds | ✅ Fast execution |

### Code Coverage

| Component | Line Coverage | Function Coverage | Status |
|-----------|---------------|-------------------|---------|
| **Overall** | 45.4% | 44.1% | ⚠️ Moderate |
| **ComponentInfo.cpp** | 88.9% | 90.9% | ✅ Excellent |
| **DWARFExtractor.cpp** | 87.5% | 100% | ✅ Excellent |
| **Utils.cpp** | 85.5% | 100% | ✅ Excellent |
| **SBOMGenerator.cpp** | 83.5% | 87.9% | ✅ Good |
| **MetadataExtractor.cpp** | 70.9% | 82.5% | ✅ Good |
| **SBOMValidator.cpp** | 60.5% | 65.2% | ⚠️ Moderate |
| **GoldPlugin.cpp** | 62.8% | 58.3% | ⚠️ Moderate |
| **LLDPlugin.cpp** | 52.2% | 33.3% | ❌ Needs improvement |
| **SBOMComparator.cpp** | 44.1% | 53.1% | ❌ Needs improvement |

### Test Categories

#### **Core Components (Well Tested)**
- **ComponentInfo**: 7 tests - Data structure and serialization
- **DWARFExtractor**: 10 tests - Debug information extraction
- **Utils**: 24 tests - Utility functions and file operations
- **SBOMGenerator**: 5 tests - SBOM generation in multiple formats

#### **SBOM Components (Recently Enhanced)**
- **SBOMValidator**: 62 tests - Validation logic and error handling
- **SBOMComparator**: 62 tests - Component comparison and diff generation
- **MetadataExtractor**: 48 tests - File metadata extraction with edge cases

#### **Plugin Components (Comprehensive Testing)**
- **LLDPlugin**: 43 tests - LLVM LLD plugin functionality
- **GoldPlugin**: 43 tests - GNU Gold plugin functionality
- **PluginInterface**: 38 tests - Common plugin interface

#### **Advanced Testing**
- **DWARFAdvanced**: 18 tests - Complex debug information scenarios
- **DWARFCrossPlatform**: 12 tests - Platform-specific behavior
- **DWARFIntegration**: 9 tests - End-to-end integration testing

### Error Handling & Edge Cases

The test suite includes comprehensive error handling and edge case testing:

- **File System Edge Cases**: Non-existent files, empty files, corrupted files, symlinks, hard links
- **Path Handling**: Unicode characters, special characters, spaces, long paths, relative paths
- **Memory Management**: Large files, memory pressure, concurrent access, resource cleanup
- **Plugin Integration**: Null pointers, invalid configurations, error recovery
- **Platform Specific**: Linux-specific ELF testing, cross-platform compatibility
- **Input Validation**: Invalid SBOM formats, malformed data, missing dependencies

### Running Tests

```bash
# Test with default settings
./scripts/build.sh --standard 17 --compiler gcc --tests

# Test specific C++ standard with Clang
./scripts/build.sh --standard 20 --compiler clang --tests

# Test all compatible standards
./scripts/build_all_standards.sh

# Run tests with coverage analysis
./tests/coverage.sh

# Run specific test suites
cd build
ctest -R "SBOMValidation" --verbose
ctest -R "LLDPlugin" --verbose
ctest -R "MetadataExtractor" --verbose
```

The test suite validates:
- C++11/14/17/20/23 feature compatibility
- Multi-compiler support (GCC and Clang)
- LLVM detection and compatibility
- Filesystem operations across standards
- SCL integration on supported platforms
- Comprehensive error handling and edge cases
- Plugin lifecycle and integration testing
- Memory management and resource cleanup

## Usage

### Using LLD (macOS/Linux) - Wrapper Approach
```bash
# Step 1: Link normally with LLD
g++ -fuse-ld=lld main.o utils.o math.o -o myapp

# Step 2: Generate SBOM using wrapper tool
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so myapp --format spdx --output myapp.spdx
```

### Using Gold (Linux only) - Plugin Interface
```bash
# Direct plugin integration (requires dependencies)
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=myapp.spdx \
    main.o utils.o math.o -o myapp

# Or use wrapper approach if plugin fails
g++ -fuse-ld=gold main.o utils.o math.o -o myapp
heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so myapp --format spdx --output myapp.spdx
```

### Plugin Compatibility

**Important:** Heimdall uses different approaches for different linkers:

- **LLVM LLD:** Uses wrapper approach with `heimdall-sbom` tool (works with all LLVM versions)
- **LLVM 19+:** Plugin interface completely changed - wrapper approach is the only reliable method
- **GNU Gold:** Uses native plugin interface with `--plugin` and `--plugin-opt` (requires dependencies)

**Current Status:** Heimdall provides universal compatibility through the wrapper approach for LLD and plugin interface for Gold with automatic fallback.

For detailed technical rationale and compatibility information, see [docs/rationale.md](docs/rationale.md).

**Quick compatibility check:**
```bash
# Check your system
ld.lld --version
ld.gold --version

# Test with the example
cd examples/heimdall-usage-example
./run_example.sh
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
- **Compatibility Layer**: Multi-standard C++ support with `heimdall::compat` namespace

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

# Using LLD (macOS/Linux) - Wrapper approach
g++ -fuse-ld=lld main.o -o main
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so main --format spdx --output main.spdx

# Using Gold (Linux only) - Plugin interface
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=main.spdx \
    main.o -o main

# View generated SBOM
cat main.spdx
```

### CMake Project

```cmake
# Find Heimdall
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

# Add SBOM generation to target (LLD wrapper approach)
add_custom_command(TARGET myapp POST_BUILD
    COMMAND heimdall-sbom ${HEIMDALL_LLD} $<TARGET_FILE:myapp> 
            --format spdx --output ${CMAKE_BINARY_DIR}/myapp.spdx
    COMMENT "Generating SBOM for myapp"
)
```

### Complex Project with Dependencies

```bash
# Build with multiple libraries using LLD (wrapper approach)
g++ -fuse-ld=lld main.o libmath.a libutils.so -o complex-app
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so complex-app --format spdx --output complex-app.spdx

# Build with multiple libraries using Gold (plugin interface)
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=complex-app.spdx \
    -Wl,--plugin-opt=verbose \
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

### Compatibility Layer

```cpp
#include "compat/compatibility.hpp"

using namespace heimdall::compat;

// Works across all C++ standards
optional<int> opt(42);
string_view sv("hello");
fs::path p("file.txt");
variant<int, std::string> v(100);

// Utility functions
string_view result = utils::to_string_view(42);
int val = utils::get_optional_value(opt, 0);
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

8. **C++ Standard compatibility issues**
   ```
   Error: 'std::filesystem' is not a member of 'std'
   ```
   Solution: Use the compatibility layer with `heimdall::compat::fs` or build with `--cpp11-14 --no-boost` for older standards.

9. **SCL activation required**
   ```
   Error: Your system GCC version (11.5.0) is not compatible with C++23
   ```
   Solution: Activate the appropriate SCL toolset:
   ```bash
   scl enable gcc-toolset-14 bash
   ./scripts/build.sh --standard 23 --compiler gcc --tests
   ```

### Platform-Specific Issues

#### macOS
- **Gold plugin not built**: This is expected behavior. Gold is not available on macOS.
- **LLD path issues**: Ensure LLVM is properly installed via Homebrew and PATH is set correctly.

#### Linux
- **Gold not found**: Install `binutils-gold` package for your distribution.
- **Plugin loading errors**: Ensure Gold was built with plugin support enabled.
- **SCL toolsets**: For C++20/23, you may need to activate SCL toolsets on RHEL/Rocky/CentOS.

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

For detailed information about linker integration rationale and technical approaches, see [docs/rationale.md](docs/rationale.md).

For comprehensive multi-standard C++ support documentation, see [docs/multi-standard-support.md](docs/multi-standard-support.md).

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/your-org/heimdall.git
cd heimdall

# Build in debug mode
./scripts/build.sh --standard 17 --compiler gcc --tests

# Test all C++ standards
for std in 11 14 17 20 23; do
    echo "Testing C++$std..."
    ./scripts/build.sh --standard $std --compiler gcc --tests
done

# Test with Clang
./scripts/build.sh --standard 17 --compiler clang --tests
```

## CMake Module Integration

Heimdall provides a CMake module for seamless SBOM generation in C++ projects. This module supports:
- Executables and libraries (static/shared/interface)
- Multi-target and installable projects
- Automatic linker detection (LLD/Gold)

### Quick Integration

1. Add the `cmake/` directory to your `CMAKE_MODULE_PATH`:
   ```cmake
   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
   ```
2. Include the Heimdall modules:
   ```cmake
   include(HeimdallConfig)
   include(HeimdallSBOM)
   ```
3. Add your targets and enable SBOM generation:
   ```cmake
   add_executable(myapp main.cpp)
   heimdall_enable_sbom(myapp FORMAT spdx-2.3 VERBOSE ON)
   ```

See [`cmake/templates/cmake-sbom-template.cmake`](cmake/templates/cmake-sbom-template.cmake) for a ready-to-use template.

### Advanced Example Projects

| Example Directory | Description |
|-------------------|-------------|
| `heimdall-cmake-module-example` | Multi-target (executable + static lib) |
| `heimdall-cmake-sharedlib-example` | Shared library + executable |
| `heimdall-cmake-interface-example` | Interface (header-only) + implementation + executable |
| `heimdall-cmake-install-example` | Installable static lib + executable + headers |

To build and test an example:
```bash
cd examples/<example-dir>
mkdir build && cd build
export HEIMDALL_ROOT="$(pwd)/../../../build"  # Adjust if needed
cmake ..
make
```

For troubleshooting and advanced options, see [`docs/usage.md`](docs/usage.md) and the comments in the template file.

