#!/bin/bash

# Simple test script for the Heimdall usage example
# This script compiles and runs the example to verify it works

set -e

echo "Testing Heimdall usage example..."

# Clean previous builds
rm -f *.o test-app

# Compile the example
echo "Compiling example..."
g++ -std=c++17 main.cpp utils.cpp math.cpp -o test-app

# Run the example
echo "Running example..."
./test-app

# Check the output
echo ""
echo "Expected output should include:"
echo "- Hello from Heimdall example!"
echo "- String: Hello, World!"
echo "- Length: 13"
echo "- Math: 2 + 3 = 5"
echo "- Math: 10 * 5 = 50"

echo ""
echo "✓ Example test completed successfully!"
echo "The example is ready to use with Heimdall plugins." 