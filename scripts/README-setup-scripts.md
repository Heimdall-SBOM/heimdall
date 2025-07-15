# Heimdall Distribution-Specific Setup Scripts

This directory contains individual setup scripts for each supported Linux distribution. These scripts are based on the corresponding Dockerfiles in the `dockerfiles/` directory and provide a more targeted approach to dependency installation.

## Available Setup Scripts

| Distribution | Script | GCC Versions | LLVM Version | Notes |
|--------------|--------|--------------|--------------|-------|
| Ubuntu 22.04+ | `setup-ubuntu.sh` | 11, 13 | 18 | Uses Ubuntu Toolchain PPA |
| Debian Bookworm | `setup-debian.sh` | 11, 12 | 18 | Uses Debian backports |
| Debian Testing | `setup-debian-testing.sh` | 12, 13, 14 | 18 | Latest GCC versions |
| Fedora Latest | `setup-fedora.sh` | 15 | 18 | Uses default Fedora GCC |
| CentOS Stream 9 | `setup-centos.sh` | 11, 13, 14 | 20 | Uses SCL for newer GCC |
| Rocky Linux 9 | `setup-rocky.sh` | 11, 13 | 16 | Uses SCL for newer GCC |
| OpenSUSE Tumbleweed | `setup-opensuse.sh` | Latest | 18 | Uses latest available |
| Arch Linux | `setup-arch.sh` | 14, 15 | 18 | Uses latest available |

## Usage

Each script follows the same interface and can be used as follows:

```bash
# Basic usage
sudo ./setup-<distro>.sh

# With options
sudo ./setup-<distro>.sh --verbose --dry-run

# Skip certain components
sudo ./setup-<distro>.sh --skip-llvm --skip-gcc
```

## Common Options

All scripts support the following options:

- `-h, --help`: Show help message
- `-v, --verbose`: Enable verbose output
- `-d, --dry-run`: Show what would be installed without actually installing
- `--skip-llvm`: Skip LLVM installation (use system LLVM if available)
- `--skip-gcc`: Skip GCC installation (use system GCC)
- `--llvm-version VERSION`: Set LLVM version to install
- `--version`: Show version information

## Distribution-Specific Features

### Ubuntu
- Adds Ubuntu Toolchain PPA for GCC 11 and 13
- Downloads LLVM installation script from apt.llvm.org
- Creates symlinks for LLVM tools

### Debian (Bookworm)
- Adds Debian backports repository for newer GCC versions
- Uses apt.llvm.org for LLVM installation
- Creates symlinks for LLVM tools

### Debian Testing
- Has access to latest GCC versions (12, 13, 14)
- Uses system LLVM packages
- Sets up GCC alternatives for multiple versions

### Fedora
- Uses default Fedora GCC (currently 15)
- Installs Ninja manually (not in repos)
- Uses system LLVM packages

### CentOS Stream 9
- Enables CRB repository for newer GCC versions
- Uses Software Collections (SCL) for GCC 13 and 14
- Installs Ninja manually (not in repos)

### Rocky Linux 9
- Enables CRB repository for newer GCC versions
- Uses Software Collections (SCL) for GCC 13
- Installs Ninja manually (not in repos)

### OpenSUSE Tumbleweed
- Uses latest available GCC version
- Uses system LLVM packages
- Creates LLD symlinks if needed

### Arch Linux
- Uses latest available GCC version
- Installs specific GCC versions if needed
- Creates comprehensive symlinks for LLVM

## C++ Standards Support

All scripts support building with multiple C++ standards:

- **C++11**: Requires GCC 4.8+ or Clang 3.3+
- **C++14**: Requires GCC 5+ or Clang 3.4+
- **C++17**: Requires GCC 7+ or Clang 5+
- **C++20**: Requires GCC 10+ or Clang 10+
- **C++23**: Requires GCC 11+ or Clang 14+

## Dependencies Installed

Each script installs the following components:

- **Build tools**: make, cmake, ninja
- **GCC compilers**: Multiple versions as appropriate for the distribution
- **LLVM/Clang toolchain**: Version specified for each distribution
- **Development libraries**: OpenSSL, ELF, Boost
- **Python 3 and pip**: For build scripts and tools
- **Git**: For version control

## Verification

Each script includes a verification step that checks:
- GCC version availability
- LLVM/Clang installation
- Build tools (cmake, ninja)
- Python installation

## Troubleshooting

### Common Issues

1. **Permission denied**: Run scripts with `sudo`
2. **Package not found**: Update your package manager first
3. **LLVM installation fails**: Try using `--skip-llvm` and use system LLVM
4. **GCC version not available**: Use `--skip-gcc` and use system GCC

### Distribution-Specific Issues

- **CentOS/Rocky**: Use `scl enable gcc-toolset-<version> bash` to access newer GCC versions
- **Ubuntu**: If PPA fails, try updating GPG keys: `sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys <key>`
- **Arch**: If packages are not found, update first: `sudo pacman -Syu`

## Migration from Monolithic Script

If you were previously using the monolithic `setup.sh` script, you can now use the distribution-specific scripts for better reliability and faster installation. The new scripts:

- Are more targeted and efficient
- Have better error handling
- Include distribution-specific optimizations
- Provide clearer feedback and verification

## Contributing

When adding support for new distributions:

1. Create a corresponding Dockerfile in `dockerfiles/`
2. Create a setup script based on the Dockerfile
3. Update this README with the new distribution
4. Test the script on the target distribution

## Comparison with Monolithic Script

| Feature | Monolithic Script | Distribution-Specific Scripts |
|---------|-------------------|------------------------------|
| Complexity | High (740 lines) | Low (200-300 lines each) |
| Maintenance | Difficult | Easy |
| Reliability | Lower | Higher |
| Speed | Slower | Faster |
| Error handling | Generic | Specific |
| Testing | Complex | Simple |

The distribution-specific scripts provide a more maintainable and reliable approach to dependency installation. 