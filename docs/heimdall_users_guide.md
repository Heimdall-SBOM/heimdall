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

### LLD Plugin
The **LLD Plugin** integrates with LLVM's LLD linker to generate SBOMs during the linking process.

**Benefits:**
- **Cross-platform support**: Works on Linux, macOS, and Windows
- **Fast linking**: LLD is significantly faster than traditional linkers
- **Modern toolchain**: Part of the LLVM ecosystem with active development
- **DWARF support**: Excellent debug information extraction capabilities
- **Plugin architecture**: Clean integration with LLD's plugin system

**Limitations:**
- **LLVM dependency**: Requires LLVM/LLD to be installed
- **Learning curve**: Different command-line syntax from traditional linkers
- **Ecosystem integration**: May require adjustments to existing build systems

**Best for:** Modern C++ projects, cross-platform development, projects already using LLVM toolchain.

### Gold Plugin
The **Gold Plugin** integrates with GNU Gold linker to generate SBOMs during the linking process.

**Benefits:**
- **Linux native**: Designed specifically for Linux ELF binaries
- **Mature ecosystem**: Well-established in Linux development
- **Performance**: Optimized for Linux systems
- **BFD support**: Full Binary File Descriptor library integration
- **Traditional workflow**: Familiar to Linux developers

**Limitations:**
- **Linux only**: Not available on macOS or Windows
- **ELF format only**: Limited to Linux binary formats
- **GNU dependency**: Requires GNU binutils and BFD libraries

**Best for:** Linux-only projects, traditional Linux development workflows, projects requiring BFD features.

### Choosing Between Plugins

**Use LLD Plugin if:**
- You need cross-platform support (Linux, macOS, Windows)
- You're already using LLVM toolchain
- You want the fastest linking performance
- You need advanced DWARF debug information extraction

**Use Gold Plugin if:**
- You're developing Linux-only applications
- You prefer traditional GNU toolchain
- You need BFD library features
- You want maximum compatibility with existing Linux build systems

**Use Both if:**
- You're developing cross-platform software
- You want to compare SBOM generation between linkers
- You need to support different build environments

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
- **C++17** compatible compiler (GCC 7+, Clang 5+)
- **CMake** 3.16+
- **LLVM/LLD** development libraries
- **GNU Gold linker** (Linux only, optional for Gold plugin)
- **OpenSSL** development libraries
- **GoogleTest** (automatically downloaded if not found)

### macOS Setup
```bash
# Install LLVM/LLD and other dependencies
brew install llvm cmake ninja openssl
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build Heimdall (LLD plugin only)
./build.sh
```

### Linux Setup

#### Ubuntu/Debian
```bash
# Install all dependencies including Gold linker
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    binutils \
    binutils-dev \
    libssl-dev \
    libelf-dev \
    libgtest-dev \
    pkg-config

# Install LLVM 19 (recommended for full DWARF support)
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" | sudo tee /etc/apt/sources.list.d/llvm.list
sudo apt-get update
sudo apt-get install -y llvm-19-dev liblld-19-dev

# Build Heimdall with both LLD and Gold plugins
./build.sh
```

**Note**: 
- LLVM 19 is recommended for full DWARF debug info support
- `binutils` provides the Gold linker (`ld.gold`)
- `binutils-dev` provides the BFD development files needed for the Gold plugin
- `libgtest-dev` provides GoogleTest for unit testing (automatically downloaded if not available)

### Installing GNU Gold Linker

GNU Gold is the default linker on most modern Linux distributions. If not available:

**Ubuntu/Debian:**
```bash
sudo apt-get install binutils-gold
```

**Fedora/RHEL/CentOS:**
```bash
sudo yum install binutils-gold
```

**Arch Linux:**
```bash
sudo pacman -S binutils
```

**Verify Gold installation:**
```bash
ld.gold --version
```

**Note**: GNU Gold is not available on macOS. Use LLVM LLD linker instead, which provides similar performance benefits and better macOS integration.

**Troubleshooting BFD headers**: If you encounter "bfd.h not found" errors:
```bash
sudo apt-get install -y binutils-dev libbfd-dev
```

**Alternative LLVM versions**:
```bash
# For LLVM 14 (Ubuntu default)
sudo apt-get install -y llvm-dev liblld-14-dev

# For LLVM 15
sudo apt-get install -y llvm-15-dev liblld-15-dev

# For LLVM 16
sudo apt-get install -y llvm-16-dev liblld-16-dev
```

#### Fedora/RHEL/CentOS
```bash
# Install dependencies (required for build and tests)
sudo yum install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    llvm-devel \
    lld-devel \
    binutils-gold \
    openssl-devel \
    pkgconfig \
    zlib-devel \
    llvm-googletest \
    binutils-devel

# Build Heimdall
./build.sh
```

**Note**: The `binutils-devel` package provides the BFD headers required for the Gold plugin.

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S \
    base-devel \
    cmake \
    ninja \
    llvm \
    lld \
    binutils \
    openssl \
    pkgconf

# Build Heimdall
./build.sh
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
- `--sanitizers` : Enable AddressSanitizer and UBSan
- `--no-lld` : Disable LLD plugin
- `--no-gold` : Disable Gold plugin
- `--no-tests` : Skip building tests
- `--no-examples` : Skip building examples
- `--build-dir DIR` : Set custom build directory
- `--install-dir DIR` : Set custom install directory

