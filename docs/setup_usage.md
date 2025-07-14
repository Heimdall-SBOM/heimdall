# Heimdall Setup Script Usage Guide

This document provides detailed instructions for using the `setup.sh` script to install all necessary dependencies for building Heimdall on various Linux distributions.

## Overview

The `setup.sh` script automatically detects your Linux distribution and installs all required dependencies for building Heimdall with support for C++11, C++14, C++17, C++20, and C++23 standards.

## Prerequisites

- Root access (the script must be run with `sudo`)
- Internet connection for downloading packages and LLVM headers
- At least 2GB of free disk space for all dependencies

## Basic Usage

### Quick Start

```bash
# Clone the repository
git clone https://github.com/your-org/heimdall.git
cd heimdall

# Run the setup script
sudo ./setup.sh
```

### Command Line Options

```bash
./setup.sh [OPTIONS]

Options:
  -h, --help              Show help message
  -v, --verbose           Enable verbose output
  -d, --dry-run          Show what would be installed without installing
  --skip-llvm            Skip LLVM installation (use system LLVM if available)
  --skip-gcc             Skip GCC installation (use system GCC)
  --version              Show version information
```

## Distribution-Specific Examples

### Ubuntu 22.04+

Ubuntu provides excellent support for multiple GCC and LLVM versions through the Ubuntu Toolchain PPA.

#### What the script installs:
- **GCC versions**: 11, 13 (via Ubuntu Toolchain PPA)
- **LLVM version**: 18
- **Build tools**: build-essential, cmake, ninja-build
- **Development libraries**: libssl-dev, libelf-dev, libboost-filesystem-dev, libboost-system-dev

#### Example usage:

```bash
# Basic installation
sudo ./setup.sh

# With verbose output to see what's happening
sudo ./setup.sh --verbose

# Dry run to see what would be installed
sudo ./setup.sh --dry-run

# Skip LLVM installation (if you already have LLVM 18)
sudo ./setup.sh --skip-llvm
```

#### Manual verification after installation:

```bash
# Check GCC versions
gcc --version
gcc-11 --version
gcc-13 --version

# Check LLVM
llvm-config --version

# Check build tools
cmake --version
ninja --version

# Check development libraries
pkg-config --exists openssl && echo "OpenSSL found"
pkg-config --exists libelf && echo "ELF library found"
```

#### Building Heimdall on Ubuntu:

```bash
# After running setup.sh, build Heimdall

# Build with C++17 (default)
./scripts/build.sh --standard 17 --compiler gcc --tests

# Build with C++20
./scripts/build.sh --standard 20 --compiler gcc --tests

# Build with C++23
./scripts/build.sh --standard 23 --compiler gcc --tests

# Build with Clang instead of GCC
./scripts/build.sh --standard 17 --compiler clang --tests
```

### Rocky Linux 9

Rocky Linux 9 uses Software Collections (SCL) for newer GCC versions and provides LLVM 16 through the default repositories.

#### What the script installs:
- **GCC versions**: 11 (default), 13 (via SCL)
- **LLVM version**: 16
- **Build tools**: gcc, gcc-c++, cmake, ninja (manual download)
- **Development libraries**: openssl-devel, elfutils-libelf-devel, boost-devel

#### Example usage:

```bash
# Basic installation
sudo ./setup.sh

# With verbose output
sudo ./setup.sh --verbose

# Dry run
sudo ./setup.sh --dry-run

# Skip GCC installation (use system GCC only)
sudo ./setup.sh --skip-gcc
```

#### Manual verification after installation:

```bash
# Check default GCC
gcc --version

# Check SCL GCC (if installed)
scl enable gcc-toolset-13 -- gcc --version

# Check LLVM
llvm-config --version

# Check build tools
cmake --version
ninja --version
```

#### Building Heimdall on Rocky Linux:

```bash
# For C++11 and C++17 (using default GCC)
./scripts/build.sh --standard 17 --compiler gcc --tests

# For C++20/23 (requires SCL activation)
scl enable gcc-toolset-13 bash
./scripts/build.sh --standard 23 --compiler gcc --tests
```

### CentOS Stream 9

CentOS Stream 9 is similar to Rocky Linux but provides LLVM 20 and additional GCC versions through the CRB repository.

#### What the script installs:
- **GCC versions**: 11 (default), 13, 14 (via SCL)
- **LLVM version**: 20
- **Build tools**: gcc, gcc-c++, cmake, ninja (manual download)
- **Development libraries**: openssl-devel, elfutils-libelf-devel, boost-devel

