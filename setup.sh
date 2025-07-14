#!/bin/bash

# Heimdall Setup Script
# This script detects your Linux distribution and installs all necessary dependencies
# for building Heimdall with support for C++11, C++14, C++17, C++20, and C++23

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
DISTRO=""
DISTRO_VERSION=""
PACKAGE_MANAGER=""
INSTALL_CMD=""
UPDATE_CMD=""
GCC_VERSIONS=()
LLVM_VERSION=""
VERBOSE=false
DRY_RUN=false
SKIP_LLVM=false
SKIP_GCC=false

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
Heimdall Setup Script v${VERSION}

This script automatically detects your Linux distribution and installs all necessary
dependencies for building Heimdall with support for multiple C++ standards.

USAGE:
    ./setup.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --version              Show version information

SUPPORTED DISTRIBUTIONS:
    Ubuntu 22.04+          - GCC 11, 13 + LLVM 18
    Debian Bookworm        - GCC 11, 12 + LLVM 18
    Debian Testing         - GCC 12, 13, 14 + LLVM 18
    CentOS Stream 9        - GCC 11, 13, 14 + LLVM 20
    Fedora Latest          - GCC 15 + LLVM 18
    Arch Linux             - GCC 14, 15 + LLVM 18
    OpenSUSE Tumbleweed    - GCC 11, 13 + LLVM 18
    Rocky Linux 9          - GCC 11, 13 + LLVM 16

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup.sh                    # Auto-detect and install dependencies
    ./setup.sh --verbose          # Install with verbose output
    ./setup.sh --dry-run          # Show what would be installed
    ./setup.sh --skip-llvm        # Skip LLVM installation

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compilers (multiple versions)
    - LLVM/Clang toolchain
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/your-repo/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to detect Linux distribution
detect_distro() {
    print_subheader "Detecting Linux distribution..."
    
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
    elif [ -f /etc/redhat-release ]; then
        DISTRO="redhat"
        DISTRO_VERSION=$(cat /etc/redhat-release | sed -E 's/.*release ([0-9]+).*/\1/')
    elif [ -f /etc/debian_version ]; then
        DISTRO="debian"
        DISTRO_VERSION=$(cat /etc/debian_version)
    else
        print_error "Could not detect Linux distribution"
        exit 1
    fi
    
    print_status "Detected: $DISTRO $DISTRO_VERSION"
    
    # Set package manager and commands
    case $DISTRO in
        ubuntu|debian)
            PACKAGE_MANAGER="apt"
            INSTALL_CMD="apt-get install -y"
            UPDATE_CMD="apt-get update"
            ;;
        centos|rocky|rhel|fedora)
            PACKAGE_MANAGER="dnf"
            INSTALL_CMD="dnf install -y"
            UPDATE_CMD="dnf update -y"
            ;;
        arch)
            PACKAGE_MANAGER="pacman"
            INSTALL_CMD="pacman -S --noconfirm"
            UPDATE_CMD="pacman -Syu --noconfirm"
            ;;
        opensuse)
            PACKAGE_MANAGER="zypper"
            INSTALL_CMD="zypper install -y"
            UPDATE_CMD="zypper update -y"
            ;;
        *)
            print_error "Unsupported distribution: $DISTRO"
            exit 1
            ;;
    esac
    
    print_status "Package manager: $PACKAGE_MANAGER"
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to install basic dependencies
install_basic_deps() {
    print_subheader "Installing basic dependencies..."
    
    case $DISTRO in
        ubuntu)
            if [ "$DRY_RUN" = true ]; then
                echo "Would run: apt-get update"
                echo "Would install: software-properties-common wget gnupg lsb-release curl"
            else
                apt-get update
                apt-get install -y software-properties-common wget gnupg lsb-release curl
            fi
            ;;
        debian)
            if [ "$DRY_RUN" = true ]; then
                echo "Would run: apt-get update"
                echo "Would install: software-properties-common wget gnupg lsb-release curl"
            else
                apt-get update
                apt-get install -y software-properties-common wget gnupg lsb-release curl
            fi
            ;;
        centos|rocky)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: epel-release wget unzip"
            else
                dnf install -y epel-release wget unzip
            fi
            ;;
        fedora)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: wget unzip"
            else
                dnf install -y wget unzip
            fi
            ;;
        arch)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: base-devel wget curl"
            else
                pacman -S --noconfirm base-devel wget curl
            fi
            ;;
        opensuse)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: wget curl"
            else
                zypper install -y wget curl
            fi
            ;;
    esac
}

