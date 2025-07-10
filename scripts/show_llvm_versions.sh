#!/bin/bash

# Show LLVM Versions and Compatibility
# Displays available LLVM versions and their compatibility with C++ standards

# Do NOT set -e, so the script never exits early

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

# Source the LLVM version manager functions
source "$(dirname "$0")/llvm_version_manager.sh"

echo "Heimdall LLVM Version Compatibility"
echo "=================================="
echo ""

# Show available versions
print_status "Detecting available LLVM versions..."
available_versions=($(detect_llvm_versions))

if [ ${#available_versions[@]} -eq 0 ]; then
    print_error "No LLVM versions found"
    echo ""
    echo "Installation instructions:"
    echo "  Ubuntu/Debian: sudo apt-get install llvm-18-dev liblld-18-dev"
    echo "  CentOS/RHEL: sudo dnf install llvm-devel-18.1.5 lld-devel-18.1.5"
    echo "  macOS: brew install llvm@18"
    # Do not exit, continue to show the matrix as unavailable
fi

echo "Available LLVM versions:"
for version_info in "${available_versions[@]}"; do
    version_name=$(echo "$version_info" | cut -d':' -f1)
    version_number=$(echo "$version_info" | cut -d':' -f2)
    echo "  - $version_name: $version_number"
done
echo ""

# Check compatibility for each C++ standard
standards=("11" "14" "17" "20" "23")
compat_versions=()
compat_status=()
compat_llvm=()

echo "[DEBUG] Starting compatibility loop"
for standard in "${standards[@]}"; do
    echo "[DEBUG] Checking C++$standard"
    # Try to select a compatible version
    selected_version=$(select_llvm_version "$standard" "${available_versions[@]}")
    select_result=$?
    if [ $select_result -eq 0 ]; then
        compat_versions+=("$selected_version")
        compat_status+=("✅ Compatible")
        # Get version details
        if [ "$selected_version" = "default" ]; then
            llvm_config="llvm-config"
        else
            llvm_config="llvm-config-${selected_version}"
        fi
        if command -v "$llvm_config" >/dev/null 2>&1; then
            llvm_version=$("$llvm_config" --version 2>/dev/null | head -n1 | cut -d' ' -f3)
            compat_llvm+=("$llvm_version")
        else
            compat_llvm+=("?")
        fi
    else
        compat_versions+=("-")
        compat_status+=("❌ Not available")
        compat_llvm+=("-")
    fi
done

echo "[DEBUG] compat_versions: ${compat_versions[@]}"
echo "[DEBUG] compat_status: ${compat_status[@]}"
echo "[DEBUG] compat_llvm: ${compat_llvm[@]}"

echo "Compatibility Matrix:"
echo "===================="
printf "| %-11s | %-15s | %-18s | %-10s |\n" "C++ Standard" "Required LLVM" "Status" "LLVM Version"
echo "|-------------|-----------------|--------------------|------------|"
for i in "${!standards[@]}"; do
    case ${standards[$i]} in
        11|14)
            required="7-18"
            ;;
        17)
            required="11+"
            ;;
        20|23)
            required="19+"
            ;;
    esac
    printf "| C++%-9s | %-15s | %-18s | %-10s |\n" \
        "${standards[$i]}" "$required" "${compat_status[$i]}" "${compat_llvm[$i]}"
done

echo ""
echo "Recommendations:"
echo "================"
echo "• For C++11/14: Install LLVM 7-18 (recommended: LLVM 18)"
echo "• For C++17: Install LLVM 11+ (recommended: LLVM 18)"
echo "• For C++20/23: Install LLVM 19+ (recommended: LLVM 19)"
echo ""
echo "Installation commands:"
echo "====================="
echo "Ubuntu/Debian:"
echo "  sudo apt-get install llvm-18-dev liblld-18-dev"
echo "  sudo apt-get install llvm-19-dev liblld-19-dev"
echo ""
echo "CentOS/RHEL:"
echo "  sudo dnf install llvm-devel-18.1.5 lld-devel-18.1.5"
echo "  sudo dnf install llvm-devel-19.0.0 lld-devel-19.0.0"
echo ""
echo "macOS:"
echo "  brew install llvm@18"
echo "  brew install llvm@19"
echo ""
echo "[DEBUG] Script completed" 