#!/bin/bash

# Heimdall CentOS Setup Script
# This script installs all necessary dependencies for building Heimdall on CentOS
# Based on dockerfiles/Dockerfile.centos

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
LLVM_VERSION="20"
LLVM_TAG="llvmorg-20.1.3"

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
Heimdall CentOS Setup Script v${VERSION}

This script installs all necessary dependencies for building Heimdall on CentOS.

USAGE:
    ./setup-centos.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --llvm-version VERSION Set LLVM version to install (default: 20)
    --gcc-version VERSION  Set GCC version to use (11, 13, 14)
    --version              Show version information

SUPPORTED CENTOS VERSIONS:
    CentOS Stream 9        - GCC 11, 13, 14 + LLVM 20

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup-centos.sh                    # Install all dependencies
    ./setup-centos.sh --verbose          # Install with verbose output
    ./setup-centos.sh --dry-run          # Show what would be installed
    ./setup-centos.sh --skip-llvm        # Skip LLVM installation
    ./setup-centos.sh --gcc-version 13   # Use GCC 13

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compilers (11, 13, 14 via SCL)
    - LLVM/Clang toolchain (20)
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/your-repo/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall CentOS Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to check CentOS version
check_centos_version() {
    print_subheader "Checking CentOS version..."
    
    if [ ! -f /etc/os-release ]; then
        print_error "This script is designed for CentOS systems"
        exit 1
    fi
    
    . /etc/os-release
    if [ "$ID" != "centos" ]; then
        print_error "This script is designed for CentOS systems, detected: $ID"
        exit 1
    fi
    
    print_status "Detected CentOS $VERSION_ID"
    
    # Check if version is supported (Stream 9+)
    if [ "$VERSION_ID" = "9" ] || [ "$VERSION_ID" = "stream9" ]; then
        print_status "CentOS Stream 9 detected - fully supported"
    else
        print_warning "CentOS $VERSION_ID may not be fully supported. Some features may not work correctly."
    fi
}

# Function to install prerequisites
install_prerequisites() {
    print_subheader "Installing prerequisites..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would run: dnf update -y"
        echo "Would install: wget unzip"
    else
        dnf update -y
        dnf install -y wget unzip
    fi
}

# Function to install build tools
install_build_tools() {
    print_subheader "Installing build tools..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: gcc gcc-c++ cmake python3 python3-pip git binutils"
        echo "Would download and install ninja manually"
    else
        dnf install -y gcc gcc-c++ cmake python3 python3-pip git binutils
        
        # Install ninja manually (not available in CentOS repos)
        curl -L -o /tmp/ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip
        unzip /tmp/ninja.zip -d /usr/local/bin
        chmod +x /usr/local/bin/ninja
        rm /tmp/ninja.zip
    fi
}

# Function to install development libraries
install_dev_libs() {
    print_subheader "Installing development libraries..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: openssl-devel elfutils-libelf-devel pkgconfig boost-devel boost-filesystem boost-system"
    else
        dnf install -y openssl-devel elfutils-libelf-devel pkgconfig boost-devel boost-filesystem boost-system
    fi
}

# Function to install GCC versions
install_gcc_versions() {
    if [ "$SKIP_GCC" = true ]; then
        print_warning "Skipping GCC installation as requested"
        return
    fi
    
    print_subheader "Installing GCC versions..."
    
    # Enable CRB repository for newer GCC versions
    if [ "$DRY_RUN" = true ]; then
        echo "Would enable CRB repository"
        echo "Would install: gcc-toolset-13 gcc-toolset-14"
    else
        dnf config-manager --set-enabled crb
        dnf install -y gcc-toolset-13 gcc-toolset-14
    fi
    
    print_status "Available GCC versions: 11 (default), 13, 14"
}

# Function to install LLVM
install_llvm() {
    if [ "$SKIP_LLVM" = true ]; then
        print_warning "Skipping LLVM installation as requested"
        return
    fi
    
    print_subheader "Installing LLVM ${LLVM_VERSION}..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: llvm-devel lld-devel"
    else
        # CentOS Stream 9 only provides llvm-devel (no versioned packages)
        dnf install -y llvm-devel lld-devel
    fi
}

# Function to create LLVM symlinks
create_llvm_symlinks() {
    if [ "$SKIP_LLVM" = true ]; then
        return
    fi
    
    print_subheader "Creating LLVM symlinks..."
    
    # CentOS Stream 9 uses unversioned llvm-config, no symlink needed
    if [ "$DRY_RUN" = true ]; then
        echo "Would check for existing llvm-config (CentOS uses unversioned)"
    else
        if [ ! -f /usr/bin/llvm-config ]; then
            print_warning "llvm-config not found, LLVM installation may have failed"
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
    
    # Check GCC versions
    if [ "$SKIP_GCC" != true ]; then
        if command -v gcc >/dev/null 2>&1; then
            gcc_version=$(gcc --version | head -n1 | awk '{print $3}' | cut -d. -f1)
            print_status "Default GCC $gcc_version: ✓"
        else
            print_warning "Default GCC: ✗"
        fi
        
        if command -v scl >/dev/null 2>&1; then
            if scl list | grep -q gcc-toolset-13; then
                print_status "GCC 13 (SCL): ✓"
            else
                print_warning "GCC 13 (SCL): ✗"
            fi
            
            if scl list | grep -q gcc-toolset-14; then
                print_status "GCC 14 (SCL): ✓"
            else
                print_warning "GCC 14 (SCL): ✗"
            fi
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
    print_header "Heimdall CentOS Setup Script v${VERSION}"
    
    # Parse command line arguments
    parse_args "$@"
    
    # Check if running as root
    check_root
    
    # Check CentOS version
    check_centos_version
    
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
    print_status "Note: Use 'scl enable gcc-toolset-13 bash' or 'scl enable gcc-toolset-14 bash' to use newer GCC versions."
}

# Run main function with all arguments
main "$@" 