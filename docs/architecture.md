# Heimdall SBOM Generator Architecture

This document provides a comprehensive overview of the Heimdall SBOM (Software Bill of Materials) generator architecture, design patterns, and implementation details.

## 1. Overview

### Introduction to SBOM Formats

Software Bill of Materials (SBOM) documents are critical for supply chain security and software transparency. Heimdall supports the two leading SBOM standards:

**SPDX (Software Package Data Exchange)**
- **SPDX 2.3**: Tag-value format, well-established industry standard
- **SPDX 3.0/3.0.1**: JSON format with enhanced relationship modeling
- Provides comprehensive license compliance and security vulnerability tracking
- Strong ecosystem support with tools like FOSSology, REUSE, and commercial scanners

**CycloneDX**
- **CycloneDX 1.4**: Established JSON format for security-focused SBOMs
- **CycloneDX 1.5**: Added enhanced vulnerability and licensing support
- **CycloneDX 1.6**: Latest version with improved evidence and compliance features
- Designed specifically for application security and vulnerability management
- Strong integration with security tools like OWASP Dependency-Track

### Project Description

Heimdall is a comprehensive SBOM generation tool that integrates directly with the linking process via linker plugins for both LLVM LLD and GNU Gold linkers. It automatically captures all components that actually make it into final binaries, providing accurate and complete software bills of materials.

### Main Features and Goals

- **Dual Linker Support**: Native plugins for both LLVM LLD and GNU Gold linkers
- **Multi-Format Output**: Generates SPDX 2.3, 3.0, 3.0.1 and CycloneDX 1.4, 1.5, 1.6 compliant SBOMs
- **Comprehensive Analysis**: Extracts versions, licenses, checksums, dependencies, and debug information
- **Cross-Platform Support**: Native support for Linux and macOS with ELF, Mach-O, and PE formats
- **Package Manager Integration**: Recognizes components from Conan, vcpkg, and system packages
- **Security Focus**: Enables vulnerability scanning and compliance tracking

### Intended Audience

- **Software Developers**: Integrate SBOM generation into build systems
- **DevOps Engineers**: Automate SBOM creation in CI/CD pipelines
- **Security Teams**: Track software components for vulnerability management
- **Compliance Officers**: Generate compliance reports for regulatory requirements
- **Open Source Maintainers**: Provide transparency into project dependencies

## 2. System Architecture

### High-Level System Design

The Heimdall system follows a modular architecture with clear separation of concerns:

```mermaid
graph TB
    subgraph "Input Sources"
        A[Object Files]
        B[Static Libraries]
        C[Shared Libraries]
        D[Executables]
    end
    
    subgraph "Linker Integration"
        E[LLD Plugin]
        F[Gold Plugin]
        G[Wrapper Tool]
    end
    
    subgraph "Core Processing"
        H[Plugin Interface]
        I[Metadata Extractor]
        J[DWARF Extractor]
        K[Component Info]
    end
    
    subgraph "SBOM Generation"
        L[SBOM Generator]
        M[SPDX Formatter]
        N[CycloneDX Formatter]
    end
    
    subgraph "Output Formats"
        O[SPDX 2.3 Tag-Value]
        P[SPDX 3.0/3.0.1 JSON]
        Q[CycloneDX 1.4/1.5/1.6 JSON]
    end
    
    A --> E
    B --> E
    C --> E
    D --> E
    
    A --> F
    B --> F
    C --> F
    D --> F
    
    A --> G
    B --> G
    C --> G
    D --> G
    
    E --> H
    F --> H
    G --> H
    
    H --> I
    H --> J
    I --> K
    J --> K
    
    K --> L
    L --> M
    L --> N
    
    M --> O
    M --> P
    N --> Q
```

### Component Overview

The system consists of several key architectural layers:

1. **Input Layer**: Handles various binary formats (ELF, Mach-O, PE, archives)
2. **Integration Layer**: Provides linker plugin interfaces for LLD and Gold
3. **Processing Layer**: Extracts metadata, debug information, and dependencies
4. **Generation Layer**: Creates standardized SBOM documents
5. **Output Layer**: Formats data according to SPDX and CycloneDX specifications

## 3. Module Design

### Input Parsers

The input parsing system handles multiple binary formats and file types:

