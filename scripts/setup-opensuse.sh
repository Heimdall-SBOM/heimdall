#!/bin/bash

# Heimdall OpenSUSE Setup Script
# This script installs all necessary dependencies for building Heimdall on OpenSUSE
# Based on dockerfiles/Dockerfile.opensuse

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
Heimdall OpenSUSE Setup Script v${VERSION}

This script installs all necessary dependencies for building Heimdall on OpenSUSE.

USAGE:
    ./setup-opensuse.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --llvm-version VERSION Set LLVM version to install (default: 18)
    --version              Show version information

SUPPORTED OPENSUSE VERSIONS:
    OpenSUSE Tumbleweed    - GCC 11, 13 + LLVM 18

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup-opensuse.sh                    # Install all dependencies
    ./setup-opensuse.sh --verbose          # Install with verbose output
    ./setup-opensuse.sh --dry-run          # Show what would be installed
    ./setup-opensuse.sh --skip-llvm        # Skip LLVM installation

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compiler (latest version)
    - LLVM/Clang toolchain (18)
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/your-repo/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall OpenSUSE Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to check OpenSUSE version
check_opensuse_version() {
    print_subheader "Checking OpenSUSE version..."
    
    if [ ! -f /etc/os-release ]; then
        print_error "This script is designed for OpenSUSE systems"
        exit 1
    fi
    
    . /etc/os-release
    if [ "$ID" != "opensuse-tumbleweed" ] && [ "$ID" != "opensuse" ]; then
        print_error "This script is designed for OpenSUSE systems, detected: $ID"
        exit 1
    fi
    
    print_status "Detected OpenSUSE $VERSION_ID"
    
    # Check if version is supported (Tumbleweed recommended)
    if [ "$ID" = "opensuse-tumbleweed" ]; then
        print_status "OpenSUSE Tumbleweed detected - fully supported"
    else
        print_warning "OpenSUSE $VERSION_ID may not be fully supported. Tumbleweed is recommended."
    fi
}

# Function to install prerequisites
install_prerequisites() {
    print_subheader "Installing prerequisites..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would run: zypper update -y"
        echo "Would install: wget curl"
    else
        zypper update -y
        zypper install -y wget curl
    fi
}

# Function to install build tools
install_build_tools() {
    print_subheader "Installing build tools..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: gcc gcc-c++ cmake ninja python3 python3-pip git binutils"
    else
        zypper install -y gcc gcc-c++ cmake ninja python3 python3-pip git binutils
    fi
}

# Function to install development libraries
install_dev_libs() {
    print_subheader "Installing development libraries..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: openssl-devel libelf-devel pkg-config boost-devel"
    else
        zypper install -y openssl-devel libelf-devel pkg-config boost-devel
    fi
}

# Function to install GCC versions
install_gcc_versions() {
    if [ "$SKIP_GCC" = true ]; then
        print_warning "Skipping GCC installation as requested"
        return
    fi
    
    print_subheader "Installing GCC versions..."
    
    # OpenSUSE Tumbleweed uses the latest GCC version by default
    # No need to set alternatives as gcc and g++ are already the default
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would use default OpenSUSE GCC (latest version)"
    fi
    
    print_status "Available GCC version: latest"
}

# Function to install LLVM
install_llvm() {
    if [ "$SKIP_LLVM" = true ]; then
        print_warning "Skipping LLVM installation as requested"
        return
    fi
    
    print_subheader "Installing LLVM ${LLVM_VERSION}..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: llvm llvm-devel lld"
    else
        # Install LLVM - use unversioned packages which should provide llvm-config
        zypper install -y llvm llvm-devel lld
    fi
}

# Function to create LLVM symlinks
create_llvm_symlinks() {
    if [ "$SKIP_LLVM" = true ]; then
        return
    fi
    
    print_subheader "Creating LLVM symlinks..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would check for LLD binaries and create symlinks if needed"
    else
        # Create LLD symlinks if needed
        if [ -f /usr/bin/ld.lld ]; then
            print_status "LLD found at /usr/bin/ld.lld"
        else
            print_status "LLD not found, checking for versioned binary"
            if [ -f /usr/bin/ld.lld-${LLVM_VERSION} ]; then
                ln -sf /usr/bin/ld.lld-${LLVM_VERSION} /usr/bin/ld.lld
            fi
        fi

        if [ -f /usr/bin/lld ]; then
            print_status "LLD found at /usr/bin/lld"
        else
            print_status "LLD not found, checking for versioned binary"
            if [ -f /usr/bin/lld-${LLVM_VERSION} ]; then
                ln -sf /usr/bin/lld-${LLVM_VERSION} /usr/bin/lld
            fi
        fi
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
    
    # Check GCC version
    if [ "$SKIP_GCC" != true ]; then
        if command -v gcc >/dev/null 2>&1; then
            gcc_version=$(gcc --version | head -n1 | awk '{print $3}' | cut -d. -f1)
            print_status "GCC $gcc_version: ✓"
        else
            print_warning "GCC: ✗"
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
    
    if command -v python3 >/dev/null 2>&1; then
        print_status "Python 3: ✓"
    else
        print_warning "Python 3: ✗"
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
    print_header "Heimdall OpenSUSE Setup Script v${VERSION}"
    
    # Parse command line arguments
    parse_args "$@"
    
    # Check if running as root
    check_root
    
    # Check OpenSUSE version
    check_opensuse_version
    
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