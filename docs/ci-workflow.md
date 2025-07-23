# CI Workflow Documentation

This document describes the comprehensive CI (Continuous Integration) workflow that builds and tests Heimdall across multiple C++ standards and compilers.

## Overview

The CI workflow (`.github/workflows/ci.yml`) provides:

- **Multi-standard testing**: C++11 and C++23 builds
- **Multi-compiler testing**: GCC and Clang support
- **Unit test execution**: Comprehensive test suite validation
- **Code coverage analysis**: Coverage reporting and upload
- **Example builds**: Validation of example projects
- **Manual triggers**: Selective testing capabilities

## Workflow Jobs

### Core Build and Test Jobs

| Job | C++ Standard | Compiler | LLVM Version | Purpose |
|-----|-------------|----------|--------------|---------|
| `cpp11-gcc` | C++11 | GCC | LLVM 18 | Legacy compatibility testing |
| `cpp11-clang` | C++11 | Clang 18 | LLVM 18 | Legacy compatibility testing |
| `cpp23-gcc` | C++23 | GCC 13 | LLVM 20 | Latest standard testing |
| `cpp23-clang` | C++23 | Clang 20 | LLVM 20 | Latest standard testing |

### Additional Jobs

| Job | Trigger | Purpose |
|-----|---------|---------|
| `coverage` | Push to main | Code coverage analysis and reporting |
| `examples` | Push to main | Example project validation |
| `summary` | Always | Build status summary |

## Triggers

### Automatic Triggers

- **Push to main/develop**: Runs all jobs
- **Pull requests to main/develop**: Runs core build/test jobs

### Manual Triggers

The workflow can be manually triggered with selective options:

```bash
# Run all tests
# (default behavior)

# Test only C++11
cpp_standard: 11

# Test only C++23
cpp_standard: 23

# Test only GCC
compiler: gcc

# Test only Clang
compiler: clang

# Test specific combination
cpp_standard: 23
compiler: clang
```

## Environment Setup

### Pre-built DevContainer Image

All jobs use the pre-built devcontainer image (`ghcr.io/${{ github.repository_owner }}/heimdall-devcontainer:latest`) which includes:

- **Base System**: Ubuntu 22.04
- **Build Tools**: build-essential, cmake, ninja-build, git
- **Compilers**: GCC 11, GCC 13, Clang
- **LLVM**: LLVM 19 with LLD
- **Dependencies**: All required libraries and development packages
- **LLD Headers**: Pre-installed LLD headers for plugin development

### Compiler Versions

| C++ Standard | GCC Version | Clang Version | LLVM Version |
|-------------|-------------|---------------|--------------|
| C++11 | GCC 11/13 | Clang | LLVM 19 |
| C++23 | GCC 13 | Clang | LLVM 19 |

### Container Usage

The CI workflow uses the container specification:

```yaml
container:
  image: ghcr.io/${{ github.repository_owner }}/heimdall-devcontainer:latest
  options: --user root
```

This provides:
- **Consistency**: Same environment as development
- **Speed**: No dependency installation during CI
- **Reliability**: Pre-tested and validated environment

## Build Process

Each job follows this process:

1. **Container Setup**: Use pre-built devcontainer image
2. **Checkout**: Repository with submodules
3. **Setup Environment**: Configure LLVM and compiler paths
4. **Build**: Run `./scripts/build.sh` with appropriate flags
5. **Test**: Execute unit tests with CTest
6. **Upload Artifacts**: Test results and build artifacts

**Note**: No dependency installation is needed as the devcontainer image includes all required packages.

### Build Command

```bash
./scripts/build.sh --standard <cpp_standard> --compiler <compiler> --clean --tests
```

## Test Execution

Tests are run using CTest with the following configuration:

```bash
CTEST_OUTPUT_ON_FAILURE=1
CTEST_PARALLEL_LEVEL=2
ctest --output-on-failure --verbose
```

### Test Artifacts

Test results are uploaded as artifacts with:
- **Retention**: 7 days
- **Path**: `$BUILD_DIR/Testing/`
- **Naming**: `test-results-cpp<standard>-<compiler>`

