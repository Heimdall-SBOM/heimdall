# Heimdall Multi-Standard C++ Support

## Overview

Heimdall now supports multiple C++ standards (C++11, C++14, C++17, C++20, C++23) with a robust compatibility layer that ensures consistent behavior across different compiler versions and standard library implementations.

## Supported Standards

| C++ Standard | Status | Features | Requirements |
|--------------|--------|----------|--------------|
| C++11 | ✅ Working | Basic C++11 features | LLVM 7-18, Compatibility mode |
| C++14 | ✅ Working | C++14 features + compatibility | LLVM 7-18, Compatibility mode |
| C++17 | ✅ Working | Standard library features | LLVM 11+, Standard library |
| C++20 | ✅ Working | C++20 features + {fmt} | LLVM 19+, {fmt} library |
| C++23 | ✅ Working | C++23 features + std::format | LLVM 19+, Standard library |

## Build Commands

### Standard Build Commands
```bash
# C++11 (with compatibility mode)
./build.sh --cxx-standard 11 --tests --cpp11-14 --no-boost

# C++14 (with compatibility mode)
./build.sh --cxx-standard 14 --tests --cpp11-14 --no-boost

# C++17 (standard library)
./build.sh --cxx-standard 17 --tests

# C++20 (with {fmt} library)
./build.sh --cxx-standard 20 --tests

# C++23 (with std::format)
./build.sh --cxx-standard 23 --tests
```

### Build Script Options
- `--cxx-standard VERSION`: Set C++ standard (11, 14, 17, 20, 23)
- `--tests`: Enable tests
- `--no-tests`: Disable tests
- `--cpp11-14`: Enable C++11/14 compatibility mode
- `--no-boost`: Disable Boost.Filesystem requirement
- `--coverage`: Enable coverage reporting
- `--build-type TYPE`: Set build type (Debug, Release, etc.)

## Compatibility Layer

### Architecture
The compatibility layer (`src/compat/compatibility.hpp`) provides:

1. **Standard Library Includes**: All standard library headers are included **outside** any namespace to prevent pollution
2. **Namespace Aliases**: Provides `heimdall::compat::fs` for filesystem operations
3. **Type Compatibility**: Re-exports standard types or provides custom implementations
4. **Utility Functions**: Common utilities in `heimdall::compat::utils`

### Key Features

#### C++11/14 Compatibility
- Custom `optional<T>` implementation
- Custom `string_view` implementation  
- Custom `variant<T...>` implementation (limited)
- Custom `span<T>` implementation
- Boost.Filesystem support (optional)

#### C++17+ Compatibility
- Uses standard library types directly
- `std::optional`, `std::string_view`, `std::variant`
- `std::filesystem` support
- `std::span` (C++20+)

#### C++20/23 Compatibility
- Standard library types
- Concepts support
- Ranges support (C++20+)
- Format support (`{fmt}` for C++20, `std::format` for C++23)

## Test Suite

The test suite (`tests/test_cyclonedx_all_versions_cpp.cpp`) validates:

### C++11 Features
- `auto` keyword
- Range-based for loops
- Lambda expressions
- `nullptr`
- Uniform initialization
- `decltype`
- Trailing return types

### C++14 Features
- Auto return type deduction
- Generic lambdas
- Binary literals
- Digit separators
- Variable templates

### Compatibility Features
- `heimdall::compat::optional`
- `heimdall::compat::string_view`
- `heimdall::compat::fs::path`
- `heimdall::compat::variant`
- `heimdall::compat::utils` functions

## Build System

### CMake Configuration
- Automatic C++ standard detection
- Conditional Boost.Filesystem linking
- LLVM version compatibility checks
- Test framework integration

### Key CMake Variables
- `CMAKE_CXX_STANDARD`: Target C++ standard
- `ENABLE_CPP11_14`: Enable compatibility mode
- `USE_BOOST_FILESYSTEM`: Use Boost.Filesystem
- `ENABLE_TESTS`: Enable test suite

## Issues Resolved

### 1. Namespace Pollution
**Problem**: Standard library was being included inside `heimdall::compat::std` namespace
**Solution**: Moved all standard library includes outside any namespace

### 2. Boost.Filesystem Dependency
**Problem**: C++11/14 required Boost.Filesystem even when not needed
**Solution**: Added `--no-boost` option and conditional Boost linking

### 3. CMake Configuration
**Problem**: CMake automatically required Boost for C++11/14
**Solution**: Modified CMakeLists.txt to respect explicit Boost settings

### 4. Test Compatibility
**Problem**: Tests used features not available in older standards
**Solution**: Enhanced test suite with conditional compilation and compatibility types

## Usage Examples

### Using Compatibility Types
```cpp
#include "compat/compatibility.hpp"

using namespace heimdall::compat;

// Works across all C++ standards
optional<int> opt(42);
string_view sv("hello");
fs::path p("file.txt");
variant<int, std::string> v(100);
```

### Using Utility Functions
```cpp
#include "compat/compatibility.hpp"

using namespace heimdall::compat;

// Utility functions work across standards
string_view sv = utils::to_string_view(42);
int val = utils::get_optional_value(opt, 0);
std::string result = utils::format_string("Value: {}", 42);
```

## Future Enhancements

1. **Enhanced C++11/14 Support**: More complete custom implementations
2. **Better Error Handling**: Standardized error types across standards
3. **Performance Optimizations**: Standard-specific optimizations
4. **Additional Utilities**: More compatibility utilities
5. **Documentation**: API documentation for compatibility layer

## Troubleshooting

### Common Issues

1. **Boost.Filesystem Not Found**
   ```bash
   # Use --no-boost option
   ./build.sh --cxx-standard 14 --tests --cpp11-14 --no-boost
   ```

2. **LLVM Version Warnings**
   - C++11/14: Consider using LLVM 7-18 for best compatibility
   - C++17+: LLVM 11+ recommended
   - C++20/23: LLVM 19+ required

3. **Test Failures**
   - Ensure compatibility header is included
   - Check C++ standard detection
   - Verify LLVM version compatibility

### Debugging
```bash
# Verbose build
make VERBOSE=1

# Debug configuration
./build.sh --build-type Debug --cxx-standard 17 --tests

# Check CMake configuration
cmake --build build --target help
```

## Contributing

When adding new features:

1. **Test All Standards**: Ensure compatibility across C++11-23
2. **Use Compatibility Types**: Prefer `heimdall::compat` types
3. **Add Tests**: Include tests for new functionality
4. **Update Documentation**: Document new features and APIs

