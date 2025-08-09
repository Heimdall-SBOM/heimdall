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
 * @file PEExtractor.cpp
 * @brief PE (Portable Executable) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 */

#include "PEExtractor.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <winnt.h>
#endif

namespace heimdall
{

// PIMPL implementation
class PEExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   // Implementation methods
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
   bool extractVersionInfoImpl(const std::string& filePath, std::string& versionInfo);
   std::vector<std::string> extractImportsImpl(const std::string& filePath);
   std::vector<std::string> extractExportsImpl(const std::string& filePath);

   public:
   // Helper methods for PE parsing
   bool validatePEHeaderImpl(const std::string& filePath) const;
   bool processCOFFSymbolTableImpl(const std::string& filePath, std::vector<SymbolInfo>& symbols);
   bool processPESectionsImpl(const std::string& filePath, std::vector<SectionInfo>& sections);
   std::vector<std::string> extractDependenciesFromIATImpl(const std::string& filePath);
   bool extractVersionInfoFromResourcesImpl(const std::string& filePath, std::string& versionInfo);

   template <typename T>
   bool        readPEHeader(const std::string& filePath, T& header) const;

   // Helper method to read PE file header (reduces code duplication)
   bool        readPEFileHeader(const std::string& filePath, uint16_t& machine) const;

   std::string getArchitectureString(uint16_t machine) const;
   std::string getFileTypeString(uint16_t machine) const;
};

// Public interface implementation
PEExtractor::PEExtractor() : pImpl(std::make_unique<Impl>()) {}

PEExtractor::~PEExtractor() = default;

PEExtractor::PEExtractor(const PEExtractor& other) : pImpl(std::make_unique<Impl>(*other.pImpl)) {}

PEExtractor::PEExtractor(PEExtractor&& other) noexcept : pImpl(std::move(other.pImpl)) {}

PEExtractor& PEExtractor::operator=(const PEExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

PEExtractor& PEExtractor::operator=(PEExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool PEExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   return pImpl->extractSymbolsImpl(filePath, symbols);
}

bool PEExtractor::extractSections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
   return pImpl->extractSectionsImpl(filePath, sections);
}

bool PEExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   return pImpl->extractVersionImpl(filePath, version);
}

std::vector<std::string> PEExtractor::extractDependencies(const std::string& filePath)
{
   return pImpl->extractDependenciesFromIATImpl(filePath);
}

bool PEExtractor::extractFunctions(const std::string& filePath, std::vector<std::string>& functions)
{
   // PEExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;   // Suppress unused parameter warning
   (void)functions;  // Suppress unused parameter warning
   return false;
}

bool PEExtractor::extractCompileUnits(const std::string&        filePath,
                                      std::vector<std::string>& compileUnits)
{
   // PEExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;      // Suppress unused parameter warning
   (void)compileUnits;  // Suppress unused parameter warning
   return false;
}

bool PEExtractor::extractSourceFiles(const std::string&        filePath,
                                     std::vector<std::string>& sourceFiles)
{
   // PEExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;     // Suppress unused parameter warning
   (void)sourceFiles;  // Suppress unused parameter warning
   return false;
}

bool PEExtractor::canHandle(const std::string& filePath) const
{
   return pImpl->canHandleImpl(filePath);
}

std::string PEExtractor::getFormatName() const
{
   return pImpl->getFormatNameImpl();
}

int PEExtractor::getPriority() const
{
   return pImpl->getPriorityImpl();
}

// PE specific methods
bool PEExtractor::extractBuildId(const std::string& filePath, std::string& buildId)
{
   return pImpl->extractBuildIdImpl(filePath, buildId);
}

std::string PEExtractor::getArchitecture(const std::string& filePath)
{
   return pImpl->getArchitectureImpl(filePath);
}

bool PEExtractor::is64Bit(const std::string& filePath)
{
   return pImpl->is64BitImpl(filePath);
}

std::string PEExtractor::getFileType(const std::string& filePath)
{
   return pImpl->getFileTypeImpl(filePath);
}

bool PEExtractor::extractVersionInfo(const std::string& filePath, std::string& versionInfo)
{
   return pImpl->extractVersionInfoImpl(filePath, versionInfo);
}

std::vector<std::string> PEExtractor::extractImports(const std::string& filePath)
{
   return pImpl->extractImportsImpl(filePath);
}

std::vector<std::string> PEExtractor::extractExports(const std::string& filePath)
{
   return pImpl->extractExportsImpl(filePath);
}

