# Heimdall TODO

This document tracks all missing implementation tasks and planned features for the Heimdall SBOM generator.

## Current Status

- ✅ **macOS (ARM64/x86_64)**: LLD plugin working, Mach-O support implemented
- ✅ **Linux (x86_64/ARM64)**: LLD plugin working, Gold plugin framework exists
- ❌ **Windows**: No support implemented
- ✅ **Core Library**: Basic functionality implemented
- ✅ **Test Suite**: 20 unit tests passing with real shared library testing
- ✅ **Documentation**: Markdown-based documentation

## Critical Missing Components

### 🚨 **Windows Support (Complete)**
- **No Windows linker plugins**: Only LLD and Gold plugins exist
- **No PE file format implementation**: PE symbol/section extraction is stubbed out
- **No Windows build system**: CMake doesn't handle Windows-specific configurations
- **No Windows linker integration**: No support for MSVC link.exe or other Windows linkers

### 🚨 **Linux ELF Implementation (Partial)**
- **ELF symbol extraction**: Currently stubbed out with "not fully implemented" messages
- **ELF section extraction**: Not implemented
- **ELF build ID extraction**: Not implemented
- **ELF dependency extraction**: Missing dynamic library dependency parsing

### 🚨 **Debug Information Extraction (Missing)**
- **DWARF parsing**: No implementation for extracting source files and debug info
- **PDB support**: No Windows debug info support
- **Source file extraction**: Not implemented

### 🚨 **Package Manager Integration (Stubbed)**
- **Conan metadata**: Basic stubs exist but no real implementation
- **vcpkg metadata**: Not implemented
- **System package detection**: Limited implementation

## Partially Implemented Components

### 🟡 **Archive File Support**
- **Archive member extraction**: Not implemented
- **Archive symbol extraction**: Not implemented

### 🟡 **Version Detection**
- **File content version extraction**: Basic regex implementation
- **Symbol-based version extraction**: Basic implementation
- **Path-based version extraction**: Basic implementation

### 🟡 **License Detection**
- **File content license detection**: Basic implementation
- **Symbol-based license detection**: Basic implementation

---

## Detailed Task List

### **Phase 1: Critical Foundation (2-3 months)**

#### **Windows Support - High Priority**

- [ ] **Create Windows Linker Plugin**
  - [ ] Create `src/msvc/MSVCPlugin.cpp`
  - [ ] Create `src/msvc/MSVCAdapter.cpp`
  - [ ] Create `src/msvc/MSVCAdapter.hpp`
  - [ ] Implement MSVC link.exe plugin interface
  - [ ] Support for Windows DLL injection during linking
  - [ ] Integration with Visual Studio build system

- [ ] **Implement PE File Format Support**
  ```cpp
  // src/common/MetadataExtractor.cpp - PE functions
  bool extractPESymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
  bool extractPESections(const std::string& filePath, std::vector<SectionInfo>& sections);
  bool extractPEVersion(const std::string& filePath, std::string& version);
  bool extractPECompanyName(const std::string& filePath, std::string& company);
  ```
  - [ ] Use Windows API or libraries like `libpe` for PE parsing
  - [ ] Extract symbols from PE symbol tables
  - [ ] Parse PE sections and headers
  - [ ] Extract version information from PE resources

- [ ] **Windows Build System Integration**
  ```cmake
  # CMakeLists.txt - Windows support
  if(PLATFORM_WINDOWS)
      find_package(VisualStudio REQUIRED)
      find_package(WindowsSDK REQUIRED)
      # Add Windows-specific build targets
  endif()
  ```

#### **Linux ELF Support - High Priority**

- [ ] **Complete ELF Symbol Extraction**
  ```cpp
  // src/common/MetadataExtractor.cpp
  bool extractELFSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) {
      #ifdef __linux__
      // Use libelf or similar library
      // Parse ELF symbol tables
      // Extract symbol names, addresses, sizes
      #endif
  }
  ```

