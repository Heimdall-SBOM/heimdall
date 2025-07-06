#!/bin/bash

set -e

echo "=== Heimdall SBOM Debug Script ==="
echo "Current directory: $(pwd)"
echo "Date: $(date)"
echo

# Check if we're in the right directory
if [ ! -f "tests/heimdall_tests" ]; then
    echo "ERROR: heimdall_tests not found in tests/ directory"
    exit 1
fi

echo "=== Test Executable Info ==="
ls -la tests/heimdall_tests
file tests/heimdall_tests
echo

echo "=== Library Dependencies ==="
ldd tests/heimdall_tests
echo

echo "=== Available Plugins ==="
ls -la *.so
echo

echo "=== Running SBOM Consistency Tests ==="
# Run the tests with verbose output and capture both stdout and stderr
./tests/heimdall_tests --gtest_filter="*PluginSBOMConsistencyTest*" --gtest_output=xml:sbom_test_results.xml 2>&1 | tee /tmp/sbom_test_debug.log

echo
echo "=== Test Results ==="
if [ -f "sbom_test_results.xml" ]; then
    echo "XML results file created"
    ls -la sbom_test_results.xml
else
    echo "No XML results file found"
fi

echo
echo "=== Debug Log ==="
if [ -f "/tmp/sbom_test_debug.log" ]; then
    echo "Debug log created at /tmp/sbom_test_debug.log"
    echo "Last 50 lines:"
    tail -50 /tmp/sbom_test_debug.log
else
    echo "No debug log found"
fi

echo
echo "=== Checking for Generated SBOM Files ==="
find /tmp -name "*test*.spdx" -o -name "*test*.cyclonedx.json" 2>/dev/null | head -10

echo
echo "=== System Library Info ==="
echo "OpenSSL libraries:"
find /usr/lib* -name "libssl.so*" 2>/dev/null | head -5
find /usr/lib* -name "libcrypto.so*" 2>/dev/null | head -5

echo
echo "System C library:"
find /usr/lib* -name "libc.so*" 2>/dev/null | head -5

echo
echo "=== Done ===" 