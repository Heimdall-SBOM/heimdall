#!/bin/bash

# SPDX-only Heimdall Usage Example Script
# This script demonstrates how to use Heimdall to generate only SPDX SBOMs

set -e

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

# Check if we're in the right directory
if [[ ! -f "main.cpp" ]]; then
    print_error "main.cpp not found. Please run this script from the heimdall-usage-spdx-example directory."
    exit 1
fi

print_status "Starting SPDX-only Heimdall usage example..."

# Clean up any previous builds
print_status "Cleaning previous builds..."
rm -f *.o app-lld app-gold *.spdx

# Step 1: Compile source files to object files
print_status "Step 1: Compiling source files to object files..."
g++ -c main.cpp -o main.o
g++ -c utils.cpp -o utils.o
g++ -c math.cpp -o math.o
print_success "Compilation completed"

# Check if Heimdall is built
HEIMDALL_BUILD_DIR="../../build-cpp23"
if [[ ! -d "$HEIMDALL_BUILD_DIR" ]]; then
    print_error "Heimdall build directory not found: $HEIMDALL_BUILD_DIR"
    print_error "Please build Heimdall first:"
    print_error "  cd ../../"
    print_error "  ./scripts/build.sh --standard 23 --compiler gcc --tests"
    exit 1
fi

# Step 2: Link with LLD and Heimdall plugin (wrapper approach)
print_status "Step 2: Linking with LLD and generating SPDX SBOM using wrapper approach..."

if command -v ld.lld >/dev/null 2>&1; then
    if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-lld.so" ]]; then
        if g++ -fuse-ld=lld main.o utils.o math.o -o app-lld; then
            print_success "LLD linking completed"
            HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
            if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                print_status "Generating SPDX SBOM using heimdall-sbom wrapper..."
                "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-lld --format spdx --output app-lld.spdx && print_success "SPDX SBOM generated: app-lld.spdx" || print_warning "SPDX SBOM generation failed."
            else
                print_warning "heimdall-sbom tool not found at $HEIMDALL_SBOM_PATH. Please ensure Heimdall is built."
            fi
        else
            print_error "LLD linking failed"
        fi
    else
        print_error "Heimdall LLD plugin not found in $HEIMDALL_BUILD_DIR/lib/"
    fi
else
    print_warning "LLD not found. Skipping LLD linking."
fi

# Step 3: Link with Gold and Heimdall plugin (Linux only)
print_status "Step 3: Linking with Gold and Heimdall plugin (Linux only)..."

if command -v ld.gold >/dev/null 2>&1; then
    if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so" ]]; then
        print_status "Attempting Gold linking with Heimdall plugin..."
        if g++ -fuse-ld=gold -Wl,--plugin=$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so \
            -Wl,--plugin-opt=sbom-output=app-gold.spdx \
            main.o utils.o math.o -o app-gold 2>/dev/null; then
            print_success "Gold linking completed with plugin"
        else
            print_warning "Gold plugin linking failed. Trying wrapper approach..."
            if g++ -fuse-ld=gold main.o utils.o math.o -o app-gold; then
                print_success "Gold linking completed (without plugin)"
                HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
                if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                    print_status "Generating SPDX SBOM using heimdall-sbom wrapper for Gold..."
                    "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-gold.so app-gold --format spdx --output app-gold.spdx && print_success "SPDX SBOM generated: app-gold.spdx" || print_warning "SPDX SBOM generation failed."
                else
                    print_warning "heimdall-sbom tool not found at $HEIMDALL_SBOM_PATH. Please ensure Heimdall is built."
                fi
            else
                print_error "Gold linking failed"
            fi
        fi
    else
        print_error "Heimdall Gold plugin not found in $HEIMDALL_BUILD_DIR/lib/"
    fi
else
    print_warning "Gold not found. Skipping Gold linking."
fi

# Step 4: Run the programs and show results
print_status "Step 4: Running programs and showing results..."

echo ""
echo "=== Program Output ==="

if [[ -f "app-lld" ]]; then
    echo "LLD version:"
    ./app-lld
    echo ""
else
    print_warning "LLD version not available"
fi

if [[ -f "app-gold" ]]; then
    echo "Gold version:"
    ./app-gold
    echo ""
else
    print_warning "Gold version not available"
fi

echo "=== Generated SPDX Files ==="
ls -la *.spdx 2>/dev/null || print_warning "No SPDX files found"

echo ""
if [[ -f "app-lld.spdx" ]]; then
    echo "LLD SPDX (first 10 lines):"
    head -10 app-lld.spdx
    echo "..."
    echo ""
fi
if [[ -f "app-gold.spdx" ]]; then
    echo "Gold SPDX (first 10 lines):"
    head -10 app-gold.spdx
    echo "..."
    echo ""
fi

print_success "SPDX-only Heimdall usage example completed!" 