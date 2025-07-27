#!/bin/bash
set -e

# Get the build directory
BUILD_DIR=$(cd $(dirname $0)/../build && pwd)
echo "Generating SPDX SBOMs in: $BUILD_DIR"

# Check if heimdall-sbom is available
if ! command -v heimdall-sbom >/dev/null 2>&1; then
    echo "Error: heimdall-sbom not found in PATH"
    echo "Please ensure Heimdall is built and heimdall-sbom is available"
    exit 1
fi

# Check if the executable exists
if [ ! -f "$BUILD_DIR/bin/spdx_validation_app" ]; then
    echo "Error: spdx_validation_app not found in $BUILD_DIR/bin/"
    echo "Please build the example first"
    exit 1
fi

# Generate SPDX 2.3 (tag-value format)
echo "Generating SPDX 2.3 (tag-value) SBOM..."
heimdall-sbom --format spdx --spdx-version 2.3 --output "$BUILD_DIR/spdx-validation-example-2.3.spdx" "$BUILD_DIR/bin/spdx_validation_app"

# Generate SPDX 3.0 (JSON format)
echo "Generating SPDX 3.0 (JSON) SBOM..."
heimdall-sbom --format spdx --spdx-version 3.0 --output "$BUILD_DIR/spdx-validation-example-3.0.json" "$BUILD_DIR/bin/spdx_validation_app"

# Generate SPDX 3.0.1 (JSON format)
echo "Generating SPDX 3.0.1 (JSON) SBOM..."
heimdall-sbom --format spdx --spdx-version 3.0.1 --output "$BUILD_DIR/spdx-validation-example-3.0.1.json" "$BUILD_DIR/bin/spdx_validation_app"

echo "\nGenerated SPDX SBOMs:"
ls -la "$BUILD_DIR"/*.spdx "$BUILD_DIR"/*.json 2>/dev/null || echo "No SBOM files found"

echo ""
echo "SBOM generation complete!" 