- [ ] **ELF Section Extraction**
  ```cpp
  bool extractELFSections(const std::string& filePath, std::vector<SectionInfo>& sections) {
      #ifdef __linux__
      // Parse ELF section headers
      // Extract section names, addresses, sizes, flags
      #endif
  }
  ```

- [ ] **ELF Dependency Extraction**
  ```cpp
  std::vector<std::string> extractELFDependencies(const std::string& filePath) {
      #ifdef __linux__
      // Parse ELF dynamic section
      // Extract DT_NEEDED entries
      // Resolve library paths
      #endif
  }
  ```

#### **Cross-Platform Foundation - High Priority**

- [ ] **Archive File Support**
  ```cpp
  // src/common/MetadataExtractor.cpp
  bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members);
  bool extractArchiveSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
  ```

- [ ] **Enhanced Version Detection**
  ```cpp
  // src/common/VersionDetector.hpp
  class VersionDetector {
      std::string detectFromFileContent(const std::string& filePath);
      std::string detectFromSymbols(const std::vector<SymbolInfo>& symbols);
      std::string detectFromPath(const std::string& filePath);
      std::string detectFromMetadata(const std::string& filePath);
  };
  ```

- [ ] **Enhanced License Detection**
  ```cpp
  // src/common/LicenseDetector.hpp
  class LicenseDetector {
      std::string detectFromFileContent(const std::string& filePath);
      std::string detectFromSymbols(const std::vector<SymbolInfo>& symbols);
      std::string detectFromPath(const std::string& filePath);
      std::string detectFromPackageManager(const std::string& filePath);
  };
  ```

### **Phase 2: Enhanced Features (3-4 months)**

#### **Windows Support - Medium Priority**

- [ ] **Windows Debug Information (PDB)**
  ```cpp
  // src/common/DebugInfoExtractor.hpp
  class PDBExtractor {
      bool extractSourceFiles(const std::string& pdbPath, std::vector<std::string>& sources);
      bool extractCompileUnits(const std::string& pdbPath, std::vector<std::string>& units);
  };
  ```

- [ ] **Windows Package Manager Support**
  ```cpp
  // src/common/PackageManagerDetector.cpp
  bool detectVcpkgMetadata(ComponentInfo& component);
  bool detectNuGetMetadata(ComponentInfo& component);
  bool detectChocolateyMetadata(ComponentInfo& component);
  ```

- [ ] **Windows-specific Utils**
  ```cpp
  // src/common/Utils.cpp - Windows paths
  std::vector<std::string> getWindowsLibrarySearchPaths();
  std::string findWindowsLibrary(const std::string& libraryName);
  ```

#### **Linux Support - Medium Priority**

- [ ] **ELF Build ID Extraction**
  ```cpp
  bool extractELFBuildId(const std::string& filePath, std::string& buildId) {
      #ifdef __linux__
      // Parse ELF note sections
      // Extract build ID from .note.gnu.build-id
      #endif
  }
  ```

- [ ] **Linux Package Manager Integration**
  ```cpp
  // src/common/PackageManagerDetector.cpp
  bool detectRpmMetadata(ComponentInfo& component);
  bool detectDebMetadata(ComponentInfo& component);
  bool detectPacmanMetadata(ComponentInfo& component);
  ```

- [ ] **Linux-specific Debug Info**
  ```cpp
  // src/common/DebugInfoExtractor.hpp
  class DWARFExtractor {
      bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sources);
      bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& units);
  };
  ```

#### **Cross-Platform Features - Medium Priority**

- [ ] **Package Manager Integration**
  ```cpp
  // src/common/PackageManagerDetector.cpp
  bool detectConanMetadata(ComponentInfo& component);
  bool detectVcpkgMetadata(ComponentInfo& component);
  bool detectSystemMetadata(ComponentInfo& component);
  ```

- [ ] **Enhanced SBOM Formats**
  ```cpp
  // src/common/SBOMGenerator.cpp
  bool generateSWID(const std::string& outputPath);
  bool generateSPDXTag(const std::string& outputPath);
  ```

