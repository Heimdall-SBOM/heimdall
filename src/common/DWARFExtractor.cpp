#include "DWARFExtractor.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <memory>
#include <atomic>
#ifdef __linux__
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>
#include <cctype>
#endif

#ifdef LLVM_DWARF_AVAILABLE
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugLine.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugLine.h>
#include <llvm/DebugInfo/DIContext.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#endif

namespace heimdall {

#ifdef LLVM_DWARF_AVAILABLE
// Global LLVM objects to prevent premature destruction
std::unique_ptr<llvm::MemoryBuffer> g_buffer;
std::unique_ptr<llvm::object::ObjectFile> g_objectFile;
std::unique_ptr<llvm::DWARFContext> g_context;
#endif

#ifdef LLVM_DWARF_AVAILABLE
static std::once_flag llvm_init_flag;
void DWARFExtractor::ensureLLVMInitialized() {
    std::call_once(llvm_init_flag, []() {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
    });
}
#endif

DWARFExtractor::DWARFExtractor() {
    std::cout << "DEBUG: DWARFExtractor constructor called" << std::endl;
    std::cout << "DEBUG: About to check LLVM_DWARF_AVAILABLE" << std::endl;
#ifdef LLVM_DWARF_AVAILABLE
    std::cout << "DEBUG: LLVM_DWARF_AVAILABLE is defined" << std::endl;
    // No per-instance LLVM initialization
#else
    std::cout << "DEBUG: LLVM_DWARF_AVAILABLE is NOT defined" << std::endl;
#endif
    std::cout << "DEBUG: DWARFExtractor constructor completed" << std::endl;
}

DWARFExtractor::~DWARFExtractor() {
    std::cout << "DEBUG: DWARFExtractor destructor called" << std::endl;
#ifdef LLVM_DWARF_AVAILABLE
    // No per-instance LLVM cleanup
#endif
    std::cout << "DEBUG: DWARFExtractor destructor completed" << std::endl;
}

bool DWARFExtractor::extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
        auto numUnits = context->getNumCompileUnits();
        for (uint32_t i = 0; i < numUnits; ++i) {
            auto unit = context->getUnitAtIndex(i);
            if (unit) {
                extractSourceFilesFromDie(unit->getUnitDIE(), sourceFiles);
            }
        }
        return !sourceFiles.empty();
    }
    // Fallback to heuristic if LLVM fails
#endif
    return extractSourceFilesHeuristic(filePath, sourceFiles);
}

bool DWARFExtractor::extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits) {
#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
        auto numUnits = context->getNumCompileUnits();
        for (uint32_t i = 0; i < numUnits; ++i) {
            auto unit = context->getUnitAtIndex(i);
            if (unit) {
                extractCompileUnitFromDie(unit->getUnitDIE(), compileUnits);
            }
        }
        return !compileUnits.empty();
    }
    // Fallback to heuristic if LLVM fails
#endif
    (void)filePath;
    (void)compileUnits;
    return false;
}

bool DWARFExtractor::extractFunctions(const std::string& filePath, std::vector<std::string>& functions) {
#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
        auto numUnits = context->getNumCompileUnits();
        for (uint32_t i = 0; i < numUnits; ++i) {
            auto unit = context->getUnitAtIndex(i);
            if (unit) {
                extractFunctionsFromDie(unit->getUnitDIE(), functions);
            }
        }
        return !functions.empty();
    }
    // Fallback to heuristic if LLVM fails
#endif
    (void)filePath;
    (void)functions;
    return false;
}

bool DWARFExtractor::extractLineInfo(const std::string& filePath, std::vector<std::string>& lineInfo) {
#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
        auto numUnits = context->getNumCompileUnits();
        for (uint32_t i = 0; i < numUnits; ++i) {
            auto unit = context->getUnitAtIndex(i);
            if (unit) {
                auto lineTable = context->getLineTableForUnit(unit);
                if (lineTable) {
                    for (const auto& row : lineTable->Rows) {
                        lineInfo.push_back(std::to_string(row.Line));
                    }
                }
            }
        }
        return !lineInfo.empty();
    }
    // Fallback to heuristic if LLVM fails
