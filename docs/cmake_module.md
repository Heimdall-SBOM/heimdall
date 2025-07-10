# Heimdall CMake Module Design

*Heimdall Project*  
*Last updated: January 2025*

## Executive Summary

This document outlines the design for a CMake module that integrates Heimdall's SBOM generation capabilities into CMake-based build systems. The module will provide seamless SBOM generation for C++ projects using both LLVM LLD and GNU Gold linkers, leveraging Heimdall's existing wrapper and plugin approaches.

## Rationale

### Current State Analysis

Heimdall currently provides SBOM generation through two distinct approaches:

1. **LLD Wrapper Approach**: Uses `heimdall-sbom` tool for post-processing analysis
2. **Gold Plugin Approach**: Uses native plugin interface during linking

The current usage requires manual integration in build systems:

```bash
# Manual LLD approach
g++ -fuse-ld=lld main.o -o app
heimdall-sbom ../../build/lib/heimdall-lld.so app --format spdx --output app.spdx

# Manual Gold approach  
g++ -fuse-ld=gold -Wl,--plugin=../../build/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app.spdx main.o -o app
```

### Problems Addressed

1. **Manual Integration**: Users must manually add SBOM generation steps
2. **Build System Fragmentation**: Different approaches for different linkers
3. **Configuration Complexity**: Users must manage plugin paths and options
4. **Inconsistent Output**: No standardized SBOM generation across projects
5. **Limited Automation**: SBOM generation not integrated into build process

### Benefits of CMake Module

1. **Unified Interface**: Single CMake function for all linkers
2. **Automatic Detection**: Automatically detects available linkers and plugins
3. **Build Integration**: SBOM generation as part of normal build process
4. **Configuration Management**: Centralized SBOM configuration
5. **Cross-Platform Support**: Works on macOS (LLD only) and Linux (LLD + Gold)

## Detailed Design

### Module Architecture

```
HeimdallCMake/
├── FindHeimdall.cmake          # Find Heimdall installation
├── HeimdallSBOM.cmake          # Main SBOM generation functions
├── HeimdallConfig.cmake        # Configuration and options
└── templates/
    ├── sbom-config.json        # Default SBOM configuration
    └── cmake-sbom-template.cmake  # Template for user projects
```

### Core Functions

#### 1. `heimdall_enable_sbom(target [options])`

Main function to enable SBOM generation for a CMake target.

**Parameters:**
- `target`: CMake target name
- `options`: Optional configuration parameters

**Example Usage:**
```cmake
# Basic usage
heimdall_enable_sbom(myapp)

# With options
heimdall_enable_sbom(myapp
    FORMAT spdx
    OUTPUT myapp.spdx
    VERBOSE ON
    INCLUDE_SYSTEM_LIBS OFF
)
```

#### 2. `heimdall_find_plugins()`

Automatically detects available Heimdall plugins and linkers.

**Returns:**
- `HEIMDALL_LLD_PLUGIN_PATH`: Path to LLD plugin
- `HEIMDALL_GOLD_PLUGIN_PATH`: Path to Gold plugin  
- `HEIMDALL_SBOM_TOOL_PATH`: Path to heimdall-sbom tool
- `HEIMDALL_LLD_AVAILABLE`: Boolean for LLD availability
- `HEIMDALL_GOLD_AVAILABLE`: Boolean for Gold availability

#### 3. `heimdall_generate_sbom(target binary_path [options])`

Low-level function for manual SBOM generation.

**Parameters:**
- `target`: CMake target name
- `binary_path`: Path to binary for analysis
- `options`: SBOM generation options

### Configuration Options

#### Global Options

```cmake
# SBOM Configuration
set(HEIMDALL_SBOM_FORMAT "spdx" CACHE STRING "Default SBOM format")
set(HEIMDALL_SBOM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom" CACHE PATH "SBOM output directory")
set(HEIMDALL_SBOM_VERBOSE OFF CACHE BOOL "Enable verbose SBOM generation")
set(HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS OFF CACHE BOOL "Include system libraries in SBOM")

# Linker Preferences
set(HEIMDALL_PREFERRED_LINKER "auto" CACHE STRING "Preferred linker (lld, gold, auto)")
set(HEIMDALL_FALLBACK_TO_WRAPPER ON CACHE BOOL "Fallback to wrapper approach if plugin fails")

# Plugin Paths
set(HEIMDALL_PLUGIN_SEARCH_PATHS "" CACHE STRING "Additional plugin search paths")
```