# Function to install build tools
install_build_tools() {
    print_subheader "Installing build tools..."
    
    case $DISTRO in
        ubuntu)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: build-essential cmake ninja-build python3 python3-pip git binutils-gold"
            else
                apt-get install -y build-essential cmake ninja-build python3 python3-pip git binutils-gold
            fi
            ;;
        debian)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: build-essential cmake ninja-build python3 python3-pip git binutils-gold"
            else
                apt-get install -y build-essential cmake ninja-build python3 python3-pip git binutils-gold
            fi
            ;;
        centos|rocky|fedora)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: gcc gcc-c++ cmake python3 python3-pip git binutils"
                echo "Would download and install ninja manually"
            else
                dnf install -y gcc gcc-c++ cmake python3 python3-pip git binutils
                # Install ninja manually
                curl -L -o /tmp/ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip
                unzip /tmp/ninja.zip -d /usr/local/bin
                chmod +x /usr/local/bin/ninja
                rm /tmp/ninja.zip
            fi
            ;;
        arch)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: base-devel cmake ninja python python-pip git"
            else
                pacman -S --noconfirm base-devel cmake ninja python python-pip git
            fi
            ;;
        opensuse)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: gcc gcc-c++ cmake ninja python3 python3-pip git binutils"
            else
                zypper install -y gcc gcc-c++ cmake ninja python3 python3-pip git binutils
            fi
            ;;
    esac
}

# Function to install development libraries
install_dev_libs() {
    print_subheader "Installing development libraries..."
    
    case $DISTRO in
        ubuntu|debian)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: libssl-dev libelf-dev pkg-config libboost-filesystem-dev libboost-system-dev"
            else
                apt-get install -y libssl-dev libelf-dev pkg-config libboost-filesystem-dev libboost-system-dev
            fi
            ;;
        centos|rocky|fedora)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: openssl-devel elfutils-libelf-devel pkgconfig boost-devel boost-filesystem boost-system"
            else
                dnf install -y openssl-devel elfutils-libelf-devel pkgconfig boost-devel boost-filesystem boost-system
            fi
            ;;
        arch)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: openssl elfutils pkg-config boost boost-libs"
            else
                pacman -S --noconfirm openssl elfutils pkg-config boost boost-libs
            fi
            ;;
        opensuse)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: openssl-devel libelf-devel pkg-config boost-devel"
            else
                zypper install -y openssl-devel libelf-devel pkg-config boost-devel
            fi
            ;;
    esac
}

# Function to install GCC versions
install_gcc_versions() {
    if [ "$SKIP_GCC" = true ]; then
        print_warning "Skipping GCC installation as requested"
        return
    fi
    
    print_subheader "Installing GCC versions..."
    
    case $DISTRO in
        ubuntu)
            # Add Ubuntu Toolchain PPA
            if [ "$DRY_RUN" = true ]; then
                echo "Would add Ubuntu Toolchain PPA"
                echo "Would install: gcc-11 g++-11 gcc-13 g++-13"
            else
                add-apt-repository ppa:ubuntu-toolchain-r/test -y
                apt-get update
                apt-get install -y gcc-11 g++-11 gcc-13 g++-13
            fi
            GCC_VERSIONS=(11 13)
            ;;
        debian)
            # Add backports for newer GCC
            if [ "$DRY_RUN" = true ]; then
                echo "Would add Debian backports repository"
                echo "Would install: gcc-11 g++-11 gcc-12 g++-12"
            else
                echo "deb http://deb.debian.org/debian bookworm-backports main" >> /etc/apt/sources.list
                apt-get update
                apt-get install -y gcc-11 g++-11 gcc-12 g++-12
            fi
            GCC_VERSIONS=(11 12)
            ;;
        centos)
            if [ "$DRY_RUN" = true ]; then
                echo "Would enable CRB repository"
                echo "Would install: gcc-toolset-13 gcc-toolset-14"
            else
                dnf config-manager --set-enabled crb
                dnf install -y gcc-toolset-13 gcc-toolset-14
            fi
            GCC_VERSIONS=(11 13 14)
            ;;
        rocky)
            if [ "$DRY_RUN" = true ]; then
                echo "Would enable CRB repository"
                echo "Would install: gcc-toolset-13"
            else
                dnf config-manager --set-enabled crb
                dnf install -y gcc-toolset-13
            fi
            GCC_VERSIONS=(11 13)
            ;;
        fedora)
            # Fedora uses default GCC (currently 15)
            if [ "$DRY_RUN" = true ]; then
                echo "Would use default Fedora GCC (15)"
            fi
            GCC_VERSIONS=(15)
            ;;
        arch)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: gcc14"
            else
                pacman -S --noconfirm gcc14
            fi
            GCC_VERSIONS=(14 15)
            ;;
        opensuse)
            # OpenSUSE Tumbleweed uses the latest GCC version by default
            if [ "$DRY_RUN" = true ]; then
                echo "Would use default OpenSUSE GCC (latest version)"
            fi
            GCC_VERSIONS=(latest)
            ;;
    esac
    
    print_status "Available GCC versions: ${GCC_VERSIONS[*]}"
}

