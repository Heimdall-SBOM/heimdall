# Heimdall Compiler Plugins

Heimdall provides compiler plugins for GCC and Clang that enable enhanced SBOM generation during the compilation phase. These plugins extract detailed metadata from the source code during compilation, providing richer information than post-compilation binary analysis alone.

## Overview

The compiler plugins integrate directly with the compilation process to capture:
- **Source file information**: All source files processed during compilation
- **Preprocessor details**: Include files, macro definitions, and compiler flags
- **AST metadata**: Functions, classes, namespaces, and global variables
- **Compilation context**: Compiler version, target architecture, and optimization settings

## Supported Compilers

| Compiler | Plugin | Status | Requirements |
|----------|--------|--------|--------------|
| **GCC** | `heimdall-gcc-plugin.so` | ✅ Available | GCC plugin development headers |
| **Clang** | `heimdall-clang-plugin.so` | ✅ Available | LLVM/Clang development libraries |

## Installation and Setup

### Automatic Installation

The compiler plugins are **automatically built and enabled** when using the Heimdall build script:

```bash
# Compiler plugins are enabled by default
./scripts/build.sh --standard 17 --compiler gcc --tests
```

### Dependencies

The setup scripts automatically install the required dependencies:

#### Ubuntu/Debian
```bash
sudo ./scripts/setup-ubuntu.sh  # Installs gcc-plugin-dev
sudo ./scripts/setup-debian.sh  # Installs gcc-plugin-dev
```

#### RHEL/Rocky/CentOS/Fedora
```bash
sudo ./scripts/setup-rocky.sh   # Installs gcc-plugin-devel
sudo ./scripts/setup-fedora.sh  # Installs gcc-plugin-devel
```

### Manual Dependencies

If you need to install dependencies manually:

#### Ubuntu/Debian
```bash
sudo apt-get install gcc-plugin-dev  # For GCC plugin
sudo apt-get install llvm-dev clang-dev  # For Clang plugin
```

#### RHEL/Rocky/CentOS/Fedora
```bash
sudo dnf install gcc-plugin-devel  # For GCC plugin
sudo dnf install llvm-devel clang-devel  # For Clang plugin
```

## Usage

### GCC Plugin

The GCC plugin integrates with the GCC compilation process:

```bash
# Compile with GCC plugin
gcc -fplugin=/path/to/heimdall-gcc-plugin.so \
    -fplugin-arg-heimdall-gcc-plugin-output-dir=./sbom \
    -fplugin-arg-heimdall-gcc-plugin-verbose \
    -c source.cpp -o source.o
```

#### GCC Plugin Options

| Option | Description | Example |
|--------|-------------|---------|
| `output-dir` | Directory for metadata output | `-fplugin-arg-heimdall-gcc-plugin-output-dir=./sbom` |
| `format` | Output format (json/xml) | `-fplugin-arg-heimdall-gcc-plugin-format=json` |
| `verbose` | Enable verbose logging | `-fplugin-arg-heimdall-gcc-plugin-verbose` |
| `include-system-headers` | Include system headers | `-fplugin-arg-heimdall-gcc-plugin-include-system-headers` |

### Clang Plugin

The Clang plugin provides similar functionality for Clang/LLVM:

```bash
# Compile with Clang plugin
clang++ -load /path/to/heimdall-clang-plugin.so \
        -plugin heimdall-sbom \
        -plugin-arg-heimdall-sbom-output-dir=./sbom \
        -plugin-arg-heimdall-sbom-verbose \
        -c source.cpp -o source.o
```

#### Clang Plugin Options

| Option | Description | Example |
|--------|-------------|---------|
| `output-dir` | Directory for metadata output | `-plugin-arg-heimdall-sbom-output-dir=./sbom` |
| `format` | Output format (json/xml) | `-plugin-arg-heimdall-sbom-format=json` |
| `verbose` | Enable verbose logging | `-plugin-arg-heimdall-sbom-verbose` |
| `include-system-headers` | Include system headers | `-plugin-arg-heimdall-sbom-include-system-headers` |

## Integration with Build Systems

### CMake Integration

The plugins can be integrated into CMake builds:

```cmake
# Enable compiler plugins for enhanced SBOM generation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fplugin=${HEIMDALL_GCC_PLUGIN}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fplugin-arg-heimdall-gcc-plugin-output-dir=${CMAKE_BINARY_DIR}/sbom")
```

### Makefile Integration

```makefile
# Add compiler plugin flags
CXXFLAGS += -fplugin=$(HEIMDALL_BUILD_DIR)/src/compiler/gcc/heimdall-gcc-plugin.so
CXXFLAGS += -fplugin-arg-heimdall-gcc-plugin-output-dir=./sbom
CXXFLAGS += -fplugin-arg-heimdall-gcc-plugin-verbose

# For Clang
CLANG_PLUGIN_FLAGS = -load $(HEIMDALL_BUILD_DIR)/src/compiler/clang/heimdall-clang-plugin.so
CLANG_PLUGIN_FLAGS += -plugin heimdall-sbom
CLANG_PLUGIN_FLAGS += -plugin-arg-heimdall-sbom-output-dir=./sbom
```

