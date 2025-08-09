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
 * @file MachOExtractor.cpp
 * @brief Mach-O (Mach Object) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 */

#include "MachOExtractor.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
// Conditionally include DWARFExtractor for C++17+
#if __cplusplus >= 201703L
#include "DWARFExtractor.hpp"
#else
#include "LightweightDWARFParser.hpp"
#endif

#ifdef __APPLE__
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#else
// Define Mach-O constants for non-Apple platforms
#define FAT_MAGIC     0xcafebabe
#define FAT_CIGAM     0xbebafeca
#define FAT_MAGIC_64  0xcafebabf
#define FAT_CIGAM_64  0xbfbafeca
#define MH_MAGIC      0xfeedface
#define MH_CIGAM      0xcefaedfe
#define MH_MAGIC_64   0xfeedfacf
#define MH_CIGAM_64   0xcffaedfe

// Define basic Mach-O structures for cross-platform compatibility
struct mach_header {
    uint32_t magic;
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
};

struct mach_header_64 {
    uint32_t magic;
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};

struct fat_header {
    uint32_t magic;
    uint32_t nfat_arch;
};

struct fat_arch {
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t offset;
    uint32_t size;
    uint32_t align;
};

struct load_command {
    uint32_t cmd;
    uint32_t cmdsize;
};

struct uuid_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint8_t uuid[16];
};

struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
};

struct segment_command {
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
    uint32_t vmaddr;
    uint32_t vmsize;
    uint32_t fileoff;
    uint32_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct section {
    char sectname[16];
    char segname[16];
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct section_64 {
    char sectname[16];
    char segname[16];
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
};

struct dylib_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t dylib_name_offset;
    uint32_t dylib_timestamp;
    uint32_t dylib_current_version;
    uint32_t dylib_compatibility_version;
};

struct nlist {
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;
    int16_t n_desc;
    uint32_t n_value;
};

struct nlist_64 {
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;
    uint16_t n_desc;
    uint64_t n_value;
};
#endif

namespace heimdall
{

// PIMPL implementation
class MachOExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   // Implementation methods will be added here
   bool extractSymbolsImpl(const std::string& filePath, std::vector<SymbolInfo>& symbols);
   bool extractSectionsImpl(const std::string& filePath, std::vector<SectionInfo>& sections);
   bool extractVersionImpl(const std::string& filePath, std::string& version);
   std::vector<std::string> extractDependenciesImpl(const std::string& filePath);
   bool                     canHandleImpl(const std::string& filePath) const;
   std::string              getFormatNameImpl() const;
   int                      getPriorityImpl() const;
   bool                     extractBuildIdImpl(const std::string& filePath, std::string& buildId);
   std::string              getArchitectureImpl(const std::string& filePath);
   bool                     is64BitImpl(const std::string& filePath);
   std::string              getFileTypeImpl(const std::string& filePath);
   bool                     isUniversalBinaryImpl(const std::string& filePath);
   std::vector<std::string> getUniversalArchitecturesImpl(const std::string& filePath);

   public:
   // Helper methods for Mach-O parsing
   bool validateMachOHeaderImpl(const std::string& filePath) const;
   bool processSymbolTableImpl(const std::string& filePath, std::vector<SymbolInfo>& symbols);
   bool processLoadCommandsImpl(const std::string& filePath, std::vector<SectionInfo>& sections);
   std::vector<std::string> extractDependenciesFromLoadCommandsImpl(const std::string& filePath);

   template <typename T>
   bool        readMachOHeader(const std::string& filePath, T& header) const;