## Coverage Analysis

The coverage job runs on pushes to main and provides:

### Coverage Tools

- **LCOV**: Line coverage analysis
- **gcovr**: Coverage report generation
- **Codecov**: Coverage reporting service

### Coverage Process

1. Build with coverage flags
2. Run test suite
3. Generate coverage reports
4. Upload to Codecov
5. Store artifacts for 30 days

### Coverage Artifacts

- **Path**: `coverage/`
- **Retention**: 30 days
- **Upload**: Codecov integration

## Example Validation

The examples job validates that all example projects build correctly:

### Example Projects

- CMake module examples
- Shared library examples
- Interface examples
- Installation examples

### Build Process

1. Build main project (C++17, GCC)
2. Build all examples
3. Upload example builds as artifacts

## Summary Job

The summary job provides a comprehensive overview of all build results:

### Summary Content

- Build status for each configuration
- Coverage generation status
- Example build status
- Overall workflow status

### Summary Format

```markdown
## Build and Test Summary

| Configuration | Status |
|---------------|--------|
| C++11 GCC | ✅ Passed |
| C++11 Clang | ✅ Passed |
| C++23 GCC | ✅ Passed |
| C++23 Clang | ✅ Passed |

**Coverage and Examples:**
- Code Coverage: ✅ Generated
- Examples: ✅ Built
```

## Artifacts

### Test Results

- `test-results-cpp11-gcc`
- `test-results-cpp11-clang`
- `test-results-cpp23-gcc`
- `test-results-cpp23-clang`

### Coverage Reports

- `coverage-report` (30 days retention)

### Example Builds

- `example-builds` (7 days retention)

## Troubleshooting

### Common Issues

1. **DevContainer Image Issues**
   - Ensure the devcontainer image is built and pushed to Docker Hub
   - Check that the image name matches the repository owner
   - Verify the image contains all required dependencies

2. **Compiler Version Issues**
   - C++23 requires GCC 13+ or Clang 14+
   - The devcontainer image includes GCC 13 and Clang
   - Check that the correct compiler is selected in environment setup

3. **Test Failures**
   - Review test output in artifacts
   - Check for missing dependencies
   - Verify test environment setup

4. **Coverage Generation Issues**
   - Ensure coverage tools are installed
   - Check build flags for coverage
   - Verify test execution

### Debugging

1. **Enable Debug Output**
   - Add `--verbose` to build commands
   - Check CTest output for details

2. **Local Reproduction**
   - Use the same Ubuntu 22.04 environment
   - Install identical package versions
   - Follow the same setup steps

3. **Artifact Analysis**
   - Download test result artifacts
   - Review coverage reports
   - Check build logs

## Performance Considerations

### Parallel Execution

- Jobs run in parallel when possible
- CTest uses parallel test execution
- Build uses all available CPU cores

### Resource Usage

- Each job runs on a fresh Ubuntu 22.04 runner
- Jobs are isolated to prevent interference
- Artifacts are cleaned up automatically

### Optimization

- LLVM installation is cached
- Build artifacts are preserved between steps
- Test results are uploaded incrementally

## Future Enhancements

### Planned Features

1. **Additional C++ Standards**
   - C++14 and C++17 testing
   - C++26 support when available

2. **Platform Expansion**
   - macOS testing
   - Windows testing
   - ARM64 testing

3. **Advanced Testing**
   - Performance benchmarks
   - Memory leak detection
   - Static analysis

4. **Integration Testing**
   - End-to-end workflow testing
   - Plugin integration testing
   - SBOM generation validation

### Configuration Options

- Custom test suites
- Selective dependency testing
- Performance profiling
- Debug builds

## Contributing

When modifying the CI workflow:

1. **Test Locally**: Verify changes work in a local environment
2. **Update Documentation**: Keep this document current
3. **Consider Performance**: Minimize workflow execution time
4. **Maintain Compatibility**: Ensure backward compatibility
5. **Add Tests**: Include tests for new features

### Workflow Best Practices

- Use specific versions for reproducibility
- Minimize external dependencies
- Provide clear error messages
- Include debugging information
- Optimize for speed and reliability 