#### Target-Specific Options

```cmake
# Per-target SBOM options
set_target_properties(myapp PROPERTIES
    HEIMDALL_SBOM_FORMAT "cyclonedx"
    HEIMDALL_SBOM_OUTPUT "myapp.cyclonedx.json"
    HEIMDALL_SBOM_VERBOSE ON
    HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS ON
)
```

### Implementation Strategy

#### Phase 1: Plugin Detection and Configuration

1. **Find Heimdall Installation**
   - Search for Heimdall in standard locations
   - Support custom installation paths
   - Validate plugin availability

2. **Detect Available Linkers**
   - Check for `ld.lld` availability
   - Check for `ld.gold` availability
   - Test plugin compatibility

3. **Configure Build Environment**
   - Set appropriate linker flags
   - Configure plugin options
   - Setup fallback mechanisms

#### Phase 2: SBOM Generation Integration

1. **LLD Integration (Wrapper Approach)**
   ```cmake
   # Add post-build command for SBOM generation
   add_custom_command(TARGET ${target} POST_BUILD
       COMMAND ${HEIMDALL_SBOM_TOOL_PATH} 
               ${HEIMDALL_LLD_PLUGIN_PATH} 
               $<TARGET_FILE:${target}>
               --format ${sbom_format}
               --output ${sbom_output}
       COMMENT "Generating SBOM for ${target}"
       VERBATIM
   )
   ```

2. **Gold Integration (Plugin Approach)**
   ```cmake
   # Add linker options for plugin integration
   target_link_options(${target} PRIVATE
       "LINKER:--plugin=${HEIMDALL_GOLD_PLUGIN_PATH}"
       "LINKER:--plugin-opt=sbom-output=${sbom_output}"
       "LINKER:--plugin-opt=format=${sbom_format}"
   )
   ```

3. **Fallback Mechanism**
   ```cmake
   # If plugin fails, fallback to wrapper approach
   if(HEIMDALL_FALLBACK_TO_WRAPPER)
       add_custom_command(TARGET ${target} POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E echo "Plugin failed, using wrapper approach"
           COMMAND ${HEIMDALL_SBOM_TOOL_PATH} 
                   ${HEIMDALL_GOLD_PLUGIN_PATH} 
                   $<TARGET_FILE:${target}>
                   --format ${sbom_format}
                   --output ${sbom_output}
       )
   endif()
   ```

#### Phase 3: Advanced Features

1. **Multi-Format Support**
   - Generate multiple SBOM formats simultaneously
   - Support for SPDX 2.3, 3.0, 3.0.1
   - Support for CycloneDX 1.4, 1.5, 1.6

2. **Dependency Analysis**
   - Extract component dependencies
   - Include transitive dependencies
   - Generate dependency graphs

3. **Custom Metadata**
   - Project-specific metadata injection
   - License information
   - Version information
   - Build environment details

### File Structure

```
cmake/
├── FindHeimdall.cmake
├── HeimdallSBOM.cmake
├── HeimdallConfig.cmake
└── templates/
    ├── sbom-config.json
    └── cmake-sbom-template.cmake
```

## Implementation Steps

### Step 1: Create FindHeimdall.cmake

**Purpose**: Locate Heimdall installation and plugins

**Key Features**:
- Search in standard installation paths
- Support custom HEIMDALL_ROOT variable
- Validate plugin availability
- Set up CMake variables for other modules

**Implementation**:
```cmake
# FindHeimdall.cmake
include(FindPackageHandleStandardArgs)

# Search paths
set(HEIMDALL_SEARCH_PATHS
    ${HEIMDALL_ROOT}
    $ENV{HEIMDALL_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}/build
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp23
    /usr/local/lib/heimdall
    /opt/heimdall
)

# Find plugins
find_library(HEIMDALL_LLD_PLUGIN heimdall-lld
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES lib
)

find_library(HEIMDALL_GOLD_PLUGIN heimdall-gold
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES lib
)

# Find heimdall-sbom tool
find_program(HEIMDALL_SBOM_TOOL heimdall-sbom
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES bin src/tools
)

# Validate findings
find_package_handle_standard_args(Heimdall
    REQUIRED_VARS HEIMDALL_SBOM_TOOL
    FOUND_VAR HEIMDALL_FOUND
)
```