   std::string getArchitectureString(uint32_t cputype, uint32_t cpusubtype) const;
   std::string getFileTypeString(uint32_t filetype) const;
};

// Public interface implementation
MachOExtractor::MachOExtractor() : pImpl(std::make_unique<Impl>()) {}

MachOExtractor::~MachOExtractor() = default;

MachOExtractor::MachOExtractor(const MachOExtractor& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

MachOExtractor::MachOExtractor(MachOExtractor&& other) noexcept : pImpl(std::move(other.pImpl)) {}

MachOExtractor& MachOExtractor::operator=(const MachOExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

MachOExtractor& MachOExtractor::operator=(MachOExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool MachOExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   return pImpl->extractSymbolsImpl(filePath, symbols);
}

bool MachOExtractor::extractSections(const std::string&        filePath,
                                     std::vector<SectionInfo>& sections)
{
   return pImpl->extractSectionsImpl(filePath, sections);
}

bool MachOExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   return pImpl->extractVersionImpl(filePath, version);
}

std::vector<std::string> MachOExtractor::extractDependencies(const std::string& filePath)
{
   return pImpl->extractDependenciesFromLoadCommandsImpl(filePath);
}

bool MachOExtractor::extractFunctions(const std::string&        filePath,
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

bool MachOExtractor::extractCompileUnits(const std::string&        filePath,
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

bool MachOExtractor::extractSourceFiles(const std::string&        filePath,
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

bool MachOExtractor::canHandle(const std::string& filePath) const
{
   return pImpl->canHandleImpl(filePath);
}

std::string MachOExtractor::getFormatName() const
{
   return pImpl->getFormatNameImpl();
}

int MachOExtractor::getPriority() const
{
   return pImpl->getPriorityImpl();
}

// Mach-O specific methods
bool MachOExtractor::extractBuildId(const std::string& filePath, std::string& buildId)
{
   return pImpl->extractBuildIdImpl(filePath, buildId);
}

std::string MachOExtractor::getArchitecture(const std::string& filePath)
{
   return pImpl->getArchitectureImpl(filePath);
}

bool MachOExtractor::is64Bit(const std::string& filePath)
{
   return pImpl->is64BitImpl(filePath);
}

std::string MachOExtractor::getFileType(const std::string& filePath)
{
   return pImpl->getFileTypeImpl(filePath);
}

bool MachOExtractor::isUniversalBinary(const std::string& filePath)
{
   return pImpl->isUniversalBinaryImpl(filePath);
}

std::vector<std::string> MachOExtractor::getUniversalArchitectures(const std::string& filePath)
{
   return pImpl->getUniversalArchitecturesImpl(filePath);
}

// Private helper methods
bool MachOExtractor::validateMachOHeader(const std::string& filePath) const
{
   return pImpl->validateMachOHeaderImpl(filePath);
}

bool MachOExtractor::processSymbolTable(const std::string&       filePath,
                                        std::vector<SymbolInfo>& symbols)
{
   return pImpl->processSymbolTableImpl(filePath, symbols);
}

bool MachOExtractor::processLoadCommands(const std::string&        filePath,
                                         std::vector<SectionInfo>& sections)
{
   return pImpl->processLoadCommandsImpl(filePath, sections);
}

std::vector<std::string> MachOExtractor::extractDependenciesFromLoadCommands(
   const std::string& filePath)
{
   return pImpl->extractDependenciesFromLoadCommandsImpl(filePath);
}

// PIMPL implementation methods
bool MachOExtractor::Impl::extractSymbolsImpl(const std::string&       filePath,
                                              std::vector<SymbolInfo>& symbols)
{
   symbols.clear();

   if (!validateMachOHeaderImpl(filePath))
   {
      return false;
   }

   return processSymbolTableImpl(filePath, symbols);
}

bool MachOExtractor::Impl::extractSectionsImpl(const std::string&        filePath,
                                               std::vector<SectionInfo>& sections)
{
   sections.clear();

   if (!validateMachOHeaderImpl(filePath))
   {
      return false;
   }

   return processLoadCommandsImpl(filePath, sections);
}

bool MachOExtractor::Impl::extractVersionImpl(const std::string& filePath, std::string& version)
{
   version.clear();

   if (!validateMachOHeaderImpl(filePath))
   {
      return false;
   }

   // For Mach-O files, version information is typically embedded in the binary
   // or can be extracted from specific load commands. This is a simplified implementation.
   version = "Unknown";
   return true;
}

std::vector<std::string> MachOExtractor::Impl::extractDependenciesImpl(const std::string& filePath)
{
   if (!validateMachOHeaderImpl(filePath))
   {
      return {};
   }

   return extractDependenciesFromLoadCommandsImpl(filePath);
}

bool MachOExtractor::Impl::canHandleImpl(const std::string& filePath) const
{
   return validateMachOHeaderImpl(filePath);
}

std::string MachOExtractor::Impl::getFormatNameImpl() const
{
   return "Mach-O";
}

int MachOExtractor::Impl::getPriorityImpl() const
{
   return 80;  // High priority for Mach-O files
}

bool MachOExtractor::Impl::extractBuildIdImpl(const std::string& filePath, std::string& buildId)
{
#ifdef __APPLE__
   buildId.clear();

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_UUID)
         {
            struct uuid_command uuid_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));

            // Format UUID as string
            char uuid_str[37];
            snprintf(uuid_str, sizeof(uuid_str),
                     "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                     uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                     uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                     uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                     uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
            buildId = uuid_str;
            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_UUID)
         {
            struct uuid_command uuid_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));

            char uuid_str[37];
            snprintf(uuid_str, sizeof(uuid_str),
                     "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                     uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                     uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                     uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                     uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
            buildId = uuid_str;
            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return false;
#else
   buildId.clear();
   return false;
#endif
}

std::string MachOExtractor::Impl::getArchitectureImpl(const std::string& filePath)
{
   if (!validateMachOHeaderImpl(filePath))
   {
      return "Unknown";
   }

   // Check if it's a universal binary first
   if (isUniversalBinaryImpl(filePath))
   {
      auto architectures = getUniversalArchitecturesImpl(filePath);
      if (!architectures.empty())
      {
         return architectures[0];  // Return first architecture
      }
   }

   // For single architecture files, read the header
#ifdef __APPLE__
   mach_header header;
   if (readMachOHeader(filePath, header))
   {
      return getArchitectureString(header.cputype, header.cpusubtype);
   }

   mach_header_64 header64;
   if (readMachOHeader(filePath, header64))
   {
      return getArchitectureString(header64.cputype, header64.cpusubtype);
   }
#endif

   return "Unknown";
}

bool MachOExtractor::Impl::is64BitImpl(const std::string& filePath)
{
   if (!validateMachOHeaderImpl(filePath))
   {
      return false;
   }

#ifdef __APPLE__
   // Try to read as 64-bit header first
   mach_header_64 header64;
   if (readMachOHeader(filePath, header64))
   {
      return true;
   }

   // Try to read as 32-bit header
   mach_header header;
   if (readMachOHeader(filePath, header))
   {
      return true;
   }

   return false;
#else
   return false;
#endif
}

std::string MachOExtractor::Impl::getFileTypeImpl(const std::string& filePath)
{
   if (!validateMachOHeaderImpl(filePath))
   {
      return "Unknown";
   }
   else
   {
#ifdef __APPLE__
      mach_header header;
      if (readMachOHeader(filePath, header))
      {
         return getFileTypeString(header.filetype);
      }

      mach_header_64 header64;
      if (readMachOHeader(filePath, header64))
      {
         return getFileTypeString(header64.filetype);
      }
#endif
   }

   return "Unknown";
#else
   return "Unknown";
#endif
}

bool MachOExtractor::Impl::isUniversalBinaryImpl(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   uint32_t magic;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

   // Check for fat magic numbers
   return (magic == FAT_MAGIC || magic == FAT_CIGAM || magic == FAT_MAGIC_64 ||
           magic == FAT_CIGAM_64);
}

std::vector<std::string> MachOExtractor::Impl::getUniversalArchitecturesImpl(
   const std::string& filePath)
{
   std::vector<std::string> architectures;

   if (!isUniversalBinaryImpl(filePath))
   {
      return architectures;
   }
   else
   {
#ifdef __APPLE__
       std::ifstream file(filePath, std::ios::binary);
       if (!file.is_open())
       {
          return architectures;
       }

       fat_header fatHeader;
       file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));

       // Check if byte swapping is needed (once for the whole file)
       bool needsByteSwap = (fatHeader.magic == FAT_CIGAM || fatHeader.magic == FAT_CIGAM_64);
       
       if (needsByteSwap)
       {
          fatHeader.nfat_arch = __builtin_bswap32(fatHeader.nfat_arch);
       }

       for (uint32_t i = 0; i < fatHeader.nfat_arch; ++i)
       {
          fat_arch arch;
          file.read(reinterpret_cast<char*>(&arch), sizeof(arch));

          if (needsByteSwap)
          {
             arch.cputype    = __builtin_bswap32(arch.cputype);
             arch.cpusubtype = __builtin_bswap32(arch.cpusubtype);
          }

          architectures.push_back(getArchitectureString(arch.cputype, arch.cpusubtype));
       }
#endif
   }

   return architectures;
}

// Private helper methods
bool MachOExtractor::Impl::validateMachOHeaderImpl(const std::string& filePath) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   uint32_t magic;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