```mermaid
classDiagram
    class MetadataExtractor {
        +extractMetadata(ComponentInfo&) bool
        +extractVersionInfo(ComponentInfo&) bool
        +extractLicenseInfo(ComponentInfo&) bool
        +extractSymbolInfo(ComponentInfo&) bool
        +extractArchInfo(ComponentInfo&) bool
        +extractCodeSignInfo(ComponentInfo&) bool
        -parseELF(string) bool
        -parseMachO(string) bool
        -parsePE(string) bool
        -parseArchive(string) bool
    }
    
    class DWARFExtractor {
        +extractDebugInfo(ComponentInfo&) bool
        +extractSourceFiles(ComponentInfo&) bool
        +extractFunctions(ComponentInfo&) bool
        +extractCompileUnits(ComponentInfo&) bool
        -parseDWARFSections(binary) bool
        -extractLineNumbers(binary) bool
    }
    
    class AdaExtractor {
        +extractAdaMetadata(ComponentInfo&) bool
        +parseALIFiles(string) bool
        +extractAdaUnits(ComponentInfo&) bool
        +extractAdaDependencies(ComponentInfo&) bool
    }
    
    class Utils {
        +getFileType(string) FileType
        +calculateChecksum(string) string
        +resolveLibraryPath(string) string
        +extractDependencies(string) vector~string~
    }
    
    MetadataExtractor --> Utils
    DWARFExtractor --> Utils
    AdaExtractor --> Utils
```

**Key Features:**
- **Multi-format Support**: Handles ELF (Linux), Mach-O (macOS), PE (Windows), and archive formats
- **Debug Information**: Extracts DWARF debug data for source file mapping
- **Dependency Resolution**: Resolves library dependencies and @rpath references
- **Checksum Calculation**: Generates SHA-256 checksums for integrity verification
- **Ada Support**: Specialized extraction for Ada Language Interface (ALI) files

### Plugin Architecture

The plugin system provides a unified interface for different linkers:

```mermaid
classDiagram
    class PluginInterface {
        <<abstract>>
        +initialize() bool
        +cleanup() void
        +processInputFile(string) void
        +processLibrary(string) void
        +processSymbol(string, uint64, uint64) void
        +setOutputPath(string) void
        +setFormat(string) void
        +generateSBOM() void
        +getComponentCount() size_t
        #sbomGenerator SBOMGenerator*
        #processedComponents vector~ComponentInfo~
    }
    
    class LLDPlugin {
        +initialize() bool
        +cleanup() void
        +processInputFile(string) void
        +processLibrary(string) void
        +processSymbol(string, uint64, uint64) void
        +setOutputPath(string) void
        +setFormat(string) void
        +generateSBOM() void
        -adapter LLDAdapter*
    }
    
    class GoldPlugin {
        +initialize() bool
        +cleanup() void
        +processInputFile(string) void
        +processLibrary(string) void
        +processSymbol(string, uint64, uint64) void
        +setOutputPath(string) void
        +setFormat(string) void
        +generateSBOM() void
        -impl Impl*
    }
    
    class PluginConfig {
        +outputPath string
        +format string
        +cyclonedxVersion string
        +spdxVersion string
        +verbose bool
        +extractDebugInfo bool
        +includeSystemLibraries bool
        +transitiveDependencies bool
    }
    
    PluginInterface <|-- LLDPlugin
    PluginInterface <|-- GoldPlugin
    PluginInterface --> PluginConfig
```

**Design Patterns:**
- **Abstract Factory**: PluginInterface defines common interface
- **Strategy Pattern**: Different implementation strategies for LLD vs Gold
- **Configuration Pattern**: Centralized configuration management
- **PIMPL Idiom**: Private implementation to hide linker-specific details

### SBOM Generator

The core SBOM generation engine handles multiple output formats:

```mermaid
classDiagram
    class SBOMGenerator {
        +processComponent(ComponentInfo&) void
        +generateSBOM() void
        +setOutputPath(string) void
        +setFormat(string) void
        +setSPDXVersion(string) void
        +setCycloneDXVersion(string) void
        +getComponentCount() size_t
        +hasComponent(string) bool
        -impl Impl*
    }
    
    class SBOMGenerator_Impl {
        +generateSPDX(string) bool
        +generateSPDX3JSON(string) bool
        +generateCycloneDX(string) bool
        +generateSPDXDocument() string
        +generateSPDX2_3_Document() string
        +generateSPDX3_0_0_Document() string
        +generateSPDX3_0_1_Document() string
        +generateCycloneDXDocument() string
        -components map~string, ComponentInfo~
        -metadataExtractor MetadataExtractor*
        -outputPath string
        -format string
        -spdxVersion string
        -cyclonedxVersion string
    }
    
    class ComponentInfo {
        +name string
        +filePath string
        +version string
        +supplier string
        +license string
        +checksum string
        +fileType FileType
        +symbols vector~SymbolInfo~
        +dependencies vector~string~
        +sourceFiles vector~string~
        +properties map~string, string~
        +codeSignInfo CodeSignInfo
        +buildConfig BuildConfigInfo
        +platformInfo PlatformInfo
    }
    
    SBOMGenerator --> SBOMGenerator_Impl
    SBOMGenerator_Impl --> ComponentInfo
```

**Key Capabilities:**
- **Multi-Version Support**: Generates SPDX 2.3, 3.0, 3.0.1 and CycloneDX 1.4, 1.5, 1.6
- **Dependency Tracking**: Handles both direct and transitive dependencies
- **Component Deduplication**: Prevents duplicate entries in SBOM output
- **Rich Metadata**: Includes debug info, code signing, and platform details
- **Schema Compliance**: Ensures output validates against official schemas

### Output Formatters

Specialized formatters handle different SBOM specifications:

```mermaid
classDiagram
    class SPDXFormatter {
        +generateSPDX2_3_TagValue() string
        +generateSPDX3_0_JSON() string
        +generateSPDX3_0_1_JSON() string
        +generateDocumentHeader() string
        +generatePackageSection() string
        +generateFileSection() string
        +generateRelationships() string
        -validateSPDXLicense(string) string
        -generateSPDXId(string) string
        -generateVerificationCode() string
    }
    
    class CycloneDXFormatter {
        +generateCycloneDX1_4() string
        +generateCycloneDX1_5() string
        +generateCycloneDX1_6() string
        +generateMetadata() string
        +generateComponents() string
        +generateDependencies() string
        +generateVulnerabilities() string
        -generatePURL(ComponentInfo) string
        -generateEvidence(ComponentInfo) string
        -generateProperties(ComponentInfo) string
    }
    
    class OutputValidator {
        +validateSPDX(string) bool
        +validateCycloneDX(string) bool
        +validateAgainstSchema(string, string) bool
        -loadSchema(string) Schema
        -validateJSON(string, Schema) bool
    }
    
    SPDXFormatter --> OutputValidator
    CycloneDXFormatter --> OutputValidator
```

### CLI Interface

Command-line interface for standalone SBOM generation:

```mermaid
classDiagram
    class HeimdallSBOM {
        +main(int, char**) int
        +parseArguments(int, char**) bool
        +validateArguments() bool
        +processBinary(string) bool
        +generateOutput() bool
        -config PluginConfig
        -generator SBOMGenerator
    }
    
    class HeimdallValidate {
        +main(int, char**) int
        +validateSBOM(string) bool
        +validateSchema(string, string) bool
        +generateReport() bool
        -validator SBOMValidator
    }
    
    class SBOMValidator {
        +validateSPDXDocument(string) bool
        +validateCycloneDXDocument(string) bool
        +checkRequiredFields() bool
        +validateRelationships() bool
        +validateChecksums() bool
    }
    
    HeimdallSBOM --> SBOMGenerator
    HeimdallValidate --> SBOMValidator
```

## 4. Data Flow

### SBOM Generation Flow for SPDX

The following diagram shows how Heimdall processes input files to generate SPDX-compliant SBOMs:

```mermaid
sequenceDiagram
    participant L as Linker (LLD/Gold)
    participant P as Plugin Interface
    participant M as Metadata Extractor
    participant D as DWARF Extractor
    participant G as SBOM Generator
    participant S as SPDX Formatter
    participant F as Output File
    
    L->>P: Process input file
    P->>M: Extract metadata
    M->>M: Parse binary format (ELF/Mach-O/PE)
    M->>M: Calculate checksum
    M->>M: Extract version info
    M-->>P: Return ComponentInfo
    
    P->>D: Extract debug information
    D->>D: Parse DWARF sections
    D->>D: Extract source files
    D->>D: Extract function names
    D-->>P: Update ComponentInfo
    
    P->>G: Process component
    G->>G: Deduplicate components
    G->>G: Resolve dependencies
    G->>G: Process transitive deps
    
    L->>P: Linking complete
    P->>G: Generate SBOM
    G->>S: Format as SPDX
    
    alt SPDX 2.3
        S->>S: Generate tag-value format
        S->>S: Create document header
        S->>S: Generate package section
        S->>S: Generate file sections
        S->>S: Generate relationships
    else SPDX 3.0/3.0.1
        S->>S: Generate JSON format
        S->>S: Create @graph structure
        S->>S: Generate elements
        S->>S: Generate relationships
    end
    
    S->>F: Write SBOM document
```

### CycloneDX Generation Flow

The following diagram shows the CycloneDX-specific generation process:

```mermaid
sequenceDiagram
    participant P as Plugin Interface
    participant G as SBOM Generator
    participant C as CycloneDX Formatter
    participant F as Output File
    
    P->>G: Generate SBOM (CycloneDX)
    G->>C: Format as CycloneDX
    
    C->>C: Generate metadata section
    C->>C: Identify main component
    C->>C: Generate tool information
    
    C->>C: Generate components array
    loop For each component
        C->>C: Determine component type
        C->>C: Generate BOM reference
        C->>C: Generate PURL
        C->>C: Generate hashes
        C->>C: Generate properties
        
        alt CycloneDX 1.6
            C->>C: Generate evidence section
            C->>C: Generate licenses evidence
        end
        
        C->>C: Generate external references
    end
    
    C->>C: Generate dependencies section
    loop For each component with deps
        C->>C: Map dependency references
        C->>C: Create dependsOn relationships
    end
    
    alt CycloneDX 1.5/1.6
        C->>C: Generate vulnerability section
        C->>C: Generate compliance section
    end
    
    C->>F: Write CycloneDX JSON document
```

## 5. Extensibility and Plugins

### Plugin Development Framework

Heimdall provides a comprehensive framework for developing new plugins:

```mermaid
graph TB
    subgraph "Plugin Framework"
        A[PluginInterface]
        B[PluginConfig]
        C[PluginStatistics]
        D[PluginUtils]
    end
    
    subgraph "Core Services"
        E[MetadataExtractor]
        F[SBOMGenerator]
        G[Utils]
        H[ComponentInfo]
    end
    
    subgraph "Extension Points"
        I[Custom Extractors]
        J[Format Processors]
        K[Output Formatters]
        L[Validation Rules]
    end
    
    A --> E
    A --> F
    B --> A
    C --> A
    D --> A
    
    I --> E
    J --> F
    K --> F
    L --> F
```

**Adding New Linker Support:**

1. **Implement PluginInterface**: Create new plugin class inheriting from PluginInterface
2. **Linker Integration**: Implement linker-specific callbacks and hooks
3. **Component Processing**: Use existing MetadataExtractor and SBOMGenerator
4. **Configuration**: Extend PluginConfig for linker-specific options

**Example Plugin Structure:**
```cpp
class CustomLinkerPlugin : public PluginInterface
{
public:
    bool initialize() override;
    void processInputFile(const std::string& filePath) override;
    void processLibrary(const std::string& libraryPath) override;
    void generateSBOM() override;
    
private:
    std::unique_ptr<CustomAdapter> adapter;
};
```

**Adding New Output Formats:**

1. **Extend SBOMGenerator**: Add new format generation methods
2. **Create Formatter**: Implement format-specific generation logic
3. **Schema Validation**: Add validation for new format
4. **Documentation**: Update format support matrix

### Format Extension Architecture

