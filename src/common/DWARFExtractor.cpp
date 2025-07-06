/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/**
 * @file DWARFExtractor.cpp
 * @brief Implementation of robust DWARF debug information extractor using LLVM libraries
 * @author Trevor Bakker
 * @date 2025
 *
 * This file implements the DWARFExtractor class, which provides comprehensive
 * DWARF parsing capabilities using LLVM's DWARF libraries. It includes
 * detailed error handling, fallback heuristics, and thread-safety notes.
 *
 * @warning This implementation is NOT thread-safe due to LLVM's DWARF library limitations.
 *          See heimdall-limitations.md for details.
 */
#include "DWARFExtractor.hpp"
#include <algorithm>
#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include "Utils.hpp"
#ifdef __linux__
#include <elf.h>
#include <fcntl.h>
#include <libelf.h>
#include <unistd.h>
#include <cctype>
#endif

#ifdef LLVM_DWARF_AVAILABLE
#include <llvm/DebugInfo/DIContext.h>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugLine.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
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

/**
 * @brief DWARFExtractor constructor
 *
 * Initializes the DWARFExtractor instance. No per-instance LLVM initialization is performed.
 * Prints debug information if compiled with debug output enabled.
 */
DWARFExtractor::DWARFExtractor() {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("DWARFExtractor constructor called");
    Utils::debugPrint("About to check LLVM_DWARF_AVAILABLE");
#endif
#ifdef LLVM_DWARF_AVAILABLE
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LLVM_DWARF_AVAILABLE is defined");
#endif
    // No per-instance LLVM initialization
#else
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LLVM_DWARF_AVAILABLE is NOT defined");
#endif
#endif
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("DWARFExtractor constructor completed");
#endif
}

/**
 * @brief DWARFExtractor destructor
 *
 * Cleans up the DWARFExtractor instance. No per-instance LLVM cleanup is performed.
 * Prints debug information if compiled with debug output enabled.
 */
DWARFExtractor::~DWARFExtractor() {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("DWARFExtractor destructor called");
#endif
#ifdef LLVM_DWARF_AVAILABLE
    // No per-instance LLVM cleanup
#endif
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("DWARFExtractor destructor completed");
#endif
}

/**
 * @brief Extract source files from DWARF debug information
 *
 * Attempts to extract all source file paths from the DWARF information in the given ELF file.
 * Uses LLVM's DWARFContext if available, otherwise falls back to a heuristic method.
 * Handles edge cases where the file is missing, unreadable, or contains no DWARF info.
 *
 * @param filePath Path to the ELF file containing DWARF info
 * @param sourceFiles Output vector to store extracted source file paths
 * @return true if extraction was successful and any source files were found, false otherwise
 */
