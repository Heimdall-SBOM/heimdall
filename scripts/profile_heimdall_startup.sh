#!/bin/bash

echo "=== Heimdall-sbom Startup Profiling ==="
echo "======================================"

# Test 1: Measure just the startup time (no processing)
echo "Test 1: Startup time only"
time ./build-cpp17/src/tools/heimdall-sbom --help > /dev/null 2>&1

echo -e "\nTest 2: Startup + minimal processing"
time ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format spdx --output=test_startup.json > /dev/null 2>&1

echo -e "\nTest 3: Startup with strace to see system calls"
echo "Running with strace (first 50 lines)..."
strace -o heimdall_startup_strace.log -tt ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format spdx --output=test_strace.json > /dev/null 2>&1
head -50 heimdall_startup_strace.log

echo -e "\nTest 4: Analyze startup system calls"
echo "System calls taking > 1ms during startup:"
grep -E " [0-9]+\.[0-9]{6} " heimdall_startup_strace.log | awk '$2 > 0.001 {print $0}' | head -20

echo -e "\nTest 5: Plugin loading breakdown"
echo "dlopen calls:"
grep "dlopen" heimdall_startup_strace.log

echo -e "\ndlsym calls:"
grep "dlsym" heimdall_startup_strace.log

echo -e "\nTest 6: File I/O during startup"
echo "File operations during startup:"
grep -E "(open|read|stat)" heimdall_startup_strace.log | head -20

echo -e "\n=== Startup Analysis ==="
echo "If startup time is > 100ms, the bottleneck is likely:"
echo "1. Plugin loading and initialization"
echo "2. LLVM library loading"
echo "3. File I/O operations"
echo "4. Argument parsing and validation"

echo -e "\n=== Recommendations ==="
echo "If plugin loading is slow:"
echo "  - Consider static linking for critical components"
echo "  - Implement plugin caching"
echo "  - Pre-load plugins in background"
echo ""
echo "If LLVM loading is slow:"
echo "  - Use RTLD_NOW instead of RTLD_LAZY"
echo "  - Pre-resolve LLVM symbols"
echo "  - Cache LLVM contexts"
echo ""
echo "If file I/O is slow:"
echo "  - Minimize file operations during startup"
echo "  - Use memory-mapped files"
echo "  - Cache file metadata" 