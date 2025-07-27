<img src="https://github.com/Heimdall-SBOM/heimdall/blob/main/docs/images/heimdall-cpp.png?raw=true" alt="C/C++ and Heimdall" width="50%">

# C/C++ SBOM Generation with Heimdall

Heimdall extracts C/C++-specific metadata from object files, executables, and debug information and stores this comprehensive information in SPDX and CycloneDX SBOM formats.  

## Overview

Heimdall's C/C++ extractor provides comprehensive SBOM generation for C/C++ applications by combining:
- **Binary analysis** of object files and executables
- **DWARF debug information** parsing for C/C++-specific metadata
- **Symbol table analysis** for function and variable information
- **Cross-referencing** between binary and source information

## Data Sources

### 1. Object Files and Executables

#### **Extracted Information:**
- **File format**: ELF, PE, Mach-O detection
- **Architecture**: Target platform information (x86_64, ARM64, etc.)
- **Symbols**: Function and variable names with mangling
- **Sections**: Code, data, debug sections
- **Dependencies**: Dynamic and static library dependencies
- **Debug information**: DWARF debug info (if available)
- **Checksums**: SHA1, SHA256 file integrity
- **File type**: Executable, shared library, object file
- **Compiler information**: Compiler version and flags

#### **Example Extraction:**
```bash
# Binary analysis of C++ executable
./heimdall-sbom lib/heimdall-lld.so myapp --format cyclonedx
```

**Output includes:**
- File checksums and integrity information
- Dynamic library dependencies (libstdc++.so.6, libc.so.6, libm.so.6, etc)
- Symbol table information with C++ name mangling
- Debug section analysis
- Dependency linking for direct and transivitive dependencies
- Compiler identification (GCC, Clang, MSVC)

### 2. DWARF Debug Information

#### **Extracted Information:**

##### **Source File Information:**
- **Source file names**: `.cpp`, `.c`, `.h`, `.hpp` file mappings
- **File paths**: Absolute and relative paths
- **File timestamps**: When files were compiled
- **File checksums**: Integrity verification
- **Include relationships**: Header file dependencies

##### **Function Information:**
- **Function names**: Demangled C++ function names
- **Function signatures**: Parameter types and return types
- **Function locations**: Source file and line numbers
- **Function scope**: Namespace and class membership
- **Function attributes**: Inline, static, virtual, etc.

##### **Variable Information:**
- **Variable names**: Local and global variables
- **Variable types**: C/C++ type information
- **Variable scope**: Local, global, static, thread-local
- **Variable locations**: Source file and line numbers
- **Variable attributes**: Const, volatile, etc.

##### **Type Information:**
- **Class definitions**: C++ class and struct information
- **Template instantiations**: C++ template specializations
- **Type hierarchies**: Inheritance relationships
- **Type sizes**: Memory layout information
- **Type attributes**: POD, trivial, standard layout

##### **Namespace Information:**
- **Namespace names**: C++ namespace hierarchy
- **Namespace contents**: Functions and classes in namespaces
- **Using declarations**: Imported namespaces
- **Namespace aliases**: Type aliases and using declarations

#### **Example DWARF Information:**
```
DW_TAG_compile_unit
  DW_AT_name: "main.cpp"
  DW_AT_comp_dir: "/home/user/project/src"
  DW_AT_producer: "GNU C++17 11.2.0"
  DW_AT_language: DW_LANG_C_plus_plus_17

DW_TAG_namespace
  DW_AT_name: "myapp"
  
  DW_TAG_class_type
    DW_AT_name: "Calculator"
    DW_AT_byte_size: 24
    
    DW_TAG_member
      DW_AT_name: "value"
      DW_AT_type: DW_TAG_base_type (int)
      
    DW_TAG_subprogram
      DW_AT_name: "add"
      DW_AT_linkage_name: "_ZN5myapp10Calculator3addEi"
      DW_AT_decl_file: 1
      DW_AT_decl_line: 15
```

### 3. Symbol Table Analysis

#### **Extracted Information:**

##### **C++ Name Mangling:**
- **Demangled names**: Human-readable function names
- **Mangled names**: Linker symbols for function resolution
- **Overload resolution**: C++ function overloading
- **Template specializations**: Template instantiation names

##### **Symbol Types:**
- **Function symbols**: Global and static functions
- **Variable symbols**: Global and static variables
- **Weak symbols**: Weakly linked symbols
- **Undefined symbols**: External dependencies

##### **Linkage Information:**
- **Internal linkage**: Static functions and variables
- **External linkage**: Exported functions and variables
- **Weak linkage**: Weakly linked symbols
- **Hidden visibility**: Symbol visibility attributes

#### **Example Symbol Analysis:**
```
Symbol: _ZN5myapp10Calculator3addEi
Demangled: myapp::Calculator::add(int)
Type: Function
Linkage: External
Visibility: Default
Section: .text
```

## SBOM Format Storage

### SPDX 2.3 Format

#### **Package Information:**
```
PackageName: myapp
PackageVersion: 1.0.0
PackageSupplier: Organization: MyCompany
PackageDownloadLocation: NOASSERTION
FilesAnalyzed: true
PackageVerificationCode: 4e3211c67a2d28fced849ee1bb76e7391b93feba
PackageChecksum: SHA256: a1b2c3d4e5f6...
PackageHomePage: https://github.com/mycompany/myapp
PackageLicenseDeclared: MIT
PackageLicenseConcluded: MIT
PackageCopyrightText: Copyright (c) 2025 MyCompany
PackageSummary: A C++ calculator application
PackageDescription: A simple calculator application demonstrating C++ SBOM generation
PackageComment: Generated by Heimdall SBOM Generator
```

#### **File Information:**
```
FileName: src/main.cpp
FileType: SOURCE
FileChecksum: SHA256: f1e2d3c4b5a6...
LicenseConcluded: MIT
LicenseInfoInFile: MIT
FileCopyrightText: Copyright (c) 2025 MyCompany
FileComment: Main application entry point
```

#### **Dependencies:**
```
Relationship: myapp CONTAINS src/main.cpp
Relationship: myapp CONTAINS src/calculator.cpp
Relationship: myapp DEPENDS_ON libstdc++
Relationship: myapp DEPENDS_ON libc
```

### CycloneDX Format

#### **Component Information:**
```json
{
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2025-07-19T22:27:55Z",
    "tools": [
      {
        "vendor": "Heimdall",
        "name": "SBOM Generator",
        "version": "2.0.0"
      }
    ]
  },
  "components": [
    {
      "type": "application",
      "name": "myapp",
      "version": "1.0.0",
      "purl": "pkg:generic/myapp@1.0.0",
      "properties": [
        {
          "name": "compiler",
          "value": "GNU C++17 11.2.0"
        },
        {
          "name": "sourceFiles",
          "value": "main.cpp, calculator.cpp, calculator.h, utils.cpp, utils.h"
        },
        {
          "name": "buildFlags",
          "value": "-std=c++17 -O2 -g -Wall -Wextra"
        },
        {
          "name": "architecture",
          "value": "x86_64"
        },
        {
          "name": "fileFormat",
          "value": "ELF"
        }
      ],
      "dependencies": [
        "libstdc++",
        "libc",
        "libm"
      ]
    }
  ]
}
```

## Enhanced Metadata Extraction

### Currently Extracted

#### **From Binary Files:**
- File format and architecture
- Symbol table information with demangling
- Dynamic library dependencies
- Debug information (if available)
- File integrity checksums
- Section information
- Compiler identification

#### **From DWARF Information:**
- Source file mappings and relationships
- Function and variable type information
- C++ class and namespace hierarchies
- Template instantiation details
- Build configuration and compiler flags
- Debug symbol information

#### **Security Information:**
```json
{
  "properties": [
    {
      "name": "security.compiler",
      "value": "GNU C++17 11.2.0"
    },
    {
      "name": "security.buildFlags",
      "value": "-std=c++17 -O2 -g -Wall -Wextra -fstack-protector-strong"
    },
    {
      "name": "security.architecture",
      "value": "x86_64"
    },
    {
      "name": "security.fileFormat",
      "value": "ELF"
    }
  ]
}
```

#### **Function Call Graph:**
```json
{
  "properties": [
    {
      "name": "functions.calls",
      "value": "[main] -> [Calculator::add], [main] -> [Calculator::subtract], [Calculator::add] -> [validate_input]"
    },
    {
      "name": "functions.namespaces",
      "value": "myapp::Calculator, myapp::Utils, std::"
    },
    {
      "name": "functions.templates",
      "value": "std::vector<int>, std::string, std::unique_ptr<Calculator>"
    }
  ]
}
```

#### **Type System Information:**
```json
{
  "properties": [
    {
      "name": "types.classes",
      "value": "Calculator, Utils, InputValidator"
    },
    {
      "name": "types.inheritance",
      "value": "Calculator extends BaseCalculator, InputValidator implements Validator"
    },
    {
      "name": "types.templates",
      "value": "std::vector<T>, std::unique_ptr<T>, std::shared_ptr<T>"
    },
    {
      "name": "types.variables",
      "value": "int result, std::string input, Calculator calc"
    }
  ]
}
```

#### **Build Reproducibility:**
```json
{
  "properties": [
    {
      "name": "build.timestamps",
      "value": "main.cpp: 20250719161512, calculator.cpp: 20250719161443"
    },
    {
      "name": "build.checksums",
      "value": "main.cpp: b2efb2f5, calculator.cpp: f03e478f"
    },
    {
      "name": "build.compiler",
      "value": "GNU C++17 11.2.0"
    },
    {
      "name": "build.flags",
      "value": "-std=c++17 -O2 -g -Wall -Wextra"
    }
  ]
}
```

#### **Source File Analysis:**
```json
{
  "properties": [
    {
      "name": "source.files",
      "value": "main.cpp, calculator.cpp, calculator.h, utils.cpp, utils.h"
    },
    {
      "name": "source.includes",
      "value": "#include <iostream>, #include <vector>, #include \"calculator.h\""
    },
    {
      "name": "source.namespaces",
      "value": "myapp, std, boost"
    },
    {
      "name": "source.templates",
      "value": "std::vector, std::unique_ptr, std::shared_ptr"
    }
  ]
}
```

## Usage Examples

### Basic SBOM Generation
```bash
# Generate SPDX 2.3 SBOM
./heimdall-sbom lib/heimdall-lld.so myapp --format spdx-2.3 --output cpp_sbom.spdx.json

# Generate CycloneDX SBOM  
./heimdall-sbom lib/heimdall-lld.so myapp --format cyclonedx-1.6 --output cpp_sbom.cdx.json
```

### Integration with Build Systems