### Step 2: Create HeimdallConfig.cmake

**Purpose**: Define configuration options and defaults

**Key Features**:
- Global configuration options
- Target-specific property definitions
- Default value management
- Validation functions

**Implementation**:
```cmake
# HeimdallConfig.cmake

# Global configuration options
option(HEIMDALL_ENABLE_SBOM "Enable SBOM generation" ON)
option(HEIMDALL_VERBOSE "Enable verbose SBOM output" OFF)
option(HEIMDALL_INCLUDE_SYSTEM_LIBS "Include system libraries in SBOM" OFF)

# Default values
set(HEIMDALL_SBOM_FORMAT "spdx" CACHE STRING "Default SBOM format")
set(HEIMDALL_SBOM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom" CACHE PATH "SBOM output directory")
set(HEIMDALL_PREFERRED_LINKER "auto" CACHE STRING "Preferred linker (lld, gold, auto)")

# Validate format
set_property(CACHE HEIMDALL_SBOM_FORMAT PROPERTY STRINGS 
    "spdx" "spdx-2.3" "spdx-3.0" "spdx-3.0.1" 
    "cyclonedx" "cyclonedx-1.4" "cyclonedx-1.5" "cyclonedx-1.6"
)

# Validate linker
set_property(CACHE HEIMDALL_PREFERRED_LINKER PROPERTY STRINGS 
    "auto" "lld" "gold"
)
```

### Step 3: Create HeimdallSBOM.cmake

**Purpose**: Main SBOM generation functions

**Key Features**:
- `heimdall_enable_sbom()` function
- Automatic linker detection
- Plugin vs wrapper approach selection
- Post-build command generation

**Implementation**:
```cmake
# HeimdallSBOM.cmake

include(CMakeParseArguments)

function(heimdall_enable_sbom target)
    cmake_parse_arguments(HEIMDALL_SBOM
        "VERBOSE;INCLUDE_SYSTEM_LIBS"
        "FORMAT;OUTPUT;LINKER"
        ""
        ${ARGN}
    )

    # Find Heimdall
    find_package(Heimdall REQUIRED)
    
    # Detect available linkers
    heimdall_detect_linkers()
    
    # Configure SBOM generation
    heimdall_configure_sbom_generation(${target})
endfunction()

function(heimdall_detect_linkers)
    # Check for LLD
    find_program(LLD_LINKER ld.lld)
    if(LLD_LINKER)
        set(HEIMDALL_LLD_AVAILABLE TRUE PARENT_SCOPE)
    endif()
    
    # Check for Gold (Linux only)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_program(GOLD_LINKER ld.gold)
        if(GOLD_LINKER)
            set(HEIMDALL_GOLD_AVAILABLE TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

function(heimdall_configure_sbom_generation target)
    # Determine best approach based on available linkers
    if(HEIMDALL_GOLD_AVAILABLE AND HEIMDALL_PREFERRED_LINKER STREQUAL "gold")
        heimdall_configure_gold_plugin(${target})
    elseif(HEIMDALL_LLD_AVAILABLE)
        heimdall_configure_lld_wrapper(${target})
    else()
        message(WARNING "No suitable linker found for SBOM generation")
    endif()
endfunction()
```

### Step 4: Create Usage Examples

**Purpose**: Demonstrate module usage

**Examples**:
```cmake
# Basic usage
find_package(Heimdall REQUIRED)
heimdall_enable_sbom(myapp)

# Advanced usage
heimdall_enable_sbom(myapp
    FORMAT cyclonedx-1.6
    OUTPUT myapp.cyclonedx.json
    VERBOSE ON
    INCLUDE_SYSTEM_LIBS ON
)

# Multi-format generation
heimdall_enable_sbom(myapp FORMAT spdx)
heimdall_enable_sbom(myapp FORMAT cyclonedx)
```

### Step 5: Integration Testing

**Purpose**: Validate module functionality

**Test Cases**:
1. Plugin detection on different platforms
2. LLD wrapper approach functionality
3. Gold plugin approach functionality
4. Fallback mechanism validation
5. Multi-format generation
6. Error handling and reporting

## Detailed TODO Steps

### Phase 1: Foundation (Week 1-2)

#### Week 1: Core Infrastructure
- [ ] **Day 1-2**: Create `FindHeimdall.cmake`
  - Implement plugin search logic
  - Add validation functions
  - Test on macOS and Linux
  - Document search paths and variables

