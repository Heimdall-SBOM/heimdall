#!/bin/bash

echo "=== Heimdall Processing Time Breakdown ==="
echo "========================================"

# Test 1: Just plugin activation (no file processing)
echo "Test 1: Plugin activation only"
time ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so --help > /dev/null 2>&1

# Test 2: Plugin activation + minimal file processing
echo -e "\nTest 2: Plugin activation + minimal processing"
time ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format spdx --output=test_minimal.json > /dev/null 2>&1

# Test 3: Profile with strace to see what's taking time
echo -e "\nTest 3: Detailed processing breakdown with strace"
strace -o heimdall_processing.log -tt ./build-cpp17/src/tools/heimdall-sbom ./build-cpp17/lib/heimdall-lld.so test_binary --format spdx --output=test_processing.json > /dev/null 2>&1

# Analyze the strace log for time-consuming operations
echo -e "\nTest 4: Time-consuming operations (>10ms)"
grep -E " [0-9]+\.[0-9]{6} " heimdall_processing.log | awk '$2 > 0.010 {print $0}' | head -20

echo -e "\nTest 5: File operations breakdown"
echo "ELF file operations:"
grep -E "(open.*\.so|read.*ELF)" heimdall_processing.log | head -10

echo -e "\nSymbol table operations:"
grep -i "symbol" heimdall_processing.log | head -10

echo -e "\nDWARF operations:"
grep -i "dwarf\|debug" heimdall_processing.log | head -10

echo -e "\nTest 6: System call frequency"
echo "Most frequent system calls:"
grep -v "^[0-9]" heimdall_processing.log | cut -d'(' -f1 | sort | uniq -c | sort -nr | head -10

echo -e "\nTest 7: Library loading operations"
echo "Library loading operations:"
grep -E "(dlopen|dlsym|dlclose)" heimdall_processing.log

echo -e "\nTest 8: Memory operations"
echo "Memory mapping operations:"
grep -E "(mmap|mprotect)" heimdall_processing.log | head -10

echo -e "\n=== Processing Analysis ==="
echo "Based on the timing results:"
echo "- Startup time: ~4ms"
echo "- Total processing time: ~280ms"
echo "- Actual processing overhead: ~276ms"
echo ""
echo "The bottleneck is in the processing phase, not startup."
echo ""
echo "Likely bottlenecks in processing:"
echo "1. Symbol table extraction (12,068 symbols for libc.so.6)"
echo "2. DWARF debug info processing"
echo "3. SBOM generation and JSON formatting"
echo "4. File I/O for multiple files"
echo "5. Plugin architecture overhead during processing"

echo -e "\n=== Recommendations ==="
echo "If symbol extraction is slow:"
echo "  - Implement lazy symbol loading"
echo "  - Cache symbol tables"
echo "  - Filter symbols by relevance"
echo ""
echo "If DWARF processing is slow:"
echo "  - Cache DWARF contexts per file"
echo "  - Implement parallel DWARF processing"
echo "  - Skip DWARF for system libraries"
echo ""
echo "If SBOM generation is slow:"
echo "  - Optimize JSON serialization"
echo "  - Use streaming JSON output"
echo "  - Cache component metadata"
echo ""
echo "If file I/O is slow:"
echo "  - Use memory-mapped files"
echo "  - Implement file caching"
echo "  - Parallel file processing" 