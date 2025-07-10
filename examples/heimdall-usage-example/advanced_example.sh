#!/bin/bash

# Advanced Heimdall Usage Example
# Demonstrates different SBOM formats and options

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

print_status "Starting Advanced Heimdall usage example..."

# Clean up any previous builds
print_status "Cleaning previous builds..."
rm -f *.o app-* *.json *.spdx *.cyclonedx.json

# Compile source files
print_status "Compiling source files..."
g++ -c main.cpp -o main.o
g++ -c utils.cpp -o utils.o
g++ -c math.cpp -o math.o
print_success "Compilation completed"

# Check if Heimdall is built
HEIMDALL_BUILD_DIR="../../build-cpp23"
if [[ ! -d "$HEIMDALL_BUILD_DIR" ]]; then
    print_error "Heimdall build directory not found: $HEIMDALL_BUILD_DIR"
    exit 1
fi

# Check if LLD is available
if ! command -v ld.lld >/dev/null 2>&1; then
    print_error "LLD not found. This example requires LLD."
    exit 1
fi

# Example 1: Default JSON format
print_status "Example 1: Default JSON format..."
if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-lld.so" ]]; then
    print_status "Linking with LLD and generating SBOM using wrapper approach..."
    # Link normally with LLD (no plugin interface)
    if g++ -fuse-ld=lld main.o utils.o math.o -o app-default; then
        print_success "LLD linking completed"
        # Use wrapper approach: run heimdall-sbom on the output binary
        HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
        if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
            print_status "Generating SBOM using heimdall-sbom wrapper..."
            "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-default --format spdx --output app-default.json && print_success "SBOM generated using wrapper approach: app-default.json" || print_warning "SBOM generation with wrapper approach failed."
        else
            print_warning "heimdall-sbom tool not found at $HEIMDALL_SBOM_PATH. Please ensure Heimdall is built."
        fi
    else
        print_error "LLD linking failed"
    fi
else
    print_error "Heimdall LLD plugin not found"
    exit 1
fi

# Example 2: SPDX format
print_status "Example 2: SPDX format..."
if g++ -fuse-ld=lld main.o utils.o math.o -o app-spdx; then
    HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
    if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
        "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-spdx --format spdx --output app.spdx && print_success "SPDX SBOM generated: app.spdx" || print_warning "SPDX format example failed."
    else
        print_warning "SPDX format example failed. heimdall-sbom not found."
    fi
else
    print_warning "SPDX format example failed. LLD linking failed."
fi

# Example 3: CycloneDX format
print_status "Example 3: CycloneDX format..."
if g++ -fuse-ld=lld main.o utils.o math.o -o app-cyclonedx; then
    HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
    if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
        "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-cyclonedx --format cyclonedx --output app.cyclonedx.json && print_success "CycloneDX SBOM generated: app.cyclonedx.json" || print_warning "CycloneDX format example failed."
    else
        print_warning "CycloneDX format example failed. heimdall-sbom not found."
    fi
else
    print_warning "CycloneDX format example failed. LLD linking failed."
fi

# Example 4: Verbose output
print_status "Example 4: Verbose output..."
if g++ -fuse-ld=lld main.o utils.o math.o -o app-verbose; then
    HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
    if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
        "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-verbose --format spdx --output app-verbose.json && print_success "Verbose SBOM generated: app-verbose.json" || print_warning "Verbose output example failed."
    else
        print_warning "Verbose output example failed. heimdall-sbom not found."
    fi
else
    print_warning "Verbose output example failed. LLD linking failed."
fi

# Example 5: Custom component name
print_status "Example 5: Custom component name..."
if g++ -fuse-ld=lld main.o utils.o math.o -o app-custom; then
    HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
    if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
        "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-lld.so app-custom --format spdx --output app-custom.json && print_success "Custom component SBOM generated: app-custom.json" || print_warning "Custom component example failed."
    else
        print_warning "Custom component example failed. heimdall-sbom not found."
    fi
else
    print_warning "Custom component example failed. LLD linking failed."
fi

