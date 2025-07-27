#!/bin/bash

# Profile DWARF extraction bottleneck analysis
# This script helps determine if the DWARF bottleneck is in LLVM code or our code

set -e

echo "=== DWARF Bottleneck Analysis ==="
echo "This script will help determine if the DWARF bottleneck is in LLVM code or our code"
echo

# Check if we have a test binary
TEST_BINARY=""
if [ -f "test_binary" ]; then
    TEST_BINARY="test_binary"
elif [ -f "build/test_binary" ]; then
    TEST_BINARY="build/test_binary"
else
    echo "No test binary found. Creating a simple test binary..."
    cat > test_dwarf.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Hello, DWARF test!\n");
    return 0;
}
EOF
    gcc -g -o test_binary test_dwarf.c
    TEST_BINARY="test_binary"
fi

echo "Using test binary: $TEST_BINARY"
echo

# Function to measure time
measure_time() {
    local start_time=$(date +%s%N)
    "$@"
    local end_time=$(date +%s%N)
    local duration=$(( (end_time - start_time) / 1000000 ))
    echo "$duration"
}

# Test 1: Profile our DWARF extraction
echo "=== Test 1: Profiling our DWARF extraction ==="
echo "Running heimdall-sbom with DWARF extraction enabled..."

# Run with debug output to see what's happening
export HEIMDALL_DEBUG=1

echo "Timing heimdall-sbom execution..."
duration=$(measure_time ./build/tools/heimdall-sbom --plugin=build/plugins/lld_plugin.so "$TEST_BINARY" --output=test_sbom.json)

echo "Heimdall-sbom execution time: ${duration}ms"
echo

# Test 2: Profile LLVM DWARF context creation directly
echo "=== Test 2: Profiling LLVM DWARF context creation ==="

cat > test_llvm_dwarf.cpp << 'EOF'
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/InitLLVM.h>
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }
    
    std::string filePath = argv[1];
    
    // Initialize LLVM
    llvm::InitLLVM X(argc, argv);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create memory buffer
    auto bufferOrErr = llvm::MemoryBuffer::getFile(filePath);
    if (!bufferOrErr) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        return 1;
    }
    
    auto buffer = std::move(bufferOrErr.get());
    
    // Create object file
    auto objOrErr = llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
    if (!objOrErr) {
        std::cerr << "Failed to create object file" << std::endl;
        return 1;
    }
    
    auto objFile = std::move(objOrErr.get());
    
    // Create DWARF context
    auto context = llvm::DWARFContext::create(
        *objFile, 
        llvm::DWARFContext::ProcessDebugRelocations::Process, 
        nullptr, 
        "",
        [](llvm::Error) {},
        [](llvm::Error) {},
        false
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (context) {
        std::cout << "LLVM DWARF context creation successful" << std::endl;
        std::cout << "Number of compile units: " << context->getNumCompileUnits() << std::endl;
        std::cout << "LLVM DWARF context creation time: " << duration.count() << "ms" << std::endl;
    } else {
        std::cout << "LLVM DWARF context creation failed" << std::endl;
    }
    
    return 0;
}
EOF

# Try to compile the LLVM test
echo "Compiling LLVM DWARF test..."
if g++ -std=c++17 -I/usr/include/llvm -I/usr/include/llvm-c -llldCommon -llldCore -llldDriver -llldELF -llldMachO -llldMinGW -llldWasm -llldYAML -lLLVM -o test_llvm_dwarf test_llvm_dwarf.cpp 2>/dev/null; then
    echo "LLVM DWARF test compiled successfully"
    echo "Testing LLVM DWARF context creation..."
    ./test_llvm_dwarf "$TEST_BINARY"
else
    echo "Could not compile LLVM test (LLVM headers not found or incompatible)"
    echo "This suggests the bottleneck might be in our code's LLVM integration"
fi
echo

# Test 3: Profile file I/O operations
echo "=== Test 3: Profiling file I/O operations ==="
echo "Testing file read performance..."

start_time=$(date +%s%N)
cat "$TEST_BINARY" > /dev/null
end_time=$(date +%s%N)
file_read_time=$(( (end_time - start_time) / 1000000 ))

echo "File read time: ${file_read_time}ms"
echo

# Test 4: Profile ELF parsing
echo "=== Test 4: Profiling ELF parsing ==="

cat > test_elf_parse.cpp << 'EOF'
#include <elf.h>
#include <libelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

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

    // Count debug sections
    Elf_Scn* scn = nullptr;
    int debug_sections = 0;
    
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
                debug_sections++;
            }
        }
    }

    elf_end(elf);
    close(fd);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "ELF parsing successful" << std::endl;
    std::cout << "Debug sections found: " << debug_sections << std::endl;
    std::cout << "ELF parsing time: " << duration.count() << "ms" << std::endl;
    
    return 0;
}
EOF

echo "Compiling ELF parsing test..."
if g++ -std=c++17 -lelf -o test_elf_parse test_elf_parse.cpp 2>/dev/null; then
    echo "ELF parsing test compiled successfully"
    echo "Testing ELF parsing performance..."
    ./test_elf_parse "$TEST_BINARY"
else
    echo "Could not compile ELF parsing test"
fi
echo

# Test 5: Profile our heuristic fallback
echo "=== Test 5: Profiling our heuristic DWARF extraction ==="
echo "Testing heuristic DWARF extraction (without LLVM)..."

cat > test_heuristic_dwarf.cpp << 'EOF'
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
    std::vector<std::string> sourceFiles;
    
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

    // Simulate our heuristic DWARF extraction
    Elf_Scn* scn = nullptr;
    bool found_debug_info = false;
    
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
                found_debug_info = true;
                // Simulate extracting some data from debug sections
                Elf_Data* data = elf_getdata(scn, nullptr);
                if (data && data->d_size > 0) {
                    // Just count the size, don't actually parse
                    sourceFiles.push_back("debug_section_" + std::to_string(data->d_size));
                }
            }
        }
    }

    elf_end(elf);
    close(fd);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Heuristic DWARF extraction completed" << std::endl;
    std::cout << "Debug info found: " << (found_debug_info ? "yes" : "no") << std::endl;
    std::cout << "Debug sections processed: " << sourceFiles.size() << std::endl;
    std::cout << "Heuristic DWARF extraction time: " << duration.count() << "ms" << std::endl;
    
    return 0;
}
EOF

echo "Compiling heuristic DWARF test..."
if g++ -std=c++17 -lelf -o test_heuristic_dwarf test_heuristic_dwarf.cpp 2>/dev/null; then
    echo "Heuristic DWARF test compiled successfully"
    echo "Testing heuristic DWARF extraction performance..."
    ./test_heuristic_dwarf "$TEST_BINARY"
else
    echo "Could not compile heuristic DWARF test"
fi
echo

# Cleanup
echo "=== Cleanup ==="
rm -f test_llvm_dwarf.cpp test_llvm_dwarf
rm -f test_elf_parse.cpp test_elf_parse
rm -f test_heuristic_dwarf.cpp test_heuristic_dwarf
rm -f test_dwarf.c test_binary

echo
echo "=== Analysis Summary ==="
echo "This profiling helps determine where the DWARF bottleneck is:"
echo "1. If LLVM DWARF context creation is slow -> bottleneck is in LLVM code"
echo "2. If our heuristic extraction is slow -> bottleneck is in our code"
echo "3. If file I/O is slow -> bottleneck is in file operations"
echo "4. If ELF parsing is slow -> bottleneck is in ELF library"
echo
echo "Compare the timing results above to identify the primary bottleneck." 