### **Phase 3: Optimization & Polish (2-3 months)**

#### **Performance & Quality - Low Priority**

- [ ] **Performance Optimizations**
  ```cpp
  // src/common/ParallelProcessor.hpp
  class ParallelMetadataExtractor {
      void processFilesInParallel(const std::vector<std::string>& files);
  };
  ```

- [ ] **Memory Usage Optimization**
  - [ ] Implement streaming for large files
  - [ ] Add memory usage monitoring
  - [ ] Optimize symbol table parsing

- [ ] **Error Handling & Recovery**
  - [ ] Graceful handling of corrupted files
  - [ ] Better error messages and diagnostics
  - [ ] Recovery mechanisms for partial failures

#### **Advanced Features - Low Priority**

- [ ] **Advanced Metadata Extraction**
  - [ ] Compiler version detection
  - [ ] Build timestamp extraction
  - [ ] Architecture detection
  - [ ] Optimization level detection

- [ ] **Security Features**
  - [ ] Digital signature verification
  - [ ] Checksum validation
  - [ ] Integrity checking

---

## Testing Tasks

### **Windows Test Suite**
- [ ] **Create `tests/test_windows_support.cpp`**
  - [ ] `TEST_F(WindowsSupportTest, PEFileDetection)`
  - [ ] `TEST_F(WindowsSupportTest, PESymbolExtraction)`
  - [ ] `TEST_F(WindowsSupportTest, MSVCPluginIntegration)`
  - [ ] `TEST_F(WindowsSupportTest, PDBExtraction)`

### **Linux Test Suite**
- [ ] **Create `tests/test_linux_support.cpp`**
  - [ ] `TEST_F(LinuxSupportTest, ELFSymbolExtraction)`
  - [ ] `TEST_F(LinuxSupportTest, ELFDependencyExtraction)`
  - [ ] `TEST_F(LinuxSupportTest, GoldPluginIntegration)`
  - [ ] `TEST_F(LinuxSupportTest, DWARFExtraction)`

### **Cross-Platform Test Suite**
- [ ] **Create `tests/test_cross_platform.cpp`**
  - [ ] `TEST_F(CrossPlatformTest, ArchiveFileSupport)`
  - [ ] `TEST_F(CrossPlatformTest, VersionDetection)`
  - [ ] `TEST_F(CrossPlatformTest, LicenseDetection)`
  - [ ] `TEST_F(CrossPlatformTest, PackageManagerDetection)`

### **Integration Tests**
- [ ] **Create `tests/test_integration.cpp`**
  - [ ] End-to-end SBOM generation tests
  - [ ] Plugin integration tests
  - [ ] Build system integration tests

---

## Documentation Tasks

### **Installation & Setup Guides**
- [ ] **Create `docs/windows-installation.md`**
  - [ ] Visual Studio requirements
  - [ ] Windows SDK setup
  - [ ] MSVC linker integration
  - [ ] Troubleshooting guide

- [ ] **Create `docs/linux-elf-support.md`**
  - [ ] ELF file format details
  - [ ] Symbol extraction methods
  - [ ] Dependency resolution
  - [ ] Performance considerations

- [ ] **Update `docs/gold-installation.md`**
  - [ ] Add more distribution support
  - [ ] Include troubleshooting section
  - [ ] Add performance benchmarks

### **API Documentation**
- [ ] **Create `docs/api-reference.md`**
  - [ ] Complete API documentation
  - [ ] Usage examples
  - [ ] Platform-specific notes
  - [ ] Performance guidelines

- [ ] **Create `docs/plugin-development.md`**
  - [ ] Plugin development guide
  - [ ] Custom linker integration
  - [ ] Plugin API reference

### **User Guides**
- [ ] **Create `docs/usage-examples.md`**
  - [ ] Simple C/C++ project examples
  - [ ] CMake integration examples
  - [ ] Makefile integration examples
  - [ ] CI/CD integration examples