#### Example usage:

```bash
# Basic installation
sudo ./setup.sh

# With verbose output
sudo ./setup.sh --verbose

# Dry run
sudo ./setup.sh --dry-run

# Skip LLVM installation
sudo ./setup.sh --skip-llvm
```

#### Manual verification after installation:

```bash
# Check default GCC
gcc --version

# Check SCL GCC versions
scl enable gcc-toolset-13 -- gcc --version
scl enable gcc-toolset-14 -- gcc --version

# Check LLVM
llvm-config --version

# Check build tools
cmake --version
ninja --version
```

#### Building Heimdall on CentOS:

```bash
# For C++11 and C++17 (using default GCC)
./scripts/build.sh --standard 17 --compiler gcc --tests

# For C++20 (using GCC 13)
scl enable gcc-toolset-13 bash
./scripts/build.sh --standard 20 --compiler gcc --tests

# For C++23 (using GCC 14)
scl enable gcc-toolset-14 bash
./scripts/build.sh --standard 23 --compiler gcc --tests
```

### Arch Linux

Arch Linux provides the latest versions of GCC and LLVM through its rolling release model.

#### What the script installs:
- **GCC versions**: 14, 15 (default)
- **LLVM version**: 18
- **Build tools**: base-devel, cmake, ninja
- **Development libraries**: openssl, elfutils, boost, boost-libs

### OpenSUSE Tumbleweed

OpenSUSE Tumbleweed is a rolling release distribution that provides recent versions of GCC and LLVM.

#### What the script installs:
- **GCC version**: Latest (default OpenSUSE GCC)
- **LLVM version**: 18
- **Build tools**: gcc, gcc-c++, cmake, ninja
- **Development libraries**: openssl-devel, libelf-devel, boost-devel

#### Example usage:

```bash
# Basic installation
sudo ./setup.sh

# With verbose output
sudo ./setup.sh --verbose

# Dry run
sudo ./setup.sh --dry-run

# Skip GCC installation (use default GCC only)
sudo ./setup.sh --skip-gcc
```

#### Manual verification after installation:

```bash
# Check default GCC
gcc --version

# Check GCC 14 (if installed)
gcc-14 --version

# Check LLVM
llvm-config --version

# Check build tools
cmake --version
ninja --version
```

#### Building Heimdall on Arch:

```bash
# For all C++ standards (Arch has latest GCC)

# Build with C++17
./scripts/build.sh --standard 17 --compiler gcc --tests

# Build with C++20
./scripts/build.sh --standard 20 --compiler gcc --tests

# Build with C++23
./scripts/build.sh --standard 23 --compiler gcc --tests

# Build with Clang instead of GCC
./scripts/build.sh --standard 17 --compiler clang --tests
```

#### Building Heimdall on OpenSUSE:

```bash
# For all C++ standards (OpenSUSE has recent GCC)

# Build with C++17
./scripts/build.sh --standard 17 --compiler gcc --tests

# Build with C++20
./scripts/build.sh --standard 20 --compiler gcc --tests

# Build with C++23
./scripts/build.sh --standard 23 --compiler gcc --tests

# Build with Clang instead of GCC
./scripts/build.sh --standard 17 --compiler clang --tests
```

## Advanced Usage

### Dry Run Mode

Use dry run mode to see what the script would install without actually installing anything:

```bash
sudo ./setup.sh --dry-run
```

This is useful for:
- Understanding what packages will be installed
- Checking if your system is supported
- Planning disk space requirements
- Troubleshooting before actual installation

### Verbose Mode

Enable verbose output to see detailed information about the installation process:

```bash
sudo ./setup.sh --verbose
```

This shows:
- Package manager commands being executed
- Download progress for manual installations
- Symlink creation details
- Verification steps

### Skipping Components

#### Skip LLVM Installation

If you already have a compatible LLVM version installed:

```bash
sudo ./setup.sh --skip-llvm
```

This is useful when:
- You have LLVM installed from a different source
- You want to use a system-provided LLVM version
- You're troubleshooting LLVM-related issues

#### Skip GCC Installation

If you want to use only the system's default GCC:

```bash
sudo ./setup.sh --skip-gcc
```

This is useful when:
- You only need one GCC version
- You want to minimize disk usage
- You're building for a specific C++ standard only

### Combining Options

You can combine multiple options:

```bash
# Verbose dry run
sudo ./setup.sh --verbose --dry-run

# Skip both LLVM and GCC, verbose output
sudo ./setup.sh --skip-llvm --skip-gcc --verbose
```

## Troubleshooting

