# NOTE: Modern Build System and Compatibility (2024)

> **Heimdall features a modern build system with automatic compiler and LLVM selection.**
>
> - All build scripts are in the `scripts/` directory.
> - The build system automatically detects and selects the best available compiler (GCC/Clang) and LLVM version for each C++ standard.
> - C++20/23 builds require GCC 13+ or Clang 14+ for `<format>` support.
> - Use `./scripts/show_build_compatibility.sh` to check which standards you can build and get installation instructions for missing components.
> - Build all compatible standards: `./scripts/build_all_standards.sh`
> - Build a specific standard: `./scripts/build.sh --standard 20 --all`
> - Clean all build artifacts: `./scripts/clean.sh`
>
> If a required version is missing, the build is skipped and a clear message is shown.

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

---

