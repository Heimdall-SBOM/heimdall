# Heimdall Limitations

This document outlines known limitations and constraints of the Heimdall project.

## LLVM DWARF Thread-Safety Limitations

### **Critical: LLVM DWARF Libraries Are Not Thread-Safe**

**Issue:** LLVM's DWARF debug information extraction libraries are not designed for concurrent use and will cause segmentation faults when used from multiple threads simultaneously.

**Symptoms:**
- Segmentation faults during concurrent DWARF extraction operations
- Crashes when multiple `DWARFExtractor` instances are created simultaneously
- Memory corruption when rapidly constructing/destroying DWARFExtractor instances
- Unpredictable behavior in multi-threaded environments

**Root Cause:**
- LLVM's global state management is not thread-safe
- DWARF context creation and destruction involves global LLVM state
- Multiple threads accessing LLVM's internal data structures cause race conditions
- LLVM's memory management is not designed for concurrent access

**Impact:**
- Cannot use `DWARFExtractor` from multiple threads simultaneously
- Cannot create multiple `DWARFExtractor` instances in parallel
- Stress testing with rapid construction/destruction will fail
- Concurrent extraction operations are not supported

**Workarounds:**
1. **Serial Usage:** Use only one `DWARFExtractor` instance at a time
2. **Single Thread:** Perform all DWARF operations on a single thread
3. **Process Isolation:** Use separate processes for parallel DWARF extraction
4. **Caching:** Reuse DWARFExtractor instances instead of creating new ones

**Code Example - DO NOT DO THIS:**
```cpp
// ❌ WRONG - Will cause segmentation fault
std::thread t1([]() {
    DWARFExtractor extractor1;
    extractor1.extractSourceFiles("file1.elf", sourceFiles1);
});

std::thread t2([]() {
    DWARFExtractor extractor2;
    extractor2.extractSourceFiles("file2.elf", sourceFiles2);
});

t1.join();
t2.join();
```

**Code Example - CORRECT APPROACH:**
```cpp
// ✅ CORRECT - Serial usage
DWARFExtractor extractor;
extractor.extractSourceFiles("file1.elf", sourceFiles1);
extractor.extractSourceFiles("file2.elf", sourceFiles2);
```

## Platform Limitations

### **DWARF Support by Platform**

| Platform | DWARF Support | Notes |
|----------|---------------|-------|
| Linux (ELF) | ✅ Full | LLVM DWARF + heuristic fallback |
| macOS (Mach-O) | ⚠️ Limited | Heuristic parsing only, no LLVM DWARF |
| Windows (PE) | ⚠️ Limited | Heuristic parsing only, no LLVM DWARF |

### **Architecture Support**

| Architecture | Status | Notes |
|--------------|--------|-------|
| x86_64 | ✅ Full | All features supported |
| x86 | ✅ Full | All features supported |
| ARM64 | ⚠️ Limited | Basic support, may have issues |
| ARM32 | ⚠️ Limited | Basic support, may have issues |
| Other | ❌ Unsupported | No testing or support |

## Build System Limitations

### **LLVM Version Compatibility**

| LLVM Version | Status | Notes |
|--------------|--------|-------|
| 18.1.x | ✅ Recommended | Most stable, tested extensively |
| 19.1.x | ⚠️ Experimental | May have API changes |
| < 18.0 | ❌ Unsupported | No compatibility |

### **Compiler Requirements**

- **C++11** minimum (basic functionality)
- **C++14/17/23** recommended (enhanced features)
- **GCC 4.8+** or **Clang 3.3+** for C++11
- **GCC 7+** or **Clang 6+** recommended for C++17+
- **MSVC 2019+** for Windows builds

## Performance Limitations

### **Memory Usage**

- **Large Binaries:** DWARF extraction can use significant memory for large binaries (>100MB)
- **Concurrent Processing:** Not supported due to thread-safety limitations
- **Memory Leaks:** Potential for memory leaks under stress conditions

### **Processing Speed**

- **Small Files (< 1MB):** ~50-100ms per file
- **Medium Files (1-10MB):** ~100-500ms per file  
- **Large Files (> 10MB):** ~500ms-2s per file
- **Very Large Files (> 100MB):** May take 5-10 seconds

## Feature Limitations

### **DWARF Information Extraction**

| Feature | Status | Notes |
|---------|--------|-------|
| Source Files | ✅ Full | Complete support |
| Compile Units | ✅ Full | Complete support |
| Function Names | ✅ Full | Complete support |
| Line Numbers | ⚠️ Partial | Basic support, may be incomplete |
| Variable Information | ❌ Not Supported | Not implemented |
| Type Information | ❌ Not Supported | Not implemented |
| Call Graph | ❌ Not Supported | Not implemented |

### **File Format Support**

| Format | Status | Notes |
|--------|--------|-------|
| ELF Executables | ✅ Full | Complete support |
| ELF Shared Libraries | ✅ Full | Complete support |
| ELF Object Files | ✅ Full | Complete support |
| Static Libraries (.a) | ✅ Full | Complete support |
| Mach-O Files | ⚠️ Limited | Basic symbol extraction only |
| PE Files | ⚠️ Limited | Basic symbol extraction only |
| Archive Files | ✅ Full | Complete support |

## Testing Limitations

### **Concurrent Testing**

- **Concurrent DWARF Tests:** Removed due to thread-safety issues
- **Stress Testing:** Limited to avoid LLVM state corruption
- **Performance Testing:** Single-threaded only

### **Test Coverage**

- **Linux:** Comprehensive test coverage
- **macOS:** Limited test coverage
- **Windows:** Minimal test coverage
- **Cross-Platform:** Basic compatibility testing only

## Security Limitations

### **Input Validation**

- **File Paths:** Basic validation, may be vulnerable to path traversal
- **File Content:** Limited validation of binary file integrity
- **Memory Safety:** Depends on LLVM's memory safety

### **Resource Limits**

- **File Size:** No explicit limits, may cause memory issues with very large files
- **Concurrent Access:** Not supported due to thread-safety limitations
- **Resource Cleanup:** LLVM global state cleanup is not guaranteed

## Future Improvements

### **Planned Enhancements**

1. **Thread Safety:** Investigate alternative DWARF libraries or LLVM improvements
2. **Cross-Platform:** Improve Mach-O and PE support
3. **Performance:** Optimize memory usage and processing speed
4. **Features:** Add support for variable and type information extraction
5. **Testing:** Improve cross-platform test coverage

### **Known Issues to Address**

1. **Memory Leaks:** Investigate and fix potential memory leaks under stress
2. **Error Handling:** Improve error handling for corrupted or invalid files
3. **Documentation:** Expand API documentation and usage examples
4. **Build System:** Improve cross-platform build support

## Reporting Issues

When reporting issues related to these limitations:

1. **Include Platform Information:** OS, architecture, compiler version
2. **Provide Reproduction Steps:** Clear steps to reproduce the issue
3. **Attach Logs:** Include any error messages or crash logs
4. **Specify LLVM Version:** Include the LLVM version being used
5. **Describe Environment:** Single-threaded vs multi-threaded usage

## References

- [LLVM DWARF Documentation](https://llvm.org/docs/DebuggingInformation.html)
- [LLVM Thread Safety Issues](https://llvm.org/docs/ThreadSafety.html)
- [DWARF Debugging Information Format](http://dwarfstd.org/)
- [Heimdall Project Documentation](./README.md) 