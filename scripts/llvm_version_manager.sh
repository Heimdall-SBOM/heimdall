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
    local found_versioned=0
    
    # On macOS, check for Homebrew LLVM installations first
    if [[ "$(uname)" == "Darwin" ]]; then
        # Check for Homebrew LLVM installations
        if [ -x "/opt/homebrew/opt/llvm/bin/llvm-config" ]; then
            local llvm_version=$("/opt/homebrew/opt/llvm/bin/llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            if [ $? -eq 0 ] && [ -n "$llvm_version" ]; then
                versions+=("homebrew-llvm:${llvm_version}")
                found_versioned=1
            fi
        fi
        
        if [ -x "/opt/homebrew/opt/llvm@18/bin/llvm-config" ]; then
            local llvm_version=$("/opt/homebrew/opt/llvm@18/bin/llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            if [ $? -eq 0 ] && [ -n "$llvm_version" ]; then
                versions+=("homebrew-llvm18:${llvm_version}")
                found_versioned=1
            fi
        fi
        
        # Also check for Intel Mac Homebrew paths
        if [ -x "/usr/local/opt/llvm/bin/llvm-config" ]; then
            local llvm_version=$("/usr/local/opt/llvm/bin/llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            if [ $? -eq 0 ] && [ -n "$llvm_version" ]; then
                versions+=("homebrew-llvm-intel:${llvm_version}")
                found_versioned=1
            fi
        fi
        
        if [ -x "/usr/local/opt/llvm@18/bin/llvm-config" ]; then
            local llvm_version=$("/usr/local/opt/llvm@18/bin/llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            if [ $? -eq 0 ] && [ -n "$llvm_version" ]; then
                versions+=("homebrew-llvm18-intel:${llvm_version}")
                found_versioned=1
            fi
        fi
    fi
    
    # Check for system-installed LLVM versions (e.g., llvm-config-19)
    for version in {7..25}; do
        if command -v "llvm-config-${version}" >/dev/null 2>&1; then
            # Use timeout to prevent hanging (if available)
            if command -v timeout >/dev/null 2>&1; then
                local llvm_version=$(timeout 5 "llvm-config-${version}" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            else
                local llvm_version=$("llvm-config-${version}" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            fi
            if [ $? -eq 0 ] && [ -n "$llvm_version" ]; then
                versions+=("${version}:${llvm_version}")
                found_versioned=1
            fi
        fi
    done
    
    # Check for default llvm-config
    if command -v "llvm-config" >/dev/null 2>&1; then
        # Use timeout to prevent hanging (if available)
        if command -v timeout >/dev/null 2>&1; then
            local default_version=$(timeout 5 "llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
        else
            local default_version=$("llvm-config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
        fi
        if [ $? -eq 0 ] && [ -n "$default_version" ]; then
            # Only add unversioned if no versioned found, or if on OpenSUSE
            if [ $found_versioned -eq 0 ] || grep -qi 'opensuse' /etc/os-release 2>/dev/null; then
                versions+=("default:${default_version}")
            fi
        fi
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

    # On macOS, prefer llvm@18 for C++11/14 if available
    if [[ "$(uname)" == "Darwin" ]]; then
        if [[ "$cxx_standard" == "11" || "$cxx_standard" == "14" ]]; then
            # Prefer llvm-config-18 if available
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                if [[ "$major_version" == "18" ]]; then
                    echo "$version_name"
                    return 0
                fi
            done
            # Fallback to any compatible version >=7
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
        fi
    fi

    case $cxx_standard in
        11|14)
            # C++11/14 requires LLVM 7+ (LLVM 19 is backward compatible)
            # Find the highest compatible version
            local best_version=""
            local best_major=0
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 7 ] && [ "$major_version" -gt "$best_major" ]; then
                    best_version="$version_name"
                    best_major="$major_version"
                fi
            done
            if [ -n "$best_version" ]; then
                echo "$best_version"
                return 0
            else
                print_error "No compatible LLVM version (7+) found for C++${cxx_standard}"
                return 1
            fi
            ;;
        17)
            # C++17 requires LLVM 11+ (LLVM 19 is backward compatible)
            # Find the highest compatible version
            local best_version=""
            local best_major=0
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 11 ] && [ "$major_version" -gt "$best_major" ]; then
                    best_version="$version_name"
                    best_major="$major_version"
                fi
            done
            if [ -n "$best_version" ]; then
                echo "$best_version"
                return 0
            else
                print_error "No compatible LLVM version (11+) found for C++${cxx_standard}"
                return 1
            fi
            ;;
        20|23)
            # C++20/23 requires LLVM 18+ (LLVM 18 is compatible with C++20/23)
            # Find the highest compatible version
            local best_version=""
            local best_major=0
            for version_info in "${available_versions[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_llvm_major_version "$version_number")
                
                if [ "$major_version" -ge 18 ] && [ "$major_version" -gt "$best_major" ]; then
                    best_version="$version_name"
                    best_major="$major_version"
                fi
            done
            if [ -n "$best_version" ]; then
                echo "$best_version"
                return 0
            else
                print_error "No compatible LLVM version (18+) found for C++${cxx_standard}"
                return 1
            fi
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
    
    # Handle Homebrew LLVM installations on macOS
    if [[ "$llvm_version" == "homebrew-llvm" ]]; then
        export LLVM_CONFIG="/opt/homebrew/opt/llvm/bin/llvm-config"
    elif [[ "$llvm_version" == "homebrew-llvm18" ]]; then
        export LLVM_CONFIG="/opt/homebrew/opt/llvm@18/bin/llvm-config"
    elif [[ "$llvm_version" == "homebrew-llvm-intel" ]]; then
        export LLVM_CONFIG="/usr/local/opt/llvm/bin/llvm-config"
    elif [[ "$llvm_version" == "homebrew-llvm18-intel" ]]; then
        export LLVM_CONFIG="/usr/local/opt/llvm@18/bin/llvm-config"
    elif [ "$llvm_version" = "default" ]; then
        # Use default llvm-config
        export LLVM_CONFIG="llvm-config"
    else
        # Use specific version
        export LLVM_CONFIG="llvm-config-${llvm_version}"
    fi
    
    # Verify the LLVM version is available
    if ! command -v "$LLVM_CONFIG" >/dev/null 2>&1 && [ ! -x "$LLVM_CONFIG" ]; then
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