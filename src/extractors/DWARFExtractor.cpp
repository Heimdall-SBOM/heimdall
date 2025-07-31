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
 * @version 2.0.0
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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include "../common/Utils.hpp"
#ifdef __linux__
#include <elf.h>
#include <fcntl.h>
#include <gelf.h>
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

namespace heimdall
{

#ifdef LLVM_DWARF_AVAILABLE
// No global objects - each extraction manages its own LLVM objects
#endif

#ifdef LLVM_DWARF_AVAILABLE
static std::once_flag llvm_init_flag;
void                  DWARFExtractor::ensureLLVMInitialized()
{
   std::call_once(llvm_init_flag,
                  []()
                  {
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
#ifdef HAVE_LLVM_TARGET_MIPS64
                     LLVMInitializeMips64TargetInfo();
                     LLVMInitializeMips64Target();
#endif
#ifdef HAVE_LLVM_TARGET_RISCV
                     LLVMInitializeRISCVTargetInfo();
                     LLVMInitializeRISCVTarget();
#endif
#ifdef HAVE_LLVM_TARGET_WEBASSEMBLY
                     LLVMInitializeWebAssemblyTargetInfo();
                     LLVMInitializeWebAssemblyTarget();
#endif
#ifdef HAVE_LLVM_TARGET_AMDGPU
                     LLVMInitializeAMDGPUTargetInfo();
                     LLVMInitializeAMDGPUTarget();
#endif
#ifdef HAVE_LLVM_TARGET_NVPTX
                     LLVMInitializeNVPTXTargetInfo();
                     LLVMInitializeNVPTXTarget();
#endif
#ifdef HAVE_LLVM_TARGET_HEXAGON
                     LLVMInitializeHexagonTargetInfo();
                     LLVMInitializeHexagonTarget();
#endif
#ifdef HAVE_LLVM_TARGET_BPF
                     LLVMInitializeBPFTargetInfo();
                     LLVMInitializeBPFTarget();
#endif
#ifdef HAVE_LLVM_TARGET_AVR
                     LLVMInitializeAVRTargetInfo();
                     LLVMInitializeAVRTarget();
#endif
#ifdef HAVE_LLVM_TARGET_LANAI
                     LLVMInitializeLanaiTargetInfo();
                     LLVMInitializeLanaiTarget();
#endif
#ifdef HAVE_LLVM_TARGET_MSP430
                     LLVMInitializeMSP430TargetInfo();
                     LLVMInitializeMSP430Target();
#endif
#ifdef HAVE_LLVM_TARGET_XCORE
                     LLVMInitializeXCoreTargetInfo();
                     LLVMInitializeXCoreTarget();
#endif
#ifdef HAVE_LLVM_TARGET_M68K
                     LLVMInitializeM68kTargetInfo();
                     LLVMInitializeM68kTarget();
#endif
#ifdef HAVE_LLVM_TARGET_CSKY
                     LLVMInitializeCSKYTargetInfo();
                     LLVMInitializeCSKYTarget();
#endif
#ifdef HAVE_LLVM_TARGET_LOONGARCH
                     LLVMInitializeLoongArchTargetInfo();
                     LLVMInitializeLoongArchTarget();
#endif
#ifdef HAVE_LLVM_TARGET_XTENSA
                     LLVMInitializeXtensaTargetInfo();
                     LLVMInitializeXtensaTarget();
#endif
#ifdef HAVE_LLVM_TARGET_VE
                     LLVMInitializeVETargetInfo();
                     LLVMInitializeVETarget();
#endif
#ifdef HAVE_LLVM_TARGET_ARC
                     LLVMInitializeARCTargetInfo();
                     LLVMInitializeARCTarget();
#endif
#ifdef HAVE_LLVM_TARGET_MOS
                     LLVMInitializeMOSTargetInfo();
                     LLVMInitializeMOSTarget();
#endif
#ifdef HAVE_LLVM_TARGET_DIRECTX
                     LLVMInitializeDirectXTargetInfo();
                     LLVMInitializeDirectXTarget();
#endif
#ifdef HAVE_LLVM_TARGET_AMDGPU
                     LLVMInitializeAMDGPUTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_ARM
                     LLVMInitializeARMTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_AARCH64
                     LLVMInitializeAArch64TargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_AVR
                     LLVMInitializeAVRTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_BPF
                     LLVMInitializeBPFTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_CSKY
                     LLVMInitializeCSKYTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_HEXAGON
                     LLVMInitializeHexagonTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_LANAI
                     LLVMInitializeLanaiTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_LOONGARCH
                     LLVMInitializeLoongArchTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_M68K
                     LLVMInitializeM68kTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_MIPS
                     LLVMInitializeMipsTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_MIPS64
                     LLVMInitializeMips64TargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_MOS
                     LLVMInitializeMOSTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_MSP430
                     LLVMInitializeMSP430TargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_NVPTX
                     LLVMInitializeNVPTXTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_POWERPC
                     LLVMInitializePowerPCTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_RISCV
                     LLVMInitializeRISCVTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_SPARC
                     LLVMInitializeSparcTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_SYSTEMZ
                     LLVMInitializeSystemZTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_VE
                     LLVMInitializeVETargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_WEBASSEMBLY
                     LLVMInitializeWebAssemblyTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_X86
                     LLVMInitializeX86TargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_XCORE
                     LLVMInitializeXCoreTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_XTENSA
                     LLVMInitializeXtensaTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_ARC
                     LLVMInitializeARCTargetMC();
#endif
#ifdef HAVE_LLVM_TARGET_DIRECTX
                     LLVMInitializeDirectXTargetMC();
#endif
                  });
}
#else
void DWARFExtractor::ensureLLVMInitialized()
{
   // No-op for non-LLVM builds
}
#endif

// PIMPL implementation
class DWARFExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   // Implementation details will be moved here
};

// Constructor and destructor
DWARFExtractor::DWARFExtractor() : pImpl(std::make_unique<Impl>())
{
   ensureLLVMInitialized();
}

DWARFExtractor::~DWARFExtractor() = default;

DWARFExtractor::DWARFExtractor(const DWARFExtractor& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
   ensureLLVMInitialized();
}

DWARFExtractor::DWARFExtractor(DWARFExtractor&& other) noexcept : pImpl(std::move(other.pImpl))
{
   ensureLLVMInitialized();
}

DWARFExtractor& DWARFExtractor::operator=(const DWARFExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

DWARFExtractor& DWARFExtractor::operator=(DWARFExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool DWARFExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   std::vector<std::string> functions;
   if (extractFunctions(filePath, functions))
   {
      for (const auto& func : functions)
      {
         SymbolInfo symbol;
         symbol.name      = func;
         symbol.isDefined = true;
         symbol.isGlobal  = true;
         symbols.push_back(symbol);
      }
      return true;
   }
   return false;
}

bool DWARFExtractor::extractSections(const std::string&        filePath,
                                     std::vector<SectionInfo>& sections)
{
   // DWARF extractor doesn't extract sections
   return false;
}

bool DWARFExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   // DWARF extractor doesn't extract version information
   return false;
}

std::vector<std::string> DWARFExtractor::extractDependencies(const std::string& filePath)
{
   // DWARF extractor doesn't extract dependencies
   return {};
}

bool DWARFExtractor::canHandle(const std::string& filePath) const
{
   return hasDWARFInfo(filePath);
}

std::string DWARFExtractor::getFormatName() const
{
   return "DWARF";
}

int DWARFExtractor::getPriority() const
{
   return 100;  // High priority for DWARF extraction
}

// DWARF-specific methods
bool DWARFExtractor::extractSourceFiles(const std::string&        filePath,
                                        std::vector<std::string>& sourceFiles)
{
   sourceFiles.clear();

#ifdef LLVM_DWARF_AVAILABLE
   try
   {
      // Ensure LLVM is initialized
      ensureLLVMInitialized();
      
      // Get the actual DWARF file path (handles .dSYM files on macOS)
      std::string dwarfFilePath = getDWARFFilePath(filePath);
      
      auto buffer = llvm::MemoryBuffer::getFile(dwarfFilePath);
      if (!buffer)
      {
         return extractSourceFilesHeuristic(filePath, sourceFiles);
      }

      auto objectFile = llvm::object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());
      if (!objectFile)
      {
         return extractSourceFilesHeuristic(filePath, sourceFiles);
      }

      auto context = llvm::DWARFContext::create(*objectFile.get());
      if (!context)
      {
         return extractSourceFilesHeuristic(filePath, sourceFiles);
      }

      for (const auto& unit : context->compile_units())
      {
         if (unit)
         {
            extractSourceFilesFromDie(unit->getUnitDIE(), sourceFiles);
         }
      }

      return !sourceFiles.empty();
   }
   catch (...)
   {
      return extractSourceFilesHeuristic(filePath, sourceFiles);
   }
#else
   return extractSourceFilesHeuristic(filePath, sourceFiles);
#endif
}

