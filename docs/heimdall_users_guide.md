# Heimdall User Guide

## Table of Contents
- [Introduction](#introduction)
  - [What is an SBOM?](#what-is-an-sbom)
  - [What is Heimdall?](#what-is-heimdall)
- [Getting Heimdall](#getting-heimdall)
- [Building Heimdall from Source](#building-heimdall-from-source)
  - [Prerequisites](#prerequisites)
  - [Build Steps](#build-steps)
- [Running Tests](#running-tests)
- [Using Heimdall in Your Project](#using-heimdall-in-your-project)
  - [CMake Integration](#cmake-integration)
  - [Makefile Integration](#makefile-integration)
  - [Command-Line Usage](#command-line-usage)
- [Generating SBOMs](#generating-sboms)
  - [CycloneDX Generation](#cyclonedx-generation)
  - [SPDX Generation](#spdx-generation)
- [Troubleshooting & FAQ](#troubleshooting--faq)
- [Further Reading](#further-reading)

---

## Introduction

### What is an SBOM?
A **Software Bill of Materials (SBOM)** is a detailed list of all components, libraries, and dependencies that make up a software application. SBOMs are essential for:
- Security: Identifying vulnerable components
- Compliance: Ensuring license compatibility
- Transparency: Understanding supply chain risks

Common SBOM formats include **SPDX** and **CycloneDX**.

### What is Heimdall?
**Heimdall** is an open-source SBOM generator designed for C/C++ projects. It can extract metadata from binaries, libraries, and source code, and generate SBOMs in both SPDX and CycloneDX formats. Heimdall supports integration as a linker plugin (LLD/Gold), as a library, or as a standalone tool.

---

## Getting Heimdall

Clone the official repository from GitHub:

```bash
git clone https://github.com/Heimdall-SBOM/heimdall.git
cd heimdall
```

---

## Building Heimdall from Source

### Prerequisites
- **CMake** (>= 3.16)
- **C++17** compatible compiler (GCC or Clang)
- **OpenSSL** development libraries
- **LLVM/LLD** (for LLD plugin support)
- **binutils-dev** (for Gold plugin support)
- **GoogleTest** (automatically downloaded if not found)

On Ubuntu/Debian, install dependencies:
```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev llvm-dev lld binutils-dev
```

On Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake openssl-devel llvm-devel lld binutils-devel
```

### Build Steps
You can use the provided build script or build manually with CMake.

#### Using the Build Script
```bash
./build.sh
```

#### Manual Build
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

#### Optional Build Flags
- `--debug` : Build in debug mode
- `--no-lld` : Disable LLD plugin
- `--no-gold` : Disable Gold plugin
- `--no-tests` : Skip building tests
- `--no-examples` : Skip building examples

---

## Running Tests

After building, run the test suite:

```bash
cd build
ctest --verbose
```

Or, using the build script:
```bash
./build.sh  # Tests run automatically unless --no-tests is specified
```

You can also run the test executable directly:
```bash
./build/tests/heimdall_tests
```

---

## Using Heimdall in Your Project

### CMake Integration
Add Heimdall as a dependency in your `CMakeLists.txt`:

```cmake
find_package(Heimdall REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE heimdall-core)
```

If using the plugin, ensure the plugin `.so` files are available and loaded by your linker.

### Makefile Integration
Assuming Heimdall is installed to `/usr/local` or your custom prefix:

```makefile
CXXFLAGS += -I/usr/local/include/heimdall
LDFLAGS  += -L/usr/local/lib -lheimdall-core

myapp: main.o
	$(CXX) $(CXXFLAGS) -o myapp main.o $(LDFLAGS)
```

For plugin usage, pass the plugin to your linker (see below).

### Command-Line Usage
You can use Heimdall as a linker plugin or as a standalone tool.

#### LLD Plugin Example
```bash
ld.lld --plugin-opt=load:./heimdall-lld.so \
      --plugin-opt=sbom-output:myapp.json \
      main.o -o myapp
```

#### Gold Plugin Example
```bash
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.json \
        main.o -o myapp
```

#### Standalone (Planned)
A standalone CLI is planned for future releases. For now, use the plugin or library integration.

---

## Generating SBOMs

Heimdall can generate SBOMs in both **CycloneDX** and **SPDX** formats. The output format is controlled by the `--plugin-opt` or `--plugin-opt` flag.

### CycloneDX Generation

**LLD Plugin:**
```bash
ld.lld --plugin-opt=load:./heimdall-lld.so \
      --plugin-opt=sbom-output:myapp.cyclonedx.json \
      --plugin-opt=sbom-format=cyclonedx \
      main.o -o myapp
```

**Gold Plugin:**
```bash
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.cyclonedx.json \
        --plugin-opt sbom-format=cyclonedx \
        main.o -o myapp
```

### SPDX Generation

**LLD Plugin:**
```bash
ld.lld --plugin-opt=load:./heimdall-lld.so \
      --plugin-opt=sbom-output:myapp.spdx.json \
      --plugin-opt=sbom-format=spdx \
      main.o -o myapp
```

**Gold Plugin:**
```bash
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.spdx.json \
        --plugin-opt sbom-format=spdx \
        main.o -o myapp
```

---

## Troubleshooting & FAQ

**Q: I get linker errors about missing Heimdall libraries.**
- Make sure you have installed Heimdall (`make install` or `./build.sh` with install step).
- Check your `LD_LIBRARY_PATH` or system library path includes the Heimdall install location.

**Q: The plugin is not generating an SBOM.**
- Ensure you are passing the correct `--plugin-opt` flags.
- Check that the plugin `.so` file is in the path you specify.
- Run with `--plugin-opt=verbose=1` for more debug output (if supported).

**Q: How do I generate both SPDX and CycloneDX?**
- Run the linker twice with different `sbom-format` options and output files.

**Q: Can I use Heimdall with other build systems?**
- Yes! As long as you can link to the core library or load the plugin, you can use Heimdall.

---

## Further Reading
- [Heimdall GitHub Repository](https://github.com/Heimdall-SBOM/heimdall)
- [SPDX Specification](https://spdx.dev/specifications/)
- [CycloneDX Specification](https://cyclonedx.org/specification/)
- [OpenSSF SBOM Resources](https://openssf.org/sbom/) 