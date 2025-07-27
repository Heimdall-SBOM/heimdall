# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Enhanced DWARF debug information extraction capabilities
- Improved heuristic fallback for source file extraction
- Added safe string utilities to prevent buffer overread vulnerabilities
- Enhanced compatibility with multiple C++ standards (C++11 through C++23)
- Added comprehensive test suite with 193 tests
- Added comprehensive open source project files (CHANGELOG.md, SECURITY.md, issue templates)

### Changed
- Fixed critical security vulnerability in compatibility header strlen usage
- Improved LLVM plugin linking and dependency resolution
- Enhanced error handling and recovery mechanisms
- Updated CMake build system for better cross-platform compatibility

### Fixed
- Resolved unsafe strlen usage that could cause buffer overread
- Fixed LLVM library linking issues in LLD plugin
- Corrected DWARF source file extraction tests
- Fixed Utils.cpp function implementations to match test expectations
- Resolved CMakeLists.txt issues with test file inclusion

### Security
- **CRITICAL**: Fixed buffer overread vulnerability in string_view constructor
- Added safe string utilities: `safe_strlen()`, `is_null_terminated()`, `safe_string_view()`
- Implemented bounded string operations using `strnlen()` instead of `strlen()`

## [0.3.0] - 2025-07-11

### Added
- Comprehensive open source project governance files
- GitHub issue templates for bug reports, feature requests, and documentation
- Pull request template with security and quality checklists
- Security policy with vulnerability reporting guidelines
- Enhanced DWARF debug information extraction with heuristic fallbacks
- Safe string utilities to prevent buffer overread vulnerabilities
- Comprehensive test suite with 193 tests (190 passed, 3 skipped)

### Changed
- Fixed critical security vulnerability in compatibility header strlen usage
- Improved LLVM plugin linking and dependency resolution
- Enhanced error handling and recovery mechanisms
- Updated CMake build system for better cross-platform compatibility

### Fixed
- **CRITICAL**: Fixed buffer overread vulnerability in string_view constructor
- Fixed LLVM library linking issues in LLD plugin
- Corrected DWARF source file extraction tests
- Fixed Utils.cpp function implementations to match test expectations
- Resolved CMakeLists.txt issues with test file inclusion
- Fixed remaining test failures and unsafe strlen usage
- Fixed DWARF extraction and improved test reliability
- Fixed utility and path handling issues

### Security
- **CRITICAL**: Fixed buffer overread vulnerability in string_view constructor
- Added safe string utilities: `safe_strlen()`, `is_null_terminated()`, `safe_string_view()`
- Implemented bounded string operations using `strnlen()` instead of `strlen()`
- Fixed SonarQube BLOCKER and CRITICAL issues with noexcept move constructors
- Replaced macros with constexpr for better type safety
- Fixed const global variables and plugin const correctness

## [0.2.1] - 2025-07-10

### Fixed
- Fixed SonarQube BLOCKER and CRITICAL issues
- Added noexcept move constructors to ValidationResult
- Used reentrant version of gmtime_r for thread safety
- Corrected possible buffer overflow issues
- Fixed macro to constexpr conversions
- Fixed const global variables and plugin const correctness

## [0.2.0] - 2024-12-19

### Added
- Initial release of Heimdall SBOM generator
- Support for LLVM LLD and GNU Gold linkers
- Multiple SBOM format support (SPDX 2.3, 3.0, 3.0.1 and CycloneDX 1.4, 1.5, 1.6)
- DWARF debug information extraction
- Package manager integration (Conan, vcpkg, system packages)
- Cross-platform support for macOS and Linux
- Comprehensive testing framework with GoogleTest
- Multi-standard C++ support (C++11 through C++23)
- CMake build system with automatic compiler detection
- Plugin infrastructure for linker integration

### Features
- Dual Linker Support: Works with both LLVM LLD and GNU Gold linkers
- Multiple SBOM Formats: Generates SPDX and CycloneDX compliant reports
- Comprehensive Component Analysis: Extracts versions, licenses, checksums, dependencies
- Performance Optimized: Minimal overhead during linking
- CI/CD Ready: Seamless integration with modern build systems
- Security Focused: Enables vulnerability scanning and compliance tracking

## [0.1.1] - 2024-12-15

### Added
- Comprehensive C++11/14/17/20/23 compatibility layer
- Full C++20 support: concepts, ranges, format, span, expected, source_location
- Full C++23 support: generator, flat_map, flat_set, print, stacktrace, mdspan
- Comprehensive test suite for all C++ standards
- LLVM detector supporting C++23
- Utility functions for cross-standard compatibility
- Conditional compilation based on C++ standard
- Boost.Filesystem detection and integration
- Multi-standard build system with GCC/Clang support
- SCL toolsets support for modern GCC versions on RHEL/Rocky/CentOS
- Comprehensive examples and developer guides
- Format-specific examples (SPDX-only, CycloneDX-only, general)
- Comprehensive documentation with user and developer guides
- CMake module for easy integration
- Advanced CMake examples and templates

### Changed
- Restructured build system for multi-standard C++ compatibility
- Enhanced LLVM detection and compatibility
- Improved CMake integration and module system
- Updated documentation structure and content

## [0.1.0] - 2024-12-01

### Added
- Initial project setup
- Basic CMake configuration
- Core SBOM generation functionality
- LLVM integration framework
- Basic documentation structure

---

## Version History

- **0.1.0**: Initial project setup and basic functionality
- **0.1.1**: Comprehensive C++ multi-standard support and build system restructuring
- **0.2.0**: First feature-complete release with dual linker support
- **0.2.1**: Security and code quality improvements
- **0.3.0**: Critical security fixes and open source governance
- **Unreleased**: Current development version with ongoing improvements

## Contributing

When contributing to this project, please update this changelog by adding a new entry under the `[Unreleased]` section. Follow the format above and include:

- **Added**: for new features
- **Changed**: for changes in existing functionality
- **Deprecated**: for soon-to-be removed features
- **Removed**: for now removed features
- **Fixed**: for any bug fixes
- **Security**: for security-related changes 