# Function to install LLVM
install_llvm() {
    if [ "$SKIP_LLVM" = true ]; then
        print_warning "Skipping LLVM installation as requested"
        return
    fi
    
    print_subheader "Installing LLVM..."
    
    case $DISTRO in
        ubuntu|debian)
            if [ "$DRY_RUN" = true ]; then
                echo "Would download and run LLVM installation script"
                echo "Would install: llvm-18-dev lld-18"
            else
                wget https://apt.llvm.org/llvm.sh
                chmod +x llvm.sh
                ./llvm.sh 18
                apt-get update
                apt-get install -y llvm-18-dev lld-18
                rm llvm.sh
            fi
            LLVM_VERSION="18"
            ;;
        centos)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: llvm-devel lld-devel"
            else
                dnf install -y llvm-devel lld-devel
            fi
            LLVM_VERSION="20"
            ;;
        rocky)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: llvm16-devel llvm-devel lld-devel"
            else
                dnf install -y llvm16-devel llvm-devel lld-devel
            fi
            LLVM_VERSION="16"
            ;;
        fedora)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: llvm18-devel llvm-devel lld-devel"
            else
                dnf install -y llvm18-devel llvm-devel lld-devel
            fi
            LLVM_VERSION="18"
            ;;
        arch)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: llvm18 lld18"
            else
                pacman -S --noconfirm llvm18 lld18
            fi
            LLVM_VERSION="18"
            ;;
        opensuse)
            if [ "$DRY_RUN" = true ]; then
                echo "Would install: llvm llvm-devel lld"
            else
                zypper install -y llvm llvm-devel lld
            fi
            LLVM_VERSION="20"
            ;;
    esac
    
    print_status "LLVM version: $LLVM_VERSION"
}

# Function to create symlinks
create_symlinks() {
    if [ "$DRY_RUN" = true ]; then
        print_warning "Skipping symlink creation in dry-run mode"
        return
    fi
    
    print_subheader "Creating symlinks..."
    
    # Create LLVM symlinks
    if [ -n "$LLVM_VERSION" ]; then
        if [ "$DISTRO" = "opensuse" ]; then
            # OpenSUSE provides only unversioned llvm-config, no symlinks needed
            :
        else
            if [ -f "/usr/bin/llvm-config-$LLVM_VERSION" ]; then
                ln -sf /usr/bin/llvm-config-$LLVM_VERSION /usr/bin/llvm-config
            fi
            if [ -f "/usr/bin/ld.lld-$LLVM_VERSION" ]; then
                ln -sf /usr/bin/ld.lld-$LLVM_VERSION /usr/bin/ld.lld
            fi
            if [ -f "/usr/bin/lld-$LLVM_VERSION" ]; then
                ln -sf /usr/bin/lld-$LLVM_VERSION /usr/bin/lld
            fi
        fi
    fi
    
    # Create GCC symlinks for Ubuntu/Debian
    if [[ "$DISTRO" == "ubuntu" || "$DISTRO" == "debian" ]]; then
        for version in "${GCC_VERSIONS[@]}"; do
            if [ -f "/usr/bin/gcc-$version" ]; then
                update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$version $((100 + version))
                update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-$version $((100 + version))
            fi
        done
    fi
    
    # OpenSUSE uses the latest GCC version by default, no symlinks needed
    # The default gcc and g++ are already the latest version
    
    # Create GCC symlinks for CentOS/Rocky
    if [[ "$DISTRO" == "centos" || "$DISTRO" == "rocky" ]]; then
        for version in "${GCC_VERSIONS[@]}"; do
            if [ "$version" = "13" ] && [ -d "/opt/rh/gcc-toolset-13" ]; then
                ln -sf /opt/rh/gcc-toolset-13/root/usr/bin/gcc /usr/bin/gcc
                ln -sf /opt/rh/gcc-toolset-13/root/usr/bin/g++ /usr/bin/g++
            elif [ "$version" = "14" ] && [ -d "/opt/rh/gcc-toolset-14" ]; then
                ln -sf /opt/rh/gcc-toolset-14/root/usr/bin/gcc /usr/bin/gcc
                ln -sf /opt/rh/gcc-toolset-14/root/usr/bin/g++ /usr/bin/g++
            fi
        done
    fi
    
    # Create GCC symlinks for Arch
    if [ "$DISTRO" = "arch" ]; then
        for version in "${GCC_VERSIONS[@]}"; do
            if [ "$version" = "14" ] && [ -f "/usr/bin/gcc-14" ]; then
                ln -sf /usr/bin/gcc-14 /usr/bin/gcc
                ln -sf /usr/bin/g++-14 /usr/bin/g++
            fi
        done
    fi
}

