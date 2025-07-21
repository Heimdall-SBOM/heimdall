# Heimdall Test Suite

This directory contains the comprehensive unit test suite for the Heimdall SBOM generator project.

## Overview

The test suite uses Google Test framework and provides extensive coverage of all major components:

- **Utils**: Utility functions for file operations, string manipulation, and checksum generation
- **ComponentInfo**: Component metadata representation and management
- **SBOMGenerator**: SBOM generation in SPDX and CycloneDX formats
- **MetadataExtractor**: Binary file analysis and metadata extraction
- **DWARFExtractor**: DWARF debug information extraction (LLVM-based)

## Code Coverage

- **Current coverage:** ![Coverage](https://img.shields.io/badge/coverage-47.3%25-yellow.svg)
- Coverage is generated using `tests/coverage.sh` and includes all source files, including `src/tools/*.cpp`.
- To update the badge and report, run:
  ```bash
  ./tests/coverage.sh
  ```
- The full HTML report is available at `build/coverage/html/index.html` after running the script.

## Important Thread-Safety Limitations

**⚠️ CRITICAL: DWARF Tests Are Not Thread-Safe**

The DWARF extraction functionality uses LLVM's DWARF libraries, which are **NOT thread-safe**. 
Concurrent DWARF tests have been removed to prevent segmentation faults and crashes.

**What This Means:**
- Cannot run multiple `DWARFExtractor` instances simultaneously
- Cannot use DWARF functionality from multiple threads
- Concurrent tests have been removed from the test suite
- All DWARF operations must be performed serially

**For More Information:**
- See `heimdall-limitations.md` for detailed thread-safety documentation
- Check test file headers for specific thread-safety warnings
- Review `DWARFExtractor.hpp` for usage guidelines

## Test Structure

```
tests/
├── main.cpp                    # Test runner entry point
├── test_utils.cpp             # Utils class unit tests
├── test_component_info.cpp    # ComponentInfo class unit tests
├── test_sbom_generator.cpp    # SBOMGenerator class unit tests
├── test_metadata_extractor.cpp # MetadataExtractor class unit tests
├── test_dwarf_extractor.cpp   # Basic DWARF extraction tests
├── test_dwarf_advanced.cpp    # Advanced DWARF functionality tests
├── test_dwarf_cross_platform.cpp # Cross-platform DWARF tests
├── test_dwarf_integration.cpp # DWARF integration tests
├── data/                      # Test data files
│   └── test_files/           # Sample files for testing
│       ├── empty.txt         # Empty file for testing
│       ├── sample.txt        # Sample text file
│       └── binary.bin        # Sample binary file
├── CMakeLists.txt            # Test build configuration
└── README.md                 # This file
```

## Building and Running Tests

### Prerequisites

- Google Test framework (automatically found by CMake)
- C++17 compatible compiler
- CMake 3.16+

### Build Tests

```bash
# From project root
./build.sh

# Or manually
mkdir -p build
cd build
cmake ..
make heimdall_tests
```

### Run All Tests

```bash
# From build directory
make test

# Or run the test executable directly
./heimdall_tests

# Or use CTest
ctest --verbose
```

### Run Specific Test Suites

```bash
# Run only Utils tests
make test-utils

# Run only ComponentInfo tests
make test-component-info

# Run only SBOMGenerator tests
make test-sbom-generator

# Run only MetadataExtractor tests
make test-metadata-extractor

# Run only DWARF tests (serial execution)
make test-dwarf-extractor
make test-dwarf-advanced
make test-dwarf-cross-platform
make test-dwarf-integration

# Or use gtest filters
./heimdall_tests --gtest_filter="UtilsTest.*"
./heimdall_tests --gtest_filter="ComponentInfoTest.*"
./heimdall_tests --gtest_filter="SBOMGeneratorTest.*"
./heimdall_tests --gtest_filter="MetadataExtractorTest.*"
./heimdall_tests --gtest_filter="DWARF*"
```

### Run Individual Tests

```bash
# Run specific test
./heimdall_tests --gtest_filter="UtilsTest.FileExists"

# Run tests matching pattern
./heimdall_tests --gtest_filter="*FileExists*"

# Run tests with verbose output
./heimdall_tests --gtest_verbose
```

## Test Coverage

### Utils Tests (`test_utils.cpp`)

Tests for utility functions including:

- **File Operations**: `fileExists`, `isDirectory`, `getFileSize`
- **Path Manipulation**: `getFileExtension`, `getFileName`, `normalizePath`, `joinPaths`
- **Checksum Generation**: `generateSHA256`, `generateSHA256FromString`
- **String Operations**: `toLower`, `toUpper`, `trim`, `split`, `replace`
- **JSON Operations**: `isValidJSON`, `escapeJSON`
- **License Detection**: `detectLicense`
- **File Type Detection**: `getFileType`

### ComponentInfo Tests (`test_component_info.cpp`)

Tests for component metadata management:

- **Constructors**: Default and parameterized constructors
- **Property Access**: Setting and getting component properties
- **Collections**: Adding symbols and dependencies
- **Checksum Generation**: File checksum calculation
- **Validation**: Component validity checking
- **Serialization**: String representation
- **Comparison**: Equality operators
- **Memory Management**: Copy/move constructors and assignment

### SBOMGenerator Tests (`test_sbom_generator.cpp`)

Tests for SBOM generation functionality:

- **Configuration**: Setting output path and format
- **Component Management**: Adding, removing, and finding components
- **SBOM Generation**: SPDX and CycloneDX format generation
- **File Output**: Writing SBOM files to disk
- **Error Handling**: Invalid paths and formats
- **Edge Cases**: Empty components, duplicates, large files
- **Memory Management**: Copy/move operations

### MetadataExtractor Tests (`test_metadata_extractor.cpp`)

Tests for binary file analysis:

- **File Type Detection**: Various file formats and extensions
- **Metadata Extraction**: Name, version, supplier, license extraction
- **Binary Analysis**: Symbol and dependency extraction
- **Error Handling**: Non-existent files, directories, broken symlinks
- **Edge Cases**: Empty files, large files, special characters
- **Platform Support**: Different file types across platforms

### DWARF Tests (Thread-Safety Limited)

**⚠️ IMPORTANT: These tests are designed to run serially due to LLVM thread-safety limitations.**

#### Basic DWARF Tests (`test_dwarf_extractor.cpp`)

Tests for core DWARF extraction functionality:

- **Source File Extraction**: Extracting source file paths from DWARF info
- **Function Extraction**: Extracting function names and signatures
- **Compile Unit Extraction**: Extracting compilation unit information
- **Line Information**: Extracting line number mappings
- **Error Handling**: Invalid files, missing DWARF sections
- **Fallback Mechanisms**: Heuristic parsing when LLVM DWARF fails

#### Advanced DWARF Tests (`test_dwarf_advanced.cpp`)

Tests for advanced DWARF features:

- **Detailed Function Information**: Function parameters, return types, scopes
- **Line Number Details**: Source line to address mappings
- **Error Scenarios**: Corrupted DWARF data, truncated files, permission issues
- **Performance Benchmarks**: Single-threaded performance testing
- **Memory Stress Tests**: Large file handling and memory management
- **Edge Cases**: Various DWARF format versions and configurations

#### Cross-Platform DWARF Tests (`test_dwarf_cross_platform.cpp`)

Tests for platform-specific DWARF handling:

- **ELF Format Support**: Linux executable and library DWARF extraction
- **Mach-O Format Support**: macOS binary DWARF extraction (limited)
- **PE Format Support**: Windows binary DWARF extraction (limited)
- **Architecture Support**: x86, x86_64, ARM DWARF handling
- **Format Detection**: Automatic detection of binary formats
- **Platform-Specific Features**: OS-specific DWARF extensions

#### DWARF Integration Tests (`test_dwarf_integration.cpp`)

Tests for DWARF integration with other components:

- **MetadataExtractor Integration**: DWARF data in component metadata
- **Plugin System Integration**: DWARF extraction via plugin interface
- **SBOM Generation**: DWARF information in generated SBOMs
- **End-to-End Workflows**: Complete DWARF extraction pipelines
- **Performance Integration**: DWARF extraction in larger workflows
- **Memory Management**: Integration-level memory handling

## Test Archive Generation (Static Libraries)

Some tests require static library archives (e.g., `libtest.a`, `libtest_with_syms.a`, and dummy `libz.a` files for Conan, Spack, and vcpkg test directories). **These files are not checked into version control.**

Instead, they are automatically generated as part of the test build using CMake custom commands. When you build the tests, CMake will:

- Compile `test_lib.c` to `test_lib.o`
- Create `libtest.a` and `libtest_with_syms.a` from `test_lib.o`
- Create small dummy `libz.a` files in the appropriate package manager subdirectories

This ensures that a fresh clone/build will always have the necessary test archives for the test suite to run successfully.

**You do not need to manually create or check in these files.**

## Test Data

The `data/test_files/` directory contains sample files used by tests:

- `empty.txt`: Empty file for testing edge cases
- `sample.txt`: Sample text file with known content
- `binary.bin`: Sample binary file for testing binary operations

The `testdata/` directory contains source files and generated archives for archive and package manager tests:

- `test_lib.c`: Source file for generating test object and archive files
- `libtest.a`, `libtest_with_syms.a`: Generated static libraries for archive tests
- `conan/lib/libz.a`, `spack/.../libz.a`, `vcpkg/.../libz.a`: Dummy static libraries for package manager tests (generated)
- `notanarchive.txt`: Dummy file for negative archive tests

All `.a` files and `test_lib.o` are generated automatically by CMake and are not tracked in git.

## Test Environment

Tests run in a controlled environment:

- **Temporary Directories**: Each test creates its own temporary directory
- **Cleanup**: All test artifacts are automatically cleaned up
- **Isolation**: Tests are independent and can run in any order
- **Timeout**: Tests have a 300-second timeout to prevent hanging

## Continuous Integration

The test suite is integrated with the build system:

- Tests run automatically during `./build.sh`
- Test results are reported in the build output
- Failed tests cause the build to fail
- Test coverage is tracked and reported

## Adding New Tests

To add new tests:

1. **Create Test File**: Add a new `.cpp` file in the tests directory
2. **Include Headers**: Include the necessary headers and Google Test
3. **Write Test Class**: Create a test class inheriting from `::testing::Test`
4. **Add Test Methods**: Use `TEST_F` macro to define test methods
5. **Update CMakeLists.txt**: Add the new test file to the executable
6. **Run Tests**: Verify your tests pass

### Example Test Structure

```cpp
#include <gtest/gtest.h>
#include "YourClass.hpp"

class YourClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test fixtures
    }

    void TearDown() override {
        // Clean up test fixtures
    }
};

TEST_F(YourClassTest, TestMethod) {
    // Your test code here
    EXPECT_TRUE(true);
}
```

## Best Practices

1. **Test Independence**: Each test should be independent and not rely on other tests
2. **Clean Setup/Teardown**: Always clean up resources in `TearDown()`
3. **Descriptive Names**: Use descriptive test and method names
4. **Edge Cases**: Test edge cases and error conditions
5. **Assertions**: Use appropriate assertions (`EXPECT_*`, `ASSERT_*`)
6. **Documentation**: Document complex test scenarios

## Troubleshooting

### Common Issues

1. **Google Test Not Found**: Install Google Test development package
2. **Test Failures**: Check test output for specific failure details
3. **Timeout Issues**: Increase timeout in CMakeLists.txt if needed
4. **Permission Errors**: Ensure write permissions in test directories

### Debugging Tests

```bash
# Run with debug output
./heimdall_tests --gtest_verbose

# Run specific failing test
./heimdall_tests --gtest_filter="TestClass.FailingTest"

# Run with gdb
gdb --args ./heimdall_tests --gtest_filter="TestClass.FailingTest"
```

## Performance

- **Fast Execution**: Most tests complete in milliseconds
- **Parallel Execution**: Tests can run in parallel (use `--gtest_parallel`)
- **Memory Efficient**: Tests use minimal memory and clean up properly
- **CI Friendly**: Optimized for continuous integration environments

## Code Coverage

The test suite supports code coverage analysis using gcov. Coverage reports help identify untested code paths and ensure comprehensive testing.

### Enabling Coverage

To enable code coverage, build with the `ENABLE_COVERAGE` option:

```bash
# From project root
mkdir -p build
cd build
cmake -DENABLE_COVERAGE=ON ..
make -j$(nproc)
```

### Running Coverage Analysis

#### Using the Coverage Script (Recommended)

```bash
# From project root
./tests/coverage.sh
```

This script will:
- Automatically enable coverage if not already enabled
- Build the project with coverage instrumentation
- Run all tests to generate coverage data
- Generate both text and HTML coverage reports
- Display a summary of coverage results

#### Using CMake Targets

```bash
# From build directory
make coverage        # Run tests and generate coverage
make coverage-clean  # Clean coverage data
```

#### Manual Coverage Generation

```bash
# From build directory
ctest --output-on-failure
gcov -r -b -s $(pwd)/.. CMakeFiles/heimdall-core.dir/src/common/*.gcno
gcov -r -b -s $(pwd)/.. CMakeFiles/heimdall_tests.dir/*.gcno
```

### Coverage Reports

Coverage reports are generated in the `build/coverage/` directory:

- `coverage_summary.txt`: Text summary of coverage statistics
- `*.gcov`: Detailed coverage files for each source file
- `html/` (if lcov is available): HTML coverage report

### Coverage Targets

The following coverage targets are available when `ENABLE_COVERAGE=ON`:

- `coverage`: Run tests and generate coverage reports
- `coverage-clean`: Clean all coverage data files

### Coverage Requirements

- **GCC/G++**: Coverage instrumentation is built into GCC
- **lcov** (optional): For HTML coverage reports
  - Ubuntu/Debian: `sudo apt-get install lcov`
  - CentOS/RHEL: `sudo yum install lcov`
  - macOS: `brew install lcov`

### Coverage Best Practices

1. **Regular Coverage Runs**: Run coverage analysis regularly during development
2. **Coverage Goals**: Aim for high coverage (>80%) on critical code paths
3. **Coverage Gaps**: Use coverage reports to identify untested code
4. **Coverage Cleanup**: Clean coverage data between runs for accurate results
5. **CI Integration**: Include coverage analysis in continuous integration 