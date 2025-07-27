# Testing Heimdall

This guide covers all the different ways to run tests for the Heimdall SBOM Generator project.

## Overview

The Heimdall project includes comprehensive tests for all major components:
- Metadata extraction
- DWARF debug information parsing
- Plugin interfaces
- SBOM generation
- Component information handling
- Utilities and helper functions
- Linux support features
- Package manager detection

## Prerequisites

Before running tests, ensure you have the following dependencies installed:

- **CMake** (3.16 or later)
- **C++ Compiler** (GCC or Clang)
- **LLVM/LLD** (for LLD plugin tests)
- **OpenSSL** (for checksum generation tests)
- **libelf** (for ELF binary parsing tests)
- **BFD libraries** (for Gold plugin tests)

## Method 1: Using the Build Script (Recommended)

The easiest way to run tests is using the provided `build.sh` script, which automatically builds and tests the project:

### Basic Test Run
```bash
# Build and run all tests
./build.sh
```

### Debug Mode Testing
```bash
# Build in debug mode and run tests
./build.sh --debug
```

### Testing with Sanitizers
```bash
# Build with AddressSanitizer and UBSan, then run tests
./build.sh --sanitizers
```

### Skipping Tests
```bash
# Build without running tests
./build.sh --no-tests
```

### Build Script Options

The build script supports several options:

| Option | Description |
|--------|-------------|
| `--debug` | Build in debug mode |
| `--sanitizers` | Enable AddressSanitizer and UBSan |
| `--no-lld` | Disable LLD plugin build |
| `--no-gold` | Disable Gold plugin build |
| `--no-shared-core` | Disable shared core library |
| `--no-tests` | Disable test suite |
| `--no-examples` | Disable example projects |
| `--build-dir DIR` | Set build directory (default: build) |
| `--install-dir DIR` | Set install directory (default: install) |
| `--help, -h` | Show help message |

## Method 2: Manual CMake Build and Test

For more control over the build process:

### Step 1: Create Build Directory
```bash
mkdir -p build
cd build
```

### Step 2: Configure CMake
```bash
# Configure with tests enabled
cmake -DBUILD_TESTS=ON ..

# Or with additional options
cmake -DBUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_LLD_PLUGIN=ON \
      -DBUILD_GOLD_PLUGIN=ON \
      ..
```

### Step 3: Build the Project
```bash
# Build with all available cores
make -j$(nproc)

# Or specify number of jobs
make -j4
```

### Step 4: Run Tests
```bash
# Run all tests
make test

# Or use CTest directly
ctest

# Run with verbose output
ctest --verbose

# Run with output on failure
ctest --output-on-failure
```

## Method 3: Run Specific Tests

You can run individual test executables directly:

```bash
cd build

# Run specific test executables
./tests/test_metadata_extractor
./tests/test_dwarf_extractor
./tests/test_plugin_interface
./tests/test_sbom_generator
./tests/test_component_info
./tests/test_utils
./tests/test_linux_support
./tests/test_package_manager_and_archive
```

## Method 4: Advanced CTest Options

CTest provides many options for controlling test execution:

### Verbose Output
```bash
ctest --verbose
```

### Parallel Execution
```bash
# Use all available cores
ctest -j$(nproc)

# Use specific number of jobs
ctest -j4
```

### Timeout Control
```bash
# Set timeout for all tests (in seconds)
ctest --timeout 300
```

### Output Control
```bash
# Show output on failure
ctest --output-on-failure

# Show output for all tests
ctest --output-on-failure --verbose
```

### Test Filtering
```bash
# Run tests matching a pattern
ctest -R "test_metadata"     # Only tests with "test_metadata" in name
ctest -R "test_dwarf"        # Only DWARF-related tests
ctest -R "test_plugin"       # Only plugin-related tests

# Exclude tests matching a pattern
ctest -E "test_dwarf"        # Exclude DWARF tests

# Run tests with specific labels
ctest -L "unit"              # Only unit tests
ctest -L "integration"       # Only integration tests
```

### Test Discovery
```bash
# List all available tests
ctest --show-only

# List tests with labels
ctest --show-only --verbose
```