   // Check for Mach-O magic numbers
   return (magic == MH_MAGIC || magic == MH_CIGAM || magic == MH_MAGIC_64 || magic == MH_CIGAM_64 ||
           magic == FAT_MAGIC || magic == FAT_CIGAM || magic == FAT_MAGIC_64 ||
           magic == FAT_CIGAM_64);
}

bool MachOExtractor::Impl::processSymbolTableImpl(const std::string&       filePath,
                                                  std::vector<SymbolInfo>& symbols)
{
#ifdef __APPLE__
   symbols.clear();

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Read magic to check for fat or thin
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      // Fat binary: read first architecture only for now
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_SYMTAB)
         {
            struct symtab_command symtab{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));

            // Read string table
            std::vector<char> strtab(symtab.strsize);
            file.seekg(symtab.stroff);
            file.read(strtab.data(), symtab.strsize);

            // Read symbol table
            file.seekg(symtab.symoff);
            for (uint32_t j = 0; j < symtab.nsyms; ++j)
            {
               struct nlist_64 nlsym{};
               file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));

               SymbolInfo sym;
               if (nlsym.n_un.n_strx < symtab.strsize)
               {
                  std::string raw_name = &strtab[nlsym.n_un.n_strx];
                  // Strip leading underscore for Mach-O symbols
                  if (!raw_name.empty() && raw_name[0] == '_')
                     sym.name = raw_name.substr(1);
                  else
                     sym.name = raw_name;
               }
               else
                  sym.name = "<badstrx>";

               sym.address   = nlsym.n_value;
               sym.size      = 0;  // Mach-O doesn't store symbol size
               sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
               sym.isGlobal  = (nlsym.n_type & N_EXT);
               sym.section   = std::to_string(nlsym.n_sect);
               symbols.push_back(sym);
            }

            break;
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_SYMTAB)
         {
            struct symtab_command symtab{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));

            std::vector<char> strtab(symtab.strsize);
            file.seekg(symtab.stroff);
            file.read(strtab.data(), symtab.strsize);

            file.seekg(symtab.symoff);
            for (uint32_t j = 0; j < symtab.nsyms; ++j)
            {
               struct nlist nlsym{};
               file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));

               SymbolInfo sym;
               if (nlsym.n_un.n_strx < symtab.strsize)
               {
                  std::string raw_name = &strtab[nlsym.n_un.n_strx];
                  // Strip leading underscore for Mach-O symbols
                  if (!raw_name.empty() && raw_name[0] == '_')
                     sym.name = raw_name.substr(1);
                  else
                     sym.name = raw_name;
               }
               else
                  sym.name = "<badstrx>";

               sym.address   = nlsym.n_value;
               sym.size      = 0;
               sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
               sym.isGlobal  = (nlsym.n_type & N_EXT);
               sym.section   = std::to_string(nlsym.n_sect);
               symbols.push_back(sym);
            }
            break;
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return !symbols.empty();
#else
   symbols.clear();
   return false;
