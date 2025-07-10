#!/bin/bash

# Heimdall Build Compatibility Checker
# Shows compatibility status for all C++ standards and provides installation guidance

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

echo "Heimdall Build Compatibility Checker"
echo "==================================="
echo ""

# Check LLVM versions
print_status "Checking LLVM compatibility..."
LLVM_VERSIONS=($(./scripts/llvm_version_manager.sh --quiet 11 2>/dev/null || echo ""))
LLVM_11_14_AVAILABLE=false
LLVM_17_AVAILABLE=false
LLVM_20_23_AVAILABLE=false

if [ -n "$LLVM_VERSIONS" ]; then
    LLVM_11_14_AVAILABLE=true
fi

LLVM_VERSIONS=($(./scripts/llvm_version_manager.sh --quiet 17 2>/dev/null || echo ""))
if [ -n "$LLVM_VERSIONS" ]; then
    LLVM_17_AVAILABLE=true
fi

LLVM_VERSIONS=($(./scripts/llvm_version_manager.sh --quiet 20 2>/dev/null || echo ""))
if [ -n "$LLVM_VERSIONS" ]; then
    LLVM_20_23_AVAILABLE=true
fi

# Check compiler versions
print_status "Checking compiler compatibility..."
COMPILER_VERSIONS=($(./scripts/compiler_version_manager.sh --quiet 11 2>/dev/null || echo ""))
COMPILER_11_14_AVAILABLE=false
COMPILER_17_AVAILABLE=false
COMPILER_20_23_AVAILABLE=false

if [ -n "$COMPILER_VERSIONS" ]; then
    COMPILER_11_14_AVAILABLE=true
fi

COMPILER_VERSIONS=($(./scripts/compiler_version_manager.sh --quiet 17 2>/dev/null || echo ""))
if [ -n "$COMPILER_VERSIONS" ]; then
    COMPILER_17_AVAILABLE=true
fi

COMPILER_VERSIONS=($(./scripts/compiler_version_manager.sh --quiet 20 2>/dev/null || echo ""))
if [ -n "$COMPILER_VERSIONS" ]; then
    COMPILER_20_23_AVAILABLE=true
fi

# Display compatibility matrix
echo "Compatibility Matrix:"
echo "===================="
echo "| C++ Standard | LLVM Required | Compiler Required | LLVM Status | Compiler Status | Overall Status |"
echo "|-------------|---------------|-------------------|-------------|-----------------|----------------|"

# C++11
if [ "$LLVM_11_14_AVAILABLE" = true ] && [ "$COMPILER_11_14_AVAILABLE" = true ]; then
    LLVM_STATUS="✅ Available"
    COMPILER_STATUS="✅ Available"
    OVERALL_STATUS="✅ Compatible"
else
    LLVM_STATUS="❌ Missing"
    COMPILER_STATUS="❌ Missing"
    OVERALL_STATUS="❌ Incompatible"
fi
echo "| C++11        | 7-18           | GCC 4.8+ / Clang 3.3+ | $LLVM_STATUS | $COMPILER_STATUS | $OVERALL_STATUS |"

# C++14
if [ "$LLVM_11_14_AVAILABLE" = true ] && [ "$COMPILER_11_14_AVAILABLE" = true ]; then
    LLVM_STATUS="✅ Available"
    COMPILER_STATUS="✅ Available"
    OVERALL_STATUS="✅ Compatible"
else
    LLVM_STATUS="❌ Missing"
    COMPILER_STATUS="❌ Missing"
    OVERALL_STATUS="❌ Incompatible"
fi
echo "| C++14        | 7-18           | GCC 6+ / Clang 3.4+   | $LLVM_STATUS | $COMPILER_STATUS | $OVERALL_STATUS |"

# C++17
if [ "$LLVM_17_AVAILABLE" = true ] && [ "$COMPILER_17_AVAILABLE" = true ]; then
    LLVM_STATUS="✅ Available"
    COMPILER_STATUS="✅ Available"
    OVERALL_STATUS="✅ Compatible"
