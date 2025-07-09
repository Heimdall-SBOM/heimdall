#!/bin/bash

# Build script for C++17 compatibility
echo "Building Heimdall with C++17 standard..."

# Clean previous build
rm -rf build_cpp17

# Configure with C++17
cmake -B build_cpp17 -DCMAKE_BUILD_TYPE=Release -DUSE_CPP23=OFF

# Build
cmake --build build_cpp17 -j$(nproc)

echo "C++17 build complete!"
echo "To run tests: ctest --test-dir build_cpp17 --output-on-failure" 