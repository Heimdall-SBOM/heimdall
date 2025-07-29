#!/bin/bash

# Heimdall Setup Script - Menu Interface
# This script provides a graphical/text-based menu to select your operating system
# and then calls the appropriate OS-specific setup script

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
VERSION="2.0.0"

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

This script provides a menu interface to select your operating system and install
all necessary dependencies for building Heimdall with support for multiple C++ standards.

USAGE:
    ./setup.sh [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -d, --dry-run          Show what would be installed without actually installing
    --skip-llvm            Skip LLVM installation (use system LLVM if available)
    --skip-gcc             Skip GCC installation (use system GCC)
    --auto-detect          Auto-detect OS and run setup without menu
    --version              Show version information

SUPPORTED OPERATING SYSTEMS:
    Ubuntu 22.04+          - GCC 11, 13 + LLVM 18, 19
    Debian Bookworm        - GCC 11, 12 + LLVM 18
    Debian Testing         - GCC 12, 13, 14 + LLVM 18
    CentOS Stream 9        - GCC 11, 13, 14 + LLVM 20
    Fedora Latest          - GCC 15 + LLVM 18
    Arch Linux             - GCC 14, 15 + LLVM 18
    OpenSUSE Tumbleweed    - GCC 11, 13 + LLVM 18
    Rocky Linux 9          - GCC 11, 13 + LLVM 16
    macOS                  - Xcode + LLVM via Homebrew

C++ STANDARDS SUPPORT:
    C++11  - Requires GCC 4.8+ or Clang 3.3+
    C++14  - Requires GCC 5+ or Clang 3.4+
    C++17  - Requires GCC 7+ or Clang 5+
    C++20  - Requires GCC 10+ or Clang 10+
    C++23  - Requires GCC 11+ or Clang 14+

EXAMPLES:
    ./setup.sh                    # Show menu and select OS
    ./setup.sh --auto-detect      # Auto-detect OS and install
    ./setup.sh --verbose          # Show menu with verbose option
    ./setup.sh --dry-run          # Show menu with dry-run option

DEPENDENCIES INSTALLED:
    - Build tools (make, cmake, ninja)
    - GCC compilers (multiple versions)
    - LLVM/Clang toolchain
    - Development libraries (OpenSSL, ELF, Boost)
    - Python 3 and pip
    - Git

For more information, visit: https://github.com/Heimdall-SBOM/heimdall
EOF
}

# Function to show version
show_version() {
    echo "Heimdall Setup Script v${VERSION}"
    echo "Copyright (c) 2024 Heimdall Project"
}

# Function to detect operating system
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            case $ID in
                ubuntu) echo "ubuntu" ;;
                debian) echo "debian" ;;
                centos) echo "centos" ;;
                rocky) echo "rocky" ;;
                fedora) echo "fedora" ;;
                arch) echo "arch" ;;
                opensuse*) echo "opensuse" ;;
                *) echo "unknown" ;;
            esac
        else
            echo "unknown"
        fi
    else
        echo "unknown"
    fi
}

# Function to check if setup script exists
check_setup_script() {
    local os=$1
    local script_path="scripts/setup-${os}.sh"
    
    if [ -f "$script_path" ]; then
        return 0
    else
        return 1
    fi
}

# Function to show menu
show_menu() {
    clear
    print_header "Heimdall Setup Script v${VERSION}"
    echo ""
    print_subheader "Select your operating system:"
    echo ""
    echo "  ${GREEN}1)${NC} Ubuntu 22.04+"
    echo "  ${GREEN}2)${NC} Debian Bookworm"
    echo "  ${GREEN}3)${NC} Debian Testing"
    echo "  ${GREEN}4)${NC} CentOS Stream 9"
    echo "  ${GREEN}5)${NC} Fedora Latest"
    echo "  ${GREEN}6)${NC} Arch Linux"
    echo "  ${GREEN}7)${NC} OpenSUSE Tumbleweed"
    echo "  ${GREEN}8)${NC} Rocky Linux 9"
    echo "  ${GREEN}9)${NC} macOS"
    echo "  ${GREEN}10)${NC} Auto-detect OS"
    echo "  ${GREEN}11)${NC} Show help"
    echo "  ${GREEN}12)${NC} Exit"
    echo ""
    echo -n "Enter your choice (1-12): "
}

# Function to get OS name from menu choice
get_os_from_choice() {
    case $1 in
        1) echo "ubuntu" ;;
        2) echo "debian" ;;
        3) echo "debian-testing" ;;
        4) echo "centos" ;;
        5) echo "fedora" ;;
        6) echo "arch" ;;
        7) echo "opensuse" ;;
        8) echo "rocky" ;;
        9) echo "macos" ;;
        10) echo "auto" ;;
        *) echo "invalid" ;;
    esac
}