```mermaid
classDiagram
    class FormatProcessor {
        <<abstract>>
        +generateDocument(vector~ComponentInfo~) string
        +validateOutput(string) bool
        +getFormatVersion() string
        +getSupportedVersions() vector~string~
    }
    
    class SPDXProcessor {
        +generateDocument(vector~ComponentInfo~) string
        +generateSPDX2_3() string
        +generateSPDX3_0() string
        +validateSPDX(string) bool
    }
    
    class CycloneDXProcessor {
        +generateDocument(vector~ComponentInfo~) string
        +generateCycloneDX1_4() string
        +generateCycloneDX1_5() string
        +generateCycloneDX1_6() string
        +validateCycloneDX(string) bool
    }
    
    class CustomFormatProcessor {
        +generateDocument(vector~ComponentInfo~) string
        +validateCustomFormat(string) bool
        +getCustomSchema() string
    }
    
    FormatProcessor <|-- SPDXProcessor
    FormatProcessor <|-- CycloneDXProcessor
    FormatProcessor <|-- CustomFormatProcessor
```

### Configuration and Customization

The plugin system supports extensive configuration options:

```yaml
# heimdall-config.yaml
output:
  format: "spdx"  # spdx, cyclonedx, custom
  version: "3.0.1"  # format-specific version
  path: "./sbom.json"
  
processing:
  extract_debug_info: true
  include_system_libraries: false
  transitive_dependencies: true
  parallel_processing: true
  
filters:
  exclude_patterns:
    - "*/test/*"
    - "*/build/*"
  include_patterns:
    - "*/src/*"
    - "*/lib/*"
    
metadata:
  calculate_checksums: true
  extract_licenses: true
  extract_versions: true
  follow_symlinks: false
  
linker:
  lld:
    enabled: true
    plugin_path: "./lib/heimdall-lld.so"
  gold:
    enabled: true
    plugin_path: "./lib/heimdall-gold.so"
```

## 6. Error Handling and Logging

### Error Management Strategy

Heimdall implements a comprehensive error handling and logging system:

```mermaid
graph TB
    subgraph "Error Categories"
        A[Fatal Errors]
        B[Processing Errors]
        C[Warnings]
        D[Debug Messages]
    end
    
    subgraph "Error Handlers"
        E[Plugin Error Handler]
        F[File Processing Handler]
        G[Format Validation Handler]
        H[Dependency Resolution Handler]
    end
    
    subgraph "Logging System"
        I[Log Manager]
        J[File Logger]
        K[Console Logger]
        L[Structured Logger]
    end
    
    A --> E
    B --> F
    C --> G
    D --> H
    
    E --> I
    F --> I
    G --> I
    H --> I
    
    I --> J
    I --> K
    I --> L
```

**Error Classification:**

1. **Fatal Errors**: Plugin initialization failures, invalid configurations
2. **Processing Errors**: File parsing failures, dependency resolution issues
3. **Warnings**: Missing metadata, unsupported file formats
4. **Debug Messages**: Detailed processing information

**Logging Architecture:**

```cpp
namespace heimdall {
namespace logging {

enum class LogLevel {
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

class Logger {
public:
    static void log(LogLevel level, const std::string& message);
    static void setLogLevel(LogLevel level);
    static void setLogFile(const std::string& path);
    static void enableStructuredLogging(bool enable);
};

// Convenience macros
#define LOG_FATAL(msg) Logger::log(LogLevel::FATAL, msg)
#define LOG_ERROR(msg) Logger::log(LogLevel::ERROR, msg)
#define LOG_WARNING(msg) Logger::log(LogLevel::WARNING, msg)
#define LOG_INFO(msg) Logger::log(LogLevel::INFO, msg)
#define LOG_DEBUG(msg) Logger::log(LogLevel::DEBUG, msg)

}}
```

**Error Recovery Strategies:**

- **Graceful Degradation**: Continue processing when non-critical errors occur
- **Fallback Mechanisms**: Use alternative methods when primary approaches fail
- **Partial Results**: Generate SBOMs with available data when some components fail
- **Error Reporting**: Detailed error messages with suggestions for resolution

### Example Error Scenarios

1. **Missing Dependencies**: Continue processing but mark dependency as unresolved
2. **Corrupted Binary**: Skip file but log detailed error information
3. **Permission Issues**: Attempt alternative access methods or skip with warning
4. **Format Validation**: Generate best-effort output with validation warnings

## 7. Example Use Case

### Step-by-Step Walkthrough: CMake Build with Heimdall

This section demonstrates a complete workflow using Heimdall with CMake, including both Gold and LLD plugin usage.