## Method 5: Coverage Testing

For code coverage analysis:

### Using Coverage Scripts
```bash
# Run comprehensive coverage analysis
./tests/coverage.sh

# Run simple coverage
./tests/simple_coverage.sh
```

### Manual Coverage Setup
```bash
# Configure with coverage enabled
cmake -DBUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_COVERAGE=ON \
      ..

# Build
make -j$(nproc)

# Run tests
ctest

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info '*/tests/*' --output-file coverage.info
lcov --list coverage.info
```

## Available Test Files

The project includes comprehensive tests organized by functionality:

### Core Functionality Tests
- **`test_metadata_extractor.cpp`** - Metadata extraction from binaries
- **`test_metadata_extractor_extended.cpp`** - Extended metadata extraction tests
- **`test_sbom_generator.cpp`** - SBOM generation functionality
- **`test_component_info.cpp`** - Component information handling
- **`test_utils.cpp`** - Utility functions
- **`test_utils_extended.cpp`** - Extended utility function tests

### DWARF Debug Information Tests
- **`test_dwarf_extractor.cpp`** - Basic DWARF extraction
- **`test_dwarf_integration.cpp`** - DWARF integration tests
- **`test_dwarf_advanced.cpp`** - Advanced DWARF functionality
- **`test_dwarf_cross_platform.cpp`** - Cross-platform DWARF tests

### Plugin System Tests
- **`test_plugin_interface.cpp`** - Plugin interface consistency
- **`test_plugin_sbom_consistency.cpp`** - SBOM consistency across plugins

### Platform-Specific Tests
- **`test_linux_support.cpp`** - Linux-specific functionality
- **`test_package_manager_and_archive.cpp`** - Package manager detection

## Test Data

Test data is located in the `tests/data/` and `tests/testdata/` directories and includes:
- Sample ELF binaries
- Test archives
- Package manager metadata files
- DWARF debug information samples

## Troubleshooting

### Common Issues

#### Tests Fail to Build
- Ensure all dependencies are installed
- Check that LLVM/LLD is properly configured
- Verify OpenSSL development libraries are available

#### DWARF Tests Fail
- Ensure LLVM DWARF headers are available
- Check that the system has the required DWARF libraries
- Verify that test binaries contain debug information

#### Plugin Tests Fail
- Ensure LLD and Gold linkers are available
- Check that plugin libraries are built correctly
- Verify that test binaries are compatible with the linkers

#### Coverage Tests Fail
- Ensure lcov and gcov are installed
- Check that the build is configured with coverage flags
- Verify that the compiler supports coverage instrumentation

### Debugging Test Failures

#### Enable Verbose Output
```bash
ctest --verbose --output-on-failure
```

#### Run Individual Tests
```bash
cd build
./tests/test_name --gtest_verbose
```

#### Check Test Logs
```bash
# View CTest logs
cat build/Testing/Temporary/LastTest.log

# View individual test output
cat build/Testing/Temporary/LastTest.log.tmp
```

## Continuous Integration

The project includes GitHub Actions workflows for automated testing:
- **CI Pipeline** - Runs tests on multiple platforms
- **Code Quality** - Static analysis and formatting checks
- **Security Scanning** - SonarQube analysis

## Performance Testing

For performance testing:

```bash
# Build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
make -j$(nproc)

# Run tests with timing
ctest --verbose --output-on-failure
```

## Best Practices

1. **Always run tests after making changes**
2. **Use debug builds for development**
3. **Run coverage tests periodically**
4. **Check for memory leaks with sanitizers**
5. **Verify tests pass on different platforms**
6. **Keep test data up to date**

## Quick Reference

### Most Common Commands
```bash
# Quick test run
./build.sh

# Debug build and test
./build.sh --debug

# Test with sanitizers
./build.sh --sanitizers

# Manual build and test
mkdir -p build && cd build
cmake -DBUILD_TESTS=ON ..
make -j$(nproc)
ctest --output-on-failure
```

### Environment Variables
- `HEIMDALL_TEST_DATA_DIR` - Set test data directory
- `CTEST_OUTPUT_ON_FAILURE=1` - Show output on test failure