bool DWARFExtractor::extractSourceFiles(const std::string& filePath,
                                        std::vector<std::string>& sourceFiles) {
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

/**
 * @brief Extract compile units from DWARF debug information
 *
 * Extracts the names of all compile units present in the DWARF information of the given ELF file.
 * Uses LLVM's DWARFContext if available. If unavailable or extraction fails, returns false.
 *
 * @param filePath Path to the ELF file containing DWARF info
 * @param compileUnits Output vector to store extracted compile unit names
 * @return true if extraction was successful and any compile units were found, false otherwise
 */
bool DWARFExtractor::extractCompileUnits(const std::string& filePath,
                                         std::vector<std::string>& compileUnits) {
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

/**
 * @brief Extract function names from DWARF debug information
 *
 * Extracts all function names from the DWARF information in the given ELF file.
 * Uses LLVM's DWARFContext if available. If unavailable or extraction fails, returns false.
 *
 * @param filePath Path to the ELF file containing DWARF info
 * @param functions Output vector to store extracted function names
 * @return true if extraction was successful and any functions were found, false otherwise
 */
bool DWARFExtractor::extractFunctions(const std::string& filePath,
                                      std::vector<std::string>& functions) {
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
        if (!functions.empty()) {
            return true;
        }
    }
    // Fallback to symbol table extraction if LLVM DWARF fails
#endif
    
    // Fallback: Extract functions from symbol table
    return extractFunctionsFromSymbolTable(filePath, functions);
}

/**
 * @brief Extract line number information from DWARF debug information
 *
 * Extracts line number information from the DWARF debug info in the given ELF file.
 * Uses LLVM's DWARFContext if available. If unavailable or extraction fails, returns false.
 *
 * @param filePath Path to the ELF file containing DWARF info
 * @param lineInfo Output vector to store line number information
 * @return true if extraction was successful and any line info was found, false otherwise
 */
bool DWARFExtractor::extractLineInfo(const std::string& filePath,
                                     std::vector<std::string>& lineInfo) {
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

/**
 * @brief Check if DWARF information is available in the file
 *
 * Checks whether the given ELF file contains any DWARF debug information.
 * Uses LLVM's DWARFContext if available. If unavailable or extraction fails, returns false.
 *
 * @param filePath Path to the ELF file to check
 * @return true if DWARF info is present, false otherwise
 */
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
/**
 * @brief Initialize LLVM DWARF context for a file
 *
 * Attempts to create a DWARFContext for the given ELF file using LLVM's APIs.
 * Handles file existence, readability, and error cases. Returns nullptr if context creation fails.
 *
 * @param filePath Path to the ELF file
 * @return raw pointer to DWARF context if creation succeeded, nullptr otherwise
 */
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
            std::cout << "DWARFExtractor: Failed to create object file from " << filePath
                      << std::endl;
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
            auto newContext = llvm::DWARFContext::create(
                *g_objectFile, llvm::DWARFContext::ProcessDebugRelocations::Process, nullptr, "",
                [](llvm::Error) {},  // RecoverableErrorHandler - ignore errors
                [](llvm::Error) {},  // WarningHandler - ignore warnings
                false);              // ThreadSafe - single-threaded for now

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
            std::cout << "DWARFExtractor: Exception during DWARF context creation: " << e.what()
                      << std::endl;
#endif
            return nullptr;
        } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
            std::cout << "DWARFExtractor: Unknown exception during DWARF context creation"
                      << std::endl;
#endif
            return nullptr;
        }

    } catch (const std::exception& e) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Exception during DWARF context creation: " << e.what()
                  << std::endl;
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

/**
 * @brief Extract source files from a DWARF compile unit DIE
 *
 * Recursively traverses the DWARF DIE tree to extract all source file paths referenced
 * in the compile unit. Handles edge cases where DIEs are missing or malformed.
 *
 * @param die DWARF DIE (Debugging Information Entry)
 * @param sourceFiles Output vector for source files
 */
