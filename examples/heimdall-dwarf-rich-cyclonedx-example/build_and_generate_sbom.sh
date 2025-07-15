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
    # Try to find the binary by searching the build dir
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

# Locate Heimdall
HEIMDALL_BIN=""
if [ -f "$EXAMPLE_DIR/../../build-cpp17/src/heimdall" ]; then
    HEIMDALL_BIN="$EXAMPLE_DIR/../../build-cpp17/src/heimdall"
elif command -v heimdall &>/dev/null; then
    HEIMDALL_BIN="heimdall"
elif [ -f "$EXAMPLE_DIR/../../src/heimdall" ]; then
    HEIMDALL_BIN="$EXAMPLE_DIR/../../src/heimdall"
fi
if [ -z "$HEIMDALL_BIN" ]; then
    print_error "Heimdall binary not found. Please build Heimdall and ensure it is in your PATH or in ../../build-cpp17/src/heimdall."
    exit 1
fi
print_success "Using Heimdall: $HEIMDALL_BIN"

# Generate CycloneDX SBOM with DWARF info
print_info "Generating CycloneDX SBOM with DWARF debug info..."
"$HEIMDALL_BIN" --input "$BINARY_PATH" --format cyclonedx --output "$SBOM_NAME" --extract-debug-info --verbose

if [ -f "$SBOM_NAME" ]; then
    print_success "SBOM generated: $SBOM_NAME"
else
    print_error "SBOM generation failed."
    exit 1
fi

print_success "All steps completed successfully!" 