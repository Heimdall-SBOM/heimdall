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
 * @file ELFExtractor.cpp
 * @brief ELF (Executable and Linkable Format) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 *
 * This file implements the ELFExtractor class for extracting metadata from
 * ELF binary files. It provides both basic file reading capabilities and
 * advanced libelf-based parsing when available.
 */

#include "ELFExtractor.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "../utils/BinaryReader.hpp"
#include "../utils/FileUtils.hpp"
// Conditionally include DWARFExtractor for C++17+
#if __cplusplus >= 201703L
#include "DWARFExtractor.hpp"
#else
#include "LightweightDWARFParser.hpp"
#endif
#include "common/Utils.hpp"

#ifdef __linux__
#include <elf.h>
#include <fcntl.h>
#include <libelf.h>
#include <unistd.h>
#else
// Define ELF constants for non-Linux systems
#define ELFCLASS32 1
#define ELFCLASS64 2
#define EV_CURRENT 1
#endif

namespace heimdall
{

// PIMPL implementation
class ELFExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   // Configuration
   bool verbose          = false;
   bool extractDebugInfo = true;
   bool suppressWarnings = false;

   // ELF-specific configuration
   bool useLibelf           = true;
   bool extractLocalSymbols = false;
   bool extractDebugSymbols = false;

   // Default DWARF extraction methods
   bool extractFunctionsImpl(const std::string& filePath, std::vector<std::string>& functions)
   {
      // ELFExtractor doesn't support DWARF extraction by default
      // This would need to be implemented with DWARF parsing libraries
      (void)filePath;   // Suppress unused parameter warning
      (void)functions;  // Suppress unused parameter warning
      return false;
   }

   bool extractCompileUnitsImpl(const std::string& filePath, std::vector<std::string>& compileUnits)
   {
      // ELFExtractor doesn't support DWARF extraction by default
      // This would need to be implemented with DWARF parsing libraries
      (void)filePath;      // Suppress unused parameter warning
      (void)compileUnits;  // Suppress unused parameter warning
      return false;
   }

   bool extractSourceFilesImpl(const std::string& filePath, std::vector<std::string>& sourceFiles)
   {
      // ELFExtractor doesn't support DWARF extraction by default
      // This would need to be implemented with DWARF parsing libraries
      (void)filePath;     // Suppress unused parameter warning
      (void)sourceFiles;  // Suppress unused parameter warning
      return false;
   }
};

// Constructor and destructor
ELFExtractor::ELFExtractor() : pImpl(std::make_unique<Impl>()) {}

ELFExtractor::~ELFExtractor() = default;

ELFExtractor::ELFExtractor(const ELFExtractor& other) : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

ELFExtractor::ELFExtractor(ELFExtractor&& other) noexcept : pImpl(std::move(other.pImpl)) {}

ELFExtractor& ELFExtractor::operator=(const ELFExtractor& other)
{
   if (this != &other)
   {
      *pImpl = *other.pImpl;
   }
   return *this;
}

ELFExtractor& ELFExtractor::operator=(ELFExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool ELFExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   symbols.clear();

#ifdef __linux__
   if (pImpl->useLibelf)
   {
      elf_version(EV_CURRENT);
      int fd = open(filePath.c_str(), O_RDONLY);
      if (fd < 0)
      {
         return false;
      }

      Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
      if (!elf)
      {
         close(fd);
         return false;
      }

      bool success = processSymbolTable(elf, symbols);
      elf_end(elf);
      close(fd);
      return success;
   }

#endif

   // Fallback: basic symbol extraction without libelf
   return false;
}

bool ELFExtractor::extractSections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   sections.clear();

#ifdef __linux__
   if (pImpl->useLibelf)
   {
      elf_version(EV_CURRENT);
      int fd = open(filePath.c_str(), O_RDONLY);
      if (fd < 0)
      {
         return false;
      }

      Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
      if (!elf)
      {
         close(fd);
         return false;
      }

      bool success = processSections(elf, sections);
      elf_end(elf);
      close(fd);
      return success;
   }
#endif

   // Fallback: basic section extraction without libelf
   return false;
}

bool ELFExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   // Try to extract version from various sources
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Read ELF header to determine architecture
   char magic[4];
   file.read(magic, 4);
   if (file.gcount() != 4 || magic[0] != 0x7f || magic[1] != 'E' || magic[2] != 'L' ||
       magic[3] != 'F')
   {
      return false;
   }

   // Read ELF class (32-bit vs 64-bit)
   char elfClass;
   file.read(&elfClass, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

   // Read ELF data encoding
   char dataEncoding;
   file.read(&dataEncoding, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

   // Read ELF version
   char elfVersion;
   file.read(&elfVersion, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

   // Read OS/ABI
   char osAbi;
   file.read(&osAbi, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

   // Read ABI version
   char abiVersion;
   file.read(&abiVersion, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

   // Skip padding
   char padding[7];
   file.read(padding, 7);
   if (file.gcount() != 7)
   {
      return false;
   }

   // Read ELF type
   uint16_t elfType;
   file.read(reinterpret_cast<char*>(&elfType), 2);
   if (file.gcount() != 2)
   {
      return false;
   }

   // Read machine type
   uint16_t machine;
   file.read(reinterpret_cast<char*>(&machine), 2);
   if (file.gcount() != 2)
   {
      return false;
   }

   // Read ELF version
   uint32_t versionField;
   file.read(reinterpret_cast<char*>(&versionField), 4);
   if (file.gcount() != 4)
   {
      return false;
   }

   // Build version string
   std::ostringstream oss;
   oss << "ELF";
#ifdef __linux__
   if (elfClass == ELFCLASS64)
   {
      oss << "64";
   }
   else
   {
      oss << "32";
   }
#else
   // On non-Linux systems, use raw values
   if (elfClass == 2)  // ELFCLASS64 = 2
   {
      oss << "64";
   }
   else if (elfClass == 1)  // ELFCLASS32 = 1
   {
      oss << "32";
   }
   else
   {
      oss << "unknown";
   }
#endif
   oss << "-v" << static_cast<int>(elfVersion);

   // Add machine architecture
#ifdef __linux__
   switch (machine)
   {
      case EM_X86_64:
         oss << "-x86_64";
         break;
      case EM_386:
         oss << "-i386";
         break;
      case EM_AARCH64:
         oss << "-aarch64";
         break;
      case EM_ARM:
         oss << "-arm";
         break;
      case EM_MIPS:
         oss << "-mips";
         break;
      case EM_PPC64:
         oss << "-ppc64";
         break;
      case EM_S390:
         oss << "-s390x";
         break;
      case EM_RISCV:
         oss << "-riscv64";
         break;
      default:
         oss << "-unknown";
         break;
   }
#else
   oss << "-unknown";
#endif

   version = oss.str();
   return true;
}

std::vector<std::string> ELFExtractor::extractDependencies(const std::string& filePath)
{
   std::vector<std::string> dependencies;
   if (!validateELFHeader(filePath))
   {
      return dependencies;
   }

#ifdef __linux__
   if (pImpl->useLibelf)
   {
      elf_version(EV_CURRENT);
      int fd = open(filePath.c_str(), O_RDONLY);
      if (fd < 0)
      {
         return dependencies;
      }

      Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
      if (!elf)
      {
         close(fd);
         return dependencies;
      }

      // Look for dynamic section
      Elf_Scn* scn = nullptr;
      while ((scn = elf_nextscn(elf, scn)) != nullptr)
      {
         Elf64_Shdr* shdr = elf64_getshdr(scn);
         if (!shdr || shdr->sh_type != SHT_DYNAMIC)
         {
            continue;
         }

         Elf_Data* data = elf_getdata(scn, nullptr);
         if (!data)
         {
            continue;
         }

         Elf64_Dyn* dyn    = static_cast<Elf64_Dyn*>(data->d_buf);
         size_t     numDyn = data->d_size / sizeof(Elf64_Dyn);

         // Find string table
         Elf_Scn* strscn = elf_getscn(elf, shdr->sh_link);
         if (!strscn)
         {
            continue;
         }

         Elf_Data* strdata = elf_getdata(strscn, nullptr);
         if (!strdata)
         {
            continue;
         }

         char* strtab = static_cast<char*>(strdata->d_buf);

         // Process dynamic entries
         for (size_t i = 0; i < numDyn; ++i)
         {
            if (dyn[i].d_tag == DT_NEEDED)
            {
               const char* libName = strtab + dyn[i].d_un.d_val;
               dependencies.push_back(libName);
            }
         }
      }

      elf_end(elf);
      close(fd);
   }
#endif

   return dependencies;
}

bool ELFExtractor::extractFunctions(const std::string&        filePath,
                                    std::vector<std::string>& functions)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
#else
   LightweightDWARFParser dwarfExtractor;
#endif
   return dwarfExtractor.extractFunctions(filePath, functions);
}

bool ELFExtractor::extractCompileUnits(const std::string&        filePath,
                                       std::vector<std::string>& compileUnits)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
#else
   LightweightDWARFParser dwarfExtractor;
#endif
   return dwarfExtractor.extractCompileUnits(filePath, compileUnits);
}

bool ELFExtractor::extractSourceFiles(const std::string&        filePath,
                                      std::vector<std::string>& sourceFiles)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
#else
   LightweightDWARFParser dwarfExtractor;
#endif
   return dwarfExtractor.extractSourceFiles(filePath, sourceFiles);
}

bool ELFExtractor::canHandle(const std::string& filePath) const
{
   return validateELFHeader(filePath);
}

std::string ELFExtractor::getFormatName() const
{
   return "ELF";
}

int ELFExtractor::getPriority() const
{
   return 0;  // Highest priority for ELF on Linux
}

// ELF-specific methods
bool ELFExtractor::extractBuildId(const std::string& filePath, std::string& buildId)
{
   if (!canHandle(filePath))
   {
      return false;
   }

#ifdef __linux__
   if (pImpl->useLibelf)
   {
      elf_version(EV_CURRENT);
      int fd = open(filePath.c_str(), O_RDONLY);
      if (fd < 0)
      {
         return false;
      }

      Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
      if (!elf)
      {
         close(fd);
         return false;
      }

      // Look for .note.gnu.build-id section
      Elf_Scn* scn = nullptr;
      while ((scn = elf_nextscn(elf, scn)) != nullptr)
      {
         Elf64_Shdr* shdr = elf64_getshdr(scn);
         if (!shdr)
         {
            continue;
         }

         // Get section name
         Elf_Scn* shstrscn = elf_getscn(elf, elf64_getehdr(elf)->e_shstrndx);
         if (!shstrscn)
         {
            continue;
         }

         Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
         if (!shstrshdr)
         {
            continue;
         }

         Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
         if (!shstrdata)
         {
            continue;
         }

         char*       shstrtab    = static_cast<char*>(shstrdata->d_buf);
         const char* sectionName = shstrtab + shdr->sh_name;

         if (strcmp(sectionName, ".note.gnu.build-id") == 0)
         {
            Elf_Data* data = elf_getdata(scn, nullptr);
            if (data && data->d_size > 16)
            {
               // Parse note header
               uint32_t* noteData = static_cast<uint32_t*>(data->d_buf);
               uint32_t  nameSize = noteData[0];
               uint32_t  descSize = noteData[1];
               uint32_t  noteType = noteData[2];

               if (noteType == NT_GNU_BUILD_ID && descSize > 0)
               {
                  // Extract build ID
                  const unsigned char* desc =
                     reinterpret_cast<const unsigned char*>(&noteData[3]) + nameSize;
                  std::ostringstream oss;
                  for (uint32_t i = 0; i < descSize; ++i)
                  {
                     oss << std::hex << std::setw(2) << std::setfill('0')
                         << static_cast<int>(desc[i]);
                  }
                  buildId = oss.str();
                  elf_end(elf);
                  close(fd);
                  return true;
               }
            }
         }
      }

      elf_end(elf);
      close(fd);
   }
#endif

   return false;
}

std::string ELFExtractor::getArchitecture(const std::string& filePath)
{
   if (!canHandle(filePath))
   {
      return "unknown";
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return "unknown";
   }

   // Skip to machine field in ELF header
   file.seekg(18);  // Skip magic, class, data, version, os/abi, abi version, padding
   if (!file.good())
   {
      return "unknown";
   }

   uint16_t machine;
   file.read(reinterpret_cast<char*>(&machine), 2);
   if (file.gcount() != 2)
   {
      return "unknown";
   }

#ifdef __linux__
   switch (machine)
   {
      case EM_X86_64:
         return "x86_64";
      case EM_386:
         return "i386";
      case EM_AARCH64:
         return "aarch64";
      case EM_ARM:
         return "arm";
      case EM_MIPS:
         return "mips";
      case EM_PPC64:
         return "powerpc64";
      case EM_S390:
         return "s390x";
      case EM_RISCV:
         return "riscv64";
      default:
         return "unknown";
   }
#else
   return "unknown";
#endif
}

bool ELFExtractor::is64Bit(const std::string& filePath)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Skip magic number
   file.seekg(4);
   if (!file.good())
   {
      return false;
   }

   char elfClass;
   file.read(&elfClass, 1);
   if (file.gcount() != 1)
   {
      return false;
   }

#ifdef __linux__
   return (elfClass == ELFCLASS64);
#else
   // On non-Linux systems, use raw value (ELFCLASS64 = 2)
   return (elfClass == 2);
#endif
}

std::string ELFExtractor::getFileType(const std::string& filePath)
{
   if (!canHandle(filePath))
   {
      return "unknown";
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return "unknown";
   }

   // Skip to type field in ELF header
   file.seekg(16);  // Skip magic, class, data, version, os/abi, abi version, padding
   if (!file.good())
   {
      return "unknown";
   }

   uint16_t elfType;
   file.read(reinterpret_cast<char*>(&elfType), 2);
   if (file.gcount() != 2)
   {
      return "unknown";
   }

#ifdef __linux__
   switch (elfType)
   {
      case ET_NONE:
         return "ET_NONE";
      case ET_REL:
         return "ET_REL";
      case ET_EXEC:
         return "ET_EXEC";
      case ET_DYN:
         return "ET_DYN";
      case ET_CORE:
         return "ET_CORE";
      default:
         return "unknown";
   }
#else
   return "unknown";
#endif
}

// Private helper methods
bool ELFExtractor::validateELFHeader(const std::string& filePath) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   char magic[4];
   file.read(magic, 4);
   if (file.gcount() != 4)
   {
      return false;
   }

   return (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

#ifdef __linux__
bool ELFExtractor::processSymbolTable(Elf* elf, std::vector<SymbolInfo>& symbols)
{
   Elf_Scn* scn = nullptr;
   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      Elf64_Shdr* shdr = elf64_getshdr(scn);
      if (!shdr)
      {
         continue;
      }

      if (shdr->sh_type == SHT_SYMTAB || shdr->sh_type == SHT_DYNSYM)
      {
         Elf_Data* data = elf_getdata(scn, nullptr);
         if (!data)
         {
            continue;
         }

         // Get string table
         Elf_Scn* strscn = elf_getscn(elf, shdr->sh_link);
         if (!strscn)
         {
            continue;
         }

         Elf_Data* strdata = elf_getdata(strscn, nullptr);
         if (!strdata)
         {
            continue;
         }

         char*      strtab = static_cast<char*>(strdata->d_buf);
         Elf64_Sym* symtab = static_cast<Elf64_Sym*>(data->d_buf);
         size_t     nsyms  = data->d_size / sizeof(Elf64_Sym);

         for (size_t i = 0; i < nsyms; ++i)
         {
            Elf64_Sym& sym = symtab[i];

            // Skip null symbols
            if (sym.st_name == 0)
            {
               continue;
            }

            // Skip local symbols unless requested
            if (ELF64_ST_BIND(sym.st_info) == STB_LOCAL && !pImpl->extractLocalSymbols)
            {
               continue;
            }

            // Skip debugging symbols unless requested
            if (ELF64_ST_TYPE(sym.st_info) == STT_FILE && !pImpl->extractDebugSymbols)
            {
               continue;
            }

            SymbolInfo symbol;
            symbol.name    = strtab + sym.st_name;
            symbol.address = sym.st_value;
            symbol.size    = sym.st_size;
            symbol.isDefined =
               (ELF64_ST_TYPE(sym.st_info) != STT_NOTYPE) && (sym.st_shndx != SHN_UNDEF);
            symbol.isGlobal = (ELF64_ST_BIND(sym.st_info) == STB_GLOBAL);
            symbol.isWeak   = (ELF64_ST_BIND(sym.st_info) == STB_WEAK);

            symbols.push_back(symbol);
         }
      }
   }

   return !symbols.empty();
}

bool ELFExtractor::processSections(Elf* elf, std::vector<SectionInfo>& sections)
{
   Elf_Scn* scn = nullptr;
   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      Elf64_Shdr* shdr = elf64_getshdr(scn);
      if (!shdr)
      {
         continue;
      }

      // Get section name
      Elf_Scn* shstrscn = elf_getscn(elf, elf64_getehdr(elf)->e_shstrndx);
      if (!shstrscn)
      {
         continue;
      }

      Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
      if (!shstrshdr)
      {
         continue;
      }

      Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
      if (!shstrdata)
      {
         continue;
      }

      char*       shstrtab    = static_cast<char*>(shstrdata->d_buf);
      const char* sectionName = shstrtab + shdr->sh_name;

      SectionInfo section;
      section.name    = sectionName;
      section.address = shdr->sh_addr;
      section.size    = shdr->sh_size;
      section.flags   = shdr->sh_flags;

      // Determine section type
      switch (shdr->sh_type)
      {
         case SHT_NULL:
            section.type = "NULL";
            break;
         case SHT_PROGBITS:
            section.type = "PROGBITS";
            break;
         case SHT_SYMTAB:
            section.type = "SYMTAB";
            break;
         case SHT_STRTAB:
            section.type = "STRTAB";
            break;
         case SHT_RELA:
            section.type = "RELA";
            break;
         case SHT_HASH:
            section.type = "HASH";
            break;
         case SHT_DYNAMIC:
            section.type = "DYNAMIC";
            break;
         case SHT_NOTE:
            section.type = "NOTE";
            break;
         case SHT_NOBITS:
            section.type = "NOBITS";
            break;
         case SHT_REL:
            section.type = "REL";
            break;
         case SHT_SHLIB:
            section.type = "SHLIB";
            break;
         case SHT_DYNSYM:
            section.type = "DYNSYM";
            break;
         default:
            section.type = "UNKNOWN";
            break;
      }

      sections.push_back(section);
   }

   return !sections.empty();
}
#endif

}  // namespace heimdall