// Private helper methods
bool PEExtractor::validatePEHeader(const std::string& filePath) const
{
   return pImpl->validatePEHeaderImpl(filePath);
}

bool PEExtractor::processCOFFSymbolTable(const std::string&       filePath,
                                         std::vector<SymbolInfo>& symbols)
{
   return pImpl->processCOFFSymbolTableImpl(filePath, symbols);
}

bool PEExtractor::processPESections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
   return pImpl->processPESectionsImpl(filePath, sections);
}

std::vector<std::string> PEExtractor::extractDependenciesFromIAT(const std::string& filePath)
{
   return pImpl->extractDependenciesFromIATImpl(filePath);
}

bool PEExtractor::extractVersionInfoFromResources(const std::string& filePath,
                                                  std::string&       versionInfo)
{
   return pImpl->extractVersionInfoFromResourcesImpl(filePath, versionInfo);
}

// PIMPL implementation methods
bool PEExtractor::Impl::extractSymbolsImpl(const std::string&       filePath,
                                           std::vector<SymbolInfo>& symbols)
{
   symbols.clear();

   if (!validatePEHeaderImpl(filePath))
   {
      return false;
   }

   return processCOFFSymbolTableImpl(filePath, symbols);
}

bool PEExtractor::Impl::extractSectionsImpl(const std::string&        filePath,
                                            std::vector<SectionInfo>& sections)
{
   sections.clear();

   if (!validatePEHeaderImpl(filePath))
   {
      return false;
   }

   return processPESectionsImpl(filePath, sections);
}

bool PEExtractor::Impl::extractVersionImpl(const std::string& filePath, std::string& version)
{
   version.clear();

   if (!validatePEHeaderImpl(filePath))
   {
      return false;
   }

   // Try to extract version from resources first
   if (extractVersionInfoFromResourcesImpl(filePath, version))
   {
      return true;
   }

   // Fallback to basic version extraction
   version = "Unknown";
   return true;
}

std::vector<std::string> PEExtractor::Impl::extractDependenciesImpl(const std::string& filePath)
{
   if (!validatePEHeaderImpl(filePath))
   {
      return {};
   }

   return extractDependenciesFromIATImpl(filePath);
}

bool PEExtractor::Impl::canHandleImpl(const std::string& filePath) const
{
   return validatePEHeaderImpl(filePath);
}

std::string PEExtractor::Impl::getFormatNameImpl() const
{
   return "PE";
}

int PEExtractor::Impl::getPriorityImpl() const
{
   return 70;  // Medium-high priority for PE files
}

bool PEExtractor::Impl::extractBuildIdImpl(const std::string& filePath, std::string& buildId)
{
   buildId.clear();

   if (!validatePEHeaderImpl(filePath))
   {
      return false;
   }

   // Implementation would extract build ID from PE header or resources
   // For now, return a placeholder
   buildId = "Unknown";
   return true;
}

std::string PEExtractor::Impl::getArchitectureImpl(const std::string& filePath)
{
   uint16_t machine;
   if (!readPEFileHeader(filePath, machine))
   {
      return "Unknown";
   }

   return getArchitectureString(machine);
}

bool PEExtractor::Impl::is64BitImpl(const std::string& filePath)
{
   uint16_t machine;
   if (!readPEFileHeader(filePath, machine))
   {
      return false;
   }

#ifdef _WIN32
   // Check if it's a 64-bit PE file
   return (machine == IMAGE_FILE_MACHINE_AMD64 || machine == IMAGE_FILE_MACHINE_ARM64);
#else
   (void)filePath; // Suppress unused parameter warning
   return false;
#endif
}

std::string PEExtractor::Impl::getFileTypeImpl(const std::string& filePath)
{
   uint16_t machine;
   if (!readPEFileHeader(filePath, machine))
   {
      return "Unknown";
   }

   return getFileTypeString(machine);
}

bool PEExtractor::Impl::extractVersionInfoImpl(const std::string& filePath,
                                               std::string&       versionInfo)
{
   return extractVersionInfoFromResourcesImpl(filePath, versionInfo);
}

std::vector<std::string> PEExtractor::Impl::extractImportsImpl(const std::string& filePath)
{
   // Implementation would parse Import Address Table
   // For now, return empty vector
   return {};
}

std::vector<std::string> PEExtractor::Impl::extractExportsImpl(const std::string& filePath)
{
   // Implementation would parse Export Address Table
   // For now, return empty vector
   return {};
}

// Private helper methods
bool PEExtractor::Impl::validatePEHeaderImpl(const std::string& filePath) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   IMAGE_DOS_HEADER dosHeader;
   file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));

   if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
   {
      return false;
   }

   file.seekg(dosHeader.e_lfanew);

   uint32_t peSignature;
   file.read(reinterpret_cast<char*>(&peSignature), sizeof(peSignature));

   return (peSignature == IMAGE_NT_SIGNATURE);
}

bool PEExtractor::Impl::processCOFFSymbolTableImpl(const std::string&       filePath,
                                                   std::vector<SymbolInfo>& symbols)
{
   // Implementation would parse COFF symbol table
   // For now, return empty vector
   symbols.clear();
   return true;
}

bool PEExtractor::Impl::processPESectionsImpl(const std::string&        filePath,
                                              std::vector<SectionInfo>& sections)
{
   // Implementation would parse PE sections
   // For now, return empty vector
   sections.clear();
   return true;
}

std::vector<std::string> PEExtractor::Impl::extractDependenciesFromIATImpl(
   const std::string& filePath)
{
   // Implementation would parse Import Address Table
   // For now, return empty vector
   return {};
}

bool PEExtractor::Impl::extractVersionInfoFromResourcesImpl(const std::string& filePath,
                                                            std::string&       versionInfo)
{
   // Implementation would parse PE resources and extract VS_VERSIONINFO
   // For now, return placeholder
   versionInfo = "Unknown";
   return true;
}

template <typename T>
bool PEExtractor::Impl::readPEHeader(const std::string& filePath, T& header) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   file.read(reinterpret_cast<char*>(&header), sizeof(header));
   return file.good();
}

bool PEExtractor::Impl::readPEFileHeader(const std::string& filePath, uint16_t& machine) const
{
   if (!validatePEHeaderImpl(filePath))
   {
      return false;
   }

#ifdef _WIN32
   IMAGE_DOS_HEADER dosHeader;
   if (!readPEHeader(filePath, dosHeader))
   {
      return false;
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   file.seekg(dosHeader.e_lfanew);

   uint32_t peSignature;
   file.read(reinterpret_cast<char*>(&peSignature), sizeof(peSignature));

   if (peSignature != IMAGE_NT_SIGNATURE)
   {
      return false;
   }

   IMAGE_FILE_HEADER fileHeader;
   file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
   
   machine = fileHeader.Machine;
   return file.good();
#else
   (void)filePath; // Suppress unused parameter warning
   (void)machine;  // Suppress unused parameter warning
   return false;
#endif
}

std::string PEExtractor::Impl::getArchitectureString(uint16_t machine) const
{
#ifdef _WIN32
   switch (machine)
   {
      case IMAGE_FILE_MACHINE_I386:
         return "x86";
      case IMAGE_FILE_MACHINE_AMD64:
         return "x64";
      case IMAGE_FILE_MACHINE_ARM:
         return "ARM";
      case IMAGE_FILE_MACHINE_ARM64:
         return "ARM64";
      case IMAGE_FILE_MACHINE_IA64:
         return "IA64";
      case IMAGE_FILE_MACHINE_POWERPC:
         return "PowerPC";
      case IMAGE_FILE_MACHINE_POWERPCFP:
         return "PowerPC FP";
      default:
         return "Unknown";
   }
#else
   (void)machine; // Suppress unused parameter warning
   return "Unknown";
#endif
}

std::string PEExtractor::Impl::getFileTypeString(uint16_t machine) const
{
#ifdef _WIN32
   switch (machine)
   {
      case IMAGE_FILE_MACHINE_I386:
         return "IMAGE_FILE_MACHINE_I386";
      case IMAGE_FILE_MACHINE_AMD64:
         return "IMAGE_FILE_MACHINE_AMD64";
      case IMAGE_FILE_MACHINE_ARM:
         return "IMAGE_FILE_MACHINE_ARM";
      case IMAGE_FILE_MACHINE_ARM64:
         return "IMAGE_FILE_MACHINE_ARM64";
      case IMAGE_FILE_MACHINE_IA64:
         return "IMAGE_FILE_MACHINE_IA64";
      case IMAGE_FILE_MACHINE_POWERPC:
         return "IMAGE_FILE_MACHINE_POWERPC";
      case IMAGE_FILE_MACHINE_POWERPCFP:
         return "IMAGE_FILE_MACHINE_POWERPCFP";
      default:
         return "Unknown";
   }
#else
   (void)machine; // Suppress unused parameter warning
   return "Unknown";
#endif
}

}  // namespace heimdall
