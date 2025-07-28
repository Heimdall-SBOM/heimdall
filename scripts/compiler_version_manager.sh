#!/bin/bash

# Compiler Version Manager for Heimdall
# Manages compiler selection based on C++ standard requirements

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

# Function to detect available GCC versions
detect_gcc_versions() {
    local versions=()
    
    # On macOS, check for Homebrew GCC installations first
    if [[ "$(uname)" == "Darwin" ]]; then
        # Check for Homebrew GCC installations
        if [ -x "/opt/homebrew/opt/gcc/bin/gcc-15" ]; then
            local gcc_version=$("/opt/homebrew/opt/gcc/bin/gcc-15" --version | head -n1 | cut -d' ' -f4 | sed 's/)//')
            if [ $? -eq 0 ] && [ -n "$gcc_version" ]; then
                echo "[DEBUG] Found Homebrew gcc-15: $gcc_version" 1>&2
                versions+=("homebrew-gcc-15:${gcc_version}")
            fi
        fi
        
        # Also check for Intel Mac Homebrew paths
        if [ -x "/usr/local/opt/gcc/bin/gcc-15" ]; then
            local gcc_version=$("/usr/local/opt/gcc/bin/gcc-15" --version | head -n1 | cut -d' ' -f4 | sed 's/)//')
            if [ $? -eq 0 ] && [ -n "$gcc_version" ]; then
                echo "[DEBUG] Found Homebrew Intel gcc-15: $gcc_version" 1>&2
                versions+=("homebrew-gcc-15-intel:${gcc_version}")
            fi
        fi
    fi
    
    # Check for default gcc first (prioritize default)
    if command -v "gcc" >/dev/null 2>&1; then
        local default_version=$(gcc --version | head -n1 | cut -d' ' -f3)
        echo "[DEBUG] Found default gcc: $default_version" 1>&2
        versions+=("gcc:${default_version}")
    fi
    
    # Check for system-installed GCC versions
    for version in {7..20}; do
        if command -v "gcc-${version}" >/dev/null 2>&1; then
            local gcc_version=$("gcc-${version}" --version | head -n1 | cut -d' ' -f3 | sed 's/)//')
            echo "[DEBUG] Found system gcc-${version}: $gcc_version" 1>&2
            versions+=("gcc-${version}:${gcc_version}")
        fi
    done
    
    # Check for SCL GCC toolset versions
    for version in {7..20}; do
        if scl list-collections 2>/dev/null | grep -q "gcc-toolset-${version}" 2>/dev/null; then
            local gcc_version=$(scl enable gcc-toolset-${version} -- gcc --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            if [ -n "$gcc_version" ]; then
                # Test if the compiler actually works
                scl enable gcc-toolset-${version} -- gcc --version >/dev/null 2>&1
                if [ $? -eq 0 ]; then
                    echo "[DEBUG] Found SCL gcc-toolset-${version}: $gcc_version" 1>&2
                    versions+=("scl-gcc-toolset-${version}:${gcc_version}")
                else
                    echo "[DEBUG] SCL gcc-toolset-${version} present but not working" 1>&2
                fi
            else
                echo "[DEBUG] SCL gcc-toolset-${version} present but version string empty" 1>&2
            fi
        fi
    done
    

    echo "${versions[@]}"
}

# Function to detect available Clang versions
detect_clang_versions() {
    local versions=()
    
    # Check for system-installed Clang versions
    for version in {7..20}; do
        if command -v "clang-${version}" >/dev/null 2>&1; then
            local clang_version=$("clang-${version}" --version | head -n1 | cut -d' ' -f3)
            versions+=("clang-${version}:${clang_version}")
        fi
    done
    
    # Check for default clang
    if command -v "clang" >/dev/null 2>&1; then
        local default_version=$(clang --version | head -n1 | cut -d' ' -f4)
        versions+=("clang:${default_version}")
    fi
    
    echo "${versions[@]}"
}

# Function to get compiler major version from version string
get_compiler_major_version() {
    local version_string="$1"
    echo "$version_string" | cut -d'.' -f1
}

# Function to select appropriate compiler for C++ standard
select_compiler() {
    local cxx_standard="$1"
    local compiler_preference="$2"
    shift 2
    local available_gcc=()
    local available_clang=()
    
    # Parse the remaining arguments into GCC and Clang arrays
    for arg in "$@"; do
        if [[ "$arg" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]]; then
            available_gcc+=("$arg")
        elif [[ "$arg" =~ ^clang ]]; then
            available_clang+=("$arg")
        fi
    done
    
    case $cxx_standard in
        11)
            # C++11: GCC 4.8+ or Clang 3.3+
            # Check compiler preference first
            if [ "$compiler_preference" = "clang" ]; then
                for version_info in "${available_clang[@]}"; do
                    local version_name=$(echo "$version_info" | cut -d':' -f1)
                    local version_number=$(echo "$version_info" | cut -d':' -f2)
                    local major_version=$(get_compiler_major_version "$version_number")
                    if [ "$major_version" -ge 3 ]; then
                        echo "clang:${version_name}"
                        return 0
                    fi
                done
            fi
            
            # Default: prefer newer versions for better support
            for version_info in "${available_gcc[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [[ "$version_name" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]] && [ "$major_version" -ge 4 ]; then
                    echo "${version_name}:${version_number}"
                    return 0
                fi
            done
            for version_info in "${available_clang[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [ "$major_version" -ge 3 ]; then
                    echo "clang:${version_name}"
                    return 0
                fi
            done
            ;;
        14)
            # C++14: GCC 6+ or Clang 3.4+
            # Check compiler preference first
            if [ "$compiler_preference" = "clang" ]; then
                for version_info in "${available_clang[@]}"; do
                    local version_name=$(echo "$version_info" | cut -d':' -f1)
                    local version_number=$(echo "$version_info" | cut -d':' -f2)
                    local major_version=$(get_compiler_major_version "$version_number")
                    if [ "$major_version" -ge 3 ]; then
                        echo "clang:${version_name}"
                        return 0
                    fi
                done
            fi
            
            # Default: prefer GCC
            for version_info in "${available_gcc[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [[ "$version_name" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]] && [ "$major_version" -ge 6 ]; then
                    echo "${version_name}:${version_number}"
                    return 0
                fi
            done
            for version_info in "${available_clang[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [ "$major_version" -ge 3 ]; then
                    echo "clang:${version_name}"
                    return 0
                fi
            done
            ;;
        17)
            # C++17: GCC 7+ or Clang 5+
            # Check compiler preference first
            if [ "$compiler_preference" = "clang" ]; then
                for version_info in "${available_clang[@]}"; do
                    local version_name=$(echo "$version_info" | cut -d':' -f1)
                    local version_number=$(echo "$version_info" | cut -d':' -f2)
                    local major_version=$(get_compiler_major_version "$version_number")
                    if [ "$major_version" -ge 5 ]; then
                        echo "clang:${version_name}"
                        return 0
                    fi
                done
            fi
            
            # Default: prefer GCC
            for version_info in "${available_gcc[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [[ "$version_name" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]] && [ "$major_version" -ge 7 ]; then
                    echo "${version_name}:${version_number}"
                    return 0
                fi
            done
            for version_info in "${available_clang[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [ "$major_version" -ge 5 ]; then
                    echo "clang:${version_name}"
                    return 0
                fi
            done
            ;;
        20)
            # C++20: GCC 13+ or Clang 14+ (for <format> support)
            # Check compiler preference first
            if [ "$compiler_preference" = "clang" ]; then
                for version_info in "${available_clang[@]}"; do
                    local version_name=$(echo "$version_info" | cut -d':' -f1)
                    local version_number=$(echo "$version_info" | cut -d':' -f2)
                    local major_version=$(get_compiler_major_version "$version_number")
                    if [ "$major_version" -ge 14 ]; then
                        echo "clang:${version_name}"
                        return 0
                    fi
                done
            fi
            
            # Default: prefer GCC
            for version_info in "${available_gcc[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [[ "$version_name" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]] && [ "$major_version" -ge 13 ]; then
                    echo "${version_name}:${version_number}"
                    return 0
                fi
            done
            for version_info in "${available_clang[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [ "$major_version" -ge 14 ]; then
                    echo "clang:${version_name}"
                    return 0
                fi
            done
            ;;
        23)
            # C++23: GCC 13+ or Clang 14+ (for <format> support)
            # Check compiler preference first
            if [ "$compiler_preference" = "clang" ]; then
                for version_info in "${available_clang[@]}"; do
                    local version_name=$(echo "$version_info" | cut -d':' -f1)
                    local version_number=$(echo "$version_info" | cut -d':' -f2)
                    local major_version=$(get_compiler_major_version "$version_number")
                    if [ "$major_version" -ge 14 ]; then
                        echo "clang:${version_name}"
                        return 0
                    fi
                done
            fi
            
            # Default: prefer GCC
            for version_info in "${available_gcc[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [[ "$version_name" =~ ^(gcc|scl-gcc-toolset-|homebrew-gcc-) ]] && [ "$major_version" -ge 13 ]; then
                    echo "${version_name}:${version_number}"
                    return 0
                fi
            done
            for version_info in "${available_clang[@]}"; do
                local version_name=$(echo "$version_info" | cut -d':' -f1)
                local version_number=$(echo "$version_info" | cut -d':' -f2)
                local major_version=$(get_compiler_major_version "$version_number")
                if [ "$major_version" -ge 14 ]; then
                    echo "clang:${version_name}"
                    return 0
                fi
            done
            ;;
        *)
            print_error "Unsupported C++ standard: $cxx_standard"
            return 1
            ;;
    esac
    
    print_error "No compatible compiler found for C++${cxx_standard}"
    return 1
}