# Function to install LLD headers
install_lld_headers() {
    if [ "$DRY_RUN" = true ]; then
        print_warning "Skipping LLD headers installation in dry-run mode"
        return
    fi
    
    print_subheader "Installing LLD headers..."
    
    # Determine LLVM tag based on version
    case $LLVM_VERSION in
        16) LLVM_TAG="llvmorg-16.0.6" ;;
        18) LLVM_TAG="llvmorg-18.1.8" ;;
        20) LLVM_TAG="llvmorg-20.1.3" ;;
        *) LLVM_TAG="llvmorg-18.1.8" ;;
    esac
    
    # Create LLD headers directory
    mkdir -p /usr/local/include/lld/Common
    
    # Download LLD headers
    cd /usr/local/include/lld/Common
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
}

# Function to verify installation
verify_installation() {
    print_subheader "Verifying installation..."
    
    # Check GCC
    if command -v gcc >/dev/null 2>&1; then
        GCC_VERSION=$(gcc --version | head -n1)
        print_status "GCC: $GCC_VERSION"
    else
        print_error "GCC not found"
    fi
    
    # Check G++
    if command -v g++ >/dev/null 2>&1; then
        GPP_VERSION=$(g++ --version | head -n1)
        print_status "G++: $GPP_VERSION"
    else
        print_error "G++ not found"
    fi
    
    # Check CMake
    if command -v cmake >/dev/null 2>&1; then
        CMAKE_VERSION=$(cmake --version | head -n1)
        print_status "CMake: $CMAKE_VERSION"
    else
        print_error "CMake not found"
    fi
    
    # Check Ninja
    if command -v ninja >/dev/null 2>&1; then
        NINJA_VERSION=$(ninja --version)
        print_status "Ninja: $NINJA_VERSION"
    else
        print_error "Ninja not found"
    fi
    
    # Check LLVM
    if command -v llvm-config >/dev/null 2>&1; then
        LLVM_VERSION=$(llvm-config --version)
        print_status "LLVM: $LLVM_VERSION"
    else
        print_warning "LLVM not found (this is normal if --skip-llvm was used)"
    fi
    
    # Check Python
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_VERSION=$(python3 --version)
        print_status "Python: $PYTHON_VERSION"
    else
        print_error "Python3 not found"
    fi
    
    # Check Git
    if command -v git >/dev/null 2>&1; then
        GIT_VERSION=$(git --version)
        print_status "Git: $GIT_VERSION"
    else
        print_error "Git not found"
    fi
}

# Function to show next steps
show_next_steps() {
    print_header "Installation Complete!"
    
    cat << EOF
${GREEN}Heimdall dependencies have been successfully installed!${NC}

${CYAN}Next Steps:${NC}
1. Clone the Heimdall repository:
   git clone https://github.com/your-repo/heimdall.git
   cd heimdall

2. Build Heimdall with your preferred C++ standard:
   mkdir build && cd build
   cmake .. -DCMAKE_CXX_STANDARD=17  # For C++17
   make -j$(nproc)

3. Available C++ standards:
   - C++11:  cmake .. -DCMAKE_CXX_STANDARD=11
   - C++14:  cmake .. -DCMAKE_CXX_STANDARD=14
   - C++17:  cmake .. -DCMAKE_CXX_STANDARD=17
   - C++20:  cmake .. -DCMAKE_CXX_STANDARD=20
   - C++23:  cmake .. -DCMAKE_CXX_STANDARD=23

4. Run tests:
   make test

5. Install Heimdall:
   sudo make install

${YELLOW}Note:${NC} If you encounter any issues, please check the documentation
or report them on the project's GitHub page.

${BLUE}Happy building!${NC}
EOF
}

# Function to show system information
show_system_info() {
    print_header "System Information"
    
    echo "Distribution: $DISTRO $DISTRO_VERSION"
    echo "Package Manager: $PACKAGE_MANAGER"
    echo "GCC Versions: ${GCC_VERSIONS[*]}"
    echo "LLVM Version: $LLVM_VERSION"
    echo "Architecture: $(uname -m)"
    echo "Kernel: $(uname -r)"
    echo ""
}

# Main function
main() {
    print_header "Heimdall Setup Script v${VERSION}"
    
    # Parse command line arguments
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
    
    # Check if running as root
    check_root
    
    # Detect distribution
    detect_distro
    
    # Show system information
    show_system_info
    
    # Install dependencies
    install_basic_deps
    install_build_tools
    install_dev_libs
    install_gcc_versions
    install_llvm
    create_symlinks
    install_lld_headers
    
    # Verify installation
    verify_installation
    
    # Show next steps
    show_next_steps
}

# Run main function with all arguments
main "$@" 