- [ ] **Day 3-4**: Create `HeimdallConfig.cmake`
  - Define global configuration options
  - Implement property definitions
  - Add validation functions
  - Create default configuration templates

- [ ] **Day 5**: Create basic `HeimdallSBOM.cmake`
  - Implement `heimdall_enable_sbom()` function
  - Add linker detection logic
  - Create basic post-build command generation

#### Week 2: Linker Integration
- [ ] **Day 1-2**: Implement LLD wrapper approach
  - Add `heimdall_configure_lld_wrapper()` function
  - Implement post-build command generation
  - Test with various LLD versions
  - Add error handling and fallback logic

- [ ] **Day 3-4**: Implement Gold plugin approach
  - Add `heimdall_configure_gold_plugin()` function
  - Implement linker option configuration
  - Test plugin loading and error handling
  - Add dependency validation (elfutils, libelf)

- [ ] **Day 5**: Implement automatic approach selection
  - Add `heimdall_detect_linkers()` function
  - Implement preference-based selection
  - Add fallback mechanism
  - Test cross-platform compatibility

### Phase 2: Advanced Features (Week 3-4)

#### Week 3: Format Support and Configuration
- [ ] **Day 1-2**: Multi-format support
  - Implement format validation
  - Add SPDX 2.3, 3.0, 3.0.1 support
  - Add CycloneDX 1.4, 1.5, 1.6 support
  - Test format-specific options

- [ ] **Day 3-4**: Advanced configuration
  - Implement target-specific properties
  - Add custom metadata support
  - Implement output directory management
  - Add verbose output configuration

- [ ] **Day 5**: Template system
  - Create SBOM configuration templates
  - Implement template customization
  - Add project-specific metadata injection
  - Create usage examples

#### Week 4: Integration and Testing
- [ ] **Day 1-2**: CMake integration testing
  - Test with various CMake versions (3.16+)
  - Validate cross-platform compatibility
  - Test with different build types (Debug, Release)
  - Add integration test suite

- [ ] **Day 3-4**: Error handling and reporting
  - Implement comprehensive error messages
  - Add diagnostic information
  - Create troubleshooting guide
  - Add logging and debugging support

- [ ] **Day 5**: Documentation and examples
  - Create comprehensive documentation
  - Add usage examples for different scenarios
  - Create migration guide from manual approach
  - Add best practices guide

### Phase 3: Optimization and Polish (Week 5-6)

#### Week 5: Performance and Reliability
- [ ] **Day 1-2**: Performance optimization
  - Optimize plugin detection
  - Implement caching for repeated operations
  - Add parallel SBOM generation support
  - Optimize memory usage

- [ ] **Day 3-4**: Reliability improvements
  - Add comprehensive error recovery
  - Implement retry mechanisms
  - Add validation for generated SBOMs
  - Improve cross-compilation support

- [ ] **Day 5**: Advanced features
  - Add dependency analysis
  - Implement SBOM comparison tools
  - Add SBOM merging capabilities
  - Create SBOM validation functions

#### Week 6: Final Integration and Release
- [ ] **Day 1-2**: Final testing and validation
  - Comprehensive testing on all supported platforms
  - Performance benchmarking
  - Security review
  - User acceptance testing

- [ ] **Day 3-4**: Documentation finalization
  - Complete API documentation
  - Create migration guide
  - Add troubleshooting section
  - Create video tutorials

- [ ] **Day 5**: Release preparation
  - Create release notes
  - Prepare installation instructions
  - Create example projects
  - Final code review and cleanup

## Limitations and Considerations

### Technical Limitations

#### 1. Linker Compatibility
- **LLD Plugin Interface**: LLD doesn't support traditional linker plugins
- **Gold Dependencies**: Gold plugin requires elfutils, libelf, libdw
- **Platform Restrictions**: Gold only available on Linux
- **Version Compatibility**: Different LLVM versions have different plugin interfaces

#### 2. Build System Constraints
- **CMake Version**: Requires CMake 3.16+ for modern features
- **Compiler Support**: Limited to GCC and Clang
- **Cross-Compilation**: May not work with all cross-compilation setups
- **Parallel Builds**: SBOM generation may conflict with parallel builds

