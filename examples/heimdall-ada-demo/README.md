# Heimdall Ada Demo Application

This example demonstrates a complex Ada application with both statically and dynamically linked libraries. It is designed to generate rich SBOMs (Software Bill of Materials) using Heimdall, including:

- **CycloneDX 1.5**
- **SPDX 2.3**

## Features
- Multiple Ada source files and packages
- Static and dynamic library usage
- Data files and resources
- Designed to exercise SBOM generation for deep dependency and metadata extraction

## How to Build
- Build statically and dynamically linked versions using GNAT (GCC Ada)
- Example build commands will be provided in this directory

## How to Generate SBOMs
- Use `heimdall-sbom` with the built application and libraries
- Example commands for CycloneDX 1.5 and SPDX 2.3 will be provided

## Build Instructions

1. Install GNAT (GNU Ada compiler):
   - RHEL/Fedora: `sudo dnf install gcc-gnat`
   - Ubuntu/Debian: `sudo apt-get install gnat`
2. Build both static and dynamic versions:
   ```sh
   cd examples/heimdall-ada-demo
   make clean && make all
   ```
   - Statically linked: `bin/main_static`
   - Dynamically linked: `bin/main_dynamic`

## Generate SBOMs

1. CycloneDX 1.5:
   ```sh
   ../../build-cpp23/src/tools/heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so bin/main_static --format cyclonedx --cyclonedx-version 1.5 --output bin/main_static.cyclonedx.json
   ../../build-cpp23/src/tools/heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so bin/main_dynamic --format cyclonedx --cyclonedx-version 1.5 --output bin/main_dynamic.cyclonedx.json
   ```
2. SPDX 2.3:
   ```sh
   ../../build-cpp23/src/tools/heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so bin/main_static --format spdx --spdx-version 2.3 --output bin/main_static.spdx.json
   ../../build-cpp23/src/tools/heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so bin/main_dynamic --format spdx --spdx-version 2.3 --output bin/main_dynamic.spdx.json
   ```

## Structure
- `src/` - Ada source files
- `lib/` - Ada libraries (static and shared)
- `data/` - Data files for the application
- `bin/` - Built executables

## Goal
Demonstrate rich, real-world SBOMs with complex Ada code, multiple libraries, and data resources. 