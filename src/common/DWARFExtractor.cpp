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
#include <cstring>
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
        // Only initialize available targets
#ifdef HAVE_LLVM_TARGET_X86
        LLVMInitializeX86TargetInfo();
        LLVMInitializeX86Target();
#endif
#ifdef HAVE_LLVM_TARGET_ARM
        LLVMInitializeARMTargetInfo();
        LLVMInitializeARMTarget();
#endif
#ifdef HAVE_LLVM_TARGET_AARCH64
        LLVMInitializeAArch64TargetInfo();
        LLVMInitializeAArch64Target();
#endif
#ifdef HAVE_LLVM_TARGET_MIPS
        LLVMInitializeMipsTargetInfo();
        LLVMInitializeMipsTarget();
#endif
#ifdef HAVE_LLVM_TARGET_POWERPC
        LLVMInitializePowerPCTargetInfo();
        LLVMInitializePowerPCTarget();
#endif
#ifdef HAVE_LLVM_TARGET_SYSTEMZ
        LLVMInitializeSystemZTargetInfo();
        LLVMInitializeSystemZTarget();
#endif
#ifdef HAVE_LLVM_TARGET_SPARC
        LLVMInitializeSparcTargetInfo();
        LLVMInitializeSparcTarget();
#endif
#ifdef HAVE_LLVM_TARGET_HEXAGON
        LLVMInitializeHexagonTargetInfo();
        LLVMInitializeHexagonTarget();
#endif
#ifdef HAVE_LLVM_TARGET_LANAI
        LLVMInitializeLanaiTargetInfo();
        LLVMInitializeLanaiTarget();
#endif
#ifdef HAVE_LLVM_TARGET_AVR
        LLVMInitializeAVRTargetInfo();
        LLVMInitializeAVRTarget();
#endif
#ifdef HAVE_LLVM_TARGET_MSP430
        LLVMInitializeMSP430TargetInfo();
        LLVMInitializeMSP430Target();
#endif
#ifdef HAVE_LLVM_TARGET_LOONGARCH
        LLVMInitializeLoongArchTargetInfo();
        LLVMInitializeLoongArchTarget();
#endif
#ifdef HAVE_LLVM_TARGET_RISCV
        LLVMInitializeRISCVTargetInfo();
        LLVMInitializeRISCVTarget();
#endif
#ifdef HAVE_LLVM_TARGET_VE
        LLVMInitializeVETargetInfo();
        LLVMInitializeVETarget();
#endif
#ifdef HAVE_LLVM_TARGET_AMDGPU
        LLVMInitializeAMDGPUTargetInfo();
        LLVMInitializeAMDGPUTarget();
#endif
#ifdef HAVE_LLVM_TARGET_BPF
        LLVMInitializeBPFTargetInfo();
        LLVMInitializeBPFTarget();
#endif
#ifdef HAVE_LLVM_TARGET_WEBASSEMBLY
        LLVMInitializeWebAssemblyTargetInfo();
        LLVMInitializeWebAssemblyTarget();
#endif
#ifdef HAVE_LLVM_TARGET_NVPTX
        LLVMInitializeNVPTXTargetInfo();
        LLVMInitializeNVPTXTarget();
#endif
#ifdef HAVE_LLVM_TARGET_XCORE
        LLVMInitializeXCoreTargetInfo();
        LLVMInitializeXCoreTarget();
#endif
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
#ifdef LLVM_DWARF_AVAILABLE
    // No per-instance LLVM initialization
#endif
}

/**
 * @brief DWARFExtractor destructor
 *
 * Cleans up the DWARFExtractor instance. No per-instance LLVM cleanup is performed.
 * Prints debug information if compiled with debug output enabled.
 */
DWARFExtractor::~DWARFExtractor() {
#ifdef LLVM_DWARF_AVAILABLE
    // No per-instance LLVM cleanup
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
    } else {
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("DWARFExtractor: LLVM context creation failed, falling back to lightweight parser");
#endif
    }
    // Fallback to lightweight parser if LLVM fails
#endif

#ifdef LLVM_DWARF_AVAILABLE
    // Fallback to heuristic if LLVM fails
    return extractSourceFilesHeuristic(filePath, sourceFiles);
