# Heimdall Compiler Plugin System

## Overview

This document describes the design and implementation of compiler plugins for Heimdall to capture compile-time metadata and enhance SBOM generation capabilities. The compiler plugin system extends Heimdall's existing linker-based SBOM generation by hooking into the compilation phase to collect additional component information.

## Architecture

### Current Heimdall Architecture

Heimdall currently operates as:
- **GCC Gold Plugin**: Direct integration with GNU Gold linker via plugin interface
- **LLD Wrapper**: Post-processing approach for LLVM LLD linker using `heimdall-sbom` tool
- **Core Components**: Binary extractors (ELF, Mach-O, PE), DWARF parser, metadata extractors

### Extended Architecture with Compiler Plugins

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Source Code   │    │  Compiler Plugin │    │  Linker Plugin  │
│   (.c, .cpp)    │───▶│  (GCC/Clang)     │───▶│  (Gold/LLD)     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                              │                        │
                              ▼                        ▼
                       ┌──────────────┐         ┌──────────────┐
                       │ Compile-time │         │  Link-time   │
                       │   Metadata   │         │   Metadata   │
                       └──────────────┘         └──────────────┘
                              │                        │
                              └────────┬───────────────┘
                                       ▼
                              ┌─────────────────┐
                              │  Enhanced SBOM  │
                              │   Generation    │
                              └─────────────────┘