#endif
    (void)filePath;
    (void)lineInfo;
    return false;
}

bool DWARFExtractor::hasDWARFInfo(const std::string& filePath) {
#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
        return context->getNumCompileUnits() > 0;
    }
    // Fallback to heuristic if LLVM fails
#endif
    (void)filePath;
    return false;
}

#ifdef LLVM_DWARF_AVAILABLE
llvm::DWARFContext* DWARFExtractor::createDWARFContext(const std::string& filePath) {
    try {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Creating DWARF context for: " << filePath << std::endl;
#endif
        
        // Check if file exists and is readable
        if (filePath.empty()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: File path is empty" << std::endl;
#endif
            return nullptr;
        }
        
        std::ifstream testFile(filePath);
        if (!testFile.good()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: File is not readable: " << filePath << std::endl;
#endif
            return nullptr;
        }
        testFile.close();
        
        // Create memory buffer from file
        auto bufferOrErr = llvm::MemoryBuffer::getFile(filePath);
        if (!bufferOrErr) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Failed to read file: " << filePath << std::endl;
#endif
            return nullptr;
        }
        g_buffer = std::move(bufferOrErr.get());

        // Create object file
        auto objOrErr = llvm::object::ObjectFile::createObjectFile(g_buffer->getMemBufferRef());
        if (!objOrErr) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Failed to create object file from " << filePath << std::endl;
#endif
            return nullptr;
        }
        
        // Additional safety check - ensure object file is valid
        if (!objOrErr.get()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Object file is null after creation" << std::endl;
#endif
            return nullptr;
        }
        
        g_objectFile = std::move(objOrErr.get());

        // Create DWARF context with new LLVM 19.1.7 API
        try {
            auto newContext = llvm::DWARFContext::create(*g_objectFile, 
                                                        llvm::DWARFContext::ProcessDebugRelocations::Process,
                                                        nullptr, "", 
                                                        [](llvm::Error) {}, // RecoverableErrorHandler - ignore errors
                                                        [](llvm::Error) {}, // WarningHandler - ignore warnings
                                                        false); // ThreadSafe - single-threaded for now
            
            // Additional safety check - ensure the context is valid
            if (!newContext) {
#ifdef HEIMDALL_DEBUG_ENABLED
                std::cout << "DWARFExtractor: DWARF context creation returned null" << std::endl;
#endif
                return nullptr;
            }
            
            g_context = std::move(newContext);
            return g_context.get();
            
        } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Exception during DWARF context creation: " << e.what() << std::endl;
#endif
            return nullptr;
        } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Unknown exception during DWARF context creation" << std::endl;
#endif
            return nullptr;
        }
        
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during DWARF context creation: " << e.what() << std::endl;
#endif
        return nullptr;
    } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Unknown exception during DWARF context creation" << std::endl;
#endif
        return nullptr;
    }
    
    return nullptr;
}

void DWARFExtractor::extractSourceFilesFromDie(const llvm::DWARFDie& die, std::vector<std::string>& sourceFiles) {
    if (!die.isValid()) {
        return;
    }

    try {
        // Check for DW_AT_name attribute (source file name)
        if (auto nameAttr = die.find(llvm::dwarf::DW_AT_name)) {
            if (auto name = nameAttr->getAsCString()) {
                std::string fileName = *name;
                if (!fileName.empty() && std::find(sourceFiles.begin(), sourceFiles.end(), fileName) == sourceFiles.end()) {
                    sourceFiles.push_back(fileName);
                }
            }
        }

        // Recursively process children
        for (auto child : die.children()) {
            if (child.isValid()) {
                extractSourceFilesFromDie(child, sourceFiles);
            }
        }
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception in extractSourceFilesFromDie: " << e.what() << std::endl;
#endif
    } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Unknown exception in extractSourceFilesFromDie" << std::endl;
#endif
    }
}

