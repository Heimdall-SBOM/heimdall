#!/bin/bash

# Build script for C++ OpenSSL example with SBOM generation
# Demonstrates Heimdall plugin usage with LLD and Gold linkers

set -e

echo "[INFO] Building C++ OpenSSL example with SBOM generation"

# Check if plugins exist
if [ ! -f "../../build/lib/heimdall-lld.so" ]; then
    echo "[ERROR] LLD plugin not found. Please build the project first."
    exit 1
fi

if [ ! -f "../../build/lib/heimdall-gold.so" ]; then
    echo "[ERROR] Gold plugin not found. Please build the project first."
    exit 1
fi

echo "[SUCCESS] Found Gold plugin: ../../build/lib/heimdall-gold.so"

# Compile the C++ source
echo "[INFO] Compiling main.cpp..."
g++ -c main.cpp -o main.o -Wall -Wextra -std=c++17

# Build with LLD
echo "[INFO] Building with LLD..."
g++ main.o -o openssl_cpp_example_lld -lcrypto -lssl -fuse-ld=lld
echo "[SUCCESS] Generated LLD binary: openssl_cpp_example_lld"

# Generate SBOMs using LLD plugin
echo "[INFO] Generating SPDX 2.3 SBOM using LLD plugin..."
../../build/src/tools/sbom_loader ../../build/lib/heimdall-lld.so openssl_cpp_example_lld --format spdx-2.3 --output openssl_cpp_example.spdx.json
echo "[SUCCESS] Generated SPDX SBOM: openssl_cpp_example.spdx.json"

echo "[INFO] Generating SPDX 3.0 SBOM using LLD plugin..."
../../build/src/tools/sbom_loader ../../build/lib/heimdall-lld.so openssl_cpp_example_lld --format spdx-3.0 --output openssl_cpp_example.spdx3.json
echo "[SUCCESS] Generated SPDX 3.0 SBOM: openssl_cpp_example.spdx3.json"

echo "[INFO] Generating CycloneDX 1.6 SBOM using LLD plugin..."
../../build/src/tools/sbom_loader ../../build/lib/heimdall-lld.so openssl_cpp_example_lld --format cyclonedx-1.6 --output openssl_cpp_example.cyclonedx.json
echo "[SUCCESS] Generated CycloneDX SBOM: openssl_cpp_example.cyclonedx.json"

# Build with Gold
echo "[INFO] Building with Gold..."
g++ main.o -o openssl_cpp_example_gold -lcrypto -lssl -fuse-ld=gold
echo "[SUCCESS] Generated Gold binary: openssl_cpp_example_gold"

# Generate SBOM using Gold plugin
echo "[INFO] Generating SBOM using Gold plugin..."
../../build/src/tools/sbom_loader ../../build/lib/heimdall-gold.so openssl_cpp_example_gold --format spdx-2.3 --output openssl_cpp_example_gold.json
echo "[SUCCESS] Generated Gold SBOM: openssl_cpp_example_gold.json"

# Test the binaries
echo "[INFO] Testing LLD binary..."
./openssl_cpp_example_lld "Hello, Heimdall!"

echo "[INFO] Testing Gold binary..."
./openssl_cpp_example_gold "Hello, Heimdall!"

# Show generated files
echo "[INFO] Generated SBOM files:"
ls -la *.json

echo "[SUCCESS] Build completed successfully!"
echo "[INFO] You can now examine the generated SBOM files:"
echo "  - openssl_cpp_example.spdx.json (SPDX 2.3 format)"
echo "  - openssl_cpp_example.spdx3.json (SPDX 3.0 format)"
echo "  - openssl_cpp_example.cyclonedx.json (CycloneDX 1.6 format)"
echo "  - openssl_cpp_example_gold.json (Gold plugin format)" 