```

### Additional Information Available During Compilation

The compilation phase provides access to rich metadata that's not available during linking. Here's comprehensive information that can be detected:

#### 1. Source Code Analysis
- **Function Signatures**: Complete function declarations with parameter types and return types
- **Class/Struct Definitions**: All user-defined types, inheritance relationships, and member variables
- **Template Instantiations**: Generic code instantiations with specific type parameters
- **Namespace Usage**: All namespaces used and their hierarchical relationships
- **API Usage Patterns**: Which external APIs and libraries are being called
- **Code Complexity Metrics**: Cyclomatic complexity, lines of code, function count per file
- **Language Features Used**: C++ standard features utilized (lambdas, auto, constexpr, etc.)
- **Include Graph Analysis**: Deep dependency analysis of header file relationships

#### 2. Build Environment Context
- **Compiler Toolchain**: Exact compiler version, standard library version, linker version
- **Build Configuration**: Release vs debug builds, optimization flags, warning levels
- **Target Platform**: Architecture (x86_64, ARM, etc.), OS target, ABI version
- **Cross-Compilation**: Host vs target architecture when cross-compiling
- **Sanitizer Settings**: AddressSanitizer, ThreadSanitizer, UBSan configurations
- **Profile-Guided Optimization**: PGO settings and profile data usage
- **Link-Time Optimization**: LTO settings and whole-program optimization flags

#### 3. Preprocessor Analysis
- **Macro Expansion History**: How macros are expanded and nested
- **Conditional Compilation**: Which #ifdef branches are taken
- **Header Guard Analysis**: Include guard patterns and potential issues
- **Pragma Directives**: Compiler-specific pragmas and their effects
- **Preprocessor Arithmetic**: Constant expressions evaluated at compile time
- **Feature Test Macros**: Which C++ standard library features are available

#### 4. Security and Compliance Information
- **Buffer Overflow Patterns**: Static analysis for potential buffer overflows
- **Use-After-Free Detection**: Static analysis for memory safety issues
- **Integer Overflow Detection**: Potential arithmetic overflow situations
- **Format String Analysis**: Printf-style format string validation
- **SQL Injection Patterns**: If database queries are constructed in code
- **Cryptographic API Usage**: Detection of cryptographic library usage
- **Personal Data Handling**: Detection of patterns that might handle PII

#### 5. Third-Party Dependencies
- **Package Manager Integration**: Conan, vcpkg, pkg-config usage detection
- **Git Submodule Information**: Submodule commit hashes and remote URLs
- **Vendor Directory Analysis**: Third-party code embedded in source tree
- **System Library Dependencies**: Which system libraries are being used
- **Static vs Dynamic Linking Choices**: How dependencies are being linked

#### 6. Code Quality Metrics
- **Coding Standards Compliance**: Adherence to coding standards (Google, LLVM, etc.)
- **Documentation Coverage**: Presence of Doxygen or other documentation
- **Unit Test Coverage**: Detection of test files and their coverage
- **Dead Code Detection**: Unused functions, variables, and includes
- **Duplication Analysis**: Code duplication across files
- **Error Handling Patterns**: Exception handling and error return patterns

## Usage and Benefits

### Enhanced Compiler Plugin Benefits

1. **Complete Source File Tracking**: Capture all source files with hashes and licenses
2. **Compilation Settings**: Record compiler version, optimization levels, target architecture
3. **Enhanced Dependency Mapping**: Track include dependencies with license analysis
4. **Build Configuration**: Capture macro definitions and compiler-specific settings
5. **Development Context**: Record function names, global variables, and code structure
6. **Security Analysis**: Static analysis for common security vulnerabilities
7. **Compliance Tracking**: License detection and copyright analysis
8. **Quality Metrics**: Code quality and complexity analysis

### Enhanced SBOM Information

The compiler plugins enable Heimdall to generate SBOMs with:

- **Source Code Components**: All `.c`, `.cpp`, `.h` files used during compilation
- **Build Tool Information**: Compiler version, flags, and build environment
- **Include Dependencies**: Complete dependency graph of header files
- **Compilation Properties**: Optimization settings, debug information, target platform
- **Code Metadata**: Function names, global symbols, macro definitions

### Integration Workflow

1. **Compilation Phase**: Compiler plugins collect metadata during source compilation
2. **Intermediate Storage**: Metadata stored in JSON files in temporary directory
3. **Linking Phase**: Linker plugins/wrapper tools collect binary information
4. **SBOM Generation**: Combined metadata generates comprehensive SBOM
5. **Cleanup**: Temporary metadata files cleaned up after SBOM generation

### Example Enhanced SBOM Output

```json
{
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "metadata": {
    "timestamp": "2025-01-15T10:30:45Z",
    "tools": [
      {
        "vendor": "Heimdall",
        "name": "heimdall-compiler-plugin",
        "version": "1.0.0"
      }
    ],
    "component": {
      "type": "application",
      "name": "myapp",
      "version": "1.0.0"
    }
  },
  "components": [
    {
      "type": "file",
      "bom-ref": "src/main.cpp",
      "name": "main.cpp",
      "scope": "required",
      "hashes": [
        {
          "alg": "SHA-256",
          "content": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
        },
        {
          "alg": "SHA-1", 
          "content": "da39a3ee5e6b4b0d3255bfef95601890afd80709"
        },
        {
          "alg": "MD5",
          "content": "d41d8cd98f00b204e9800998ecf8427e"
        }
      ],
      "licenses": [
        {
          "license": {
            "id": "Apache-2.0",
            "name": "Apache License 2.0"
          }
        }
      ],
      "copyright": "Copyright 2025 The Project Authors",
      "properties": [
        {
          "name": "compiler.type",
          "value": "gcc"
        },
        {
          "name": "compiler.version", 
          "value": "11.4.0"
        },
        {
          "name": "compiler.optimization_level",
          "value": "2"
        },
        {
          "name": "compiler.debug_level",
          "value": "1"
        },
        {
          "name": "functions",
          "value": "main,process_data,cleanup"
        },
        {
          "name": "file.size",
          "value": "2048"
        },
        {
          "name": "file.modification_time",
          "value": "2025-01-15T10:25:30Z"
        },
        {
          "name": "file.authors",
          "value": "John Doe,Jane Smith"
        },
        {
          "name": "file.is_generated",
          "value": "false"
        }
      ]
    },
    {
      "type": "file",
      "bom-ref": "include/utils.h",
      "name": "utils.h",
      "scope": "required",
      "hashes": [
        {
          "alg": "SHA-256",
          "content": "a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456"
        }
      ],
      "licenses": [
        {
          "license": {
            "id": "MIT",
            "name": "MIT License"
          }
        }
      ],
      "copyright": "Copyright (c) 2025 Project Team",
      "properties": [
        {
          "name": "file.type",
          "value": "header"
        },
        {
          "name": "file.is_system",
          "value": "false"
        },
        {
          "name": "file.size",
          "value": "1536"
        },
        {
          "name": "license.confidence",
          "value": "0.92"
        }
      ]
    },
    {
      "type": "file",
      "bom-ref": "usr/include/stdio.h",
      "name": "stdio.h",
      "scope": "optional",
      "hashes": [
        {
          "alg": "SHA-256",
          "content": "f4c8996fb92427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afb"
        }
      ],
      "licenses": [
        {
          "license": {
            "id": "GPL-2.0-or-later",
            "name": "GNU General Public License v2.0 or later"
          }
        }
      ],
      "copyright": "Copyright (C) 1991-2023 Free Software Foundation",
      "properties": [
        {
          "name": "file.type",
          "value": "system_header"
        },
        {
          "name": "file.is_system",
          "value": "true"
        },
        {
          "name": "file.size",
          "value": "12288"
        },
        {
          "name": "license.confidence",
          "value": "0.88"
        }
      ]
    }
  ],
  "dependencies": [
    {
      "ref": "src/main.cpp",
      "dependsOn": ["include/utils.h", "usr/include/stdio.h"]
    }
  ]
}
```

## Future Enhancements

1. **Language Support**: Extend to other languages (Rust, Go, Ada)
2. **IDE Integration**: Provide IDE plugins for development-time SBOM generation
3. **Continuous Integration**: Integrate with CI/CD pipelines for automated SBOM tracking
4. **Version Tracking**: Track changes in SBOMs across different builds
5. **Supply Chain Analysis**: Enhanced vulnerability and license analysis with compiler metadata

## Usage Summary

### For GCC Projects
```bash
# Compile with Heimdall GCC plugin
gcc -fplugin=./lib/libheimdall-gcc-plugin.so \
    -fplugin-arg-heimdall-gcc-plugin-output-dir=./metadata \
    -c source.c -o source.o

# Link with enhanced Gold plugin
ld.gold --plugin=./lib/heimdall-gold.so source.o -o binary

# Generate enhanced SBOM
heimdall-enhanced-sbom ./lib/heimdall-gold.so ./binary \
    --format cyclonedx --output enhanced.cdx.json
```

### For Clang/LLVM Projects
```bash
# Compile with Heimdall Clang plugin
clang++ -Xclang -load -Xclang ./lib/libheimdall-clang-plugin.so \
         -Xclang -plugin -Xclang heimdall-metadata-collector \
         -Xclang -plugin-arg-heimdall-metadata-collector \
         -Xclang output-dir=./metadata \
         -c source.cpp -o source.o

# Link with enhanced LLD wrapper
lld-link source.o --plugin=./lib/heimdall-lld.so -o binary.exe

# Generate enhanced SBOM
heimdall-enhanced-sbom ./lib/heimdall-lld.so ./binary.exe \
    --format spdx --output enhanced.spdx
```
