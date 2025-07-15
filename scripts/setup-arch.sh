#!/bin/bash

# Heimdall Arch Linux Setup Script
# This script installs all necessary dependencies for building Heimdall on Arch Linux
# Based on dockerfiles/Dockerfile.arch

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script version
VERSION="1.0.0"

# Global variables
VERBOSE=false
DRY_RUN=false
SKIP_LLVM=false
SKIP_GCC=false
LLVM_VERSION="18"
LLVM_TAG="llvmorg-18.1.8"

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_subheader() {
    echo -e "${CYAN}$1${NC}"
}

# Function to show help
show_help() {
    cat << EOF
Heimdall Arch Linux Setup Script v${VERSION}

This script installs all necessary dependencies for building Heimdall on Arch Linux.

USAGE:
    ./setup-arch.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --llvm-version VERSION Set LLVM version to install (default: 18)
    --gcc-version VERSION  Set GCC version to use (14, 15)
    --version              Show version information

SUPPORTED ARCH LINUX VERSIONS:
    Arch Linux Latest       - GCC 14, 15 + LLVM 18

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup-arch.sh                    # Install all dependencies
    ./setup-arch.sh --verbose          # Install with verbose output
    ./setup-arch.sh --dry-run          # Show what would be installed
    ./setup-arch.sh --skip-llvm        # Skip LLVM installation
    ./setup-arch.sh --gcc-version 14   # Use GCC 14

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compilers (14, 15)
    - LLVM/Clang toolchain (18)
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/your-repo/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall Arch Linux Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to check Arch Linux version
check_arch_version() {
    print_subheader "Checking Arch Linux version..."
    
    if [ ! -f /etc/os-release ]; then
        print_error "This script is designed for Arch Linux systems"
        exit 1
    fi
    
    . /etc/os-release
    if [ "$ID" != "arch" ]; then
        print_error "This script is designed for Arch Linux systems, detected: $ID"
        exit 1
    fi
    
    print_status "Detected Arch Linux $VERSION_ID"
}

# Function to install prerequisites
install_prerequisites() {
    print_subheader "Installing prerequisites..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would run: pacman -Syu --noconfirm"
        echo "Would install: base-devel wget curl"
    else
        pacman -Syu --noconfirm
        pacman -S --noconfirm base-devel wget curl
    fi
}

# Function to install build tools
install_build_tools() {
    print_subheader "Installing build tools..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: base-devel cmake ninja python python-pip git"
    else
        pacman -S --noconfirm base-devel cmake ninja python python-pip git
    fi
}

# Function to install development libraries
install_dev_libs() {
    print_subheader "Installing development libraries..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: openssl elfutils pkg-config boost boost-libs"
    else
        pacman -S --noconfirm openssl elfutils pkg-config boost boost-libs
    fi
}

# Function to install GCC versions
install_gcc_versions() {
    if [ "$SKIP_GCC" = true ]; then
        print_warning "Skipping GCC installation as requested"
        return
    fi
    
    print_subheader "Installing GCC versions..."
    
    # Install GCC versions (Arch has latest by default, but we can install specific versions)
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: gcc14"
    else
        pacman -S --noconfirm gcc14
    fi
    
    print_status "Available GCC versions: 14, 15 (default)"
}

# Function to install LLVM
install_llvm() {
    if [ "$SKIP_LLVM" = true ]; then
        print_warning "Skipping LLVM installation as requested"
        return
    fi
    
    print_subheader "Installing LLVM ${LLVM_VERSION}..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: llvm${LLVM_VERSION} lld${LLVM_VERSION}"
    else
        pacman -S --noconfirm llvm${LLVM_VERSION} lld${LLVM_VERSION}
    fi
}

# Function to create LLVM symlinks
create_llvm_symlinks() {
    if [ "$SKIP_LLVM" = true ]; then
        return
    fi
    
    print_subheader "Creating LLVM symlinks..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would create symlinks for llvm-config, ld.lld, and lld"
        echo "Would create symlinks for CMake config files"
        echo "Would create symlinks for LLVM headers"
    else
        ln -sf /usr/bin/llvm-config-${LLVM_VERSION} /usr/bin/llvm-config
        ln -sf /usr/bin/ld.lld-${LLVM_VERSION} /usr/bin/ld.lld
        ln -sf /usr/bin/lld-${LLVM_VERSION} /usr/bin/lld

        # Create symlinks for CMake config files so they can be found automatically
        mkdir -p /usr/lib/cmake
        ln -sf /usr/lib/llvm${LLVM_VERSION}/lib/cmake/llvm /usr/lib/cmake/llvm

        # Create symlinks for LLVM headers so they can be found in standard include path
        mkdir -p /usr/include
        ln -sf /usr/lib/llvm${LLVM_VERSION}/include/llvm /usr/include/llvm
        ln -sf /usr/lib/llvm${LLVM_VERSION}/include/llvm-c /usr/include/llvm-c
    fi
}

# Function to install LLD headers
install_lld_headers() {
    if [ "$SKIP_LLVM" = true ]; then
        return
    fi
    
    print_subheader "Installing LLD headers..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would create /usr/local/include/lld/Common directory"
        echo "Would download LLD headers from LLVM repository"
    else
        mkdir -p /usr/local/include/lld/Common
        cd /usr/local/include/lld/Common
        
        # Download LLD headers
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Driver.h -o Driver.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Args.h -o Args.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/ErrorHandler.h -o ErrorHandler.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Filesystem.h -o Filesystem.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/LLVM.h -o LLVM.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Memory.h -o Memory.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Reproduce.h -o Reproduce.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Strings.h -o Strings.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Timer.h -o Timer.h
        curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Version.h -o Version.h
    fi
}

# Function to verify installation
verify_installation() {
    print_subheader "Verifying installation..."
    
    # Check GCC versions
    if [ "$SKIP_GCC" != true ]; then
        if command -v gcc >/dev/null 2>&1; then
            gcc_version=$(gcc --version | head -n1 | awk '{print $3}' | cut -d. -f1)
            print_status "Default GCC $gcc_version: ✓"
        else
            print_warning "Default GCC: ✗"
        fi
        
        if command -v gcc-14 >/dev/null 2>&1; then
            print_status "GCC 14: ✓"
        else
            print_warning "GCC 14: ✗"
        fi
    fi
    
    # Check LLVM
    if [ "$SKIP_LLVM" != true ]; then
        if command -v llvm-config >/dev/null 2>&1; then
            print_status "LLVM: ✓"
        else
            print_warning "LLVM: ✗"
        fi
        
        if command -v lld >/dev/null 2>&1; then
            print_status "LLD: ✓"
        else
            print_warning "LLD: ✗"
        fi
    fi
    
    # Check build tools
    if command -v cmake >/dev/null 2>&1; then
        print_status "CMake: ✓"
    else
        print_warning "CMake: ✗"
    fi
    
    if command -v ninja >/dev/null 2>&1; then
        print_status "Ninja: ✓"
    else
        print_warning "Ninja: ✗"
    fi
    
    if command -v python >/dev/null 2>&1; then
        print_status "Python: ✓"
    else
        print_warning "Python: ✗"
    fi
}

# Function to parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -d|--dry-run)
                DRY_RUN=true
                shift
                ;;
            --skip-llvm)
                SKIP_LLVM=true
                shift
                ;;
            --skip-gcc)
                SKIP_GCC=true
                shift
                ;;
            --llvm-version)
                LLVM_VERSION="$2"
                shift 2
                ;;
            --gcc-version)
                GCC_VERSION="$2"
                shift 2
                ;;
            --version)
                show_version
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Main function
main() {
    print_header "Heimdall Arch Linux Setup Script v${VERSION}"
    
    # Parse command line arguments
    parse_args "$@"
    
    # Check if running as root
    check_root
    
    # Check Arch Linux version
    check_arch_version
    
    # Install dependencies
    install_prerequisites
    install_build_tools
    install_dev_libs
    install_gcc_versions
    install_llvm
    create_llvm_symlinks
    install_lld_headers
    
    # Verify installation
    verify_installation
    
    print_header "Setup Complete!"
    print_status "All dependencies have been installed successfully."
    print_status "You can now build Heimdall using the build scripts."
}

# Run main function with all arguments
main "$@" 