#### **CMake Integration with Heimdall Module:**
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(myapp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find and configure Heimdall
find_package(Heimdall REQUIRED)

add_executable(myapp main.cpp calculator.cpp utils.cpp)
target_compile_options(myapp PRIVATE -g -Wall -Wextra)

# Enable SBOM generation for the target
heimdall_enable_sbom(myapp
    FORMATS "spdx-2.3;cyclonedx-1.6"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom"
    INCLUDE_DEBUG_INFO ON
    VERBOSE ON
)

# Alternative: Configure SBOM generation with specific options
heimdall_configure_sbom(myapp
    SPDX_VERSION "2.3"
    CYCLONEDX_VERSION "1.6"
    OUTPUT_PREFIX "myapp"
    INCLUDE_SYSTEM_LIBS OFF
    EXTRACT_DEBUG_INFO ON
)

# Example with multiple targets and shared library
add_library(calculator_lib SHARED calculator.cpp calculator.h)
add_executable(calculator_app main.cpp)

target_link_libraries(calculator_app PRIVATE calculator_lib)

# Enable SBOM for both targets
heimdall_enable_sbom(calculator_lib
    FORMATS "cyclonedx-1.6"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom"
)

heimdall_enable_sbom(calculator_app
    FORMATS "spdx-2.3;cyclonedx-1.6"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom"
    INCLUDE_DEBUG_INFO ON
)
```

#### **Heimdall CMake Module Features:**

The Heimdall CMake module provides seamless integration for SBOM generation:

- **Automatic SBOM Generation**: SBOMs are generated automatically during the build process
- **Multiple Format Support**: Generate SPDX and CycloneDX formats simultaneously
- **Debug Information Integration**: Automatically extract debug information when available
- **Dependency Analysis**: Analyze both direct and transitive dependencies
- **Customizable Output**: Configure output directories, file names, and formats
- **Build System Integration**: Works with any CMake-based build system

#### **Makefile Integration:**
```makefile
# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -g -Wall -Wextra
TARGET = myapp
SOURCES = main.cpp calculator.cpp utils.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^
	./heimdall-sbom lib/heimdall-lld.so $@ --format spdx-2.3 --output sbom.spdx.json
```

### CI/CD Integration with Heimdall Module
```yaml
# GitHub Actions example
name: Build and Generate SBOM

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake
    
    - name: Build Application with SBOM Generation
      run: |
        mkdir build && cd build
        cmake ..
        make
        # SBOM files are automatically generated during build
    
    - name: Upload SBOM Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: sbom-files
        path: |
          build/sbom/*.spdx.json
          build/sbom/*.cdx.json
```

### Advanced Usage with Debug Information
```bash
# Build with full debug information
g++ -std=c++17 -O2 -g3 -Wall -Wextra -fstack-protector-strong main.cpp calculator.cpp utils.cpp -o myapp

# Generate comprehensive SBOM with debug info
./heimdall-sbom lib/heimdall-lld.so myapp \
    --format spdx-2.3 \
    --output sbom.spdx.json \
    --include-debug-info \
    --verbose
```

## C++-Specific Features

### Template Analysis
Heimdall can analyze C++ template instantiations and provide detailed information about:
- Template parameter types
- Instantiated template classes and functions
- Template specialization information
- Template dependency relationships

### Namespace Analysis
Comprehensive namespace analysis including:
- Namespace hierarchies and nesting
- Using declarations and namespace aliases
- Namespace-scoped functions and classes
- Cross-namespace dependencies

### Class Hierarchy Analysis
Detailed class and inheritance analysis:
- Base class and derived class relationships
- Virtual function tables and polymorphism
- Class member functions and variables
- Access specifiers and visibility

### Modern C++ Features
Support for modern C++ language features:
- C++11/14/17/20/23 language standards
- Lambda expressions and closures
- Smart pointers (unique_ptr, shared_ptr, weak_ptr)
- Move semantics and rvalue references
- Constexpr and consteval functions
- Concepts and constraints (C++20)

## Conclusion

Heimdall's C/C++ extractor provides comprehensive SBOM generation by combining binary analysis with DWARF debug information parsing. This approach captures both the compiled binary information and the rich C/C++-specific metadata available in debug symbols, resulting in detailed, accurate SBOMs that support security analysis, compliance auditing, and dependency management.

The integration of DWARF debug information parsing significantly enhances the SBOM quality by providing C/C++-specific information that would not be available from binary analysis alone, including source file mappings, function signatures, class hierarchies, template instantiations, and namespace relationships.

The C/C++ extractor is particularly valuable for complex C++ applications on embedded devices that use modern language features, templates, and sophisticated class hierarchies, providing detailed insights into the application's structure and dependencies. 