- [ ] **Create `docs/troubleshooting.md`**
  - [ ] Common issues and solutions
  - [ ] Platform-specific problems
  - [ ] Performance optimization tips

---

## Build System Tasks

### **CMake Enhancements**
- [ ] **Windows Build Support**
  ```cmake
  if(PLATFORM_WINDOWS)
      find_package(VisualStudio REQUIRED)
      find_package(WindowsSDK REQUIRED)
      find_package(OpenSSL REQUIRED)
      # Add Windows-specific targets and configurations
  endif()
  ```

- [ ] **Cross-Platform Build Scripts**
  - [ ] Update `build.sh` for Windows support
  - [ ] Create `build.bat` for Windows
  - [ ] Add CI/CD pipeline configurations

- [ ] **Dependency Management**
  - [ ] Add vcpkg integration
  - [ ] Add Conan integration
  - [ ] Add package manager detection

### **CI/CD Integration**
- [ ] **GitHub Actions**
  - [ ] Windows build matrix
  - [ ] Linux build matrix
  - [ ] macOS build matrix
  - [ ] Cross-platform testing

- [ ] **Docker Support**
  - [ ] Development containers
  - [ ] Build containers
  - [ ] Testing containers

---

## Performance & Quality Tasks

### **Code Quality**
- [ ] **Static Analysis**
  - [ ] Add clang-tidy configuration
  - [ ] Add cppcheck configuration
  - [ ] Add SonarQube integration

- [ ] **Code Coverage**
  - [ ] Add coverage reporting
  - [ ] Set coverage targets
  - [ ] Add coverage badges

### **Performance Monitoring**
- [ ] **Benchmarks**
  - [ ] Create benchmark suite
  - [ ] Add performance regression tests
  - [ ] Add memory usage monitoring

- [ ] **Profiling**
  - [ ] Add profiling tools integration
  - [ ] Create performance profiles
  - [ ] Add performance documentation

---

## Future Enhancements (Post v1.0)

### **Advanced SBOM Features**
- [ ] **Vulnerability Scanning Integration**
  - [ ] CVE database integration
  - [ ] Vulnerability reporting
  - [ ] Security advisories

- [ ] **License Compliance**
  - [ ] License compatibility checking
  - [ ] License obligation tracking
  - [ ] Compliance reporting

### **Integration Features**
- [ ] **IDE Integration**
  - [ ] Visual Studio extension
  - [ ] VS Code extension
  - [ ] CLion plugin

- [ ] **Build System Integration**
  - [ ] Bazel integration
  - [ ] Gradle integration
  - [ ] Maven integration

### **Cloud & Enterprise Features**
- [ ] **Cloud Integration**
  - [ ] AWS CodeBuild integration
  - [ ] Azure DevOps integration
  - [ ] Google Cloud Build integration

- [ ] **Enterprise Features**
  - [ ] Centralized SBOM management
  - [ ] Policy enforcement
  - [ ] Audit trail

---

## Notes

### **Priority Guidelines**
- **Phase 1**: Critical for basic cross-platform functionality
- **Phase 2**: Important for production readiness
- **Phase 3**: Nice-to-have features and optimizations

### **Platform Support Matrix**
| Feature | macOS | Linux | Windows |
|---------|-------|-------|---------|
| LLD Plugin | ✅ | ✅ | ❌ |
| Gold Plugin | ❌ | ✅ | ❌ |
| MSVC Plugin | ❌ | ❌ | ❌ |
| ELF Support | ❌ | 🟡 | ❌ |
| Mach-O Support | ✅ | ❌ | ❌ |
| PE Support | ❌ | ❌ | ❌ |
| Debug Info | ❌ | ❌ | ❌ |

### **Legend**
- ✅ Implemented
- 🟡 Partially implemented
- ❌ Not implemented

---

*Last updated: July 2024*
*Version: 1.0.0* 