# Function to run OS-specific setup script
run_setup_script() {
    local os=$1
    local script_path="scripts/setup-${os}.sh"
    
    if [ ! -f "$script_path" ]; then
        print_error "Setup script not found: $script_path"
        return 1
    fi
    
    print_status "Running setup script for $os..."
    print_status "Script: $script_path"
    echo ""
    
    # Pass through all arguments to the OS-specific script
    bash "$script_path" "$@"
}

# Function to auto-detect and run setup
auto_detect_and_setup() {
    print_subheader "Auto-detecting operating system..."
    
    local detected_os=$(detect_os)
    
    if [ "$detected_os" = "unknown" ]; then
        print_error "Could not auto-detect your operating system"
        print_warning "Please select your OS manually from the menu"
        return 1
    fi
    
    print_status "Detected OS: $detected_os"
    
    if ! check_setup_script "$detected_os"; then
        print_error "No setup script available for detected OS: $detected_os"
        print_warning "Please select your OS manually from the menu"
        return 1
    fi
    
    print_status "Running setup for $detected_os..."
    run_setup_script "$detected_os" "$@"
}

# Function to show OS information
show_os_info() {
    local os=$1
    case $os in
        ubuntu)
            echo "  ${CYAN}Ubuntu 22.04+${NC}"
            echo "  - GCC 11, 13 + LLVM 18, 19"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        debian)
            echo "  ${CYAN}Debian Bookworm${NC}"
            echo "  - GCC 11, 12 + LLVM 18"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        debian-testing)
            echo "  ${CYAN}Debian Testing${NC}"
            echo "  - GCC 12, 13, 14 + LLVM 18"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        centos)
            echo "  ${CYAN}CentOS Stream 9${NC}"
            echo "  - GCC 11, 13, 14 + LLVM 20"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        fedora)
            echo "  ${CYAN}Fedora Latest${NC}"
            echo "  - GCC 15 + LLVM 18"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        arch)
            echo "  ${CYAN}Arch Linux${NC}"
            echo "  - GCC 14, 15 + LLVM 18"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        opensuse)
            echo "  ${CYAN}OpenSUSE Tumbleweed${NC}"
            echo "  - GCC 11, 13 + LLVM 18"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        rocky)
            echo "  ${CYAN}Rocky Linux 9${NC}"
            echo "  - GCC 11, 13 + LLVM 16"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
        macos)
            echo "  ${CYAN}macOS${NC}"
            echo "  - Xcode + LLVM via Homebrew"
            echo "  - Full C++11/14/17/20/23 support"
            ;;
    esac
}

# Function to confirm OS selection
confirm_selection() {
    local os=$1
    echo ""
    print_subheader "Selected Operating System:"
    show_os_info "$os"
    echo ""
    echo -n "Proceed with installation? (y/N): "
    read -r confirm
    
    case $confirm in
        [Yy]|[Yy][Ee][Ss])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

# Function to handle menu interaction
handle_menu() {
    while true; do
        show_menu
        read -r choice
        
        case $choice in
            1|2|3|4|5|6|7|8|9)
                local selected_os=$(get_os_from_choice "$choice")
                if [ "$selected_os" = "invalid" ]; then
                    print_error "Invalid choice. Please try again."
                    sleep 2
                    continue
                fi
                
                if ! check_setup_script "$selected_os"; then
                    print_error "Setup script not available for selected OS"
                    sleep 2
                    continue
                fi
                
                if confirm_selection "$selected_os"; then
                    run_setup_script "$selected_os" "$@"
                    break
                fi
                ;;
            10)
                if auto_detect_and_setup "$@"; then
                    break
                else
                    sleep 2
                    continue
                fi
                ;;
            11)
                show_help
                echo ""
                echo "Press Enter to continue..."
                read -r
                ;;
            12)
                print_status "Exiting setup script"
                exit 0
                ;;
            *)
                print_error "Invalid choice. Please enter a number between 1 and 12."
                sleep 2
                ;;
        esac
    done
}

# Function to check if running as root (for Linux)
check_root() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ "$EUID" -ne 0 ]; then
            print_error "This script must be run as root on Linux (use sudo)"
            exit 1
        fi
    fi
}

# Main function
main() {
    local auto_detect=false
    local args=()
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                args+=("$1")
                shift
                ;;
            -d|--dry-run)
                args+=("$1")
                shift
                ;;
            --skip-llvm)
                args+=("$1")
                shift
                ;;
            --skip-gcc)
                args+=("$1")
                shift
                ;;
            --auto-detect)
                auto_detect=true
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
    
    # Check if running as root (for Linux)
    check_root
    
    # Handle auto-detect mode
    if [ "$auto_detect" = true ]; then
        auto_detect_and_setup "${args[@]}"
    else
        # Show menu interface
        handle_menu "${args[@]}"
    fi
}

# Run main function with all arguments
main "$@" 