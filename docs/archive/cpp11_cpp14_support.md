# C++11/14 Support Implementation Guide

## Overview

This document outlines the architectural changes and considerations needed to add C++11 and C++14 support to the Heimdall SBOM Generator while maintaining DWARF functionality and addressing current compatibility issues.

## Current State Analysis

### Blocking Dependencies

1. **LLVM 19+ Requirements**
   - LLVM 19 requires C++17 minimum
   - Blocks C++11/14 builds completely
   - DWARF functionality heavily depends on LLVM

2. **GoogleTest Compatibility**
   - GoogleTest 1.13+ requires C++14 minimum
   - Current version: 1.14.0 (requires C++14+)

3. **Modern C++ Features Used**
   - `std::filesystem` (C++17)
   - `std::optional` (C++17)
   - `std::variant` (C++17)
   - Structured bindings (C++17)
   - `std::string_view` (C++17)

## Architectural Approach

### Multi-Version Build Strategy

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   C++11 Build   │    │   C++14 Build   │    │   C++17+ Build  │
│                 │    │                 │    │                 │
│ • LLVM 7-10     │    │ • LLVM 7-10     │    │ • LLVM 19+      │
│ • Limited DWARF │    │ • Basic DWARF   │    │ • Full DWARF    │
│ • Core features │    │ • Core features │    │ • All features  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Feature Matrix

| Feature | C++11 | C++14 | C++17+ | Notes |
|---------|-------|-------|--------|-------|
| Core SBOM Generation | ✅ | ✅ | ✅ | Basic functionality |
| SPDX Support | ✅ | ✅ | ✅ | Full support |
| CycloneDX Support | ✅ | ✅ | ✅ | Full support |
| DWARF Debug Info | ❌ | ⚠️ | ✅ | Limited in older versions |
| Advanced Metadata | ❌ | ⚠️ | ✅ | Requires modern C++ |
| Plugin System | ✅ | ✅ | ✅ | Core functionality |
| Test Coverage | ❌ | ⚠️ | ✅ | GoogleTest compatibility |

## Implementation Strategy

### 1. Conditional Compilation Architecture

```cpp
// Feature detection macros
#if __cplusplus >= 201703L
    #define HEIMDALL_CPP17_AVAILABLE 1
    #define HEIMDALL_FULL_DWARF 1
#elif __cplusplus >= 201402L
    #define HEIMDALL_CPP14_AVAILABLE 1
    #define HEIMDALL_BASIC_DWARF 1
#elif __cplusplus >= 201103L
    #define HEIMDALL_CPP11_AVAILABLE 1
    #define HEIMDALL_NO_DWARF 1
#else
    #error "C++11 or later required"
#endif
```

### 2. Backward Compatibility Layer

```cpp
// compatibility.hpp
namespace heimdall {
namespace compat {

#if HEIMDALL_CPP17_AVAILABLE
    using std::optional;
    using std::variant;
    using std::string_view;
    using std::filesystem::path;
#else
    // Custom implementations for older standards
    template<typename T>
    class optional {
        // C++11/14 compatible implementation
    };
    
    class string_view {
        // C++11/14 compatible implementation
    };
    
    class path {
        // C++11/14 compatible implementation
    };
#endif

} // namespace compat
} // namespace heimdall
```

### 3. LLVM Version Detection and Fallback

```cpp
// llvm_detection.hpp
namespace heimdall {
namespace llvm {

enum class LLVMVersion {
    UNKNOWN = 0,
    LLVM_7_10 = 1,    // C++11/14 compatible
    LLVM_11_18 = 2,   // C++14+ compatible
    LLVM_19_PLUS = 3  // C++17+ required
};

class LLVMDetector {
public:
    static LLVMVersion detectVersion();
    static bool supportsDWARF(LLVMVersion version);
    static bool supportsCXXStandard(LLVMVersion version, int standard);
};

} // namespace llvm
} // namespace heimdall
```

## CMake Configuration Changes

### Multi-Standard Build Support

```cmake
# CMakeLists.txt modifications
option(SUPPORT_CPP11 "Support C++11 builds" OFF)
option(SUPPORT_CPP14 "Support C++14 builds" OFF)
option(SUPPORT_CPP17 "Support C++17+ builds" ON)

if(SUPPORT_CPP11)
    set(CMAKE_CXX_STANDARD 11)
    set(LLVM_MIN_VERSION "7.0.0")
    set(LLVM_MAX_VERSION "10.0.0")
    set(ENABLE_DWARF OFF)
elseif(SUPPORT_CPP14)
    set(CMAKE_CXX_STANDARD 14)
    set(LLVM_MIN_VERSION "7.0.0")
    set(LLVM_MAX_VERSION "10.0.0")
    set(ENABLE_DWARF ON)
else()
    set(CMAKE_CXX_STANDARD 17)
    set(LLVM_MIN_VERSION "19.0.0")
    set(ENABLE_DWARF ON)
endif()
```

### Build Script Updates

```bash
# build.sh additions
--cpp11              Build with C++11 (limited features)
--cpp14              Build with C++14 (basic features)
--cpp17              Build with C++17+ (full features)
```

## Limitations and Trade-offs

### C++11 Limitations

1. **No DWARF Support**
   - LLVM 7-10 DWARF APIs are deprecated
   - Limited debug info extraction
   - No source file mapping

2. **Reduced Functionality**
   - No `std::filesystem` (use boost::filesystem)
   - No `std::optional` (custom implementation)
   - No structured bindings
   - Manual memory management required

3. **Testing Limitations**
   - GoogleTest 1.12 or earlier required
   - Reduced test coverage
   - Manual test framework setup

### C++14 Limitations