#### Build Examples
```bash
# Debug build with sanitizers
./build.sh --debug --sanitizers

# Build only LLD plugin (macOS default)
./build.sh --no-gold

# Build only Gold plugin (Linux only)
./build.sh --no-lld

# Custom build directory
./build.sh --build-dir mybuild --install-dir myinstall
```

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
Configure your CMake project to use the Heimdall plugins during linking:

```cmake
# Find the Heimdall plugin libraries
find_library(HEIMDALL_LLD heimdall-lld)
find_library(HEIMDALL_GOLD heimdall-gold)

# For LLD linker
if(HEIMDALL_LLD)
    target_link_options(myapp PRIVATE
        "LINKER:--plugin-opt=load:${HEIMDALL_LLD}"
        "LINKER:--plugin-opt=sbom-output:${CMAKE_BINARY_DIR}/myapp.json"
        "LINKER:--plugin-opt=sbom-format=spdx"
    )
endif()

# For Gold linker (Linux only)
if(HEIMDALL_GOLD AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(myapp PRIVATE
        "LINKER:--plugin ${HEIMDALL_GOLD}"
        "LINKER:--plugin-opt sbom-output=${CMAKE_BINARY_DIR}/myapp.json"
        "LINKER:--plugin-opt sbom-format=spdx"
    )
endif()
```

### Makefile Integration
Configure your Makefile to use the Heimdall plugins during linking:

```makefile
# LLD plugin usage
LDFLAGS += -Wl,--plugin-opt=load:/usr/lib/heimdall-plugins/heimdall-lld.so
LDFLAGS += -Wl,--plugin-opt=sbom-output:$(TARGET).json
LDFLAGS += -Wl,--plugin-opt=sbom-format=spdx

# Gold plugin usage (Linux only)
LDFLAGS += -Wl,--plugin /usr/lib/heimdall-plugins/heimdall-gold.so
LDFLAGS += -Wl,--plugin-opt sbom-output=$(TARGET).json
LDFLAGS += -Wl,--plugin-opt sbom-format=spdx

myapp: main.o
	$(CXX) $(CXXFLAGS) -o myapp main.o $(LDFLAGS)
```

### Command-Line Usage
You can use Heimdall as a linker plugin with both LLD and Gold linkers. Here's a complete example using a C++ file named `myapp.cpp`.

#### Complete Example: Compile and Link with SBOM Generation

First, create a simple C++ application:
```cpp
// myapp.cpp
#include <iostream>
#include <string>

int main() {
    std::string message = "Hello from Heimdall SBOM!";
    std::cout << message << std::endl;
    return 0;
}
```

#### LLD Plugin Example
```bash
# Step 1: Compile the C++ file to object code
clang++ -c myapp.cpp -o myapp.o

# Step 2: Link with LLD and generate SPDX SBOM
ld.lld --plugin-opt=load:./heimdall-lld.so \
      --plugin-opt=sbom-output:myapp.spdx.json \
      --plugin-opt=sbom-format=spdx \
      --plugin-opt=verbose=1 \
      myapp.o -o myapp

# Step 3: Link with LLD and generate CycloneDX SBOM
ld.lld --plugin-opt=load:./heimdall-lld.so \
      --plugin-opt=sbom-output:myapp.cyclonedx.json \
      --plugin-opt=sbom-format=cyclonedx \
      --plugin-opt=verbose=1 \
      myapp.o -o myapp
```

#### Gold Plugin Example (Linux only)
```bash
# Step 1: Compile the C++ file to object code
g++ -c myapp.cpp -o myapp.o

# Step 2: Link with Gold and generate SPDX SBOM
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.spdx.json \
        --plugin-opt sbom-format=spdx \
        --plugin-opt verbose=1 \
        myapp.o -o myapp

# Step 3: Link with Gold and generate CycloneDX SBOM
ld.gold --plugin ./heimdall-gold.so \
        --plugin-opt sbom-output=myapp.cyclonedx.json \
        --plugin-opt sbom-format=cyclonedx \
        --plugin-opt verbose=1 \
        myapp.o -o myapp
```

#### One-Line Compilation with SBOM Generation
```bash
# LLD: Compile and link in one step with SPDX SBOM
clang++ myapp.cpp -o myapp \
    -Wl,--plugin-opt=load:./heimdall-lld.so \
    -Wl,--plugin-opt=sbom-output:myapp.spdx.json \
    -Wl,--plugin-opt=sbom-format=spdx

# Gold: Compile and link in one step with CycloneDX SBOM
g++ myapp.cpp -o myapp \
    -Wl,--plugin=./heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=myapp.cyclonedx.json \
    -Wl,--plugin-opt=sbom-format=cyclonedx
```

#### Plugin Options
- `sbom-output`: Output file path for the SBOM
- `sbom-format`: Output format (`spdx` or `cyclonedx`)
- `verbose`: Enable verbose output (`1` for enabled, `0` for disabled)
- `extract-debug-info`: Extract debug information (`1` for enabled, `0` for disabled)
- `include-system-libraries`: Include system libraries in SBOM (`1` for enabled, `0` for disabled)

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