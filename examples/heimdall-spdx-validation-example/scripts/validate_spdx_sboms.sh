#!/bin/bash
set -e

# Look for build directory in the example's build subdirectory
BUILD_DIR=$(cd $(dirname $0../build && pwd)"
echo Looking for SBOMs in: $BUILD_DIR"

if  ! -d "$BUILD_DIR]; then
    echo "Build directory not found: $BUILD_DIRechoPlease build the example first using:"
    echo "  cd examples/heimdall-spdx-validation-example"
    echo mkdir -p build && cd build"
    echo "  cmake .. && make exit 1
fi

# Look for SPDX SBOM files
SBOM_FILES=($BUILD_DIR/*.spdx)

if ${#SBOM_FILES@]} -eq 0]; then
    echo "No SPDX SBOM files found in $BUILD_DIR"
    exit1fi

echo "Found ${#SBOM_FILES[@]} SPDX SBOM file(s):"
for file in${SBOM_FILES[@]}"; do
    echo "  $(basename $file)"
done

echo "
echo alidation complete - SBOMs found in build directory"
