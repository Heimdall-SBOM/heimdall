# Ada SBOM Generation with Heimdall

This document describes how Heimdall extracts Ada-specific metadata from object files, executables, and ALI (Ada Library Information) files and stores this information in SPDX and CycloneDX SBOM formats.

## Overview

Heimdall's Ada extractor provides comprehensive SBOM generation for Ada applications by combining:
- **Binary analysis** of object files and executables
- **ALI file parsing** for Ada-specific metadata
- **Cross-referencing** between binary and source information

## Data Sources

### 1. Object Files and Executables

#### **Extracted Information:**
- **File format**: ELF, PE, Mach-O detection
- **Architecture**: Target platform information
- **Symbols**: Function and variable names
- **Sections**: Code, data, debug sections
- **Dependencies**: Dynamic and static library dependencies
- **Debug information**: DWARF debug info (if available)
- **Checksums**: SHA1, SHA256 file integrity
- **File type**: Executable, shared library, object file

#### **Example Extraction:**
```bash
# Binary analysis of Ada executable
./heimdall-sbom lib/heimdall-gold.so main_static --format spdx-2.3
```

**Output includes:**
- File checksums and integrity information
- Dynamic library dependencies (libgnat-11.so, libc.so.6)
- Symbol table information
- Debug section analysis

### 2. ALI (Ada Library Information) Files

#### **Extracted Information:**

##### **Package Information:**
- **Package names**: `main`, `data_reader`, `string_utils`, `math_lib`
- **Package types**: Specification (`%s`) vs Body (`%b`)
- **Source files**: `.ads` and `.adb` file mappings

##### **Dependencies:**
- **With-clause dependencies** (`W` lines): Explicit imports
- **Runtime dependencies** (`Z` lines): System and runtime packages
- **Dependency hierarchy**: Build-time vs runtime relationships

##### **Build Configuration:**
- **Compiler version**: `GNAT Lib v11`
- **Build flags**: `NO_IO`, `NO_SECONDARY_STACK`, etc.
- **Optimization settings**: Compiler optimization flags
- **Runtime flags**: Exception handling, memory safety settings

##### **Source File Information:**
- **Source file names**: Extracted from ALI dependency lines
- **File timestamps**: When files were compiled
- **File checksums**: Integrity verification
- **Package associations**: Which source files belong to which packages

##### **Function and Type Information:**
- **Function signatures**: Parameter types and return types
- **Variable types**: Type information for all variables
- **Cross-references**: Function call relationships
- **Type system details**: Ada's rich type system information

#### **Example ALI File Content:**
```
V "GNAT Lib v11"
RV NO_IO
RV NO_SECONDARY_STACK
W ada.text_io%s         a-textio.adb            a-textio.ali
W data_reader%s         data_reader.adb         data_reader.ali
D data_reader.ads       20250719161512 b2efb2f5 data_reader%s
X 11 main.adb
6U11*Main 6b11 15l5 15t9
7a4 Data{string} 14r39
G r c none [main standard 6 11 none] [read_data_file data_reader 2 13 none]
```

## SBOM Format Storage

### SPDX 2.3 Format

#### **Package Information:**
```spdx
PackageName: heimdall-ada-demo
PackageVersion: GNAT Lib v11
PackageManager: GNAT
PackageDescription: Ada application with GNAT compiler
```

#### **File Information:**
```spdx
FileName: main_static
SPDXID: SPDXRef-main-static
FileType: APPLICATION
FileChecksum: SHA1: 7300c6bc74cd5a8d96f0bdd1032c6ec7d03e1053
FileChecksum: SHA256: 16ceca22318374aeb4aa41bc76f774bf8cd8d3dc036ab2ff499e29ae6880924f
FileComment: Source files: string_utils.adb, a-chahan.adb, a-charac.ads, a-tags.adb, a-textio.adb, ada.ads, data_reader.adb, math_lib.adb, s-conca2.adb, s-imgint.ads, s-secsta.adb
```

#### **Dependencies:**
```spdx
Relationship: SPDXRef-Package CONTAINS SPDXRef-libc-so-6
Relationship: SPDXRef-Package CONTAINS SPDXRef-libgnat-11-so
Relationship: SPDXRef-Package CONTAINS SPDXRef-main-static
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
      "name": "heimdall-ada-demo",
      "version": "GNAT Lib v11",
      "purl": "pkg:gnat/heimdall-ada-demo@GNAT%20Lib%20v11",
      "properties": [
        {
          "name": "packageManager",
          "value": "GNAT"
        },
        {
          "name": "sourceFiles",
          "value": "string_utils.adb, a-chahan.adb, a-charac.ads, a-tags.adb, a-textio.adb, ada.ads, data_reader.adb, math_lib.adb, s-conca2.adb, s-imgint.ads, s-secsta.adb"
        },
        {
          "name": "buildFlags",
          "value": "NO_IO, NO_SECONDARY_STACK, NO_STANDARD_STORAGE_POOLS, NO_DYNAMIC_SIZED_OBJECTS"
        }
      ],
      "dependencies": [
        "data_reader",
        "string_utils", 
        "math_lib",
        "main"
      ]
    }
  ]
}
```

## Enhanced Metadata Extraction

### Currently Extracted

#### **From Binary Files:**
- File format and architecture
- Symbol table information
- Dynamic library dependencies
- Debug information (if available)
- File integrity checksums
- Section information