# Function to set up compiler environment variables
setup_compiler_environment() {
    local compiler_info="$1"
    local compiler_type=$(echo "$compiler_info" | cut -d':' -f1)
    local compiler_name=$(echo "$compiler_info" | cut -d':' -f2)
    
    if [[ "$compiler_type" == "homebrew-gcc-15" ]]; then
        export CC="/opt/homebrew/opt/gcc/bin/gcc-15"
        export CXX="/opt/homebrew/opt/gcc/bin/g++-15"
    elif [[ "$compiler_type" == "homebrew-gcc-15-intel" ]]; then
        export CC="/usr/local/opt/gcc/bin/gcc-15"
        export CXX="/usr/local/opt/gcc/bin/g++-15"
    elif [ "$compiler_type" = "gcc" ]; then
        export CC="gcc"
        export CXX="g++"
    elif [[ "$compiler_type" =~ ^gcc-[0-9]+$ ]]; then
        # Handle versioned GCC (e.g., gcc-10, gcc-11, gcc-13)
        export CC="$compiler_type"
        export CXX="${compiler_type/gcc/g++}"
    elif [ "$compiler_type" = "clang" ]; then
        export CC="$compiler_name"
        export CXX="${compiler_name/clang/clang++}"
    elif [[ "$compiler_type" =~ ^clang-[0-9]+$ ]]; then
        # Handle versioned Clang (e.g., clang-18, clang-19)
        export CC="$compiler_type"
        export CXX="$compiler_type++"
    elif [[ "$compiler_type" =~ ^scl-gcc-toolset- ]]; then
        # Extract version from compiler type (e.g., scl-gcc-toolset-14 -> 14)
        local version=$(echo "$compiler_type" | sed 's/scl-gcc-toolset-//')
        
        # Create temporary wrapper scripts for SCL compilers
        local random_suffix="$$"
        local cc_wrapper="/tmp/heimdall-cc-scl-gcc-toolset-${version}-${random_suffix}.sh"
        local cxx_wrapper="/tmp/heimdall-cxx-scl-gcc-toolset-${version}-${random_suffix}.sh"
        
        # Create CC wrapper script
        cat > "$cc_wrapper" << EOF
#!/bin/bash
exec scl enable gcc-toolset-${version} -- gcc "\$@"
EOF
        chmod +x "$cc_wrapper"
        
        # Create CXX wrapper script
        cat > "$cxx_wrapper" << EOF
#!/bin/bash
exec scl enable gcc-toolset-${version} -- g++ "\$@"
EOF
        chmod +x "$cxx_wrapper"
        
        export CC="$cc_wrapper"
        export CXX="$cxx_wrapper"
        export SCL_GCC_TOOLSET_VERSION="$version"
        # Set environment variables that will be used by the build script
        export SCL_ENV="gcc-toolset-${version}"
    else
        print_error "Unknown compiler type: $compiler_type"
        return 1
    fi
    
    # Verify the compiler is available
    if [[ "$compiler_type" =~ ^scl-gcc-toolset- ]]; then
        # For SCL compilers, check if the wrapper scripts exist and are executable
        if [ ! -x "$CC" ]; then
            print_error "SCL CC wrapper script not found: $CC"
            return 1
        fi
        if [ ! -x "$CXX" ]; then
            print_error "SCL CXX wrapper script not found: $CXX"
            return 1
        fi
    elif [[ "$compiler_type" =~ ^homebrew-gcc- ]]; then
        # For Homebrew GCC, check if the executable exists
        if [ ! -x "$CC" ]; then
            print_error "Homebrew GCC compiler not found: $CC"
            return 1
        fi
        
        if [ ! -x "$CXX" ]; then
            print_error "Homebrew GCC++ compiler not found: $CXX"
            return 1
        fi
    else
        # For regular compilers, use command -v
        if ! command -v "$CC" >/dev/null 2>&1; then
            print_error "C compiler not found: $CC"
            return 1
        fi
        
        if ! command -v "$CXX" >/dev/null 2>&1; then
            print_error "C++ compiler not found: $CXX"
            return 1
        fi
    fi
    
    # Get compiler versions
    if [[ "$compiler_type" =~ ^scl-gcc-toolset- ]]; then
        # For SCL compilers, use the version we already detected
        export CC_VERSION="$compiler_name"
        export CXX_VERSION="$compiler_name"
        export CC_MAJOR_VERSION=$(get_compiler_major_version "$CC_VERSION")
        export CXX_MAJOR_VERSION=$(get_compiler_major_version "$CXX_VERSION")
    elif [[ "$compiler_type" =~ ^homebrew-gcc- ]]; then
        # For Homebrew GCC, use the version we already parsed
        export CC_VERSION="$compiler_name"
        export CXX_VERSION="$compiler_name"
        export CC_MAJOR_VERSION=$(get_compiler_major_version "$CC_VERSION")
        export CXX_MAJOR_VERSION=$(get_compiler_major_version "$CXX_VERSION")
    elif [[ "$compiler_type" == "clang" ]]; then
        # For Clang, parse version correctly
        export CC_VERSION="$($CC --version | head -n1 | cut -d' ' -f4)"
        export CXX_VERSION="$($CXX --version | head -n1 | cut -d' ' -f4)"
        export CC_MAJOR_VERSION=$(get_compiler_major_version "$CC_VERSION")
        export CXX_MAJOR_VERSION=$(get_compiler_major_version "$CXX_VERSION")
    else
        export CC_VERSION="$($CC --version | head -n1 | cut -d' ' -f3 | sed 's/)//')"
        export CXX_VERSION="$($CXX --version | head -n1 | cut -d' ' -f3 | sed 's/)//')"
        export CC_MAJOR_VERSION=$(get_compiler_major_version "$CC_VERSION")
        export CXX_MAJOR_VERSION=$(get_compiler_major_version "$CXX_VERSION")
    fi
    
    print_success "Using compiler: $CC_VERSION (major: $CC_MAJOR_VERSION)"
    print_status "C compiler: $CC"
    print_status "C++ compiler: $CXX"
}

