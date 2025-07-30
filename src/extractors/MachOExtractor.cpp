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
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef __APPLE__
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
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
   // MachOExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;   // Suppress unused parameter warning
   (void)functions;  // Suppress unused parameter warning
   return false;
}

bool MachOExtractor::extractCompileUnits(const std::string&        filePath,
                                         std::vector<std::string>& compileUnits)
{
   // MachOExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;      // Suppress unused parameter warning
   (void)compileUnits;  // Suppress unused parameter warning
   return false;
}

bool MachOExtractor::extractSourceFiles(const std::string&        filePath,
                                        std::vector<std::string>& sourceFiles)
{
   // MachOExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;     // Suppress unused parameter warning
   (void)sourceFiles;  // Suppress unused parameter warning
   return false;
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
   buildId.clear();

   if (!validateMachOHeaderImpl(filePath))
   {
      return false;
   }

   // Implementation would parse LC_UUID load command
   // For now, return a placeholder
   buildId = "Unknown";
   return true;
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
      return false;
   }
#endif

   return false;
}

std::string MachOExtractor::Impl::getFileTypeImpl(const std::string& filePath)
{
   if (!validateMachOHeaderImpl(filePath))
   {
      return "Unknown";
   }

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

   return "Unknown";
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

#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return architectures;
   }

   fat_header fatHeader;
   file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));

   // Handle endianness
   if (fatHeader.magic == FAT_CIGAM || fatHeader.magic == FAT_CIGAM_64)
   {
      // Swap bytes if needed
      fatHeader.nfat_arch = __builtin_bswap32(fatHeader.nfat_arch);
   }

   for (uint32_t i = 0; i < fatHeader.nfat_arch; ++i)
   {
      fat_arch arch;
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));

      // Handle endianness
      if (fatHeader.magic == FAT_CIGAM || fatHeader.magic == FAT_CIGAM_64)
      {
         arch.cputype    = __builtin_bswap32(arch.cputype);
         arch.cpusubtype = __builtin_bswap32(arch.cpusubtype);
      }

      architectures.push_back(getArchitectureString(arch.cputype, arch.cpusubtype));
   }
#endif

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
   // Implementation would parse LC_SYMTAB load command and symbol table
   // For now, return empty vector
   symbols.clear();
   return true;
}

bool MachOExtractor::Impl::processLoadCommandsImpl(const std::string&        filePath,
                                                   std::vector<SectionInfo>& sections)
{
   // Implementation would parse load commands and extract section information
   // For now, return empty vector
   sections.clear();
   return true;
}

std::vector<std::string> MachOExtractor::Impl::extractDependenciesFromLoadCommandsImpl(
   const std::string& filePath)
{
   // Implementation would parse LC_LOAD_DYLIB load commands
   // For now, return empty vector
   return {};
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