#### **From ALI Files:**
- Package manager identification (GNAT)
- Compiler version information
- Ada package dependencies
- Source file mappings
- Build configuration flags
- Package type information (spec/body)

### Potential Enhancements

#### **Security Information:**
```json
{
  "properties": [
    {
      "name": "security.buildFlags",
      "value": "NO_EXCEPTION_HANDLERS, NO_EXCEPTIONS, NO_DEFAULT_INITIALIZATION"
    },
    {
      "name": "security.compilerVersion", 
      "value": "GNAT Lib v11"
    },
    {
      "name": "security.runtimeFlags",
      "value": "NO_IO, NO_SECONDARY_STACK"
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
      "value": "[main] -> [read_data_file data_reader], [main] -> [factorial math_lib], [main] -> [to_upper string_utils]"
    }
  ]
}
```

#### **Type System Information:**
```json
{
  "properties": [
    {
      "name": "types.variables",
      "value": "Data{string}, Result{integer}, Upper{string}"
    },
    {
      "name": "types.functions", 
      "value": "Read_Data_File{string}, Factorial{integer}, To_Upper{string}"
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
      "value": "data_reader.ads: 20250719161512, main.adb: 20250719161443"
    },
    {
      "name": "build.checksums",
      "value": "data_reader.ads: b2efb2f5, main.adb: f03e478f"
    }
  ]
}
```

## Usage Examples

### Basic SBOM Generation
```bash
# Generate SPDX 2.3 SBOM
./heimdall-sbom lib/heimdall-gold.so main_static --format spdx-2.3 --output ada_sbom.spdx.json

# Generate CycloneDX SBOM  
./heimdall-sbom lib/heimdall-gold.so main_static --format cyclonedx-1.6 --output ada_sbom.cdx.json
```

### Integration with Build Systems
```bash
# In Makefile or build script
gnatmake -g main.adb
./heimdall-sbom lib/heimdall-gold.so main --format spdx-2.3 --output sbom.spdx.json
```

### CI/CD Integration
```yaml
# GitHub Actions example
- name: Generate Ada SBOM
  run: |
    gnatmake -g main.adb
    ./heimdall-sbom lib/heimdall-gold.so main --format spdx-2.3 --output sbom.spdx.json
    ./heimdall-sbom lib/heimdall-gold.so main --format cyclonedx-1.6 --output sbom.cdx.json
```

## Benefits

### **Security Analysis:**
- Build configuration flags for security assessment
- Compiler version for vulnerability analysis
- Runtime safety settings evaluation

### **Dependency Management:**
- Complete Ada package dependency graph
- Cross-package function call relationships
- Build-time vs runtime dependency classification

### **Compliance:**
- Package manager identification for licensing
- Source file mapping for attribution
- Build reproducibility for audit trails

### **Type Safety:**
- Ada's rich type system information
- Function signature verification
- Variable type safety guarantees

### **Build Reproducibility:**
- Exact compilation timestamps
- File integrity checksums
- Complete build configuration

## Technical Implementation

### Ada Extractor Architecture

```cpp
class AdaExtractor {
    // ALI file parsing
    bool parseAliFile(const std::string& aliFilePath, AdaPackageInfo& packageInfo);
    bool parseDependencyLine(const std::string& line, AdaPackageInfo& packageInfo);
    bool parseFunctionLine(const std::string& line, std::vector<AdaFunctionInfo>& functions);
    
    // Metadata extraction
    bool extractAdaMetadata(ComponentInfo& component, const std::vector<std::string>& aliFiles);
    bool findAliFiles(const std::string& directory, std::vector<std::string>& aliFiles);
    
    // Integration
    bool extractSourceFilePath(const std::string& aliFilePath);
    std::string extractPackageName(const std::string& aliFilePath);
};
```

### Integration with Metadata Extractor

```cpp
// In MetadataExtractor::extractMetadata()
if (!packageManagerDetected) {
    std::vector<std::string> aliFiles;
    if (findAdaAliFiles(searchPath, aliFiles)) {
        if (extractAdaMetadata(component, aliFiles)) {
            packageManagerDetected = true;
        }
    }
}
```

## Future Enhancements

### Planned Features:
1. **Enhanced function signature extraction** from X lines
2. **Cross-reference analysis** from G lines  
3. **Build configuration parsing** from RV lines
4. **Timestamp and checksum extraction** from D lines
5. **Type system information** from variable and function declarations
6. **Security flag analysis** for compliance reporting
7. **Function call graph generation** for impact analysis

### Potential Use Cases:
- **Security scanning**: Build flag analysis for vulnerabilities
- **Impact analysis**: Function call graphs for change impact
- **Compliance auditing**: Type safety and build configuration verification
- **Build reproducibility**: Complete build environment documentation
- **Dependency analysis**: Cross-package relationship mapping

## Conclusion

Heimdall's Ada extractor provides comprehensive SBOM generation by combining binary analysis with ALI file parsing. This approach captures both the compiled binary information and the rich Ada-specific metadata available in ALI files, resulting in detailed, accurate SBOMs that support security analysis, compliance auditing, and dependency management.

The integration of ALI file parsing significantly enhances the SBOM quality by providing Ada-specific information that would not be available from binary analysis alone, including package dependencies, source file mappings, build configuration, and type system details. 