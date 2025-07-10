#!/bin/bash

# Heimdall Usage Example Script
# This script demonstrates how to use Heimdall as a linker plugin

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
    print_error "main.cpp not found. Please run this script from the heimdall-usage-example directory."
    exit 1
fi

print_status "Starting Heimdall usage example..."

# Clean up any previous builds
print_status "Cleaning previous builds..."
rm -f *.o app-lld app-gold *.json *.spdx

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

# Step 2: Link with LLD and Heimdall plugin
print_status "Step 2: Linking with LLD and generating SBOM using wrapper approach..."

# Check if LLD is available
if command -v ld.lld >/dev/null 2>&1; then
    # Check if Heimdall LLD plugin exists
    if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-lld.so" ]]; then
        print_status "Linking with LLD and generating SBOM using wrapper approach..."
        # Link normally with LLD (no plugin interface)
        if g++ -fuse-ld=lld main.o utils.o math.o -o app-lld; then
            print_success "LLD linking completed"
            # Use wrapper approach: run heimdall-sbom on the output binary
            HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
            if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                print_status "Generating SBOM using heimdall-sbom wrapper..."
                "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-lld --format spdx --output app-lld-sbom.json && print_success "SBOM generated using wrapper approach: app-lld-sbom.json" || print_warning "SBOM generation with wrapper approach failed."
            else
                print_warning "heimdall-sbom tool not found at $HEIMDALL_SBOM_PATH. Please ensure Heimdall is built."
            fi
        else
            print_error "LLD linking failed"
        fi
    elif [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-lld.dylib" ]]; then
        print_status "Linking with LLD and generating SBOM using wrapper approach (macOS)..."
        # Link normally with LLD (no plugin interface)
        if g++ -fuse-ld=lld main.o utils.o math.o -o app-lld; then
            print_success "LLD linking completed"
            # Use wrapper approach: run heimdall-sbom on the output binary
            HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
            if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                print_status "Generating SBOM using heimdall-sbom wrapper..."
                "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.dylib app-lld --format spdx --output app-lld-sbom.json && print_success "SBOM generated using wrapper approach: app-lld-sbom.json" || print_warning "SBOM generation with wrapper approach failed."
            else
                print_warning "heimdall-sbom tool not found at $HEIMDALL_SBOM_PATH. Please ensure Heimdall is built."
            fi
        else
            print_error "LLD linking failed"
        fi
    else
        print_error "Heimdall LLD plugin not found in $HEIMDALL_BUILD_DIR/lib/"
        print_error "Available files:"
        ls -la $HEIMDALL_BUILD_DIR/lib/ 2>/dev/null || print_error "  No lib directory found"
    fi
else
    print_warning "LLD not found. Skipping LLD linking."
fi

# Step 3: Link with Gold and Heimdall plugin (Linux only)
print_status "Step 3: Linking with Gold and Heimdall plugin (Linux only)..."

# Check if Gold is available
if command -v ld.gold >/dev/null 2>&1; then
    # Check if Heimdall Gold plugin exists
    if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so" ]]; then
        print_status "Attempting Gold linking with Heimdall plugin..."
        if g++ -fuse-ld=gold -Wl,--plugin=$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so \
            -Wl,--plugin-opt=sbom-output=app-gold-sbom.json \
            main.o utils.o math.o -o app-gold 2>/dev/null; then
            print_success "Gold linking completed with plugin"
        else
            print_warning "Gold plugin linking failed. This may be due to:"
            print_warning "  - Missing dependencies (elfutils, libelf, etc.)"
            print_warning "  - Incompatible Gold version"
            print_warning "  - Plugin dependency issues"
            print_warning "Trying without plugin..."
            if g++ -fuse-ld=gold main.o utils.o math.o -o app-gold; then
                print_success "Gold linking completed (without plugin)"
                # Use wrapper approach: run heimdall-sbom on the output binary
                HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
                if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                    print_status "Generating SBOM using heimdall-sbom wrapper for Gold..."
                    "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-gold.so app-gold --format spdx --output app-gold-sbom.json && print_success "SBOM generated using wrapper approach: app-gold-sbom.json" || print_warning "SBOM generation with wrapper approach failed."
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

# Run LLD version if it exists
if [[ -f "app-lld" ]]; then
    echo "LLD version:"
    ./app-lld
    echo ""
else
    print_warning "LLD version not available"
fi

# Run Gold version if it exists
if [[ -f "app-gold" ]]; then
    echo "Gold version:"
    ./app-gold
    echo ""
else
    print_warning "Gold version not available"
fi

# Show generated files
echo "=== Generated Files ==="
ls -la app-* *.json *.spdx 2>/dev/null || print_warning "No generated files found"

# Show SBOM content (first few lines)
echo ""
echo "=== SBOM Content Preview ==="

if [[ -f "app-lld-sbom.json" ]]; then
    echo "LLD SBOM (first 10 lines):"
    head -10 app-lld-sbom.json
    echo "..."
    echo ""
fi

if [[ -f "app-gold-sbom.json" ]]; then
    echo "Gold SBOM (first 10 lines):"
    head -10 app-gold-sbom.json
    echo "..."
    echo ""
fi

print_success "Heimdall usage example completed!"
echo ""
print_status "Summary:"
if [[ -f "app-lld" ]]; then
    echo "  ✓ LLD version created: app-lld"
    if [[ -f "app-lld-sbom.json" ]]; then
        echo "  ✓ LLD SBOM generated: app-lld-sbom.json"
    else
        echo "  ⚠ LLD SBOM: Not generated (plugin not compatible)"
    fi
fi
if [[ -f "app-gold" ]]; then
    echo "  ✓ Gold version created: app-gold"
    if [[ -f "app-gold-sbom.json" ]]; then
        echo "  ✓ Gold SBOM generated: app-gold-sbom.json"
    else
        echo "  ⚠ Gold SBOM: Not generated (plugin not compatible)"
    fi
fi
echo ""
if [[ -f "app-lld-sbom.json" ]] || [[ -f "app-gold-sbom.json" ]]; then
    print_status "You can now examine the generated SBOM files to see the component information!"
else
    print_warning "No SBOM files were generated due to plugin dependency issues."
    print_warning "For Gold plugin functionality, install: sudo yum install elfutils-devel"
    print_warning "Wrapper approach works regardless of plugin dependencies."
fi 