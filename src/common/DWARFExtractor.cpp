#include "DWARFExtractor.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
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
#endif

namespace heimdall {

DWARFExtractor::DWARFExtractor() {
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cout << "DWARFExtractor: Initialized with LLVM DWARF support" << std::endl;
#endif
}

DWARFExtractor::~DWARFExtractor() = default;

bool DWARFExtractor::extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef LLVM_DWARF_AVAILABLE
    // Temporarily disable LLVM DWARF due to compatibility issues with LLVM 19.1.7
    // The API changes and segfault issues require more extensive refactoring
    (void)filePath;
    (void)sourceFiles;
#endif
    // Fallback to heuristic parsing
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cout << "DWARFExtractor: Using heuristic DWARF parser" << std::endl;
#endif
    return extractSourceFilesHeuristic(filePath, sourceFiles);
}

bool DWARFExtractor::extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits) {
#ifdef LLVM_DWARF_AVAILABLE
    // Temporarily disable LLVM DWARF due to compatibility issues with LLVM 19.1.7
    (void)filePath;
    (void)compileUnits;
    return false;
#else
    (void)filePath;
    (void)compileUnits;
    return false;
#endif
}

bool DWARFExtractor::extractFunctions(const std::string& filePath, std::vector<std::string>& functions) {
#ifdef LLVM_DWARF_AVAILABLE
    // Temporarily disable LLVM DWARF due to compatibility issues with LLVM 19.1.7
    (void)filePath;
    (void)functions;
    return false;
#else
    (void)filePath;
    (void)functions;
    return false;
#endif
}

bool DWARFExtractor::extractLineInfo(const std::string& filePath, std::vector<std::string>& lineInfo) {
#ifdef LLVM_DWARF_AVAILABLE
    // Temporarily disable LLVM DWARF due to compatibility issues with LLVM 19.1.7
    (void)filePath;
    (void)lineInfo;
    return false;
#else
    (void)filePath;
    (void)lineInfo;
    return false;
#endif
}

bool DWARFExtractor::hasDWARFInfo(const std::string& filePath) {
#ifdef LLVM_DWARF_AVAILABLE
    // Temporarily disable LLVM DWARF due to compatibility issues with LLVM 19.1.7
    // The API changes and segfault issues require more extensive refactoring
    (void)filePath;
    return false;
#else
    (void)filePath;
    return false;
#endif
}

#ifdef LLVM_DWARF_AVAILABLE
bool DWARFExtractor::createDWARFContext(const std::string& filePath) {
    try {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Creating DWARF context for: " << filePath << std::endl;
#endif
        
        // Check if file exists and is readable
        if (filePath.empty()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: File path is empty" << std::endl;
#endif
            return false;
        }
        
        std::ifstream testFile(filePath);
        if (!testFile.good()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: File is not readable: " << filePath << std::endl;
#endif
            return false;
        }
        testFile.close();
        
        // Create memory buffer from file
        auto bufferOrErr = llvm::MemoryBuffer::getFile(filePath);
        if (!bufferOrErr) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Failed to read file: " << filePath << std::endl;
#endif
            return false;
        }
        buffer_ = std::move(bufferOrErr.get());

        // Create object file
        auto objOrErr = llvm::object::ObjectFile::createObjectFile(buffer_->getMemBufferRef());
        if (!objOrErr) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Failed to create object file from " << filePath << std::endl;
#endif
            buffer_.reset();
            return false;
        }
        
        // Additional safety check - ensure object file is valid
        if (!objOrErr.get()) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Object file is null after creation" << std::endl;
#endif
            buffer_.reset();
            return false;
        }
        
        objectFile_ = std::move(objOrErr.get());

        // Create DWARF context with new LLVM 19.1.7 API
        try {
            auto newContext = llvm::DWARFContext::create(*objectFile_, 
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
                context_.reset(); // Ensure context is reset
                buffer_.reset();
                objectFile_.reset();
                return false;
            }
            
                    // Test if the context is usable by trying to access compile units
        try {
            auto testUnits = newContext->compile_units();
            // If we can access compile units, the context is valid
            context_ = std::move(newContext);
        } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: DWARF context is not usable" << std::endl;
#endif
            context_.reset(); // Ensure context is reset
            buffer_.reset();
            objectFile_.reset();
            return false;
        }
        } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Exception during DWARF context creation" << std::endl;
#endif
            context_.reset(); // Ensure context is reset
            buffer_.reset();
            objectFile_.reset();
            return false;
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        if (context_) {
            std::cout << "DWARFExtractor: Successfully created DWARF context" << std::endl;
        } else {
            std::cout << "DWARFExtractor: Failed to create DWARF context" << std::endl;
        }
#endif
        
        // Additional safety check - ensure context is valid
        if (!context_) {
            buffer_.reset();
            objectFile_.reset();
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception creating DWARF context: " << e.what() << std::endl;
#endif
        return false;
    } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Unknown exception creating DWARF context" << std::endl;
#endif
        return false;
    }
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