#### Project Setup

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.16)
project(ExampleApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Find Heimdall
find_package(PkgConfig REQUIRED)
pkg_check_modules(HEIMDALL REQUIRED heimdall)

# Create executable
add_executable(myapp 
    src/main.cpp 
    src/utils.cpp 
    src/math.cpp
)

# Link with libraries
target_link_libraries(myapp 
    ${CMAKE_DL_LIBS}
    pthread
    ssl
    crypto
)

# Configure Heimdall SBOM generation
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Use LLD linker
    target_link_options(myapp PRIVATE
        "SHELL:-fuse-ld=lld"
    )
    
    # Add post-build SBOM generation using wrapper
    add_custom_command(TARGET myapp POST_BUILD
        COMMAND heimdall-sbom 
            ${HEIMDALL_LLD_PLUGIN} 
            $<TARGET_FILE:myapp>
            --format spdx-3.0.1
            --output ${CMAKE_BINARY_DIR}/myapp.spdx.json
            --verbose
        COMMENT "Generating SPDX SBOM for myapp"
    )
else()
    # Use Gold linker with plugin interface
    target_link_options(myapp PRIVATE
        "SHELL:-fuse-ld=gold"
        "SHELL:-Wl,--plugin=${HEIMDALL_GOLD_PLUGIN}"
        "SHELL:-Wl,--plugin-opt=sbom-output=${CMAKE_BINARY_DIR}/myapp.cyclonedx.json"
        "SHELL:-Wl,--plugin-opt=format=cyclonedx-1.6"
        "SHELL:-Wl,--plugin-opt=verbose"
    )
endif()
```

#### Build Process Sequence

```mermaid
sequenceDiagram
    participant Dev as Developer
    participant CMake as CMake
    participant Compiler as Compiler
    participant Linker as Linker (LLD/Gold)
    participant Plugin as Heimdall Plugin
    participant Gen as SBOM Generator
    participant File as SBOM File
    
    Dev->>CMake: mkdir build && cd build
    Dev->>CMake: cmake ..
    CMake->>CMake: Configure build system
    CMake->>CMake: Find Heimdall plugins
    
    Dev->>CMake: make
    CMake->>Compiler: Compile source files
    Compiler-->>CMake: Object files (main.o, utils.o, math.o)
    
    CMake->>Linker: Link executable
    
    alt Using LLD (Clang)
        Linker->>Linker: Link with LLD
        Linker-->>CMake: Executable created
        CMake->>Plugin: heimdall-sbom (wrapper mode)
        Plugin->>Gen: Process executable
        Gen->>Gen: Extract metadata from executable
        Gen->>Gen: Resolve library dependencies
        Gen->>Gen: Generate SPDX 3.0.1 JSON
        Gen->>File: Write myapp.spdx.json
    else Using Gold (GCC)
        Linker->>Plugin: Load Heimdall Gold plugin
        Plugin->>Plugin: Initialize SBOM collection
        
        loop For each input file
            Linker->>Plugin: Process input file (main.o, utils.o, math.o)
            Plugin->>Gen: Add component info
        end
        
        loop For each library
            Linker->>Plugin: Process library (libssl.so, libcrypto.so, etc.)
            Plugin->>Gen: Add library component
        end
        
        Linker->>Plugin: Linking complete
        Plugin->>Gen: Generate SBOM
        Gen->>Gen: Generate CycloneDX 1.6 JSON
        Gen->>File: Write myapp.cyclonedx.json
    end
    
    CMake-->>Dev: Build complete with SBOM
```

#### Generated SBOM Output

**SPDX 3.0.1 JSON (LLD example):**
```json
{
  "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
  "@graph": [
    {
      "spdxId": "spdx:SPDXRef-DOCUMENT",
      "type": "SpdxDocument",
      "spdxVersion": "SPDX-3.0.1",
      "name": "myapp",
      "documentNamespace": "https://spdx.org/spdxdocs/heimdall-2025-01-27T12:34:56Z",
      "creationInfo": {
        "spdxId": "spdx:CreationInfo-1",
        "type": "CreationInfo",
        "created": "2025-01-27T12:34:56Z",
        "createdBy": [
          {
            "type": "Tool",
            "name": "Heimdall SBOM Generator-2.0.0"
          }
        ]
      },
      "dataLicense": "CC0-1.0",
      "files": [
        {
          "@id": "spdx:SPDXRef-myapp",
          "type": "File",
          "fileName": "/path/to/build/myapp",
          "checksums": [
            {
              "type": "Checksum",
              "algorithm": "SHA256",
              "checksumValue": "a1b2c3d4e5f6..."
            }
          ]
        }
      ],
      "packages": [
        {
          "@id": "spdx:SPDXRef-Package-myapp",
          "type": "Package",
          "name": "myapp",
          "versionInfo": "1.0.0"
        }
      ],
      "relationships": [
        {
          "type": "Relationship",
          "relationshipType": "CONTAINS",
          "relatedSpdxElement": "spdx:SPDXRef-myapp"
        }
      ]
    }
  ]
}
```

**CycloneDX 1.6 JSON (Gold example):**
```json
{
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2025-01-27T12:34:56Z",
    "tools": [
      {
        "vendor": "Heimdall",
        "name": "SBOM Generator",
        "version": "2.0.0"
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
      "bom-ref": "myapp-1.0.0",
      "type": "application",
      "name": "myapp",
      "version": "1.0.0",
      "description": "Executable component",
      "hashes": [
        {
          "alg": "SHA-256",
          "content": "a1b2c3d4e5f6..."
        }
      ],
      "purl": "pkg:generic/myapp@1.0.0"
    },
    {
      "bom-ref": "libssl-1.1.1",
      "type": "library",
      "name": "libssl",
      "version": "1.1.1",
      "description": "Shared library component",
      "supplier": {
        "name": "OpenSSL Project"
      },
      "purl": "pkg:system/libssl@1.1.1"
    }
  ],
  "dependencies": [
    {
      "ref": "myapp-1.0.0",
      "dependsOn": [
        "libssl-1.1.1",
        "libcrypto-1.1.1"
      ]
    }
  ]
}
```

#### Makefile Integration

For traditional Makefile builds:

```makefile
# Makefile with Heimdall integration
CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -O2
LDFLAGS = -fuse-ld=gold

# Heimdall configuration
HEIMDALL_GOLD_PLUGIN = /usr/lib/heimdall/heimdall-gold.so
SBOM_OUTPUT = myapp.cyclonedx.json
SBOM_FORMAT = cyclonedx-1.6

# Source files
SOURCES = src/main.cpp src/utils.cpp src/math.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = myapp

# Libraries
LIBS = -lssl -lcrypto -lpthread

# Build rules
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) \
		-Wl,--plugin=$(HEIMDALL_GOLD_PLUGIN) \
		-Wl,--plugin-opt=sbom-output=$(SBOM_OUTPUT) \
		-Wl,--plugin-opt=format=$(SBOM_FORMAT) \
		-Wl,--plugin-opt=verbose \
		-o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(SBOM_OUTPUT)