#### 3. SBOM Generation Limitations
- **Post-Processing**: LLD wrapper approach analyzes final binary
- **No Real-Time**: Cannot capture linking-time information
- **Limited Metadata**: Some metadata may not be available
- **Format Restrictions**: Some SBOM formats may have limitations

### Operational Limitations

#### 1. User Experience
- **Learning Curve**: Users must understand CMake integration
- **Configuration Complexity**: Multiple options may confuse users
- **Debugging Difficulty**: Plugin issues may be hard to diagnose
- **Platform Differences**: Different behavior on different platforms

#### 2. Maintenance Overhead
- **Plugin Updates**: Must track plugin interface changes
- **Linker Updates**: Must adapt to linker changes
- **CMake Updates**: Must maintain CMake compatibility
- **Platform Support**: Must support multiple platforms

#### 3. Performance Impact
- **Build Time**: SBOM generation adds to build time
- **Memory Usage**: Plugin loading increases memory usage
- **Disk Space**: Generated SBOMs consume disk space
- **Network**: May require downloading additional dependencies

### Mitigation Strategies

#### 1. Technical Mitigations
- **Fallback Mechanisms**: Automatic fallback to wrapper approach
- **Dependency Detection**: Automatic detection of required libraries
- **Error Recovery**: Comprehensive error handling and recovery
- **Caching**: Cache plugin detection and configuration

#### 2. User Experience Mitigations
- **Default Configuration**: Sensible defaults for common use cases
- **Progressive Disclosure**: Advanced options hidden by default
- **Clear Documentation**: Comprehensive examples and guides
- **Diagnostic Tools**: Built-in troubleshooting tools

#### 3. Maintenance Mitigations
- **Version Compatibility**: Support multiple plugin versions
- **Platform Abstraction**: Abstract platform-specific details
- **Modular Design**: Separate concerns for easier maintenance
- **Automated Testing**: Comprehensive test suite

## Future Enhancements

### Short-term (3-6 months)
- **IDE Integration**: Visual Studio Code, CLion support
- **CI/CD Integration**: GitHub Actions, GitLab CI templates
- **Package Manager Support**: Conan, vcpkg integration
- **Advanced Formats**: Support for additional SBOM formats

### Medium-term (6-12 months)
- **Real-time Analysis**: Integration with build-time analysis
- **Dependency Graphs**: Visual dependency analysis
- **SBOM Comparison**: Tools for comparing SBOMs
- **Vulnerability Scanning**: Integration with security tools

### Long-term (12+ months)
- **Machine Learning**: Automated component identification
- **Cloud Integration**: Cloud-based SBOM analysis
- **Compliance Tools**: Automated compliance checking
- **Enterprise Features**: Multi-project SBOM management

## Conclusion

The Heimdall CMake module will provide a seamless integration of SBOM generation into CMake-based build systems. By leveraging Heimdall's existing wrapper and plugin approaches, the module will offer a unified interface that works across different linkers and platforms.

The implementation will be phased over 6 weeks, starting with core infrastructure and progressing through advanced features and optimization. The module will include comprehensive error handling, fallback mechanisms, and extensive documentation to ensure a smooth user experience.

While there are technical limitations inherent in the current linker plugin architectures, the module will provide robust solutions that work reliably across different environments. The modular design will allow for future enhancements and adaptations to changing requirements.

The CMake module represents a significant step forward in making SBOM generation accessible to C++ developers, providing the foundation for broader adoption of SBOM practices in the C++ ecosystem. 

## Example Project and Usage Template

A complete example project using the Heimdall CMake module is provided in [`examples/heimdall-cmake-module-example`](../examples/heimdall-cmake-module-example). This demonstrates both LLD and Gold integration, automatic SBOM generation, and validation.

A ready-to-use CMake integration template is available at [`cmake/templates/cmake-sbom-template.cmake`](../cmake/templates/cmake-sbom-template.cmake). Copy and adapt this template for your own projects.

### Quick Integration Steps

1. Add the `cmake/` directory to your `CMAKE_MODULE_PATH`:
   ```cmake
   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
   ```
2. Include the Heimdall modules:
   ```cmake
   include(HeimdallConfig)
   include(HeimdallSBOM)
   ```
3. Add your target and enable SBOM generation:
   ```cmake
   add_executable(myapp main.cpp)
   heimdall_enable_sbom(myapp FORMAT spdx-2.3 VERBOSE ON)
   ```

For advanced options, troubleshooting, and further details, see the rest of this document and the comments in the template file. 