else
    LLVM_STATUS="❌ Missing"
    COMPILER_STATUS="❌ Missing"
    OVERALL_STATUS="❌ Incompatible"
fi
echo "| C++17        | 11+            | GCC 7+ / Clang 5+     | $LLVM_STATUS | $COMPILER_STATUS | $OVERALL_STATUS |"

# C++20
if [ "$LLVM_20_23_AVAILABLE" = true ] && [ "$COMPILER_20_23_AVAILABLE" = true ]; then
    LLVM_STATUS="✅ Available"
    COMPILER_STATUS="✅ Available"
    OVERALL_STATUS="✅ Compatible"
else
    LLVM_STATUS="❌ Missing"
    COMPILER_STATUS="❌ Missing"
    OVERALL_STATUS="❌ Incompatible"
fi
echo "| C++20        | 19+            | GCC 13+ / Clang 14+   | $LLVM_STATUS | $COMPILER_STATUS | $OVERALL_STATUS |"

# C++23
if [ "$LLVM_20_23_AVAILABLE" = true ] && [ "$COMPILER_20_23_AVAILABLE" = true ]; then
    LLVM_STATUS="✅ Available"
    COMPILER_STATUS="✅ Available"
    OVERALL_STATUS="✅ Compatible"
else
    LLVM_STATUS="❌ Missing"
    COMPILER_STATUS="❌ Missing"
    OVERALL_STATUS="❌ Incompatible"
fi
echo "| C++23        | 19+            | GCC 13+ / Clang 14+   | $LLVM_STATUS | $COMPILER_STATUS | $OVERALL_STATUS |"

echo ""
echo "Current System Status:"
echo "====================="

# Show available LLVM versions
print_status "Available LLVM versions:"
./scripts/show_llvm_versions.sh 2>/dev/null | grep -A 20 "Available LLVM versions:" | head -20

# Show available compilers
print_status "Available compilers:"
./scripts/compiler_version_manager.sh 17 2>/dev/null | grep -E "(CC=|CXX=|Using compiler)" || echo "  No compatible compilers found"

echo ""
echo "Installation Recommendations:"
echo "============================"

# Check what's missing and provide installation instructions
if [ "$LLVM_11_14_AVAILABLE" = false ]; then
    echo "❌ Missing LLVM 7-18 for C++11/14 support:"
    echo "   Ubuntu/Debian: sudo apt-get install llvm-18-dev liblld-18-dev"
    echo "   CentOS/RHEL: sudo dnf install llvm-devel-18.1.5 lld-devel-18.1.5"
    echo "   macOS: brew install llvm@18"
    echo ""
fi

if [ "$LLVM_20_23_AVAILABLE" = false ]; then
    echo "❌ Missing LLVM 19+ for C++20/23 support:"
    echo "   Ubuntu/Debian: sudo apt-get install llvm-19-dev liblld-19-dev"
    echo "   CentOS/RHEL: sudo dnf install llvm-devel-19.0.0 lld-devel-19.0.0"
    echo "   macOS: brew install llvm@19"
    echo ""
fi

if [ "$COMPILER_20_23_AVAILABLE" = false ]; then
    echo "❌ Missing GCC 13+ or Clang 14+ for C++20/23 support:"
    echo "   Ubuntu/Debian: sudo apt-get install gcc-13 g++-13"
    echo "   CentOS/RHEL: sudo dnf install gcc-toolset-13"
    echo "   macOS: brew install gcc@13"
    echo ""
fi

echo "✅ Ready to build standards:"
if [ "$LLVM_17_AVAILABLE" = true ] && [ "$COMPILER_17_AVAILABLE" = true ]; then
    echo "   - C++17 (full support with all features)"
fi

if [ "$LLVM_20_23_AVAILABLE" = true ] && [ "$COMPILER_20_23_AVAILABLE" = true ]; then
    echo "   - C++20 (full support with all features)"
    echo "   - C++23 (full support with all features)"
fi

echo ""
echo "To build all compatible standards:"
echo "  ./scripts/build_all_standards.sh"
echo ""
echo "To build a specific standard:"
echo "  ./scripts/build.sh --standard 17"
echo "" 