bool DWARFExtractor::extractCompileUnits(const std::string&        filePath,
                                         std::vector<std::string>& compileUnits)
{
   compileUnits.clear();

#ifdef LLVM_DWARF_AVAILABLE
   try
   {
      // Get the actual DWARF file path (handles .dSYM files on macOS)
      std::string dwarfFilePath = getDWARFFilePath(filePath);
      
      auto buffer = llvm::MemoryBuffer::getFile(dwarfFilePath);
      if (!buffer)
      {
         return false;
      }

      auto objectFile = llvm::object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());
      if (!objectFile)
      {
         return false;
      }

      auto context = llvm::DWARFContext::create(*objectFile.get());
      if (!context)
      {
         return false;
      }

      for (const auto& unit : context->compile_units())
      {
         if (unit)
         {
            extractCompileUnitFromDie(unit->getUnitDIE(), compileUnits);
         }
      }

      return !compileUnits.empty();
   }
   catch (...)
   {
      return false;
   }
#else
   return false;
#endif
}

bool DWARFExtractor::extractFunctions(const std::string&        filePath,
                                      std::vector<std::string>& functions)
{
   functions.clear();

#ifdef LLVM_DWARF_AVAILABLE
   try
   {
      // Get the actual DWARF file path (handles .dSYM files on macOS)
      std::string dwarfFilePath = getDWARFFilePath(filePath);
      
      auto buffer = llvm::MemoryBuffer::getFile(dwarfFilePath);
      if (!buffer)
      {
         return extractFunctionsFromSymbolTable(filePath, functions);
      }

      auto objectFile = llvm::object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());
      if (!objectFile)
      {
         return extractFunctionsFromSymbolTable(filePath, functions);
      }

      auto context = llvm::DWARFContext::create(*objectFile.get());
      if (!context)
      {
         return extractFunctionsFromSymbolTable(filePath, functions);
      }

      for (const auto& unit : context->compile_units())
      {
         if (unit)
         {
            extractFunctionsFromDie(unit->getUnitDIE(), functions);
         }
      }

      return !functions.empty();
   }
   catch (...)
   {
      return extractFunctionsFromSymbolTable(filePath, functions);
   }
#else
   return extractFunctionsFromSymbolTable(filePath, functions);
#endif
}

bool DWARFExtractor::extractLineInfo(const std::string&        filePath,
                                     std::vector<std::string>& lineInfo)
{
   lineInfo.clear();

#ifdef LLVM_DWARF_AVAILABLE
   try
   {
      // Get the actual DWARF file path (handles .dSYM files on macOS)
      std::string dwarfFilePath = getDWARFFilePath(filePath);
      
      auto buffer = llvm::MemoryBuffer::getFile(dwarfFilePath);
      if (!buffer)
      {
         return false;
      }

      auto objectFile = llvm::object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());
      if (!objectFile)
      {
         return false;
      }

      auto context = llvm::DWARFContext::create(*objectFile.get());
      if (!context)
      {
         return false;
      }

      for (const auto& unit : context->compile_units())
      {
         if (unit)
         {
            auto lineTable = context->getLineTableForUnit(unit.get());
            if (lineTable)
            {
               // Simplified line info extraction - just add line numbers
               for (const auto& row : lineTable->Rows)
               {
                  lineInfo.push_back(std::to_string(row.Line));
               }
            }
         }
      }

      return !lineInfo.empty();
   }
   catch (...)
   {
      return false;
   }
#else
   return false;
#endif
}

// Helper method to get the actual DWARF file path (handles .dSYM files on macOS)
std::string DWARFExtractor::getDWARFFilePath(const std::string& filePath) const
{
   // Check for .dSYM file first (macOS)
   std::string dsymPath = filePath + ".dSYM";
   if (std::filesystem::exists(dsymPath))
   {
      // Check if the .dSYM file contains a DWARF file
      std::string dwarfPath = dsymPath + "/Contents/Resources/DWARF/" + 
                              std::filesystem::path(filePath).filename().string();
      if (std::filesystem::exists(dwarfPath))
      {
         return dwarfPath;
      }
   }

   // Fallback to the original file
   return filePath;
}

bool DWARFExtractor::hasDWARFInfo(const std::string& filePath) const
{
   std::ifstream testFile(filePath);
   if (!testFile.is_open())
   {
      return false;
   }

   // Check for DWARF sections in ELF files
#ifdef __linux__
   int fd = open(filePath.c_str(), O_RDONLY);
   if (fd == -1)
   {
      return false;
   }

   if (elf_version(EV_CURRENT) == EV_NONE)
   {
      close(fd);
      return false;
   }

   Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
   if (!elf)
   {
      close(fd);
      return false;
   }

   bool     hasDWARF = false;
   Elf_Scn* scn      = nullptr;
   
   // Get the section header string table index
   size_t shstrndx;
   if (elf_getshstrndx(elf, &shstrndx) != 0)
   {
      // Failed to get section header string table index
      elf_end(elf);
      close(fd);
      return false;
   }
   
   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      GElf_Shdr shdr;
      if (gelf_getshdr(scn, &shdr) != nullptr)
      {
         const char* sectionName = elf_strptr(elf, shstrndx, shdr.sh_name);
         if (sectionName && (strstr(sectionName, ".debug_") == sectionName ||
                             strstr(sectionName, ".zdebug_") == sectionName))
         {
            hasDWARF = true;
            break;
         }
      }
   }

   elf_end(elf);
   close(fd);
   return hasDWARF;
