#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
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

EXAMPLE_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$EXAMPLE_DIR/build"
BINARY_NAME="taskmgr"
SBOM_NAME="taskmgr.cyclonedx.json"
PLUGIN_PATH="$EXAMPLE_DIR/../../build-cpp17/lib/heimdall-lld.so"
SBOM_TOOL_PATH="$EXAMPLE_DIR/../../build-cpp17/src/tools/heimdall-sbom"

# Clean previous builds
print_info "Cleaning previous build artifacts..."
rm -rf "$BUILD_DIR" "$SBOM_NAME"

# Configure and build with CMake (Debug mode)
print_info "Configuring project with CMake (Debug mode, DWARF info enabled)..."
cmake -S "$EXAMPLE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
print_info "Building example..."
cmake --build "$BUILD_DIR" -- -j$(nproc)

# Find the binary
BINARY_PATH="$BUILD_DIR/$BINARY_NAME"
if [ ! -f "$BINARY_PATH" ]; then
    BINARY_PATH=$(find "$BUILD_DIR" -type f -executable -name "$BINARY_NAME" | head -n 1)
fi
if [ ! -f "$BINARY_PATH" ]; then
    print_error "Could not find built binary: $BINARY_NAME"
    exit 1
fi
print_success "Built binary: $BINARY_PATH"

# Optionally run the binary
echo
print_info "Running the example binary (output below):"
"$BINARY_PATH" || print_warning "Binary exited with nonzero status (this may be expected for a demo)"
echo

# Locate heimdall-sbom tool
if [ -x "$SBOM_TOOL_PATH" ]; then
    SBOM_TOOL="$SBOM_TOOL_PATH"
elif command -v heimdall-sbom &>/dev/null; then
    SBOM_TOOL="heimdall-sbom"
else
    print_error "heimdall-sbom tool not found. Please build Heimdall and ensure heimdall-sbom is in ../../build-cpp17/src/tools/ or in your PATH."
    exit 1
fi
print_success "Using heimdall-sbom: $SBOM_TOOL"

# Locate plugin
if [ ! -f "$PLUGIN_PATH" ]; then
    print_error "Heimdall LLD plugin not found at $PLUGIN_PATH. Please build Heimdall."
    exit 1
fi
print_success "Using plugin: $PLUGIN_PATH"

# Generate CycloneDX SBOM with DWARF info
print_info "Generating CycloneDX SBOM with DWARF debug info..."
"$SBOM_TOOL" "$PLUGIN_PATH" "$BINARY_PATH" --format cyclonedx --output "$SBOM_NAME" --extract-debug-info --verbose

if [ -f "$SBOM_NAME" ]; then
    print_success "SBOM generated: $SBOM_NAME"
else
    print_error "SBOM generation failed."
    exit 1
fi

print_success "All steps completed successfully!" 