# Function to show available compilers
show_available_compilers() {
    print_status "Detecting available compilers..."
    local gcc_versions=($(detect_gcc_versions))
    local clang_versions=($(detect_clang_versions))
    
    if [ ${#gcc_versions[@]} -eq 0 ] && [ ${#clang_versions[@]} -eq 0 ]; then
        print_error "No compilers found"
        return 1
    fi
    
    print_success "Available GCC versions:"
    for version_info in "${gcc_versions[@]}"; do
        local version_name=$(echo "$version_info" | cut -d':' -f1)
        local version_number=$(echo "$version_info" | cut -d':' -f2)
        echo "  - $version_name: $version_number"
    done
    
    if [ ${#clang_versions[@]} -gt 0 ]; then
        print_success "Available Clang versions:"
        for version_info in "${clang_versions[@]}"; do
            local version_name=$(echo "$version_info" | cut -d':' -f1)
            local version_number=$(echo "$version_info" | cut -d':' -f2)
            echo "  - $version_name: $version_number"
        done
    fi
}

# Main function
main() {
    local quiet=0
    if [[ "$1" == "--quiet" ]]; then
        quiet=1
        shift
    fi
    local cxx_standard="$1"
    local compiler_preference="$2"
    
    if [ -z "$cxx_standard" ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "C++ standard not specified"
            echo "Usage: $0 [--quiet] <cxx_standard> [compiler_preference]"
            echo "  cxx_standard: 11, 14, 17, 20, or 23"
            echo "  compiler_preference: gcc, clang (optional)"
        fi
        exit 1
    fi
    
    # Detect available versions
    local available_gcc=($(detect_gcc_versions))
    local available_clang=($(detect_clang_versions))
    
    if [ ${#available_gcc[@]} -eq 0 ] && [ ${#available_clang[@]} -eq 0 ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "No compilers found"
        fi
        exit 1
    fi
    
    # Select appropriate compiler
    local selected_compiler=$(select_compiler "$cxx_standard" "$compiler_preference" "${available_gcc[@]}" "${available_clang[@]}")
    
    if [ $? -ne 0 ]; then
        if [[ $quiet -eq 0 ]]; then
            print_error "Failed to select compiler for C++${cxx_standard}"
            show_available_compilers
        fi
        exit 1
    fi
    
    # Set up environment
    setup_compiler_environment "$selected_compiler" > /dev/null
    
    # Only export variables if setup was successful
    if [ $? -eq 0 ]; then
        # Export variables for use by parent script (only these lines for eval)
        echo "CC=$CC"
        echo "CXX=$CXX"
        echo "CC_VERSION=$CC_VERSION"
        echo "CXX_VERSION=$CXX_VERSION"
        echo "CC_MAJOR_VERSION=$CC_MAJOR_VERSION"
        echo "CXX_MAJOR_VERSION=$CXX_MAJOR_VERSION"
        
        # Export SCL environment variables if they exist
        if [ -n "$SCL_ENV" ]; then
            echo "SCL_ENV=$SCL_ENV"
        fi
        if [ -n "$SCL_GCC_TOOLSET_VERSION" ]; then
            echo "SCL_GCC_TOOLSET_VERSION=$SCL_GCC_TOOLSET_VERSION"
        fi
    else
        exit 1
    fi

    # If not quiet and run directly, print status
    if [[ $quiet -eq 0 && "${BASH_SOURCE[0]}" == "${0}" ]]; then
        print_success "Using compiler: $CC_VERSION (major: $CC_MAJOR_VERSION)"
        print_status "C compiler: $CC"
        print_status "C++ compiler: $CXX"
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi 