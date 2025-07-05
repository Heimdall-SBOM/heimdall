# Heimdall TODO

This document tracks all missing implementation tasks and planned features for the Heimdall SBOM generator.

## Current Status (Updated 2025-01-05)

- ✅ **macOS (ARM64/x86_64)**: LLD plugin working, Mach-O support implemented, cross-platform build working
- ✅ **Linux (x86_64/ARM64)**: LLD plugin working, Gold plugin framework exists, cross-platform build working
- ❌ **Windows**: No support implemented
- ✅ **Core Library**: Basic functionality implemented with cross-platform support
- ✅ **Test Suite**: 24/36 tests passing (67% success rate) with proper platform-aware testing
- ✅ **Documentation**: Markdown-based documentation
- ✅ **Package Manager Integration (Linux/Mac)**: RPM, DEB, Pacman, Conan, vcpkg, Spack implemented
- ⚠️ **Archive File Support**: Partially working, some test failures on macOS
- ⚠️ **DWARF Support**: Temporarily disabled due to LLVM library linking issues

## Recent Progress (2025-01-05)

- ✅ **Cross-Platform Build System**: Successfully implemented platform detection and conditional compilation
- ✅ **Platform-Aware Testing**: Updated tests to properly skip Linux-specific features on macOS
- ✅ **Enhanced LLVM Detection**: Improved LLVM detection to prefer Homebrew LLVM for DWARF support
- ✅ **Platform Macros**: Added `HEIMDALL_PLATFORM_LINUX`, `HEIMDALL_PLATFORM_MACOS`, `HEIMDALL_PLATFORM_WINDOWS`
- ⚠️ **DWARF Support**: Temporarily disabled due to complex LLVM library dependencies (needs fixing)

## Critical Missing Components

### 🚨 **Windows Support (Complete)**
- **No Windows linker plugins**: Only LLD and Gold plugins exist
- **No PE file format implementation**: PE symbol/section extraction is stubbed out
- **No Windows build system**: CMake doesn't handle Windows-specific configurations
- **No Windows linker integration**: No support for MSVC link.exe or other Windows linkers

### 🚨 **DWARF Debug Info Support (Partially Working)**
- **LLVM Library Linking Issues**: Complex dependencies prevent DWARF support from working
- **Need to fix**: Individual LLVM library linking or use monolithic library approach
- **Current Status**: Temporarily disabled to get basic build working

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

#### **DWARF Support Fix - High Priority**

- [ ] **Fix LLVM Library Linking**
  ```cmake
  # CMakeLists.txt - Fix DWARF linking
  if(LLVM_DWARF_FOUND)
      # Option 1: Use monolithic LLVM library
      target_link_libraries(heimdall-core PRIVATE ${LLVM_HOMEBREW_PREFIX}/lib/libLLVM.dylib)
      
      # Option 2: Fix individual library dependencies
      target_link_libraries(heimdall-core PRIVATE 
          ${LLVM_HOMEBREW_PREFIX}/lib/libLLVMCore.a
          ${LLVM_HOMEBREW_PREFIX}/lib/libLLVMSupport.a
          # ... add all required dependencies
      )
  endif()
  ```
  - [ ] Research proper LLVM library linking approach
  - [ ] Fix missing dependencies (zstd, zlib, etc.)
  - [ ] Re-enable DWARF extractor in build
  - [ ] Test DWARF extraction on both platforms

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

### **Test Results (2025-01-05)**
- ✅ **24 tests passing** (67% success rate)
- ✅ **10 tests properly skipped** (Linux-specific features on macOS)
- ❌ **2 tests failing** (Archive support - needs fixing)

### **Test Categories**
- ✅ **Utils Tests**: 5/5 passing
- ✅ **Component Info Tests**: 7/7 passing  
- ✅ **SBOM Generator Tests**: 4/4 passing
- ✅ **Metadata Extractor Tests**: 2/2 passing
- ✅ **Linux Support Tests**: 7/7 skipped (expected on macOS)
- ✅ **Package Manager Tests**: 6/6 passing/skipped appropriately
- ❌ **Archive Support Tests**: 2/3 failing (needs fixing)

## Resolved Issues

- ✅ **Cross-Platform Build**: Successfully implemented platform detection and conditional compilation
- ✅ **Platform-Aware Testing**: Tests now properly skip unsupported features
- ✅ **LLVM Detection**: Improved to prefer newer Homebrew LLVM
- ✅ **Test Compilation**: Fixed macOS SDK path issues
- ⚠️ **DWARF Support**: Temporarily disabled due to linking issues (needs fixing)
- ⚠️ **Archive Support**: Partially working but has test failures (needs debugging)

## Next Immediate Actions

1. **Fix DWARF Support** - Resolve LLVM library linking issues
2. **Fix Archive Support** - Debug archive extraction test failures
3. **Add Windows Support** - Implement PE format and MSVC plugin
4. **Enhance Testing** - Add more comprehensive cross-platform tests

---

## Testing Tasks

### **Windows Test Suite**
- [ ] **Create `tests/test_windows_support.cpp`**
  - [ ] `TEST_F(WindowsSupportTest, PEFileDetection)`
  - [ ] `TEST_F(WindowsSupportTest, PESymbolExtraction)`
  - [ ] `TEST_F(WindowsSupportTest, MSVCPluginIntegration)`
  - [ ] `TEST_F(WindowsSupportTest, PDBExtraction)`

### **Linux Test Suite**
- [x] **Create `tests/test_linux_support.cpp`**
  - [x] `TEST_F(LinuxSupportTest, ELFSymbolExtraction)`
  - [x] `TEST_F(LinuxSupportTest, ELFDependencyExtraction)`
  - [x] `TEST_F(LinuxSupportTest, GoldPluginIntegration)`
  - [x] **DWARF tests present**
    - [x] `TEST_F(LinuxSupportTest, DWARFSourceFileExtraction)`
    - [x] `TEST_F(LinuxSupportTest, DWARFCompileUnitExtraction)`

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

*Last updated: January 2025*
*Version: 1.0.0*

## Progress Update (2024-07-19)

- ✅ **DWARF parsing**: Fully implemented and working. LLVM DWARF parser integration is complete with proper buffer lifetime management. A heuristic fallback parser is maintained as a safety measure (see docs/dwarf-heuristic-rationale.md).

## Resolved Issues

- ✅ **LLVM DWARF parser segfaults on valid ELF files** - RESOLVED
  - **Root Cause:** Incorrect buffer lifetime management in `DWARFExtractor::createDWARFContext()`
  - **Solution:** Refactored to store `MemoryBuffer`, `ObjectFile`, and `DWARFContext` as class members
  - **Status:** All DWARF tests now pass without segfaults
  - **Documentation:** See `docs/dwarf-heuristic-rationale.md` for detailed analysis 