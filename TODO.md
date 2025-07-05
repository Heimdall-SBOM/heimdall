# Heimdall TODO

This document tracks all missing implementation tasks and planned features for the Heimdall SBOM generator.

## Current Status (Updated 2025-01-05)

- ✅ **macOS (ARM64/x86_64)**: LLD plugin working, Mach-O support implemented, cross-platform build working
- ✅ **Linux (x86_64/ARM64)**: LLD plugin working, Gold plugin framework exists, cross-platform build working
- ❌ **Windows**: No support implemented
- ✅ **Core Library**: Basic functionality implemented with cross-platform support
- ✅ **Test Suite**: 117/120 tests passing (97.5% success rate) with comprehensive coverage
- ✅ **Documentation**: Markdown-based documentation
- ✅ **Package Manager Integration (Linux/Mac)**: RPM, DEB, Pacman, Conan, vcpkg, Spack implemented
- ⚠️ **Archive File Support**: Partially working, some test failures on macOS
- ✅ **DWARF Support**: Fully working with LLVM 18.1, segfault issue resolved using global variables

## Recent Progress (2025-01-05)

- ✅ **LLVM DWARF Segfault Fix**: Resolved segfault issue by using global variables for LLVM objects
- ✅ **DWARF Functionality**: All DWARF extraction methods now working correctly
- ✅ **Test Coverage**: Improved from 72/83 to 117/120 tests passing (97.5% success rate)
- ✅ **Cross-Platform Build System**: Successfully implemented platform detection and conditional compilation
- ✅ **Platform-Aware Testing**: Updated tests to properly skip Linux-specific features on macOS
- ✅ **Enhanced LLVM Detection**: Improved LLVM detection to prefer Homebrew LLVM for DWARF support
- ✅ **Platform Macros**: Added `HEIMDALL_PLATFORM_LINUX`, `HEIMDALL_PLATFORM_MACOS`, `HEIMDALL_PLATFORM_WINDOWS`
- ✅ **DWARF Support**: Fully operational with LLVM 18.1, all extraction methods working

## Critical Missing Components

### 🚨 **Windows Support (Complete)**
- **No Windows linker plugins**: Only LLD and Gold plugins exist
- **No PE file format implementation**: PE symbol/section extraction is stubbed out
- **No Windows build system**: CMake doesn't handle Windows-specific configurations
- **No Windows linker integration**: No support for MSVC link.exe or other Windows linkers

### ✅ **DWARF Debug Info Support (Fully Working)**
- **LLVM Library Linking Issues**: ✅ RESOLVED - Using global variables for LLVM objects
- **Segfault Issue**: ✅ RESOLVED - Global variables prevent premature destruction
- **Current Status**: ✅ FULLY OPERATIONAL - All DWARF extraction methods working correctly

### 🚨 **Archive Support (Partially Working)**
- **Test Failures**: Archive extraction tests failing on macOS
- **Need Investigation**: Archive member extraction not working properly
- **Current Status**: Basic functionality exists but needs debugging

## Partially Implemented Components

### 🟡 **Enhanced Version Detection**
- **File content version extraction**: Basic regex implementation
- **Symbol-based version extraction**: Basic implementation
- **Path-based version extraction**: Basic implementation

### 🟡 **Enhanced License Detection**
- **File content license detection**: Basic implementation
- **Symbol-based license detection**: Basic implementation

---

## Detailed Task List

### **Phase 1: Critical Foundation (2-3 months)**

#### **DWARF Support Fix - COMPLETED ✅**

- [x] **Fix LLVM Library Linking**
  ```cmake
  # CMakeLists.txt - Fixed DWARF linking
  if(LLVM_DWARF_FOUND)
      # Using LLVM 18.1 with proper library linking
      target_link_libraries(heimdall-core PRIVATE ${LLVM_LIBRARIES})
  endif()
  ```
  - [x] Research proper LLVM library linking approach
  - [x] Fix missing dependencies (zstd, zlib, etc.)
  - [x] Re-enable DWARF extractor in build
  - [x] Test DWARF extraction on both platforms

- [x] **Fix LLVM Object Lifetime Management**
  ```cpp
  // src/common/DWARFExtractor.hpp - Global variables for LLVM objects
  extern std::unique_ptr<llvm::MemoryBuffer> g_buffer;
  extern std::unique_ptr<llvm::object::ObjectFile> g_objectFile;
  extern std::unique_ptr<llvm::DWARFContext> g_context;
  ```
  - [x] Identify segfault root cause: premature destruction of LLVM objects
  - [x] Implement global variable solution to prevent premature destruction
  - [x] Update createDWARFContext to use global variables
  - [x] Test all DWARF extraction methods successfully

- [ ] **Archive Support Fix**
  ```cpp
  // src/common/MetadataExtractor.cpp - Fix archive extraction
  bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members);
  bool extractArchiveSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
  ```
  - [ ] Debug archive member extraction on macOS
  - [ ] Fix test data generation for archives
  - [ ] Ensure cross-platform archive support

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
  // src/common/MetadataExtractor.cpp - PE functions (currently stubbed)
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
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
      find_package(VisualStudio REQUIRED)
      find_package(WindowsSDK REQUIRED)
      # Add Windows-specific build targets
      target_compile_definitions(heimdall-core PRIVATE HEIMDALL_PLATFORM_WINDOWS=1)
  endif()
  ```

#### **Cross-Platform Foundation - Medium Priority**

- [x] **Platform Detection & Macros**
  - [x] `HEIMDALL_PLATFORM_LINUX`, `HEIMDALL_PLATFORM_MACOS`, `HEIMDALL_PLATFORM_WINDOWS` implemented
  - [x] Platform-specific compile definitions added
  - [x] Conditional compilation for platform-specific features

- [x] **Platform-Aware Testing**
  - [x] Linux-specific tests use `GTEST_SKIP()` on non-Linux platforms
  - [x] File format detection tests are platform-aware
  - [x] Proper test skipping for unsupported features

- [x] **Enhanced LLVM Detection**
  - [x] Prefer Homebrew LLVM for DWARF support
  - [x] Proper include directory handling
  - [x] Platform-specific LLVM configuration

- [x] **Package Manager Integration (Linux/Mac)**
  - [x] `detectRpmMetadata` implemented
  - [x] `detectDebMetadata` implemented
  - [x] `detectPacmanMetadata` implemented
  - [x] `detectConanMetadata` implemented
  - [x] `detectVcpkgMetadata` implemented
  - [x] `detectSpackMetadata` implemented
  - [ ] Windows package managers (not implemented)

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

#### **Windows Support - High Priority**

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

#### **Enhanced Cross-Platform Support - Medium Priority**

- [ ] **Improved Test Coverage**
  - [ ] Add more comprehensive cross-platform tests
  - [ ] Test archive support on both platforms
  - [ ] Test DWARF support when fixed
  - [ ] Add Windows-specific tests when implemented

- [ ] **Build System Enhancements**
  - [ ] Add CI/CD for multiple platforms
  - [ ] Automated testing on Linux, macOS, Windows
  - [ ] Package distribution for all platforms

### **Phase 3: Advanced Features (2-3 months)**

#### **Enhanced SBOM Generation**

- [ ] **SPDX 2.3 Support**
  - [ ] Implement latest SPDX specification
  - [ ] Support for SPDX-Lite format
  - [ ] Enhanced relationship tracking

- [ ] **CycloneDX 1.5 Support**
  - [ ] Implement latest CycloneDX specification
  - [ ] Support for vulnerability reporting
  - [ ] Enhanced metadata fields

#### **Advanced Analysis**

- [ ] **Vulnerability Scanning Integration**
  ```cpp
  // src/security/VulnerabilityScanner.hpp
  class VulnerabilityScanner {
      std::vector<Vulnerability> scanComponent(const ComponentInfo& component);
      bool hasKnownVulnerabilities(const std::string& version);
  };
  ```

- [ ] **License Compliance Checking**
  ```cpp
  // src/compliance/LicenseChecker.hpp
  class LicenseChecker {
      bool isCompatible(const std::string& license1, const std::string& license2);
      std::vector<std::string> getIncompatibleLicenses(const std::string& license);
  };
  ```

## Current Test Status

### **Test Results (2025-07-05)**
- ✅ **108 tests passing** (92.3% success rate)
- ✅ **9 tests properly skipped** (missing test data files)
- ❌ **2 tests failing** (DWARF functionality - expected due to LLVM disablement)

### **Test Coverage Summary**
- **Overall Line Coverage:** 58.2% (6,870 of 11,815 lines)
- **Overall Function Coverage:** 60.1% (4,003 of 6,664 functions)
- **Branch Coverage:** No data available

### **Coverage by Component**
| Component | Line Coverage | Function Coverage | Status |
|-----------|---------------|-------------------|---------|
| **ComponentInfo** | 88.0% | 92.0% | ✅ **Good** |
| **SBOMGenerator** | 85.8% | 100% | ✅ **Excellent** |
| **DWARFExtractor** | 95.2% | 100% | ✅ **Excellent** |
| **Utils** | 82.1% | 97.3% | ✅ **Good** |
| **MetadataExtractor** | 70.2% | 80.0% | ⚠️ **Moderate** |
| **PluginInterface** | 85.0% | 90.0% | ✅ **Good** |

### **Test Categories**
- ✅ **Utils Tests**: 24/24 passing (including extended tests)
- ✅ **Component Info Tests**: 7/7 passing  
- ✅ **SBOM Generator Tests**: 4/4 passing
- ✅ **Metadata Extractor Tests**: 20/20 passing (including extended tests)
- ⚠️ **Linux Support Tests**: 5/7 passing, 2/7 failing (DWARF tests)
- ✅ **Package Manager Tests**: 6/6 skipped (missing test data)
- ✅ **DWARF Extractor Tests**: 10/10 passing (heuristic mode)
- ✅ **PluginInterface Tests**: 36/36 passing ✅ **NEW**
- ✅ **Archive Support Tests**: 3/3 skipped (missing test data)

## Resolved Issues

- ✅ **Cross-Platform Build**: Successfully implemented platform detection and conditional compilation
- ✅ **Platform-Aware Testing**: Tests now properly skip unsupported features
- ✅ **LLVM Detection**: Improved to prefer newer Homebrew LLVM
- ✅ **Test Compilation**: Fixed macOS SDK path issues
- ⚠️ **DWARF Support**: Temporarily disabled due to linking issues (needs fixing)
- ⚠️ **Archive Support**: Partially working but has test failures (needs debugging)

## Next Immediate Actions

### **Testing Priorities (Immediate)**
1. ✅ **Add PluginInterface Tests** - Create comprehensive test suite for 0% coverage component ✅ **COMPLETED**
2. **Fix DWARF Tests** - Resolve the 2 failing DWARF-related tests
3. **Add Missing Test Data** - Create test data files for skipped package manager and archive tests
4. **Improve MetadataExtractor Coverage** - Add tests to reach 80% coverage target

### **Development Priorities**
1. **Fix DWARF Support** - Resolve LLVM library linking issues (when ready to re-enable)
2. **Add Windows Support** - Implement PE format and MSVC plugin
3. **Enhance Testing Infrastructure** - Add coverage targets and CI integration

---

## Testing Tasks

### **Priority 1: Critical Coverage Gaps**

#### **PluginInterface Test Suite - HIGH PRIORITY (0% coverage)**
- [x] **Create `tests/test_plugin_interface.cpp`** ✅ **COMPLETED**
  - [x] `TEST_F(PluginInterfaceTest, Constructor)`
  - [x] `TEST_F(PluginInterfaceTest, Destructor)`
  - [x] `TEST_F(PluginInterfaceTest, Initialize)`