.PHONY: clean
```

#### Command-Line Usage

**Using the wrapper tool (recommended for LLD):**
```bash
# Build normally
g++ -fuse-ld=lld main.o utils.o math.o -o myapp -lssl -lcrypto

# Generate SBOM using wrapper
heimdall-sbom \
    /usr/lib/heimdall/heimdall-lld.so \
    myapp \
    --format spdx-3.0.1 \
    --output myapp.spdx.json \
    --verbose \
    --include-system-libs
```

**Using Gold plugin directly:**
```bash
# Build with integrated SBOM generation
g++ -fuse-ld=gold \
    -Wl,--plugin=/usr/lib/heimdall/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=myapp.cyclonedx.json \
    -Wl,--plugin-opt=format=cyclonedx-1.6 \
    -Wl,--plugin-opt=verbose \
    -Wl,--plugin-opt=include-system-libs \
    main.o utils.o math.o -o myapp -lssl -lcrypto
```

This comprehensive example demonstrates how Heimdall integrates seamlessly into existing build workflows while providing accurate, standards-compliant SBOM generation.

<function_calls>
<invoke name="TodoWrite">
<parameter name="todos">[{"id": "1", "content": "Explore codebase structure and understand current implementation", "status": "completed", "priority": "high"}, {"id": "2", "content": "Analyze SBOM generation logic and data flow", "status": "completed", "priority": "high"}, {"id": "3", "content": "Review plugin architecture and extensibility mechanisms", "status": "completed", "priority": "medium"}, {"id": "4", "content": "Create comprehensive architecture document with Mermaid diagrams", "status": "completed", "priority": "high"}]