void DWARFExtractor::extractCompileUnitFromDie(const llvm::DWARFDie& die, std::vector<std::string>& compileUnits) {
    if (!die.isValid()) {
        return;
    }

    // Check if this is a compile unit
    if (die.getTag() == llvm::dwarf::DW_TAG_compile_unit) {
        std::string unitName = "unknown";
        
        // Try to get the compile unit name
        if (auto nameAttr = die.find(llvm::dwarf::DW_AT_name)) {
            if (auto name = nameAttr->getAsCString()) {
                unitName = *name;
            }
        }
        
        if (std::find(compileUnits.begin(), compileUnits.end(), unitName) == compileUnits.end()) {
            compileUnits.push_back(unitName);
        }
    }

    // Recursively process children
    for (auto child : die.children()) {
        extractCompileUnitFromDie(child, compileUnits);
    }
}

void DWARFExtractor::extractFunctionsFromDie(const llvm::DWARFDie& die, std::vector<std::string>& functions) {
    if (!die.isValid()) {
        return;
    }

    // Check if this is a function
    if (die.getTag() == llvm::dwarf::DW_TAG_subprogram) {
        
        std::string functionName = "unknown";
        
        // Try to get the function name
        if (auto nameAttr = die.find(llvm::dwarf::DW_AT_name)) {
            if (auto name = nameAttr->getAsCString()) {
                functionName = *name;
            }
        }
        
        if (!functionName.empty() && functionName != "unknown" && 
            std::find(functions.begin(), functions.end(), functionName) == functions.end()) {
            functions.push_back(functionName);
        }
    }

    // Recursively process children
    for (auto child : die.children()) {
        extractFunctionsFromDie(child, functions);
    }
}
#endif

bool DWARFExtractor::extractSourceFilesHeuristic(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef __linux__
    elf_version(EV_CURRENT);
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) return false;
    
    Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) { 
        close(fd); 
        return false; 
    }
    
    Elf_Scn* scn = nullptr;
    Elf64_Shdr* shdr = nullptr;
    bool found = false;
    
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        shdr = elf64_getshdr(scn);
        if (!shdr) continue;
        
        if (shdr->sh_type == SHT_PROGBITS) {
            // Look for .debug_line section
            std::string sectionName;
            {
                Elf64_Ehdr* ehdr = elf64_getehdr(elf);
                if (!ehdr) continue;
                
                Elf_Scn* shstrscn = elf_getscn(elf, ehdr->e_shstrndx);
                if (!shstrscn) continue;
                
                Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
                if (!shstrshdr) continue;
                
                Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
                if (!shstrdata) continue;
                
                char* shstrtab = static_cast<char*>(shstrdata->d_buf);
                sectionName = shstrtab + shdr->sh_name;
            }
            
            if (sectionName == ".debug_line") {
                Elf_Data* data = elf_getdata(scn, nullptr);
                if (!data) continue;
                
                const char* buf = static_cast<const char*>(data->d_buf);
                size_t sz = data->d_size;
                
                // Scan for null-terminated strings that look like file paths
                for (size_t i = 0; i + 2 < sz;) {
                    if (isprint(buf[i]) && (buf[i] == '/' || isalpha(buf[i]))) {
                        size_t start = i;
                        while (i < sz && isprint(buf[i]) && buf[i] != '\0') ++i;
                        if (i < sz && buf[i] == '\0') {
                            std::string s(buf + start, buf + i);
                            if ((s.find(".c") != std::string::npos || s.find(".h") != std::string::npos || s.find(".cpp") != std::string::npos) && s.find('/') != std::string::npos) {
                                if (std::find(sourceFiles.begin(), sourceFiles.end(), s) == sourceFiles.end()) {
                                    sourceFiles.push_back(s);
                                    found = true;
                                }
                            }
                        }
                        ++i;
                    } else {
                        ++i;
                    }
                }
            }
        }
    }
    
    elf_end(elf);
    close(fd);
    
#ifdef HEIMDALL_DEBUG_ENABLED
    if (found) {
        std::cout << "DWARFExtractor: Heuristic parser found " << sourceFiles.size() << " source files" << std::endl;
    } else {
        std::cout << "DWARFExtractor: Heuristic parser found no source files" << std::endl;
    }
#endif
    
    return found;
#else
    (void)filePath;
    (void)sourceFiles;
    return false;
#endif
}

} // namespace heimdall 