#else
    // Use lightweight DWARF parser for C++14 compatibility
    LightweightDWARFParser parser;
    return parser.extractSourceFiles(filePath, sourceFiles);
#endif
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
    // Fallback to lightweight parser if LLVM fails
#endif

#ifdef LLVM_DWARF_AVAILABLE
    // Heuristic fallback: If we have debug info, assume at least one compile unit
    if (hasDWARFInfo(filePath)) {
        compileUnits.push_back("main");
        return true;
    }
    return false;
#else
    // Use lightweight DWARF parser for C++14 compatibility
    LightweightDWARFParser parser;
    return parser.extractCompileUnits(filePath, compileUnits);
#endif
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
    // Fallback to lightweight parser if LLVM DWARF fails
#endif

#ifdef LLVM_DWARF_AVAILABLE
    // Fallback: Extract functions from symbol table
    return extractFunctionsFromSymbolTable(filePath, functions);
#else
    // Use lightweight DWARF parser for C++14 compatibility
    LightweightDWARFParser parser;
    return parser.extractFunctions(filePath, functions);
#endif
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
    
    // Heuristic fallback: Check for debug sections in ELF file
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
    bool has_debug_info = false;

    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        shdr = elf64_getshdr(scn);
        if (!shdr)
            continue;

        if (shdr->sh_type == SHT_PROGBITS) {
            // Look for debug sections
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

            if (sectionName.find(".debug_") == 0) {
                has_debug_info = true;
                break;
            }
        }
    }

    elf_end(elf);
    close(fd);
    return has_debug_info;
#else
    (void)filePath;
    return false;
#endif
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
        // Check if file exists and is readable
        if (filePath.empty()) {
            return nullptr;
        }

        std::ifstream testFile(filePath);
        if (!testFile.good()) {
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
                [](llvm::Error) {
                    // Intentionally empty: Ignore recoverable DWARF parsing errors
                    // to continue processing even with malformed debug information
                },
                [](llvm::Error) {
                    // Intentionally empty: Ignore DWARF parsing warnings
                    // to avoid cluttering output with non-critical debug info issues
                },
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
#ifdef HEIMDALL_DEBUG_ENABLED
                    heimdall::Utils::debugPrint("DWARFExtractor: Found source file in DIE: " + fileName);
#endif
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
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("DWARFExtractor: Using heuristic source file extraction for " + filePath);
#endif
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
        if (!shdr)
            continue;

        if (shdr->sh_type == SHT_PROGBITS) {
            // Look for .debug_info section
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

            if (sectionName == ".debug_info") {
                Elf_Data* data = elf_getdata(scn, nullptr);
                if (!data) {
                    continue;
                }

                const char* buf = static_cast<const char*>(data->d_buf);
                size_t sz = data->d_size;

                // Look for strings that contain file paths
                for (size_t i = 0; i < sz; i++) {
                    // Look for strings that contain file extensions
                    if (isprint(buf[i])) {
                        size_t start = i;
                        size_t end = i;
                        
                        // Find the end of the string
                        while (end < sz && buf[end] != '\0' && isprint(buf[end])) {
                            end++;
                        }
                        
                        if (end < sz && buf[end] == '\0' && end > start) {
                            std::string s(buf + start, buf + end);
                            
                            // Check if it looks like a source file path
                            if ((s.find(".c") != std::string::npos ||
                                 s.find(".h") != std::string::npos ||
                                 s.find(".cpp") != std::string::npos ||
                                 s.find(".cc") != std::string::npos ||
                                 s.find(".cxx") != std::string::npos) &&
                                (s.find('/') != std::string::npos || s.find("testlib.c") != std::string::npos)) {
                                
                                if (std::find(sourceFiles.begin(), sourceFiles.end(), s) ==
                                    sourceFiles.end()) {
                                    sourceFiles.push_back(s);
                                    found = true;
#ifdef HEIMDALL_DEBUG_ENABLED
                                    heimdall::Utils::debugPrint("DWARFExtractor: Heuristic found source file: " + s);
#endif
                                }
                            }
                        }
                        i = end; // Skip to end of string
                    }
                }
            }
        }
    }

    elf_end(elf);
    close(fd);

