# Heimdall Setup Scripts

This directory contains OS-specific setup scripts for installing Heimdall dependencies on different operating systems.

## Main Setup Script

### `setup.sh` - Interactive Menu Interface

The main setup script provides a user-friendly menu interface to select your operating system and install dependencies.

#### Usage

```bash
# Interactive menu (recommended)
sudo ./scripts/setup.sh

# Auto-detect OS and install
sudo ./scripts/setup.sh --auto-detect

# Show menu with specific options
sudo ./scripts/setup.sh --verbose
sudo ./scripts/setup.sh --dry-run
sudo ./scripts/setup.sh --skip-llvm
sudo ./scripts/setup.sh --skip-gcc
```

#### Features

- **Interactive Menu**: Select your OS from a numbered list
- **Auto-Detection**: Automatically detect your OS and run the appropriate script
- **OS Information**: Shows supported compiler versions and C++ standards for each OS
- **Confirmation**: Confirms your selection before proceeding
- **Help System**: Built-in help and documentation
- **Argument Passing**: Passes all arguments to the OS-specific scripts

#### Supported Operating Systems

| Menu Option | OS | Script | GCC Versions | LLVM Version |
|-------------|----|--------|--------------|--------------|
| 1 | Ubuntu 22.04+ | `setup-ubuntu.sh` | 11, 13 | 18, 19 |
| 2 | Debian Bookworm | `setup-debian.sh` | 11, 12 | 18 |
| 3 | Debian Testing | `setup-debian-testing.sh` | 12, 13, 14 | 18 |
| 4 | CentOS Stream 9 | `setup-centos.sh` | 11, 13, 14 | 20 |
| 5 | Fedora Latest | `setup-fedora.sh` | 15 | 18 |
| 6 | Arch Linux | `setup-arch.sh` | 14, 15 | 18 |
| 7 | OpenSUSE Tumbleweed | `setup-opensuse.sh` | 11, 13 | 18 |
| 8 | Rocky Linux 9 | `setup-rocky.sh` | 11, 13 | 16 |
| 9 | macOS | `setup-macos.sh` | Xcode | Homebrew LLVM |

## OS-Specific Setup Scripts

### Direct Usage

You can also run OS-specific scripts directly:

```bash
# Ubuntu
sudo ./scripts/setup-ubuntu.sh

# Debian
sudo ./scripts/setup-debian.sh

# CentOS
sudo ./scripts/setup-centos.sh

# Fedora
sudo ./scripts/setup-fedora.sh

# Arch Linux
sudo ./scripts/setup-arch.sh

# OpenSUSE
sudo ./scripts/setup-opensuse.sh

# Rocky Linux
sudo ./scripts/setup-rocky.sh

# macOS
./scripts/setup-macos.sh  # No sudo needed on macOS
```

### Common Options

All OS-specific scripts support these common options:

```bash
-h, --help              Show help message
-v, --verbose           Enable verbose output
-d, --dry-run          Show what would be installed without actually installing
--skip-llvm            Skip LLVM installation (use system LLVM if available)
--skip-gcc             Skip GCC installation (use system GCC)
--version              Show version information
```

### OS-Specific Options

Some scripts have additional OS-specific options:

```bash
# Ubuntu/Debian
--llvm-version VERSION  Set LLVM version to install (default: 18/19)

# Debian Testing
--gcc-version VERSION   Use specific GCC version (12, 13, 14)

# CentOS/Rocky
--enable-scl           Enable Software Collections for newer GCC versions
```

## C++ Standards Support

All setup scripts install dependencies for multiple C++ standards:

| C++ Standard | GCC Version | Clang Version | Status |
|--------------|-------------|---------------|--------|
| C++11 | 4.8+ | 3.3+ | ✅ Supported |
| C++14 | 5+ | 3.4+ | ✅ Supported |
| C++17 | 7+ | 5+ | ✅ Supported |
| C++20 | 10+ | 10+ | ✅ Supported |
| C++23 | 11+ | 14+ | ✅ Supported |

## Dependencies Installed

All scripts install the following core dependencies:

- **Build Tools**: make, cmake, ninja
- **Compilers**: GCC (multiple versions), LLVM/Clang
- **Development Libraries**: OpenSSL, ELF, Boost
- **Development Tools**: Python 3, pip, Git
- **Linker Support**: binutils-gold (Linux), LLD headers

## Troubleshooting

### Common Issues

1. **Permission Denied**
   ```bash
   # Linux: Run with sudo
   sudo ./scripts/setup.sh
   
   # macOS: No sudo needed
   ./scripts/setup.sh
   ```

2. **OS Not Detected**
   ```bash
   # Use manual selection
   ./scripts/setup.sh
   # Then select your OS from the menu
   ```

3. **Setup Script Not Found**
   ```bash
   # Check available scripts
   ls -la scripts/setup-*.sh
   
   # Run OS-specific script directly
   ./scripts/setup-[your-os].sh
   ```

4. **Package Manager Issues**
   ```bash
   # Update package lists first
   sudo apt update  # Ubuntu/Debian
   sudo dnf update  # CentOS/Rocky/Fedora
   sudo pacman -Syu # Arch Linux
   ```

### Platform-Specific Notes

#### Linux Distributions
- **Ubuntu/Debian**: Uses APT package manager
- **CentOS/Rocky/Fedora**: Uses DNF package manager
- **Arch Linux**: Uses Pacman package manager
- **OpenSUSE**: Uses Zypper package manager

#### macOS
- Requires Xcode Command Line Tools
- Uses Homebrew for LLVM installation
- No root privileges required

#### SCL (Software Collections)
- **CentOS/Rocky**: SCL provides newer GCC versions
- **Activation**: `scl enable gcc-toolset-14 bash`
- **Automatic**: Setup scripts handle SCL configuration

## Examples

### Quick Start (Recommended)
```bash
# Clone repository
git clone --recurse-submodules https://github.com/Heimdall-SBOM/heimdall.git
cd heimdall

# Run interactive setup
sudo ./scripts/setup.sh

# Select your OS from the menu
# Confirm installation
# Wait for completion
```

### Auto-Detection
```bash
# Auto-detect and install
sudo ./scripts/setup.sh --auto-detect
```

### Dry Run (Preview)
```bash
# See what would be installed
sudo ./scripts/setup.sh --dry-run
```

### Verbose Installation
```bash
# Detailed output during installation
sudo ./scripts/setup.sh --verbose
```

### Skip Components
```bash
# Skip LLVM installation
sudo ./scripts/setup.sh --skip-llvm

# Skip GCC installation
sudo ./scripts/setup.sh --skip-gcc
```

## Next Steps

After running the setup script:

1. **Build Heimdall**:
   ```bash
   ./scripts/build.sh --standard 17 --compiler gcc --tests
   ```

2. **Test Installation**:
   ```bash
   ./scripts/build_all_standards.sh
   ```

3. **Run Examples**:
   ```bash
   cd examples/heimdall-usage-example
   ./run_example.sh
   ```

## Contributing

To add support for a new operating system:

1. Create `setup-[os].sh` script
2. Follow the pattern of existing scripts
3. Add OS detection to `setup.sh`
4. Update this documentation
5. Test on the target platform

## Support

For issues with setup scripts:

1. Check the troubleshooting section above
2. Review OS-specific script help: `./scripts/setup-[os].sh --help`
3. Check system requirements and dependencies
4. Report issues on the project's GitHub page 