1. **Limited DWARF Support**
   - Basic debug info extraction
   - No advanced DWARF features
   - Limited source mapping

2. **Missing Modern Features**
   - No `std::filesystem`
   - No `std::optional`
   - No `std::variant`
   - No structured bindings

3. **LLVM Version Constraints**
   - Must use LLVM 7-10
   - Deprecated APIs
   - Limited future support

## Implementation Plan

### Phase 1: Foundation (Week 1-2)

1. **Create Compatibility Layer**
   ```cpp
   // src/compat/
   ├── optional.hpp      // C++11/14 optional implementation
   ├── string_view.hpp   // C++11/14 string_view implementation
   ├── filesystem.hpp    // C++11/14 filesystem wrapper
   └── variant.hpp       // C++11/14 variant implementation
   ```

2. **Update CMake Configuration**
   - Add C++ standard selection
   - Add LLVM version detection
   - Add conditional feature compilation

3. **Create Feature Detection Macros**
   - Define feature availability macros
   - Add compile-time feature checks

### Phase 2: Core Implementation (Week 3-4)

1. **Update Core Classes**
   ```cpp
   // src/common/
   ├── ComponentInfo.cpp     // Use compatibility layer
   ├── MetadataExtractor.cpp // Conditional DWARF support
   ├── SBOMGenerator.cpp     // Standard-agnostic implementation
   └── Utils.cpp            // Compatibility wrapper usage
   ```

2. **Implement Conditional DWARF Support**
   ```cpp
   #if HEIMDALL_FULL_DWARF
       // Full LLVM 19+ DWARF implementation
   #elif HEIMDALL_BASIC_DWARF
       // Basic LLVM 7-10 DWARF implementation
   #else
       // No DWARF support
   #endif
   ```

### Phase 3: Testing and Validation (Week 5-6)

1. **Multi-Standard Test Suite**
   ```cpp
   // tests/
   ├── test_cpp11_compatibility.cpp
   ├── test_cpp14_compatibility.cpp
   └── test_cpp17_features.cpp
   ```

2. **CI/CD Pipeline Updates**
   - Add C++11/14 build jobs
   - Add LLVM version matrix testing
   - Add feature compatibility tests

## File Structure Changes

```
heimdall/
├── src/
│   ├── compat/              # NEW: Compatibility layer
│   │   ├── optional.hpp
│   │   ├── string_view.hpp
│   │   ├── filesystem.hpp
│   │   └── variant.hpp
│   ├── common/
│   │   ├── ComponentInfo.cpp
│   │   ├── MetadataExtractor.cpp
│   │   ├── SBOMGenerator.cpp
│   │   └── Utils.cpp
│   └── llvm/                # NEW: LLVM version abstraction
│       ├── llvm_detector.hpp
│       ├── llvm_dwarf_cpp17.cpp
│       ├── llvm_dwarf_cpp14.cpp
│       └── llvm_dwarf_cpp11.cpp
├── cmake/
│   ├── CXXStandard.cmake    # NEW: C++ standard detection
│   ├── LLVMVersion.cmake    # NEW: LLVM version detection
│   └── Compatibility.cmake  # NEW: Feature compatibility
└── docs/
    ├── cpp11_cpp14_support.md
    ├── compatibility_matrix.md
    └── migration_guide.md
```

## Migration Guide

### For Existing Users

1. **C++17+ Users (Current)**
   - No changes required
   - Full functionality maintained
   - Recommended for new projects

2. **C++14 Users**
   - Use `--cpp14` flag
   - Limited DWARF support
   - Basic functionality available

3. **C++11 Users**
   - Use `--cpp11` flag
   - No DWARF support
   - Core SBOM generation only

### For Developers

1. **Adding New Features**
   ```cpp
   #if HEIMDALL_CPP17_AVAILABLE
       // Use modern C++ features
   #else
       // Use compatibility layer
   #endif
   ```

2. **Testing Multiple Standards**
   ```bash
   ./build.sh --cpp11 --no-tests
   ./build.sh --cpp14 --no-tests
   ./build.sh --cpp17
   ```

## Conclusion

Adding C++11/14 support requires significant architectural changes but provides backward compatibility for legacy systems. The key is maintaining a clean separation between modern and legacy code paths while preserving core functionality.

**Recommendations:**
1. Implement C++14 support first (easier transition)
2. Add C++11 support as optional (limited use cases)
3. Maintain C++17+ as primary target
4. Use conditional compilation for feature differences
5. Comprehensive testing across all standards

**Trade-offs:**
- Increased code complexity
- Reduced DWARF functionality in older standards
- Additional maintenance burden
- Limited LLVM version support for older standards

The implementation should prioritize maintaining clean, readable code while providing maximum compatibility across C++ standards.

## Diagrams

This document includes two DOT diagrams that visualize the compatibility matrix and implementation architecture:

### Compatibility Matrix
```bash
dot -Tpng docs/compatibility_matrix.dot -o docs/compatibility_matrix.png
dot -Tsvg docs/compatibility_matrix.dot -o docs/compatibility_matrix.svg
```

### Implementation Architecture
```bash
dot -Tpng docs/implementation_architecture.dot -o docs/implementation_architecture.png
dot -Tsvg docs/implementation_architecture.dot -o docs/implementation_architecture.svg
```

These diagrams show:
1. **Compatibility Matrix**: Relationships between C++ standards, LLVM versions, and feature availability
2. **Implementation Architecture**: How the conditional compilation and compatibility layers work together

To generate the diagrams, ensure you have Graphviz installed:
```bash
# Ubuntu/Debian
sudo apt-get install graphviz

# macOS
brew install graphviz

# CentOS/RHEL
sudo yum install graphviz
```