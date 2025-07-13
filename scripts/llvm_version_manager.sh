#!/bin/bash

# LLVM Version Manager for Heimdall
# Manages LLVM version selection based on C++ standard requirements

# set -e  # Removed to avoid propagating to parent scripts

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to detect available LLVM versions
detect_llvm_versions() {
    local versions=()
    
    # Check for system-installed LLVM versions (e.g., llvm-config-19)
    for version in {7..25}; do
        if command -v "llvm-config-${version}" >/dev/null 2>&1; then
            local llvm_version=$("llvm-config-${version}" --version | head -n1 | cut -d' ' -f3)
            versions+=("${version}:${llvm_version}")
        fi
    done
    
    # Check for default llvm-config
    if command -v "llvm-config" >/dev/null 2>&1; then
        local default_version=$("llvm-config" --version | head -n1 | cut -d' ' -f3)
        versions+=("default:${default_version}")
    fi
    
    echo "${versions[@]}"
}

# Function to get LLVM version number from version string
get_llvm_major_version() {
    local version_string="$1"
    echo "$version_string" | cut -d'.' -f1
}

# Function to select appropriate LLVM version for C++ standard
select_llvm_version() {
    local cxx_standard="$1"
    local available_versions=("${@:2}")
    
    case $cxx_standard in
        11|14)
            # C++11/14 requires LLVM 7+ (LLVM 19 is backward compatible)
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 7 ]; then
                    echo "$version_name"
                    return 0
                fi
            done
            print_error "No compatible LLVM version (7+) found for C++${cxx_standard}"
            return 1
            ;;
        17)
            # C++17 requires LLVM 11+ (LLVM 19 is backward compatible)
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 11 ]; then
                    echo "$version_name"
                    return 0
                fi
            done
            print_error "No compatible LLVM version (11+) found for C++${cxx_standard}"
            return 1
            ;;
        20|23)
            # C++20/23 requires LLVM 19+
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 19 ]; then
                    echo "$version_name"
                    return 0
                fi
            done
            print_error "No compatible LLVM version (19+) found for C++${cxx_standard}"
            return 1
            ;;
        *)
            print_error "Unsupported C++ standard: $cxx_standard"
            return 1
            ;;
    esac
}

# Function to set up LLVM environment variables
setup_llvm_environment() {
    local llvm_version="$1"
    
    if [ "$llvm_version" = "default" ]; then
        # Use default llvm-config
        export LLVM_CONFIG="llvm-config"
    else
        # Use specific version
        export LLVM_CONFIG="llvm-config-${llvm_version}"
    fi
    
    # Verify the LLVM version is available
    if ! command -v "$LLVM_CONFIG" >/dev/null 2>&1; then
        print_error "LLVM config not found: $LLVM_CONFIG"
        return 1
    fi
    
    # Get LLVM paths
    export LLVM_INCLUDE_DIRS="$($LLVM_CONFIG --includedir)"
    export LLVM_LIBRARY_DIRS="$($LLVM_CONFIG --libdir)"
    export LLVM_VERSION="$($LLVM_CONFIG --version | head -n1 | cut -d' ' -f3)"
    export LLVM_MAJOR_VERSION=$(get_llvm_major_version "$LLVM_VERSION")
    
    print_success "Using LLVM version: $LLVM_VERSION (major: $LLVM_MAJOR_VERSION)"
    print_status "LLVM config: $LLVM_CONFIG"
    print_status "LLVM include dirs: $LLVM_INCLUDE_DIRS"
    print_status "LLVM library dirs: $LLVM_LIBRARY_DIRS"
}

# Function to show available LLVM versions
show_available_versions() {
    print_status "Detecting available LLVM versions..."
    local versions=($(detect_llvm_versions))
    
    if [ ${#versions[@]} -eq 0 ]; then
        print_error "No LLVM versions found"
        return 1
    fi
    
    print_success "Available LLVM versions:"
    for version_info in "${versions[@]}"; do
        local version_name=$(echo "$version_info" | cut -d':' -f1)
        local version_number=$(echo "$version_info" | cut -d':' -f2)
        echo "  - $version_name: $version_number"
    done
}

# Main function
main() {
    local quiet=0
    if [[ "$1" == "--quiet" ]]; then
        quiet=1
        shift
    fi
    local cxx_standard="$1"
    
    if [ -z "$cxx_standard" ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "C++ standard not specified"
            echo "Usage: $0 [--quiet] <cxx_standard>"
            echo "  cxx_standard: 11, 14, 17, 20, or 23"
        fi
        exit 1
    fi
    
    # Detect available versions
    local available_versions=($(detect_llvm_versions))
    
    if [ ${#available_versions[@]} -eq 0 ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "No LLVM versions found"
            if [[ "$(uname)" == "Darwin" ]]; then
                echo -e "${YELLOW}[MACOS BUILD HELP]${NC} If you are on macOS, you likely need to install LLVM via Homebrew:"
                echo -e "    brew install llvm"
                echo -e "Then add this to your shell profile (e.g., ~/.zshrc or ~/.bash_profile):"
                echo -e "    export PATH=\"/opt/homebrew/opt/llvm/bin:\$PATH\"   # Apple Silicon (M1/M2)"
                echo -e "    export PATH=\"/usr/local/opt/llvm/bin:\$PATH\"      # Intel Mac"
                echo -e "Then restart your terminal and try again."
            else
                echo -e "${YELLOW}[BUILD HELP]${NC} Please install LLVM (version 17 or higher) and ensure 'llvm-config' is in your PATH."
            fi
        fi
        exit 1
    fi
    
    # Select appropriate version
    local selected_version=$(select_llvm_version "$cxx_standard" "${available_versions[@]}")
    
    if [ $? -ne 0 ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "Failed to select LLVM version for C++${cxx_standard}"
            show_available_versions
        fi
        exit 1
    fi
    
    # Set up environment
    setup_llvm_environment "$selected_version" > /dev/null
    
    # Only export variables if setup was successful
    if [ $? -eq 0 ]; then
        # Export variables for use by parent script (only these lines for eval)
        echo "LLVM_CONFIG=$LLVM_CONFIG"
        echo "LLVM_INCLUDE_DIRS=$LLVM_INCLUDE_DIRS"
        echo "LLVM_LIBRARY_DIRS=$LLVM_LIBRARY_DIRS"
        echo "LLVM_VERSION=$LLVM_VERSION"
        echo "LLVM_MAJOR_VERSION=$LLVM_MAJOR_VERSION"
    else
        exit 1
    fi

    # If not quiet and run directly, print status
    if [[ $quiet -eq 0 && "${BASH_SOURCE[0]}" == "${0}" ]]; then
        print_success "Using LLVM version: $LLVM_VERSION (major: $LLVM_MAJOR_VERSION)"
        print_status "LLVM config: $LLVM_CONFIG"
        print_status "LLVM include dirs: $LLVM_INCLUDE_DIRS"
        print_status "LLVM library dirs: $LLVM_LIBRARY_DIRS"
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi 