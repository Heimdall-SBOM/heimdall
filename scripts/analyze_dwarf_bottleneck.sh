#!/bin/bash

# Analyze DWARF bottleneck more precisely
set -e

echo "=== Detailed DWARF Bottleneck Analysis ==="
echo

# Use the correct binary path
HEIMDALL_BINARY="./build-cpp17/src/tools/heimdall-sbom"
PLUGIN_PATH="./build-cpp17/lib/heimdall-lld.so"

if [ ! -f "$HEIMDALL_BINARY" ]; then
    echo "Error: heimdall-sbom binary not found at $HEIMDALL_BINARY"
    exit 1
fi

if [ ! -f "$PLUGIN_PATH" ]; then
    echo "Error: LLD plugin not found at $PLUGIN_PATH"
    exit 1
fi

# Create a test binary with debug info
echo "Creating test binary with debug info..."
cat > test_dwarf.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void helper_function() {
    printf("Helper function called\n");
}

int main(int argc, char* argv[]) {
    printf("Hello, DWARF test!\n");
    if (argc > 1) {
        printf("Arguments: %s\n", argv[1]);
    }
    helper_function();
    return 0;
}
EOF

gcc -g -O0 -o test_binary test_dwarf.c

echo "Test binary created with debug info"
echo

# Function to measure time
measure_time() {
    local start_time=$(date +%s%N)
    "$@"
    local end_time=$(date +%s%N)
    local duration=$(( (end_time - start_time) / 1000000 ))
    echo "$duration"
}

# Test 1: Profile heimdall-sbom with debug output
echo "=== Test 1: Profiling heimdall-sbom with debug output ==="
export HEIMDALL_DEBUG=1

echo "Running heimdall-sbom with debug output..."
duration=$(measure_time $HEIMDALL_BINARY $PLUGIN_PATH test_binary --format spdx --output=test_sbom.json 2>&1 | tee heimdall_debug.log)

echo "Heimdall-sbom execution time: ${duration}ms"
echo

# Test 2: Profile without debug output
echo "=== Test 2: Profiling heimdall-sbom without debug output ==="
unset HEIMDALL_DEBUG

echo "Running heimdall-sbom without debug output..."
duration=$(measure_time $HEIMDALL_BINARY $PLUGIN_PATH test_binary --format spdx --output=test_sbom_no_debug.json)

echo "Heimdall-sbom execution time (no debug): ${duration}ms"
echo

# Test 3: Profile with strace to see system calls
echo "=== Test 3: Profiling with strace ==="
echo "Running heimdall-sbom with strace..."
strace -o heimdall_strace.log -T -ttt $HEIMDALL_BINARY $PLUGIN_PATH test_binary --format spdx --output=test_sbom_strace.json 2>/dev/null

echo "Strace log saved to heimdall_strace.log"
echo

# Test 4: Profile LLVM initialization separately
echo "=== Test 4: Profiling LLVM initialization ==="

cat > test_llvm_init.cpp << 'EOF'
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <chrono>

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Initialize LLVM
    llvm::InitLLVM X(0, nullptr);
    
    // Initialize targets (like in our code)
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeARMTargetInfo();
    LLVMInitializeARMTarget();
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64Target();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "LLVM initialization time: " << duration.count() << "ms" << std::endl;
    return 0;
}
EOF

echo "Compiling LLVM initialization test..."
if g++ -std=c++17 -I/usr/include/llvm -I/usr/include/llvm-c -lLLVM -o test_llvm_init test_llvm_init.cpp 2>/dev/null; then
    echo "LLVM initialization test compiled successfully"
    echo "Testing LLVM initialization performance..."
    ./test_llvm_init
else
    echo "Could not compile LLVM initialization test"
fi
echo

# Test 5: Profile file operations
echo "=== Test 5: Profiling file operations ==="

cat > test_file_ops.cpp << 'EOF'
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }
    
    std::string filePath = argv[1];
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Read file into memory (like LLVM MemoryBuffer)
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }
    
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "File read time: " << duration.count() << "ms" << std::endl;
    std::cout << "File size: " << size << " bytes" << std::endl;
    
    return 0;
}
EOF

echo "Compiling file operations test..."
g++ -std=c++17 -o test_file_ops test_file_ops.cpp
echo "Testing file read performance..."
./test_file_ops test_binary
echo

# Test 6: Profile DWARF section parsing
echo "=== Test 6: Profiling DWARF section parsing ==="

cat > test_dwarf_sections.cpp << 'EOF'
#include <elf.h>
#include <libelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }
    
    std::string filePath = argv[1];
    
    auto start = std::chrono::high_resolution_clock::now();
    
    elf_version(EV_CURRENT);
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) {
        close(fd);
        std::cerr << "Failed to create ELF object" << std::endl;
        return 1;
    }

    // Parse all DWARF sections
    Elf_Scn* scn = nullptr;
    std::vector<std::string> dwarf_sections;
    size_t total_dwarf_size = 0;
    
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        Elf64_Shdr* shdr = elf64_getshdr(scn);
        if (!shdr) continue;

        if (shdr->sh_type == SHT_PROGBITS) {
            // Get section name
            Elf64_Ehdr* ehdr = elf64_getehdr(elf);
            if (!ehdr) continue;

            Elf_Scn* shstrscn = elf_getscn(elf, ehdr->e_shstrndx);
            if (!shstrscn) continue;

            Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
            if (!shstrshdr) continue;

            Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
            if (!shstrdata) continue;

            char* shstrtab = static_cast<char*>(shstrdata->d_buf);
            std::string sectionName = shstrtab + shdr->sh_name;

            if (sectionName.find(".debug_") == 0) {
                dwarf_sections.push_back(sectionName);
                total_dwarf_size += shdr->sh_size;
                
                // Read the section data (simulate DWARF parsing)
                Elf_Data* data = elf_getdata(scn, nullptr);
                if (data && data->d_size > 0) {
                    // Just touch the data to simulate parsing
                    char* ptr = static_cast<char*>(data->d_buf);
                    volatile char dummy = ptr[0]; // Prevent optimization
                    (void)dummy;
                }
            }
        }
    }

    elf_end(elf);
    close(fd);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "DWARF section parsing completed" << std::endl;
    std::cout << "DWARF sections found: " << dwarf_sections.size() << std::endl;
    std::cout << "Total DWARF data size: " << total_dwarf_size << " bytes" << std::endl;
    std::cout << "DWARF section parsing time: " << duration.count() << "ms" << std::endl;
    
    for (const auto& section : dwarf_sections) {
        std::cout << "  - " << section << std::endl;
    }
    
    return 0;
}
EOF

echo "Compiling DWARF section parsing test..."
g++ -std=c++17 -lelf -o test_dwarf_sections test_dwarf_sections.cpp
echo "Testing DWARF section parsing performance..."
./test_dwarf_sections test_binary
echo

# Analyze the debug log
echo "=== Analysis of heimdall debug output ==="
if [ -f "heimdall_debug.log" ]; then
    echo "Debug log analysis:"
    echo "Lines mentioning DWARF:"
    grep -i dwarf heimdall_debug.log || echo "No DWARF mentions found"
    echo
    echo "Lines mentioning LLVM:"
    grep -i llvm heimdall_debug.log || echo "No LLVM mentions found"
    echo
    echo "Lines mentioning context:"
    grep -i context heimdall_debug.log || echo "No context mentions found"
    echo
fi

# Analyze strace log
echo "=== Analysis of strace output ==="
if [ -f "heimdall_strace.log" ]; then
    echo "System calls taking > 1ms:"
    grep "> 0.001" heimdall_strace.log | head -10 || echo "No slow system calls found"
    echo
    echo "File operations:"
    grep "open\|read\|write" heimdall_strace.log | head -10 || echo "No file operations found"
    echo
fi

# Cleanup
echo "=== Cleanup ==="
rm -f test_llvm_init.cpp test_llvm_init
rm -f test_file_ops.cpp test_file_ops
rm -f test_dwarf_sections.cpp test_dwarf_sections
rm -f test_dwarf.c test_binary
rm -f test_sbom*.json

echo
echo "=== Bottleneck Analysis Summary ==="
echo "Based on the profiling results:"
echo "1. If LLVM initialization is slow -> bottleneck is in LLVM initialization"
echo "2. If DWARF section parsing is slow -> bottleneck is in our ELF parsing"
echo "3. If file operations are slow -> bottleneck is in I/O"
echo "4. If heimdall-sbom is slow but individual components are fast -> bottleneck is in our integration code"
echo
echo "Check the timing results above to identify the primary bottleneck." 