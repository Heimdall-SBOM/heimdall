# LLVM Compatibility Guide for Heimdall

## Overview

Heimdall relies on LLVM libraries for DWARF debug information extraction and LLD plugin integration. However, not all LLVM versions are compatible with all C++ standards, especially for older standards like C++11 and C++14. This document outlines the compatibility matrix, known issues, and possible solutions for users who need to build Heimdall with different C++ standards.

---

## Compatibility Matrix

| C++ Standard | LLVM Version(s) | Status                | Notes |
|--------------|----------------|----------------------|-------|
| C++11        | 7–18           | ✅ Supported*         | Use compatibility mode. LLD plugin not available with LLVM 19+ |
| C++14        | 7–18           | ✅ Supported*         | Use compatibility mode. LLD plugin not available with LLVM 19+ |
| C++17        | 11+            | ✅ Supported          | LLD plugin available with LLVM 11+ |
| C++20        | 19+            | ✅ Supported          | Full features, {fmt} required |
| C++23        | 19+            | ✅ Supported          | Full features |

> *C++11/14: LLVM 19+ is **not** compatible with C++11/14 due to use of C++14/17/20 features in LLVM headers. Use LLVM 7–18 for best results if you require C++11/14 support.

---

## Known Issues

- **LLVM 19+ and C++11/14:**
  - LLVM 19 headers and libraries use C++14/17/20 features (e.g., `std::optional`, `std::string_view`, `std::index_sequence`, `if constexpr`, etc.) that are not available in C++11/14.
  - Attempting to build Heimdall with C++11/14 and LLVM 19+ will result in compiler errors.
  - The LLD plugin cannot be built or used with C++11/14 and LLVM 19+.

- **LLD Plugin:**
  - The LLD plugin requires C++14+ and LLVM 11+.
  - For C++11/14 builds, the LLD plugin is not available; only the Gold plugin is supported.

- **DWARF Extraction:**
  - DWARF extraction via LLVM works with C++11/14 only if using LLVM 7–18.
  - LLVM 19+ requires C++17 or newer for all features.

---

## Solutions and Recommendations

### 1. Use an Older LLVM Version (7–18) for C++11/14
- If you need to build Heimdall with C++11 or C++14, install LLVM 7–18.
- On Ubuntu/Debian, you can install a specific version with:
  ```bash
  sudo apt-get install llvm-18-dev liblld-18-dev
  # or for older: llvm-11-dev, llvm-12-dev, etc.
  ```
- On Fedora/RHEL:
  ```bash
  sudo dnf install llvm-devel-18.1.5 lld-devel-18.1.5
  # or use the appropriate version
  ```
- Set CMake variables to point to the correct LLVM version if multiple are installed.

### 2. Use C++17 or Newer for LLVM 19+
- If you want to use the latest LLVM (19+), build Heimdall with C++17, C++20, or C++23.
- This enables all features, including the LLD plugin and full DWARF support.

### 3. Gold Plugin as a Fallback
- The Gold plugin is available for all supported C++ standards and does not require the latest LLVM.
- If you are limited to C++11/14, use the Gold plugin for SBOM generation.
- Note: Gold is not available on macOS; use LLD with C++17+ on macOS.

### 4. Wrapper Script Approach
- If you need to generate SBOMs with LLD but are limited by LLVM/C++ version compatibility, use a wrapper script to run SBOM generation as a post-link step instead of as a plugin.
- See `docs/lld-integration-rationale.md` for details.

---

## Best Practices

- **For maximum compatibility:**
  - Use C++17 or newer and LLVM 19+ if possible.
  - For legacy projects requiring C++11/14, use LLVM 7–18 and the Gold plugin.
  - Always check your compiler and LLVM versions with `llvm-config --version` and `c++ --version`.
  - If you encounter build errors related to missing C++ features, verify your LLVM version and C++ standard settings.

- **CI/CD:**
  - Test builds with all supported C++ standards and relevant LLVM versions.
  - Document your build matrix and environment in your CI configuration.

---

## References

- [LLVM Releases](https://releases.llvm.org/)
- [Heimdall Multi-Standard Support](./multi-standard-support.md)
- [Heimdall Limitations](./heimdall-limitations.md)
- [LLD Integration Rationale](./lld-integration-rationale.md)
- [Gold Plugin API](https://sourceware.org/binutils/docs-2.39/gold/Plugin.html)

---

For further help, see the main README or open an issue on the Heimdall project repository. 