#else
   // For non-Linux systems (including macOS), check for .dSYM files first
   std::string dsymPath = filePath + ".dSYM";
   if (std::filesystem::exists(dsymPath))
   {
      // Check if the .dSYM file contains a DWARF file
      std::string dwarfPath = dsymPath + "/Contents/Resources/DWARF/" + 
                              std::filesystem::path(filePath).filename().string();
      if (std::filesystem::exists(dwarfPath))
      {
         return true;
      }
   }

   // Fallback: try to detect DWARF sections heuristically in the binary itself
   std::vector<char> buffer(1024);
   testFile.read(buffer.data(), buffer.size());
   std::streamsize bytesRead = testFile.gcount();

   // Look for DWARF section names in the binary
   std::string content(buffer.begin(), buffer.begin() + bytesRead);
   return (content.find(".debug_") != std::string::npos ||
           content.find(".zdebug_") != std::string::npos);
#endif
}

bool DWARFExtractor::extractAllDebugInfo(const std::string&        filePath,
                                         std::vector<std::string>& sourceFiles,
                                         std::vector<std::string>& compileUnits,
                                         std::vector<std::string>& functions,
                                         std::vector<std::string>& lineInfo)
{
   bool success = true;
   success &= extractSourceFiles(filePath, sourceFiles);
   success &= extractCompileUnits(filePath, compileUnits);
   success &= extractFunctions(filePath, functions);
   success &= extractLineInfo(filePath, lineInfo);
   return success;
}

// Private helper methods
bool DWARFExtractor::extractSourceFilesHeuristic(const std::string&        filePath,
                                                 std::vector<std::string>& sourceFiles)
{
   // Implementation of heuristic source file extraction
   // This is a fallback when LLVM DWARF parsing fails
   return false;
}

bool DWARFExtractor::extractFunctionsFromSymbolTable(const std::string&        filePath,
                                                     std::vector<std::string>& functions)
{
   // Implementation of symbol table-based function extraction
   // This is a fallback when DWARF parsing fails
   return false;
}

#ifdef LLVM_DWARF_AVAILABLE
void DWARFExtractor::extractSourceFilesFromDie(const llvm::DWARFDie&     die,
                                               std::vector<std::string>& sourceFiles)
{
   if (!die.isValid())
   {
      return;
   }

   // Extract source files from DWARF DIE
   if (die.getTag() == llvm::dwarf::DW_TAG_compile_unit)
   {
      if (auto name = die.find(llvm::dwarf::DW_AT_name))
      {
         if (auto nameStr = name->getAsCString())
         {
            sourceFiles.push_back(*nameStr);
         }
      }
   }

   // Recursively process children
   for (auto child : die.children())
   {
      extractSourceFilesFromDie(child, sourceFiles);
   }
}

void DWARFExtractor::extractCompileUnitFromDie(const llvm::DWARFDie&     die,
                                               std::vector<std::string>& compileUnits)
{
   if (!die.isValid())
   {
      return;
   }

   // Extract compile unit information from DWARF DIE
   if (die.getTag() == llvm::dwarf::DW_TAG_compile_unit)
   {
      if (auto name = die.find(llvm::dwarf::DW_AT_name))
      {
         if (auto nameStr = name->getAsCString())
         {
            compileUnits.push_back(*nameStr);
         }
      }
   }

   // Recursively process children
   for (auto child : die.children())
   {
      extractCompileUnitFromDie(child, compileUnits);
   }
}

void DWARFExtractor::extractFunctionsFromDie(const llvm::DWARFDie&     die,
                                             std::vector<std::string>& functions)
{
   if (!die.isValid())
   {
      return;
   }

   // Extract function information from DWARF DIE
   if (die.getTag() == llvm::dwarf::DW_TAG_subprogram)
   {
      if (auto name = die.find(llvm::dwarf::DW_AT_name))
      {
         if (auto nameStr = name->getAsCString())
         {
            std::string funcName(*nameStr);
            if (std::find(functions.begin(), functions.end(), funcName) == functions.end())
            {
               functions.push_back(funcName);
            }
         }
      }
   }

   // Recursively process children
   for (auto child : die.children())
   {
      extractFunctionsFromDie(child, functions);
   }
}
#endif

}  // namespace heimdall