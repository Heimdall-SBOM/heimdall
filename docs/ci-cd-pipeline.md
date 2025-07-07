# CI/CD Pipeline Documentation

## Overview

The Heimdall project uses a comprehensive GitHub Actions CI/CD pipeline to ensure code quality, security, and reliable releases. The pipeline consists of multiple workflows that handle different aspects of the development lifecycle.

## Workflows

### 1. CI Workflow (`.github/workflows/ci.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches

**Jobs:**

#### Linux GCC Builds
- **Matrix:** GCC 9-12, Debug/Release builds
- **Features:** Code coverage, sanitizers, static analysis, memory checking
- **Coverage:** Uploads to Codecov for coverage tracking

#### Linux Clang Builds
- **Matrix:** Clang 15-18, Debug/Release builds
- **Features:** Full test suite execution

#### macOS Builds
- **Matrix:** Clang and GCC, Debug/Release builds
- **Features:** Cross-platform compatibility testing

#### Windows Builds
- **Matrix:** MSVC and Clang-cl, Debug/Release builds
- **Features:** Windows-specific testing (LLD/Gold plugins disabled)

#### Code Quality Checks
- **clang-format:** Code formatting validation
- **clang-tidy:** Static analysis
- **cppcheck:** Additional static analysis
- **TODO/FIXME:** Checks for unresolved comments

#### Security Scanning
- **Trivy:** Vulnerability scanning
- **SARIF:** Results uploaded to GitHub Security tab

### 2. CD Workflow (`.github/workflows/cd.yml`)

**Triggers:**
- Version tags (e.g., `v1.0.0`)
- Manual workflow dispatch

**Jobs:**

#### Build and Test
- **Matrix:** All supported platforms and compilers
- **Features:** Full build, test, and artifact creation
- **Artifacts:** Platform-specific binaries and libraries

#### Create Release
- **Automatic:** GitHub release creation
- **Assets:** Pre-built binaries for all platforms
- **Notes:** Auto-generated from git history

#### Docker Image Build
- **Multi-platform:** Linux AMD64 and ARM64
- **Registry:** Docker Hub (requires secrets)

### 3. Dependencies Workflow (`.github/workflows/dependencies.yml`)

**Triggers:**
- Weekly schedule (Mondays at 9 AM UTC)
- Manual workflow dispatch

**Jobs:**

#### Check Dependencies
- **System packages:** Version checking
- **LLVM:** Version verification
- **OpenSSL:** Version verification

#### Security Scan
- **Trivy:** Vulnerability scanning
- **CodeQL:** Static analysis for security issues

#### License Compliance
- **Headers:** Apache 2.0 license verification
- **Third-party:** License compliance checking

#### Performance Testing
- **Build timing:** Performance regression detection
- **Binary sizes:** Size monitoring

## Configuration

### Environment Variables

```yaml
# CMake configuration
CMAKE_BUILD_TYPE: Release
CMAKE_CXX_STANDARD: 17

# Build options
BUILD_LLD_PLUGIN: ON
BUILD_GOLD_PLUGIN: ON
BUILD_SHARED_CORE: ON
BUILD_TESTS: ON
BUILD_EXAMPLES: ON
```

### Required Secrets

For full functionality, the following secrets should be configured in your GitHub repository:

- `DOCKERHUB_USERNAME`: Docker Hub username
- `DOCKERHUB_TOKEN`: Docker Hub access token
- `CODECOV_TOKEN`: Codecov token (optional)

## Docker Support

### Multi-stage Dockerfile

The project includes a multi-stage Dockerfile with four targets:

1. **builder:** Complete build environment
2. **runtime:** Production runtime environment
3. **dev:** Development environment with debugging tools
4. **minimal:** Minimal production image

### Usage

```bash
# Build all stages
docker build --target runtime -t heimdall:latest .

# Build development environment
docker build --target dev -t heimdall:dev .

# Build minimal production image
docker build --target minimal -t heimdall:minimal .
```

## Local Development

### Prerequisites

- CMake 3.16+
- C++11 compatible compiler (minimum)
- C++14/17/23 compatible compiler (recommended)
- OpenSSL development libraries
- libelf development libraries
- LLVM 19+ (for DWARF support)

### Quick Start

```bash
# Clone the repository
git clone https://github.com/your-org/heimdall.git
cd heimdall

# Build the project
./build.sh

# Run tests
cd build
ctest --output-on-failure --verbose

# Install
make install
```

### Running CI Locally

You can run CI checks locally using the provided scripts:

```bash
# Code formatting check
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# Static analysis
cd build
make clang-tidy
make cppcheck

# Memory checking
valgrind --tool=memcheck --leak-check=full ctest
```

## Monitoring and Metrics

### Code Coverage

- **Tool:** LCOV/GCOVR
- **Platform:** Codecov
- **Threshold:** Configurable coverage targets

### Performance Metrics

- **Build time:** Tracked for regression detection
- **Binary size:** Monitored for bloat prevention
- **Test execution time:** Performance regression testing

### Security Metrics

- **Vulnerability scanning:** Trivy integration
- **Static analysis:** CodeQL and clang-tidy
- **License compliance:** Automated header checking

## Troubleshooting

### Common Issues

1. **LLVM not found:**
   ```bash
   # Install LLVM 19
   wget https://apt.llvm.org/llvm.sh
   chmod +x llvm.sh
   sudo ./llvm.sh 19
   sudo apt-get install -y clang-19 lld-19 libllvm19-dev
   ```

2. **OpenSSL not found:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libssl-dev
   
   # macOS
   brew install openssl
   ```

3. **libelf not found:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libelf-dev
   
   # macOS
   brew install libelf
   ```

### Debugging CI Failures

1. **Check logs:** Review the detailed logs in GitHub Actions
2. **Reproduce locally:** Use the same environment as CI
3. **Matrix debugging:** Test specific matrix combinations locally

## Best Practices

### For Developers

1. **Always run tests locally** before pushing
2. **Use consistent formatting** (clang-format)
3. **Check for TODO/FIXME** comments
4. **Update dependencies** regularly

### For Maintainers

1. **Monitor CI metrics** for trends
2. **Review security scans** regularly
3. **Update CI configuration** as needed
4. **Maintain Docker images** for releases

## Future Enhancements

### Planned Features

1. **Automated dependency updates** with Dependabot
2. **Performance benchmarking** integration
3. **Automated release notes** generation
4. **Multi-architecture testing** (ARM64, etc.)

### Integration Opportunities

1. **SonarQube** for code quality metrics
2. **JFrog Artifactory** for artifact management
3. **Slack/Teams** notifications for CI status
4. **Grafana** dashboards for metrics visualization

## Support

For issues with the CI/CD pipeline:

1. Check the [GitHub Actions documentation](https://docs.github.com/en/actions)
2. Review the workflow logs for specific error messages
3. Create an issue in the repository with detailed information
4. Contact the maintainers for urgent issues

## Contributing

When contributing to the CI/CD pipeline:

1. Test changes locally first
2. Use feature branches for modifications
3. Update documentation for new features
4. Ensure backward compatibility
5. Add appropriate tests for new functionality 