# Example 6: Gold plugin interface (Linux only)
print_status "Example 6: Gold plugin interface (Linux only)..."
if command -v ld.gold >/dev/null 2>&1; then
    if [[ -f "$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so" ]]; then
        print_status "Attempting Gold linking with Heimdall plugin..."
        if g++ -fuse-ld=gold -Wl,--plugin=$HEIMDALL_BUILD_DIR/lib/heimdall-gold.so \
            -Wl,--plugin-opt=sbom-output=app-gold-plugin.json \
            main.o utils.o math.o -o app-gold-plugin 2>/dev/null; then
            print_success "Gold plugin SBOM generated: app-gold-plugin.json"
        else
            print_warning "Gold plugin linking failed. Trying wrapper approach..."
            if g++ -fuse-ld=gold main.o utils.o math.o -o app-gold-wrapper; then
                HEIMDALL_SBOM_PATH="../../build-cpp23/src/tools/heimdall-sbom"
                if [[ -x "$HEIMDALL_SBOM_PATH" ]]; then
                    "$HEIMDALL_SBOM_PATH" ../../build-cpp23/lib/heimdall-gold.so app-gold-wrapper --format spdx --output app-gold-wrapper.json && print_success "Gold wrapper SBOM generated: app-gold-wrapper.json" || print_warning "Gold wrapper SBOM generation failed."
                else
                    print_warning "Gold wrapper SBOM generation failed. heimdall-sbom not found."
                fi
            else
                print_warning "Gold wrapper linking failed."
            fi
        fi
    else
        print_warning "Heimdall Gold plugin not found. Skipping Gold example."
    fi
else
    print_warning "Gold not found. Skipping Gold example."
fi

# Run all versions
echo ""
echo "=== Running All Versions ==="
for app in app-default app-spdx app-cyclonedx app-verbose app-custom; do
    if [[ -f "$app" ]]; then
        echo "Running $app:"
        ./$app
        echo ""
    fi
done

# Show generated files
echo "=== Generated Files ==="
ls -la app-* *.json *.spdx *.cyclonedx.json 2>/dev/null || print_warning "No generated files found"

# Show SBOM format differences
echo ""
echo "=== SBOM Format Comparison ==="

if [[ -f "app-default.json" ]]; then
    echo "Default JSON format (first 5 lines):"
    head -5 app-default.json
    echo "..."
else
    echo "Default JSON format: No SBOM generated (plugin not compatible)"
fi

echo ""
if [[ -f "app.spdx" ]]; then
    echo "SPDX format (first 5 lines):"
    head -5 app.spdx
    echo "..."
else
    echo "SPDX format: No SBOM generated (plugin not compatible)"
fi

echo ""
if [[ -f "app.cyclonedx.json" ]]; then
    echo "CycloneDX format (first 5 lines):"
    head -5 app.cyclonedx.json
    echo "..."
else
    echo "CycloneDX format: No SBOM generated (plugin not compatible)"
fi

echo ""
if [[ -f "app-verbose.json" ]]; then
    echo "Verbose format (first 5 lines):"
    head -5 app-verbose.json
    echo "..."
else
    echo "Verbose format: No SBOM generated (plugin not compatible)"
fi

echo ""
if [[ -f "app-custom.json" ]]; then
    echo "Custom component format (first 5 lines):"
    head -5 app-custom.json
    echo "..."
else
    echo "Custom component format: No SBOM generated (plugin not compatible)"
fi

print_success "Advanced Heimdall usage example completed!"
echo ""
print_status "Summary of generated SBOMs:"
if [[ -f "app-default.json" ]]; then
    echo "  ✓ Default JSON: app-default.json"
else
    echo "  ⚠ Default JSON: Not generated (plugin not compatible)"
fi
if [[ -f "app.spdx" ]]; then
    echo "  ✓ SPDX: app.spdx"
else
    echo "  ⚠ SPDX: Not generated (plugin not compatible)"
fi
if [[ -f "app.cyclonedx.json" ]]; then
    echo "  ✓ CycloneDX: app.cyclonedx.json"
else
    echo "  ⚠ CycloneDX: Not generated (plugin not compatible)"
fi
if [[ -f "app-verbose.json" ]]; then
    echo "  ✓ Verbose: app-verbose.json"
else
    echo "  ⚠ Verbose: Not generated (plugin not compatible)"
fi
if [[ -f "app-custom.json" ]]; then
    echo "  ✓ Custom component: app-custom.json"
else
    echo "  ⚠ Custom component: Not generated (plugin not compatible)"
fi
echo ""
print_status "Note: Gold plugin requires elfutils/libelf dependencies."
print_status "For Gold plugin functionality, install: sudo yum install elfutils-devel"
print_status "Wrapper approach works regardless of plugin dependencies." 