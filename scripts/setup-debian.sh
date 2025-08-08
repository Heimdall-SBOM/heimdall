#!/bin/bash

# Heimdall Debian Setup Script
# This script installs all necessary dependencies for building Heimdall on Debian
# Based on dockerfiles/Dockerfile.debian

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
Heimdall Debian Setup Script v${VERSION}

This script installs all necessary dependencies for building Heimdall on Debian.

USAGE:
    ./setup-debian.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --llvm-version VERSION Set LLVM version to install (default: 18)
    --version              Show version information

SUPPORTED DEBIAN VERSIONS:
    Debian Bookworm        - GCC 11, 12 + LLVM 18

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup-debian.sh                    # Install all dependencies
    ./setup-debian.sh --verbose          # Install with verbose output
    ./setup-debian.sh --dry-run          # Show what would be installed
    ./setup-debian.sh --skip-llvm        # Skip LLVM installation

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compilers (11, 12)
    - LLVM/Clang toolchain (18)
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/your-repo/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall Debian Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to check Debian version
check_debian_version() {
    print_subheader "Checking Debian version..."
    
    if [ ! -f /etc/os-release ]; then
        print_error "This script is designed for Debian systems"
        exit 1
    fi
    
    . /etc/os-release
    if [ "$ID" != "debian" ]; then
        print_error "This script is designed for Debian systems, detected: $ID"
        exit 1
    fi
    
    print_status "Detected Debian $VERSION_ID"
    
    # Check if version is supported (Bookworm+)
    if [ "$VERSION_ID" = "12" ] || [ "$VERSION_ID" = "bookworm" ]; then
        print_status "Debian Bookworm detected - fully supported"
    else
        print_warning "Debian $VERSION_ID may not be fully supported. Some features may not work correctly."
    fi
}

# Function to install prerequisites
install_prerequisites() {
    print_subheader "Installing prerequisites..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would run: apt-get update"
        echo "Would install: software-properties-common wget gnupg lsb-release curl"
    else
        apt-get update
        apt-get install -y software-properties-common wget gnupg lsb-release curl
    fi
}

# Function to install build tools
install_build_tools() {
    print_subheader "Installing build tools..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: build-essential cmake ninja-build python3 python3-pip git binutils-gold"
    else
        apt-get install -y build-essential cmake ninja-build python3 python3-pip git binutils-gold
    fi
}

# Function to install development libraries
install_dev_libs() {
    print_subheader "Installing development libraries..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would install: libssl-dev libelf-dev pkg-config libboost-filesystem-dev libboost-system-dev gcc-plugin-dev"
    else
        apt-get install -y libssl-dev libelf-dev pkg-config libboost-filesystem-dev libboost-system-dev gcc-plugin-dev
    fi
}

# Function to install GCC versions
install_gcc_versions() {
    if [ "$SKIP_GCC" = true ]; then
        print_warning "Skipping GCC installation as requested"
        return
    fi
    
    print_subheader "Installing GCC versions..."
    
    # Add Debian backports for newer GCC
    if [ "$DRY_RUN" = true ]; then
        echo "Would add Debian backports repository"
        echo "Would install: gcc-11 g++-11 gcc-12 g++-12"
    else
        echo "deb http://deb.debian.org/debian bookworm-backports main" >> /etc/apt/sources.list
        apt-get update
        apt-get install -y gcc-11 g++-11 gcc-12 g++-12
    fi
    
    print_status "Available GCC versions: 11, 12"
}

# Function to install LLVM
install_llvm() {
    if [ "$SKIP_LLVM" = true ]; then
        print_warning "Skipping LLVM installation as requested"
        return
    fi
    
    print_subheader "Installing LLVM ${LLVM_VERSION}..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would download and run LLVM installation script"
        echo "Would install: clang-${LLVM_VERSION} libclang-${LLVM_VERSION}-dev libclang-common-${LLVM_VERSION}-dev clang-tools-${LLVM_VERSION} llvm-${LLVM_VERSION}-dev lld-${LLVM_VERSION}"
    else
        # Download and run LLVM installation script
        wget https://apt.llvm.org/llvm.sh
        chmod +x llvm.sh
        ./llvm.sh ${LLVM_VERSION}
        
        apt-get update
        apt-get install -y clang-${LLVM_VERSION} libclang-${LLVM_VERSION}-dev libclang-common-${LLVM_VERSION}-dev clang-tools-${LLVM_VERSION} llvm-${LLVM_VERSION}-dev lld-${LLVM_VERSION}
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
    else
        ln -sf /usr/bin/llvm-config-${LLVM_VERSION} /usr/bin/llvm-config
        ln -sf /usr/bin/ld.lld-${LLVM_VERSION} /usr/bin/ld.lld
        ln -sf /usr/bin/lld-${LLVM_VERSION} /usr/bin/lld
    fi
}

# Function to install LLD headers
install_lld_headers() {
    if [ "$SKIP_LLVM" = true ]; then
        return
    fi
    
    print_subheader "Installing LLD headers..."
    
    if [ "$DRY_RUN" = true ]; then
        echo "Would create /usr/include/lld directory"
        echo "Would create symlinks to LLVM headers"
    else
        mkdir -p /usr/include/lld
        find /usr/include/llvm-18 -name "*.h" -exec ln -sf {} /usr/include/lld/ \;
    fi
}

# Function to verify installation
verify_installation() {
    print_subheader "Verifying installation..."
    
    # Check GCC versions
    if [ "$SKIP_GCC" != true ]; then
        if command -v gcc-11 >/dev/null 2>&1; then
            print_status "GCC 11: ✓"
        else
            print_warning "GCC 11: ✗"
        fi
        
        if command -v gcc-12 >/dev/null 2>&1; then
            print_status "GCC 12: ✓"
        else
            print_warning "GCC 12: ✗"
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
    print_header "Heimdall Debian Setup Script v${VERSION}"
    
    # Parse command line arguments
    parse_args "$@"
    
    # Check if running as root
    check_root
    
    # Check Debian version
    check_debian_version
    
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