#endif
}

bool MachOExtractor::Impl::processLoadCommandsImpl(const std::string&        filePath,
                                                   std::vector<SectionInfo>& sections)
{
#ifdef __APPLE__
   sections.clear();

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_SEGMENT_64)
         {
            struct segment_command_64 seg{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&seg), sizeof(seg));

            // Process sections in this segment
            for (uint32_t j = 0; j < seg.nsects; ++j)
            {
               struct section_64 sect{};
               file.read(reinterpret_cast<char*>(&sect), sizeof(sect));

               SectionInfo section;
               section.name    = sect.sectname;
               section.address = sect.addr;
               section.size    = sect.size;
               section.flags   = sect.flags;
               section.type    = "SECT";
               sections.push_back(section);
            }
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_SEGMENT)
         {
            struct segment_command seg{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&seg), sizeof(seg));

            // Process sections in this segment
            for (uint32_t j = 0; j < seg.nsects; ++j)
            {
               struct section sect{};
               file.read(reinterpret_cast<char*>(&sect), sizeof(sect));

               SectionInfo section;
               section.name    = sect.sectname;
               section.address = sect.addr;
               section.size    = sect.size;
               section.flags   = sect.flags;
               section.type    = "SECT";
               sections.push_back(section);
            }
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return !sections.empty();
#else
   sections.clear();
   return false;