void DWARFExtractor::extractSourceFilesFromDie(const llvm::DWARFDie& die,
                                               std::vector<std::string>& sourceFiles) {
    if (!die.isValid()) {
        return;
    }

    try {
        // Check for DW_AT_name attribute (source file name)
        if (auto nameAttr = die.find(llvm::dwarf::DW_AT_name)) {
            if (auto name = nameAttr->getAsCString()) {
                std::string fileName = *name;
                if (!fileName.empty() && std::find(sourceFiles.begin(), sourceFiles.end(),
                                                   fileName) == sourceFiles.end()) {
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
        std::cout << "DWARFExtractor: Exception in extractSourceFilesFromDie: " << e.what()
                  << std::endl;
#endif
    } catch (...) {
#ifdef HEIMDALL_DEBUG_ENABLED
        std::cout << "DWARFExtractor: Unknown exception in extractSourceFilesFromDie" << std::endl;
#endif
    }
}

/**
 * @brief Extract compile unit information from a DWARF DIE
 *
 * Recursively traverses the DWARF DIE tree to extract compile unit names.
 * Handles edge cases where DIEs are missing or malformed.
 *
 * @param die DWARF DIE (Debugging Information Entry)
 * @param compileUnits Output vector for compile units
 */
void DWARFExtractor::extractCompileUnitFromDie(const llvm::DWARFDie& die,
                                               std::vector<std::string>& compileUnits) {
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

/**
 * @brief Extract function information from a DWARF DIE
 *
 * Recursively traverses the DWARF DIE tree to extract all function names.
 * Handles edge cases where DIEs are missing or malformed.
 *
 * @param die DWARF DIE (Debugging Information Entry)
 * @param functions Output vector for functions
 */
void DWARFExtractor::extractFunctionsFromDie(const llvm::DWARFDie& die,
                                             std::vector<std::string>& functions) {
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

/**
 * @brief Fallback heuristic DWARF parsing when LLVM DWARF fails
 *
 * Attempts to extract source file paths from the ELF file using a heuristic method
 * if LLVM's DWARF parsing is unavailable or fails. This method is less reliable
 * and should only be used as a last resort.
 *
 * @param filePath Path to the ELF file
 * @param sourceFiles Output vector for source files
 * @return true if any source files were found, false otherwise
 */
bool DWARFExtractor::extractSourceFilesHeuristic(const std::string& filePath,
                                                 std::vector<std::string>& sourceFiles) {
#ifdef __linux__
    elf_version(EV_CURRENT);
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0)
        return false;

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
        if (!shdr)
            continue;

        if (shdr->sh_type == SHT_PROGBITS) {
            // Look for .debug_line section
            std::string sectionName;
            {
                Elf64_Ehdr* ehdr = elf64_getehdr(elf);
                if (!ehdr)
                    continue;

                Elf_Scn* shstrscn = elf_getscn(elf, ehdr->e_shstrndx);
                if (!shstrscn)
                    continue;

                Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
                if (!shstrshdr)
                    continue;

                Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
                if (!shstrdata)
                    continue;

                char* shstrtab = static_cast<char*>(shstrdata->d_buf);
                sectionName = shstrtab + shdr->sh_name;
            }

            if (sectionName == ".debug_line") {
                Elf_Data* data = elf_getdata(scn, nullptr);
                if (!data)
                    continue;

                const char* buf = static_cast<const char*>(data->d_buf);
                size_t sz = data->d_size;

                // Scan for null-terminated strings that look like file paths
                for (size_t i = 0; i + 2 < sz;) {
                    if (isprint(buf[i]) && (buf[i] == '/' || isalpha(buf[i]))) {
                        size_t start = i;
                        while (i < sz && isprint(buf[i]) && buf[i] != '\0')
                            ++i;
                        if (i < sz && buf[i] == '\0') {
                            std::string s(buf + start, buf + i);
                            if ((s.find(".c") != std::string::npos ||
                                 s.find(".h") != std::string::npos ||
                                 s.find(".cpp") != std::string::npos) &&
                                s.find('/') != std::string::npos) {
                                if (std::find(sourceFiles.begin(), sourceFiles.end(), s) ==
                                    sourceFiles.end()) {
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
        std::cout << "DWARFExtractor: Heuristic parser found " << sourceFiles.size()
                  << " source files" << std::endl;
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

/**
 * @brief Fallback function extraction using symbol table
 *
 * Extracts function names from the ELF symbol table when DWARF extraction fails.
 * This is less comprehensive than DWARF extraction but provides basic function information.
 *
 * @param filePath Path to the ELF file
 * @param functions Output vector for function names
 * @return true if any functions were found, false otherwise
 */
bool DWARFExtractor::extractFunctionsFromSymbolTable(const std::string& filePath,
                                                     std::vector<std::string>& functions) {
#ifdef __linux__
    elf_version(EV_CURRENT);
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

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
        if (!shdr) {
            continue;
        }

        if (shdr->sh_type == SHT_SYMTAB) {
            Elf_Data* data = elf_getdata(scn, nullptr);
            if (!data) {
                continue;
            }

            Elf64_Sym* syms = (Elf64_Sym*)data->d_buf;
            int num_syms = data->d_size / sizeof(Elf64_Sym);

            for (int i = 0; i < num_syms; i++) {
                Elf64_Sym& sym = syms[i];
                
                // Check if this is a function symbol (STT_FUNC)
                if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC) {
                    // Get symbol name
                    char* name = elf_strptr(elf, shdr->sh_link, sym.st_name);
                    if (name && strlen(name) > 0) {
                        std::string funcName(name);
                        
                        // Filter out common system functions and internal symbols
                        if (!funcName.empty() && 
                            funcName[0] != '_' && 
                            funcName.find("__") == std::string::npos &&
                            funcName.find("_start") == std::string::npos &&
                            funcName.find("_fini") == std::string::npos &&
                            funcName.find("_init") == std::string::npos) {
                            
                            if (std::find(functions.begin(), functions.end(), funcName) == functions.end()) {
                                functions.push_back(funcName);
                                found = true;
                            }
                        }
                    }
                }
            }
        }
    }

    elf_end(elf);
    close(fd);

    return found;
#else
    (void)filePath;
    (void)functions;
    return false;
#endif
}

}  // namespace heimdall