### Common Issues

#### Permission Denied
```bash
[ERROR] This script must be run as root (use sudo)
```
**Solution**: Run the script with `sudo`:
```bash
sudo ./setup.sh
```

#### Unsupported Distribution
```bash
[ERROR] Unsupported distribution: <distro>
```
**Solution**: The script supports Ubuntu, Debian, CentOS, Rocky, Fedora, Arch, and OpenSUSE. For other distributions, you may need to install dependencies manually.

#### Package Not Found
```bash
[ERROR] Package <package-name> not found
```
**Solution**: 
1. Update your package manager: `sudo apt update` or `sudo dnf update`
2. Check if the package name is correct for your distribution
3. Try running with `--verbose` to see more details

#### LLVM Installation Issues
```bash
[ERROR] LLVM installation failed
```
**Solution**:
1. Check your internet connection
2. Try running with `--skip-llvm` and install LLVM manually
3. Check if your distribution supports the required LLVM version

### Manual Dependency Installation

If the setup script fails, you can install dependencies manually:

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build python3 python3-pip git
sudo apt-get install -y gcc-11 g++-11 gcc-13 g++-13
sudo apt-get install -y libssl-dev libelf-dev pkg-config libboost-filesystem-dev libboost-system-dev
```

#### CentOS/Rocky
```bash
sudo dnf install -y gcc gcc-c++ cmake python3 python3-pip git
sudo dnf install -y openssl-devel elfutils-libelf-devel pkgconfig boost-devel
sudo dnf config-manager --set-enabled crb
sudo dnf install -y gcc-toolset-13 gcc-toolset-14
```

#### Arch Linux
```bash
sudo pacman -S --noconfirm base-devel cmake ninja python python-pip git
sudo pacman -S --noconfirm openssl elfutils pkg-config boost boost-libs
sudo pacman -S --noconfirm gcc14 llvm18 lld18
```

#### OpenSUSE Tumbleweed
```bash
sudo zypper install -y gcc gcc-c++ cmake ninja python3 python3-pip git
sudo zypper install -y openssl-devel libelf-devel pkg-config boost-devel
sudo zypper install -y llvm18-devel lld18
```

## Verification

After running the setup script, verify the installation:

```bash
# Check GCC
gcc --version

# Check G++
g++ --version

# Check CMake
cmake --version

# Check Ninja
ninja --version

# Check LLVM (if installed)
llvm-config --version

# Check Python
python3 --version

# Check Git
git --version
```

## Next Steps

After successful installation:

1. **Clone the repository** (if not already done):
   ```bash
   git clone https://github.com/your-org/heimdall.git
   cd heimdall
   ```

2. **Build Heimdall**:
   ```bash
   # Build with C++17 (recommended)
   ./scripts/build.sh --standard 17 --compiler gcc --tests
   
   # Or build all compatible standards
   ./scripts/build_all_standards.sh
   ```

3. **Check build compatibility** (optional):
   ```bash
   ./scripts/show_build_compatibility.sh
   ```

4. **Install Heimdall** (optional):
   ```bash
   # After building, install from the build directory
   cd build-cpp17  # or whichever standard you built
   sudo make install
   ```

## Support

If you encounter issues with the setup script:

1. Check the troubleshooting section above
2. Run with `--verbose` to get more detailed output
3. Try the `--dry-run` option to see what would be installed
4. Report issues on the project's GitHub page with:
   - Your distribution and version
   - The exact error message
   - The output of `./setup.sh --verbose`

## Distribution Support Matrix

| Distribution | GCC Versions | LLVM Version | C++ Standards | Notes |
|--------------|--------------|--------------|---------------|-------|
| Ubuntu 22.04+ | 11, 13 | 18 | 11, 14, 17, 20, 23 | Full support |
| Debian Bookworm | 11, 12 | 18 | 11, 14, 17, 20 | Limited C++23 |
| Debian Testing | 12, 13, 14 | 18 | 11, 14, 17, 20, 23 | Full support |
| CentOS Stream 9 | 11, 13, 14 | 20 | 11, 14, 17, 20, 23 | SCL required for C++20/23 |
| Rocky Linux 9 | 11, 13 | 16 | 11, 14, 17, 20 | SCL required for C++20 |
| Fedora Latest | 15 | 18 | 11, 14, 17, 20, 23 | Full support |
| Arch Linux | 14, 15 | 18 | 11, 14, 17, 20, 23 | Full support |
| OpenSUSE Tumbleweed | 11, 13 | 18 | 11, 14, 17, 20, 23 | Full support | 