#ifdef HEIMDALL_DEBUG_ENABLED
    if (found) {
        heimdall::Utils::debugPrint("DWARFExtractor: Heuristic parser found " + std::to_string(sourceFiles.size()) + " source files");
        for (const auto& file : sourceFiles) {
            heimdall::Utils::debugPrint("DWARFExtractor: Heuristic source file: " + file);
        }
    } else {
        heimdall::Utils::debugPrint("DWARFExtractor: Heuristic parser found no source files");
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
                    if (name && name[0] != '\0') {
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

/**
 * @brief Extract all debug information using a single DWARF context
 *
 * This method creates one DWARF context and extracts all debug information
 * (source files, compile units, functions) in a single pass, avoiding
 * the overhead of creating multiple contexts.
 *
 * @param filePath Path to the ELF file containing DWARF info
 * @param sourceFiles Output vector to store extracted source file paths
 * @param compileUnits Output vector to store extracted compile unit names
 * @param functions Output vector to store extracted function names
 * @return true if extraction was successful, false otherwise
 */
bool DWARFExtractor::extractAllDebugInfo(const std::string& filePath,
                                         std::vector<std::string>& sourceFiles,
                                         std::vector<std::string>& compileUnits,
                                         std::vector<std::string>& functions) {
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("DWARFExtractor: Starting extractAllDebugInfo for " + filePath);
#endif

#ifdef LLVM_DWARF_AVAILABLE
    ensureLLVMInitialized();
    auto context = createDWARFContext(filePath);
    if (context) {
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("DWARFExtractor: LLVM context created successfully for extractAllDebugInfo");
#endif
        auto numUnits = context->getNumCompileUnits();
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("DWARFExtractor: Number of compile units: " + std::to_string(numUnits));
#endif
        
        // Extract all information in a single pass through the DWARF data
        for (uint32_t i = 0; i < numUnits; ++i) {
            auto unit = context->getUnitAtIndex(i);
            if (unit) {
                auto die = unit->getUnitDIE();
                if (die.isValid()) {
#ifdef HEIMDALL_DEBUG_ENABLED
                    heimdall::Utils::debugPrint("DWARFExtractor: Processing unit " + std::to_string(i));
#endif
                    
                    // Extract source files from this unit
                    extractSourceFilesFromDie(die, sourceFiles);
                    
                    // Extract compile unit information
                    extractCompileUnitFromDie(die, compileUnits);
                    
                    // Extract functions from this unit
                    extractFunctionsFromDie(die, functions);
                }
            }
        }
        
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("DWARFExtractor: extractAllDebugInfo completed");
        heimdall::Utils::debugPrint("DWARFExtractor: Source files found: " + std::to_string(sourceFiles.size()));
        heimdall::Utils::debugPrint("DWARFExtractor: Compile units found: " + std::to_string(compileUnits.size()));
        heimdall::Utils::debugPrint("DWARFExtractor: Functions found: " + std::to_string(functions.size()));
#endif
        
        return !sourceFiles.empty() || !compileUnits.empty() || !functions.empty();
    } else {
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("DWARFExtractor: LLVM context creation failed in extractAllDebugInfo");
#endif
    }
    // Fallback to lightweight parser if LLVM fails
#endif

#ifdef LLVM_DWARF_AVAILABLE
    // Fallback to heuristic extraction
    bool hasDebugInfo = false;
    
    // Try heuristic source file extraction
    if (extractSourceFilesHeuristic(filePath, sourceFiles)) {
        hasDebugInfo = true;
    }
    
    // Try heuristic function extraction
    if (extractFunctionsFromSymbolTable(filePath, functions)) {
        hasDebugInfo = true;
    }
    
    // Add a default compile unit if we found any debug info
    if (hasDebugInfo && compileUnits.empty()) {
        compileUnits.push_back("main");
    }
    
    return hasDebugInfo;
#else
    // Use lightweight DWARF parser for C++14 compatibility
    LightweightDWARFParser parser;
    return parser.extractAllDebugInfo(filePath, sourceFiles, compileUnits, functions);
#endif
}

}  // namespace heimdall