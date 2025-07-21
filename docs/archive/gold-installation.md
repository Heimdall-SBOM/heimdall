# GNU Gold Linker Installation Guide

This guide provides detailed instructions for installing the GNU Gold linker on various Linux distributions for use with the Heimdall SBOM generator.

## What is GNU Gold?

GNU Gold is a modern ELF linker designed to be faster than the traditional GNU BFD linker. It's part of the GNU Binutils package and is the default linker on many modern Linux distributions.

## Platform Support

- **Linux**: ✅ Fully supported
- **macOS**: ❌ Not available (uses Mach-O format, not ELF)
- **Windows**: ❌ Not available (uses PE format, not ELF)

## Quick Installation

### Ubuntu/Debian (Recommended)

```bash
sudo apt-get update
sudo apt-get install binutils-gold
```

### Fedora/RHEL/CentOS

```bash
sudo yum install binutils-gold
# or for newer versions:
sudo dnf install binutils-gold
```

### Arch Linux

```bash
sudo pacman -S binutils
```

### OpenSUSE

```bash
sudo zypper install binutils-gold
```

## Verification

After installation, verify that Gold is available:

```bash
ld.gold --version
```

You should see output similar to:
```
GNU gold (GNU Binutils 2.44) 2.44
Copyright (C) 2024 Free Software Foundation, Inc.
This program is free software; you may redistribute it under the terms of
the GNU General Public License version 3 or (at your option) a later version.
This program has absolutely no warranty.
```

## Building from Source

If you need a specific version of Gold or your distribution doesn't provide it, you can build from source:

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential flex bison texinfo

# Fedora/RHEL/CentOS
sudo yum install gcc gcc-c++ make flex bison texinfo

# Arch Linux
sudo pacman -S base-devel flex bison texinfo
```

### Download and Build

```bash
# Download latest binutils (includes Gold)
wget https://ftp.gnu.org/gnu/binutils/binutils-2.44.tar.gz
tar xf binutils-2.44.tar.gz
cd binutils-2.44

# Configure with Gold and plugin support
./configure \
    --enable-gold \
    --enable-plugins \
    --prefix=/usr/local \
    --disable-werror

# Build (use all available cores)
make -j$(nproc)

# Install
sudo make install

# Update library cache
sudo ldconfig
```

### Verify Installation

```bash
/usr/local/bin/ld.gold --version
```

## Plugin Support

Heimdall requires Gold to be built with plugin support. Most modern distributions include this by default, but you can verify:

```bash
ld.gold --help | grep plugin
```

If you see plugin-related options, plugin support is enabled.

## Integration with Build Systems

### Using Gold as Default Linker

To make Gold the default linker for your system:

```bash
# Create symlink (be careful - this affects system-wide linking)
sudo ln -sf /usr/bin/ld.gold /usr/bin/ld

# Or for user-specific usage, add to your shell profile:
echo 'export PATH="/usr/local/bin:$PATH"' >> ~/.bashrc
```

### CMake Integration

```cmake
# Force CMake to use Gold
set(CMAKE_LINKER /usr/bin/ld.gold)
```

### Makefile Integration

```makefile
# Use Gold linker
LDFLAGS += -fuse-ld=gold
```

## Troubleshooting

### Gold Not Found

**Error**: `ld.gold: command not found`

**Solution**: Install binutils-gold package for your distribution.

### Plugin Support Missing

**Error**: `ld.gold: unrecognized option '--plugin'`

**Solution**: Rebuild binutils with `--enable-plugins` flag.

### Permission Denied

**Error**: `ld.gold: cannot open output file: Permission denied`

**Solution**: Check write permissions for the output directory.

### Architecture Mismatch

**Error**: `ld.gold: incompatible target`

**Solution**: Ensure you're using the correct architecture version (x86_64, ARM64, etc.).

## Performance Comparison

Gold typically provides faster linking times compared to the traditional BFD linker:

| Linker | Relative Speed | Memory Usage |
|--------|---------------|--------------|
| Gold   | 1.0x (baseline) | Lower |
| BFD    | 0.7-0.8x | Higher |

## Advanced Configuration

### Environment Variables

```bash
# Set Gold as default linker
export LD=ld.gold

# Enable verbose output
export LDFLAGS="-Wl,--verbose"

# Enable plugin debugging
export LDFLAGS="-Wl,--plugin-opt=verbose"
```

### Configuration Files

Create `~/.goldrc` for user-specific settings:

```
--plugin-opt=verbose
--plugin-opt=debug
```

## Security Considerations

- Gold is part of the GNU toolchain and is actively maintained
- Regular security updates are provided through distribution package managers
- When building from source, verify the source tarball checksums

## Support

For issues with Gold itself:

- **Documentation**: https://sourceware.org/binutils/docs/ld/
- **Bug Reports**: https://sourceware.org/bugzilla/
- **Mailing Lists**: https://sourceware.org/mailing-lists/

For Heimdall-specific issues, see the main README.md file.

## Migration from BFD Linker

If you're migrating from the traditional BFD linker:

1. Install Gold: `sudo apt-get install binutils-gold`
2. Test with a simple project first
3. Update build scripts to use Gold
4. Monitor for any compatibility issues
5. Update CI/CD pipelines to use Gold

Most projects work seamlessly with Gold, but some may require minor adjustments. 