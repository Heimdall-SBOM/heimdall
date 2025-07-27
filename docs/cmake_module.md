# Heimdall CMake Module - Usage Guide

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Basic Usage](#basic-usage)
5. [Advanced Configuration](#advanced-configuration)
6. [Supported Formats](#supported-formats)
7. [Linker Integration](#linker-integration)
8. [Examples](#examples)
9. [Troubleshooting](#troubleshooting)
10. [Integration](#integration)

## Overview

The Heimdall CMake module provides seamless integration of SBOM (Software Bill of Materials) generation into CMake-based build systems. It automatically detects available linkers and plugins, configures the build process, and generates SBOMs as part of your normal build workflow.

### Key Features

- **Automatic Integration**: SBOM generation integrated into your build process
- **Multi-Linker Support**: Works with LLVM LLD and GNU Gold linkers
- **Multi-Format Support**: SPDX (2.3, 3.0, 3.0.1) and CycloneDX (1.4, 1.5, 1.6)
- **Cross-Platform**: Works on Linux and macOS
- **Zero Configuration**: Sensible defaults for immediate use
- **Flexible Configuration**: Extensive options for customization

### Supported Platforms

| Platform | LLD Support | Gold Support | Notes |
|----------|-------------|--------------|-------|
| Linux | ✅ Full | ✅ Full | Gold requires elfutils |
| macOS | ✅ Full | ❌ None | Gold not available |
| Windows | ✅ Full | ❌ None | Gold not available |

## Installation

### Prerequisites

- CMake 3.16 or later
- C++17 or later compiler
- Heimdall installation (see [Heimdall Installation Guide](heimdall-users-guide.md))

### Installing the CMake Module

#### Option 1: Copy to Your Project

```bash
# Copy the cmake directory to your project
cp -r /path/to/heimdall/cmake ./cmake

# Or clone the repository and copy
git clone https://github.com/Heimdall-SBOM/heimdall.git
cp -r heimdall/cmake ./cmake
```

#### Option 2: Use as External Project

```cmake
# In your CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
    heimdall
    GIT_REPOSITORY https://github.com/Heimdall-SBOM/heimdall.git
    GIT_TAG main
)
FetchContent_MakeAvailable(heimdall)
```

#### Option 3: System Installation

```bash
# Install to system CMake modules directory
sudo cp -r /path/to/heimdall/cmake/* /usr/share/cmake/Modules/
```

### Verifying Installation

```cmake
# Test in your CMakeLists.txt
find_package(Heimdall REQUIRED)
message(STATUS "Heimdall found: ${HEIMDALL_FOUND}")
message(STATUS "LLD plugin: ${HEIMDALL_LLD_PLUGIN}")
message(STATUS "Gold plugin: ${HEIMDALL_GOLD_PLUGIN}")
```

## Quick Start

### Basic Setup

1. **Add the module to your project:**
   ```cmake
   # CMakeLists.txt
   cmake_minimum_required(VERSION 3.16)
   project(MyProject)
   
   # Add Heimdall cmake modules to path
   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
   
   # Find and include Heimdall
   find_package(Heimdall REQUIRED)
   include(HeimdallSBOM)
   ```

2. **Create your target:**
   ```cmake
   add_executable(myapp main.cpp)
   ```

3. **Enable SBOM generation:**
   ```cmake
   heimdall_enable_sbom(myapp)
   ```

4. **Build your project:**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

The SBOM will be automatically generated during the build process!

### Generated Files

After building, you'll find:
- `build/sbom/myapp.spdx` - SPDX format SBOM
- `build/sbom/myapp.cyclonedx.json` - CycloneDX format SBOM (if enabled)

## Basic Usage

### Simple SBOM Generation

```cmake
# Enable SBOM generation with defaults
heimdall_enable_sbom(myapp)
```

**Default behavior:**
- Uses SPDX 2.3 format
- Outputs to `${CMAKE_BINARY_DIR}/sbom/`
- Uses automatic linker detection
- Includes all dependencies

### Specify Format

```cmake
# Generate CycloneDX SBOM
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)

# Generate SPDX 3.0 JSON
heimdall_enable_sbom(myapp FORMAT spdx-3.0)

# Generate SPDX 2.3 tag-value
heimdall_enable_sbom(myapp FORMAT spdx-2.3)
```

### Specify Output

```cmake
# Custom output path
heimdall_enable_sbom(myapp 
    FORMAT cyclonedx-1.6
    OUTPUT myapp.cyclonedx.json
)

# Output to specific directory
heimdall_enable_sbom(myapp 
    FORMAT spdx-3.0
    OUTPUT ${CMAKE_SOURCE_DIR}/docs/myapp.spdx.json
)
```

### Multiple Formats

```cmake
# Generate multiple formats
heimdall_enable_sbom(myapp FORMAT spdx-2.3)
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)
heimdall_enable_sbom(myapp FORMAT spdx-3.0)
```

## Advanced Configuration

### Global Configuration

Set global defaults in your `CMakeLists.txt`:

```cmake
# Global SBOM configuration
set(HEIMDALL_SBOM_FORMAT "cyclonedx-1.6" CACHE STRING "Default SBOM format")
set(HEIMDALL_SBOM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/sboms" CACHE PATH "SBOM output directory")
set(HEIMDALL_SBOM_VERBOSE ON CACHE BOOL "Enable verbose SBOM generation")
set(HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS OFF CACHE BOOL "Include system libraries")

# Linker preferences
set(HEIMDALL_PREFERRED_LINKER "auto" CACHE STRING "Preferred linker (lld, gold, auto)")
```

### Target-Specific Configuration

```cmake
# Configure specific target
set_target_properties(myapp PROPERTIES
    HEIMDALL_SBOM_FORMAT "cyclonedx-1.6"
    HEIMDALL_SBOM_OUTPUT "myapp.cyclonedx.json"
    HEIMDALL_SBOM_VERBOSE ON
    HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS ON
)

heimdall_enable_sbom(myapp)
```

### Advanced Options

```cmake
# Full configuration example
heimdall_enable_sbom(myapp
    FORMAT cyclonedx-1.6
    OUTPUT myapp.cyclonedx.json
    VERBOSE ON
    INCLUDE_SYSTEM_LIBS OFF
    LINKER lld  # Force LLD linker
)
```

### Conditional SBOM Generation

```cmake
# Only generate SBOM in Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)
endif()

# Generate different formats for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    heimdall_enable_sbom(myapp FORMAT spdx-2.3 VERBOSE ON)
else()
    heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)
endif()
```

## Supported Formats

### CycloneDX Formats

| Version | Format | Description |
|---------|--------|-------------|
| 1.4 | JSON | Basic component structure |
| 1.5 | JSON | Enhanced validation |
| 1.6 | JSON | Latest features (default) |

**Example:**
```cmake
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)
```

### SPDX Formats

| Version | Format | Description |
|---------|--------|-------------|
| 2.3 | Tag-value | Traditional SPDX format |
| 3.0 | JSON | Modern JSON format |
| 3.0.1 | JSON | Latest SPDX JSON |

**Examples:**
```cmake
# Tag-value format
heimdall_enable_sbom(myapp FORMAT spdx-2.3)

# JSON format
heimdall_enable_sbom(myapp FORMAT spdx-3.0)
```

## Linker Integration

### Automatic Detection

The module automatically detects available linkers:

```cmake
# Check what's available
message(STATUS "LLD available: ${HEIMDALL_LLD_AVAILABLE}")
message(STATUS "Gold available: ${HEIMDALL_GOLD_AVAILABLE}")
```

### Manual Linker Selection

```cmake
# Force specific linker
heimdall_enable_sbom(myapp LINKER lld)
heimdall_enable_sbom(myapp LINKER gold)
heimdall_enable_sbom(myapp LINKER auto)  # Default
```

### Linker-Specific Configuration

#### LLD Configuration

```cmake
# LLD-specific options
set_target_properties(myapp PROPERTIES
    HEIMDALL_LLD_VERBOSE ON
    HEIMDALL_LLD_DEBUG_INFO ON
)

heimdall_enable_sbom(myapp LINKER lld)
```

#### Gold Configuration

```cmake
# Gold-specific options (Linux only)
set_target_properties(myapp PROPERTIES
    HEIMDALL_GOLD_PLUGIN_OPTIONS "--plugin-opt=verbose"
)

heimdall_enable_sbom(myapp LINKER gold)
```

## Examples

### Basic Examples

#### Simple Application

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(SimpleApp)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Heimdall REQUIRED)
include(HeimdallSBOM)

add_executable(simple_app main.cpp)
heimdall_enable_sbom(simple_app)
```

#### Library with SBOM

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyLibrary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Heimdall REQUIRED)
include(HeimdallSBOM)

add_library(mylib SHARED src/lib.cpp)
heimdall_enable_sbom(mylib FORMAT cyclonedx-1.6)

add_executable(myapp src/main.cpp)
target_link_libraries(myapp mylib)
heimdall_enable_sbom(myapp FORMAT spdx-3.0)
```

### Advanced Examples

#### Multi-Format Generation

```cmake
# Generate multiple formats for the same target
add_executable(myapp main.cpp)

# SPDX formats
heimdall_enable_sbom(myapp FORMAT spdx-2.3 OUTPUT myapp.spdx)
heimdall_enable_sbom(myapp FORMAT spdx-3.0 OUTPUT myapp.spdx.json)

# CycloneDX formats
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.4 OUTPUT myapp-1.4.cdx.json)
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6 OUTPUT myapp-1.6.cdx.json)
```

#### Conditional Generation

```cmake
# Generate different SBOMs for different build types
add_executable(myapp main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug build: verbose SPDX
    heimdall_enable_sbom(myapp 
        FORMAT spdx-2.3 
        OUTPUT debug.sbom 
        VERBOSE ON
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release build: compact CycloneDX
    heimdall_enable_sbom(myapp 
        FORMAT cyclonedx-1.6 
        OUTPUT release.cdx.json
        INCLUDE_SYSTEM_LIBS OFF
    )
endif()
```

#### Complex Project Structure

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(ComplexProject)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Heimdall REQUIRED)
include(HeimdallSBOM)

# Global configuration
set(HEIMDALL_SBOM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/sboms")
set(HEIMDALL_SBOM_VERBOSE ON)

# Core library
add_library(core SHARED src/core.cpp)
heimdall_enable_sbom(core FORMAT cyclonedx-1.6)

# Utility library
add_library(utils STATIC src/utils.cpp)
heimdall_enable_sbom(utils FORMAT spdx-2.3)

# Main application
add_executable(main src/main.cpp)
target_link_libraries(main core utils)
heimdall_enable_sbom(main FORMAT spdx-3.0)

# Test executable
add_executable(tests src/tests.cpp)
target_link_libraries(tests core utils)
heimdall_enable_sbom(tests FORMAT cyclonedx-1.6)
```

### CI/CD Integration

#### GitHub Actions

```yaml
name: Build with SBOM
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake
      
      - name: Build with SBOM
        run: |
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
      
      - name: Upload SBOMs
        uses: actions/upload-artifact@v3
        with:
          name: sboms
          path: build/sboms/
```

#### Jenkins Pipeline

```groovy
pipeline {
    agent any
    
    stages {
        stage('Build') {
            steps {
                sh '''
                    mkdir build
                    cd build
                    cmake ..
                    make -j$(nproc)
                '''
            }
        }
        
        stage('Archive SBOMs') {
            steps {
                archiveArtifacts artifacts: 'build/sboms/**/*'
            }
        }
    }
}
```

## Troubleshooting

### Common Issues

#### 1. "Heimdall not found"

**Problem:** CMake can't find Heimdall installation

**Solutions:**
```cmake
# Set Heimdall root path
set(HEIMDALL_ROOT "/path/to/heimdall" CACHE PATH "Heimdall installation path")

# Or set environment variable
set(ENV{HEIMDALL_ROOT} "/path/to/heimdall")
```

#### 2. "No suitable linker found"

**Problem:** Neither LLD nor Gold is available

**Solutions:**
```bash
# Install LLD
sudo apt-get install lld  # Ubuntu/Debian
brew install llvm         # macOS

# Install Gold (Linux only)
sudo apt-get install binutils-gold
```

#### 3. "Plugin loading failed"

**Problem:** Heimdall plugins can't be loaded

**Solutions:**
```bash
# Check plugin dependencies
ldd /path/to/heimdall/lib/heimdall-lld.so

# Check permissions
chmod 755 /path/to/heimdall/lib/heimdall-lld.so

# Verify plugin path
ls -la /path/to/heimdall/lib/heimdall-lld.so
```

#### 4. "SBOM generation failed"

**Problem:** SBOM generation step fails

**Solutions:**
```cmake
# Enable verbose output for debugging
heimdall_enable_sbom(myapp VERBOSE ON)

# Check if binary exists
add_custom_command(TARGET myapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Binary: $<TARGET_FILE:myapp>"
)
```

### Debug Mode

Enable comprehensive debugging:

```cmake
# Enable all debug output
set(HEIMDALL_DEBUG ON CACHE BOOL "Enable debug output")
set(HEIMDALL_SBOM_VERBOSE ON CACHE BOOL "Enable verbose SBOM generation")

# Check what was found
message(STATUS "Heimdall found: ${HEIMDALL_FOUND}")
message(STATUS "LLD plugin: ${HEIMDALL_LLD_PLUGIN}")
message(STATUS "Gold plugin: ${HEIMDALL_GOLD_PLUGIN}")
message(STATUS "SBOM tool: ${HEIMDALL_SBOM_TOOL}")
```

### Error Codes

| Exit Code | Meaning | Solution |
|-----------|---------|----------|
| 1 | Plugin loading failed | Check plugin path and dependencies |
| 2 | Plugin initialization failed | Check plugin compatibility |
| 3 | Binary processing failed | Check binary file and permissions |
| 4 | Invalid arguments | Check CMake configuration |

## Integration

### With Other CMake Modules

#### Integration with CPack

```cmake
# Generate SBOM for packaging
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)

# Include SBOM in package
install(FILES ${CMAKE_BINARY_DIR}/sbom/myapp.cyclonedx.json
        DESTINATION share/doc/myapp
        COMPONENT documentation)

# CPack configuration
set(CPACK_PACKAGE_NAME "myapp")
set(CPACK_GENERATOR "DEB")
```

#### Integration with Testing

```cmake
# Generate SBOM for test targets
add_executable(unit_tests test_main.cpp)
heimdall_enable_sbom(unit_tests FORMAT spdx-2.3)

# Run tests and generate SBOM
add_test(NAME UnitTests COMMAND unit_tests)
```

### With External Tools

#### CycloneDX Tools

```cmake
# Generate CycloneDX SBOM
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6)

# Validate with cyclonedx-cli
add_custom_command(TARGET myapp POST_BUILD
    COMMAND cyclonedx-cli validate ${CMAKE_BINARY_DIR}/sbom/myapp.cyclonedx.json
    COMMENT "Validating CycloneDX SBOM"
)
```

#### SPDX Tools

```cmake
# Generate SPDX SBOM
heimdall_enable_sbom(myapp FORMAT spdx-3.0)

# Validate with SPDX tools
add_custom_command(TARGET myapp POST_BUILD
    COMMAND spdx-tools-validate ${CMAKE_BINARY_DIR}/sbom/myapp.spdx.json
    COMMENT "Validating SPDX SBOM"
)
```

### Custom Scripts

#### SBOM Processing Script

```bash
#!/bin/bash
# process-sboms.sh

BUILD_DIR="$1"
PROJECT_NAME="$2"

if [ -z "$BUILD_DIR" ] || [ -z "$PROJECT_NAME" ]; then
    echo "Usage: $0 <build_dir> <project_name>"
    exit 1
fi

# Process all SBOMs in build directory
find "$BUILD_DIR" -name "*.spdx" -o -name "*.cyclonedx.json" | while read sbom; do
    echo "Processing: $sbom"
    
    # Validate SBOM
    if [[ "$sbom" == *.json ]]; then
        # CycloneDX validation
        cyclonedx-cli validate "$sbom"
    else
        # SPDX validation
        spdx-tools-validate "$sbom"
    fi
    
    # Copy to project directory
    cp "$sbom" "./$PROJECT_NAME-$(basename "$sbom")"
done
```

#### Integration in CMake

```cmake
# Add custom target for SBOM processing
add_custom_target(process-sboms
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/process-sboms.sh
            ${CMAKE_BINARY_DIR}
            ${PROJECT_NAME}
    DEPENDS myapp
    COMMENT "Processing generated SBOMs"
)
```

## Best Practices

### Configuration Management

1. **Use Global Defaults**: Set sensible defaults at the project level
2. **Override Per Target**: Use target-specific properties for customization
3. **Conditional Generation**: Only generate SBOMs when needed
4. **Version Control**: Include SBOM configuration in version control

### Performance Optimization

1. **Selective Generation**: Only generate SBOMs for release builds
2. **Format Selection**: Choose appropriate format for your use case
3. **Dependency Scope**: Use `INCLUDE_SYSTEM_LIBS OFF` when possible
4. **Parallel Builds**: Ensure SBOM generation doesn't conflict with parallel builds

### Integration Guidelines

1. **CI/CD Integration**: Include SBOM generation in your CI/CD pipeline
2. **Validation**: Validate generated SBOMs as part of your build process
3. **Documentation**: Document your SBOM generation configuration
4. **Monitoring**: Monitor SBOM generation success and failures

## Conclusion

The Heimdall CMake module provides a powerful and flexible way to integrate SBOM generation into your CMake-based projects. With automatic detection, multiple format support, and extensive configuration options, it makes SBOM generation accessible to all C++ developers.

For more information, see:
- [Heimdall User Guide](heimdall-users-guide.md)
- [Heimdall SBOM Manual](heimdall-sbom-manual.md)
- [Heimdall Validate Manual](heimdall-validate-manual.md)
- [CycloneDX Specification](https://cyclonedx.org/specification/)