## Output Format

The compiler plugins generate JSON metadata files containing:

### Source File Information
```json
{
  "sourceFiles": [
    "src/main.cpp",
    "src/utils.cpp",
    "include/utils.h"
  ],
  "mainSourceFile": "src/main.cpp"
}
```

### Compilation Details
```json
{
  "compiler": {
    "type": "gcc",
    "version": "11.4.0",
    "targetArchitecture": "x86_64-linux-gnu"
  },
  "compilationFlags": {
    "optimization_level": "2",
    "debug_info": "1",
    "cpp_standard": "17"
  }
}
```

### Code Structure
```json
{
  "functions": ["main", "calculate", "process_data"],
  "globalVariables": ["config", "version"],
  "macroDefinitions": ["DEBUG", "VERSION=1.0"]
}
```

## Advanced Configuration

### Custom Configuration File

Create a `heimdall-compiler.conf` file:

```ini
[gcc-plugin]
output_dir = ./compiler-metadata
format = json
verbose = true
include_system_headers = false

[clang-plugin]
output_dir = ./compiler-metadata  
format = json
verbose = true
include_system_headers = false
```

### Environment Variables

```bash
# Configure plugin behavior
export HEIMDALL_COMPILER_OUTPUT_DIR="./sbom-metadata"
export HEIMDALL_COMPILER_VERBOSE=1
export HEIMDALL_COMPILER_FORMAT="json"
```

## Troubleshooting

### GCC Plugin Issues

**Plugin not loading:**
```bash
# Check if GCC plugin headers are installed
gcc -print-file-name=plugin
# Should output a path to plugin directory
```

**Missing dependencies:**
```bash
# Install GCC plugin development headers
sudo apt-get install gcc-plugin-dev  # Ubuntu/Debian
sudo dnf install gcc-plugin-devel    # RHEL/Fedora
```

### Clang Plugin Issues

**Plugin not found:**
```bash
# Check LLVM installation
llvm-config --version
clang --version
```

**API compatibility:**
```bash
# Ensure LLVM version compatibility
# Heimdall supports LLVM 11+ for C++17
# Heimdall supports LLVM 19+ for C++20/23
```

### Common Issues

1. **Plugin path issues**: Use absolute paths when specifying plugin locations
2. **Output directory permissions**: Ensure the output directory is writable
3. **Compiler version mismatch**: Plugin must be compiled with the same compiler version
4. **Missing debug information**: Use `-g` flag for enhanced metadata extraction

## Build Configuration

### Automatic Plugin Building

By default, the build script enables compiler plugins:

```bash
# Plugins are automatically enabled
./scripts/build.sh --standard 17 --compiler gcc
```

### Manual Plugin Control

To disable plugins during build:

```bash
cmake -DHEIMDALL_BUILD_COMPILER_PLUGINS=OFF ..
```

To enable plugins manually:

```bash
cmake -DHEIMDALL_BUILD_COMPILER_PLUGINS=ON ..
```

## Integration with Heimdall SBOM Generation

The compiler plugins work seamlessly with Heimdall's SBOM generation tools:

1. **Compilation phase**: Plugins extract metadata during compilation
2. **Linking phase**: LLD/Gold plugins capture binary information  
3. **Post-processing**: `heimdall-sbom` combines all metadata sources

This multi-phase approach provides the most comprehensive SBOM possible, capturing information from source code, compilation process, and final binary.

## Performance Impact

The compiler plugins are designed for minimal performance impact:
- **Compilation overhead**: Typically <5% increase in compilation time
- **Memory usage**: Minimal additional memory during compilation
- **Output size**: Metadata files are typically <1MB per compilation unit

## Platform Support

| Platform | GCC Plugin | Clang Plugin | Status |
|----------|------------|--------------|--------|
| **Linux x86_64** | ✅ | ✅ | Fully Supported |
| **Linux ARM64** | ✅ | ✅ | Fully Supported |
| **macOS x86_64** | ❌* | ✅ | Clang Only |
| **macOS ARM64** | ❌* | ✅ | Clang Only |

*GCC plugins are not commonly used on macOS; Clang is the primary compiler.

## Examples

See the `examples/` directory for complete examples demonstrating compiler plugin usage:

- `examples/makefile_example/`: Makefile-based build with compiler plugins
- `examples/cmake_example/`: CMake-based build with compiler plugins

## Security Considerations

- Compiler plugins run with the same privileges as the compiler
- Metadata files may contain sensitive information about your codebase
- Use appropriate file permissions for output directories
- Consider excluding compiler metadata from version control in sensitive projects