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
    try {
        if (!LLVM_DWARF_AVAILABLE) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: LLVM DWARF not available" << std::endl;
#endif
            return false;
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: About to create DWARF context" << std::endl;
#endif
        if (!createDWARFContext(filePath)) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Failed to create DWARF context for " << filePath << std::endl;
#endif
            return false;
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: DWARF context created, about to get compile units" << std::endl;
#endif
        try {
            auto compileUnits = context_->compile_units();
            for (const auto& unit : compileUnits) {
                if (!unit) continue;
                llvm::DWARFDie die = unit->getUnitDIE();
                if (die.isValid()) {
                    extractSourceFilesFromDie(die, sourceFiles);
                }
            }
            for (const auto& unit : compileUnits) {
                if (!unit) continue;
                const llvm::DWARFDebugLine::LineTable* lineTable = context_->getLineTableForUnit(unit.get());
                if (lineTable) {
                    for (const auto& row : lineTable->Rows) {
                        if (row.File > 0 && lineTable->hasFileAtIndex(row.File)) {
                            std::string fileName;
                            if (lineTable->getFileNameByIndex(row.File, "", 
                                llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, fileName)) {
                                if (!fileName.empty() && std::find(sourceFiles.begin(), sourceFiles.end(), fileName) == sourceFiles.end()) {
                                    sourceFiles.push_back(fileName);
                                }
                            }
                        }
                    }
                }
            }
            if (!sourceFiles.empty()) {
#ifdef HEIMDALL_DEBUG_ENABLED
                std::cout << "DWARFExtractor: Successfully extracted " << sourceFiles.size() << " source files using LLVM DWARF" << std::endl;
#endif
                return true;
            }
        } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: LLVM DWARF parsing failed, falling back to heuristic parser" << std::endl;
#endif
        }
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during LLVM DWARF extraction: " << e.what() << std::endl;
#endif
    } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Unknown exception during LLVM DWARF extraction" << std::endl;
#endif
    }
#endif
    // Fallback to heuristic parsing
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cout << "DWARFExtractor: Using heuristic DWARF parser" << std::endl;
#endif
    return extractSourceFilesHeuristic(filePath, sourceFiles);
}

bool DWARFExtractor::extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits) {
#ifdef LLVM_DWARF_AVAILABLE
    try {
        if (!createDWARFContext(filePath)) {
            return false;
        }
        compileUnits.clear();
        for (const auto& unit : context_->compile_units()) {
            if (unit) {
                llvm::DWARFDie die = unit->getUnitDIE();
                if (die.isValid()) {
                    extractCompileUnitFromDie(die, compileUnits);
                }
            }
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Extracted " << compileUnits.size() << " compile units from " << filePath << std::endl;
#endif
        return true;
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during compile unit extraction: " << e.what() << std::endl;
#endif
        return false;
    }
#else
    (void)filePath;
    (void)compileUnits;
    return false;
#endif
}

bool DWARFExtractor::extractFunctions(const std::string& filePath, std::vector<std::string>& functions) {
#ifdef LLVM_DWARF_AVAILABLE
    try {
        if (!createDWARFContext(filePath)) {
            return false;
        }
        functions.clear();
        for (const auto& unit : context_->compile_units()) {
            if (unit) {
                llvm::DWARFDie die = unit->getUnitDIE();
                if (die.isValid()) {
                    extractFunctionsFromDie(die, functions);
                }
            }
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Extracted " << functions.size() << " functions from " << filePath << std::endl;
#endif
        return true;
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during function extraction: " << e.what() << std::endl;
#endif
        return false;
    }
#else
    (void)filePath;
    (void)functions;
    return false;
#endif
}

bool DWARFExtractor::extractLineInfo(const std::string& filePath, std::vector<std::string>& lineInfo) {
#ifdef LLVM_DWARF_AVAILABLE
    try {
        if (!createDWARFContext(filePath)) {
            return false;
        }
        lineInfo.clear();
        for (const auto& unit : context_->compile_units()) {
            if (unit) {
                const llvm::DWARFDebugLine::LineTable* lineTable = context_->getLineTableForUnit(unit.get());
                if (lineTable) {
                    for (const auto& row : lineTable->Rows) {
                        if (row.Line > 0 && row.File > 0) {
                            std::string fileName;
                            if (lineTable->getFileNameByIndex(row.File, "", 
                                llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, fileName)) {
                                std::string info = fileName + ":" + std::to_string(row.Line);
                                if (std::find(lineInfo.begin(), lineInfo.end(), info) == lineInfo.end()) {
                                    lineInfo.push_back(info);
                                }
                            }
                        }
                    }
                }
            }
        }
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Extracted " << lineInfo.size() << " line info entries from " << filePath << std::endl;
#endif
        return true;
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during line info extraction: " << e.what() << std::endl;
#endif
        return false;
    }
#else
    (void)filePath;
    (void)lineInfo;
    return false;
#endif
}

bool DWARFExtractor::hasDWARFInfo(const std::string& filePath) {
#ifdef LLVM_DWARF_AVAILABLE
    try {
        if (!createDWARFContext(filePath)) {
            return false;
        }
        return !context_->compile_units().empty();
    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception checking DWARF info: " << e.what() << std::endl;
#endif
        return false;
    }
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
        objectFile_ = std::move(objOrErr.get());

        // Create DWARF context
        context_ = llvm::DWARFContext::create(*objectFile_);
#ifdef HEIMDALL_DEBUG_ENABLED
        if (context_) {
            std::cout << "DWARFExtractor: Successfully created DWARF context" << std::endl;
        } else {
            std::cout << "DWARFExtractor: Failed to create DWARF context" << std::endl;
        }
#endif
        return static_cast<bool>(context_);
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
#endif

} // namespace heimdall 