#endif
}

std::vector<std::string> MachOExtractor::Impl::extractDependenciesFromLoadCommandsImpl(
   const std::string& filePath)
{
#ifdef __APPLE__
   std::vector<std::string> libraries;

   std::ifstream            file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return libraries;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         // Check for dynamic library load commands
         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            // Read library name
            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            if (!libName.empty())
            {
               libraries.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            if (!libName.empty())
            {
               libraries.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return libraries;
#else
   return {};
#endif
}

template <typename T>
bool MachOExtractor::Impl::readMachOHeader(const std::string& filePath, T& header) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   file.read(reinterpret_cast<char*>(&header), sizeof(header));
   return file.good();
}

std::string MachOExtractor::Impl::getArchitectureString(uint32_t cputype, uint32_t cpusubtype) const
{
#ifdef __APPLE__
   switch (cputype)
   {
      case CPU_TYPE_X86:
         return "i386";
      case CPU_TYPE_X86_64:
         return "x86_64";
      case CPU_TYPE_ARM:
         return "arm";
      case CPU_TYPE_ARM64:
         return "arm64";
      case CPU_TYPE_POWERPC:
         return "ppc";
      case CPU_TYPE_POWERPC64:
         return "ppc64";
      default:
         return "Unknown";
   }
#else
   return "Unknown";
#endif
}

std::string MachOExtractor::Impl::getFileTypeString(uint32_t filetype) const
{
#ifdef __APPLE__
   switch (filetype)
   {
      case MH_OBJECT:
         return "MH_OBJECT";
      case MH_EXECUTE:
         return "MH_EXECUTE";
      case MH_FVMLIB:
         return "MH_FVMLIB";
      case MH_CORE:
         return "MH_CORE";
      case MH_PRELOAD:
         return "MH_PRELOAD";
      case MH_DYLIB:
         return "MH_DYLIB";
      case MH_DYLINKER:
         return "MH_DYLINKER";
      case MH_BUNDLE:
         return "MH_BUNDLE";
      case MH_DYLIB_STUB:
         return "MH_DYLIB_STUB";
      case MH_DSYM:
         return "MH_DSYM";
      case MH_KEXT_BUNDLE:
         return "MH_KEXT_BUNDLE";
      default:
         return "Unknown";
   }
#else
   return "Unknown";
#endif
}

}  // namespace heimdall
