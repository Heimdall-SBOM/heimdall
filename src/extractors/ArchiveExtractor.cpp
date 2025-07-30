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
 * @file ArchiveExtractor.cpp
 * @brief Archive (static library) extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 */

#include "ArchiveExtractor.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace heimdall
{

// Archive header structure
struct ArchiveHeader
{
   char magic[8];  // "!<arch>\n"
   char name[16];  // Member name
   char date[12];  // Modification time
   char uid[6];    // User ID
   char gid[6];    // Group ID
   char mode[8];   // File mode
   char size[10];  // File size
   char end[2];    // End marker
};

// PIMPL implementation
class ArchiveExtractor::Impl
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
   bool        extractMembersImpl(const std::string& filePath, std::vector<ArchiveMember>& members);
   std::string getArchiveFormatImpl(const std::string& filePath);
   bool        isThinArchiveImpl(const std::string& filePath);
   bool        extractSymbolTableImpl(const std::string&                  filePath,
                                      std::map<std::string, std::string>& symbolTable);
   size_t      getMemberCountImpl(const std::string& filePath);
   bool        extractMemberImpl(const std::string& filePath, const std::string& memberName,
                                 const std::string& outputPath);

   public:
   // Helper methods for archive parsing
   bool validateArchiveHeaderImpl(const std::string& filePath) const;
   bool processArchiveMembersImpl(const std::string& filePath, std::vector<ArchiveMember>& members);
   bool processArchiveSymbolTableImpl(const std::string&       filePath,
                                      std::vector<SymbolInfo>& symbols);
   bool parseArchiveHeaderImpl(const std::string& filePath, std::string& format);

   bool readArchiveHeader(const std::string& filePath, ArchiveHeader& header,
                          std::streampos offset) const;
   std::string trimString(const std::string& str) const;
   uint64_t    parseOctal(const std::string& str) const;
   std::string parseString(const std::string& str) const;
   bool        isSymbolTable(const std::string& memberName) const;
   bool        isLongNameTable(const std::string& memberName) const;
   bool        isThinArchiveMagic(const std::string& filePath) const;
};

// Public interface implementation
ArchiveExtractor::ArchiveExtractor() : pImpl(std::make_unique<Impl>()) {}

ArchiveExtractor::~ArchiveExtractor() = default;

ArchiveExtractor::ArchiveExtractor(const ArchiveExtractor& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

ArchiveExtractor::ArchiveExtractor(ArchiveExtractor&& other) noexcept
   : pImpl(std::move(other.pImpl))
{
}

ArchiveExtractor& ArchiveExtractor::operator=(const ArchiveExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

ArchiveExtractor& ArchiveExtractor::operator=(ArchiveExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool ArchiveExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   return pImpl->extractSymbolsImpl(filePath, symbols);
}

bool ArchiveExtractor::extractSections(const std::string&        filePath,
                                       std::vector<SectionInfo>& sections)
{
   return pImpl->extractSectionsImpl(filePath, sections);
}

bool ArchiveExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   return pImpl->extractVersionImpl(filePath, version);
}

std::vector<std::string> ArchiveExtractor::extractDependencies(const std::string& filePath)
{
   return pImpl->extractDependenciesImpl(filePath);
}

bool ArchiveExtractor::extractFunctions(const std::string&        filePath,
                                        std::vector<std::string>& functions)
{
   // ArchiveExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;   // Suppress unused parameter warning
   (void)functions;  // Suppress unused parameter warning
   return false;
}

bool ArchiveExtractor::extractCompileUnits(const std::string&        filePath,
                                           std::vector<std::string>& compileUnits)
{
   // ArchiveExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;      // Suppress unused parameter warning
   (void)compileUnits;  // Suppress unused parameter warning
   return false;
}

bool ArchiveExtractor::extractSourceFiles(const std::string&        filePath,
                                          std::vector<std::string>& sourceFiles)
{
   // ArchiveExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;     // Suppress unused parameter warning
   (void)sourceFiles;  // Suppress unused parameter warning
   return false;
}

bool ArchiveExtractor::canHandle(const std::string& filePath) const
{
   return pImpl->canHandleImpl(filePath);
}

std::string ArchiveExtractor::getFormatName() const
{
   return pImpl->getFormatNameImpl();
}

int ArchiveExtractor::getPriority() const
{
   return pImpl->getPriorityImpl();
}

// Archive specific methods
bool ArchiveExtractor::extractMembers(const std::string&          filePath,
                                      std::vector<ArchiveMember>& members)
{
   return pImpl->extractMembersImpl(filePath, members);
}

std::string ArchiveExtractor::getArchiveFormat(const std::string& filePath)
{
   return pImpl->getArchiveFormatImpl(filePath);
}

bool ArchiveExtractor::isThinArchive(const std::string& filePath)
{
   return pImpl->isThinArchiveImpl(filePath);
}

bool ArchiveExtractor::extractSymbolTable(const std::string&                  filePath,
                                          std::map<std::string, std::string>& symbolTable)
{
   return pImpl->extractSymbolTableImpl(filePath, symbolTable);
}

size_t ArchiveExtractor::getMemberCount(const std::string& filePath)
{
   return pImpl->getMemberCountImpl(filePath);
}

bool ArchiveExtractor::extractMember(const std::string& filePath, const std::string& memberName,
                                     const std::string& outputPath)
{
   return pImpl->extractMemberImpl(filePath, memberName, outputPath);
}

// Private helper methods
bool ArchiveExtractor::validateArchiveHeader(const std::string& filePath) const
{
   return pImpl->validateArchiveHeaderImpl(filePath);
}

bool ArchiveExtractor::processArchiveMembers(const std::string&          filePath,
                                             std::vector<ArchiveMember>& members)
{
   return pImpl->processArchiveMembersImpl(filePath, members);
}

bool ArchiveExtractor::processArchiveSymbolTable(const std::string&       filePath,
                                                 std::vector<SymbolInfo>& symbols)
{
   return pImpl->processArchiveSymbolTableImpl(filePath, symbols);
}

bool ArchiveExtractor::parseArchiveHeader(const std::string& filePath, std::string& format)
{
   return pImpl->parseArchiveHeaderImpl(filePath, format);
}

// PIMPL implementation methods
bool ArchiveExtractor::Impl::extractSymbolsImpl(const std::string&       filePath,
                                                std::vector<SymbolInfo>& symbols)
{
   symbols.clear();

   if (!validateArchiveHeaderImpl(filePath))
   {
      return false;
   }

   return processArchiveSymbolTableImpl(filePath, symbols);
}

bool ArchiveExtractor::Impl::extractSectionsImpl(const std::string&        filePath,
                                                 std::vector<SectionInfo>& sections)
{
   sections.clear();

   if (!validateArchiveHeaderImpl(filePath))
   {
      return false;
   }

   // For archives, sections are represented by members
   std::vector<ArchiveMember> members;
   if (!processArchiveMembersImpl(filePath, members))
   {
      return false;
   }

   // Convert archive members to sections
   for (const auto& member : members)
   {
      if (!isSymbolTable(member.name) && !isLongNameTable(member.name))
      {
         SectionInfo section;
         section.name    = member.name;
         section.size    = member.size;
         section.address = member.offset;
         section.type    = "archive_member";
         sections.push_back(section);
      }
   }

   return true;
}

bool ArchiveExtractor::Impl::extractVersionImpl(const std::string& filePath, std::string& version)
{
   version.clear();

   if (!validateArchiveHeaderImpl(filePath))
   {
      return false;
   }

   // Archives don't typically have version information
   // Could extract from member names or content
   version = "Unknown";
   return true;
}

std::vector<std::string> ArchiveExtractor::Impl::extractDependenciesImpl(
   const std::string& filePath)
{
   if (!validateArchiveHeaderImpl(filePath))
   {
      return {};
   }

   // Archives are static libraries, so they don't have runtime dependencies
   // But they might have build dependencies encoded in member names
   std::vector<std::string>   dependencies;

   std::vector<ArchiveMember> members;
   if (processArchiveMembersImpl(filePath, members))
   {
      for (const auto& member : members)
      {
         // Extract potential dependencies from member names
         if (member.name.find(".so") != std::string::npos ||
             member.name.find(".dll") != std::string::npos ||
             member.name.find(".dylib") != std::string::npos)
         {
            dependencies.push_back(member.name);
         }
      }
   }

   return dependencies;
}

bool ArchiveExtractor::Impl::canHandleImpl(const std::string& filePath) const
{
   return validateArchiveHeaderImpl(filePath);
}

std::string ArchiveExtractor::Impl::getFormatNameImpl() const
{
   return "Archive";
}

int ArchiveExtractor::Impl::getPriorityImpl() const
{
   return 60;  // Medium priority for archive files
}

bool ArchiveExtractor::Impl::extractMembersImpl(const std::string&          filePath,
                                                std::vector<ArchiveMember>& members)
{
   return processArchiveMembersImpl(filePath, members);
}

std::string ArchiveExtractor::Impl::getArchiveFormatImpl(const std::string& filePath)
{
   std::string format;
   if (parseArchiveHeaderImpl(filePath, format))
   {
      return format;
   }
   return "Unknown";
}

bool ArchiveExtractor::Impl::isThinArchiveImpl(const std::string& filePath)
{
   return isThinArchiveMagic(filePath);
}

bool ArchiveExtractor::Impl::extractSymbolTableImpl(const std::string&                  filePath,
                                                    std::map<std::string, std::string>& symbolTable)
{
   symbolTable.clear();

   if (!validateArchiveHeaderImpl(filePath))
   {
      return false;
   }

   // Implementation would parse the archive symbol table
   // For now, return empty map
   return true;
}

size_t ArchiveExtractor::Impl::getMemberCountImpl(const std::string& filePath)
{
   std::vector<ArchiveMember> members;
   if (processArchiveMembersImpl(filePath, members))
   {
      return members.size();
   }
   return 0;
}

bool ArchiveExtractor::Impl::extractMemberImpl(const std::string& filePath,
                                               const std::string& memberName,
                                               const std::string& outputPath)
{
   // Implementation would extract specific member to output path
   // For now, return false
   return false;
}

// Private helper methods
bool ArchiveExtractor::Impl::validateArchiveHeaderImpl(const std::string& filePath) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check for traditional ar archive magic
   char magic[8];
   file.read(magic, sizeof(magic));

   if (std::string(magic, sizeof(magic)) == "!<arch>\n")
   {
      return true;
   }

   // Check for thin archive magic
   file.seekg(0);
   file.read(magic, sizeof(magic));

   if (std::string(magic, sizeof(magic)) == "!<thin>\n")
   {
      return true;
   }

   return false;
}

bool ArchiveExtractor::Impl::processArchiveMembersImpl(const std::string&          filePath,
                                                       std::vector<ArchiveMember>& members)
{
   members.clear();

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Skip magic number
   file.seekg(8);

   std::streampos currentPos = file.tellg();
   while (file.good())
   {
      ArchiveHeader header;
      if (!readArchiveHeader(filePath, header, currentPos))
      {
         break;
      }

      ArchiveMember member;
      member.name             = trimString(std::string(header.name, sizeof(header.name)));
      member.modificationTime = trimString(std::string(header.date, sizeof(header.date)));
      member.owner            = trimString(std::string(header.uid, sizeof(header.uid)));
      member.group            = trimString(std::string(header.gid, sizeof(header.gid)));
      member.mode             = trimString(std::string(header.mode, sizeof(header.mode)));
      member.size             = parseOctal(std::string(header.size, sizeof(header.size)));
      member.offset           = static_cast<uint64_t>(currentPos);

      // Skip special members
      if (!isSymbolTable(member.name) && !isLongNameTable(member.name))
      {
         members.push_back(member);
      }

      // Move to next member
      currentPos += 60 + member.size;  // Header size + member size
      if (member.size % 2 != 0)
      {
         currentPos += 1;  // Padding
      }
      file.seekg(currentPos);
   }

   return true;
}

bool ArchiveExtractor::Impl::processArchiveSymbolTableImpl(const std::string&       filePath,
                                                           std::vector<SymbolInfo>& symbols)
{
   symbols.clear();

   // Implementation would parse the archive symbol table
   // For now, return empty vector
   return true;
}

bool ArchiveExtractor::Impl::parseArchiveHeaderImpl(const std::string& filePath,
                                                    std::string&       format)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   char magic[8];
   file.read(magic, sizeof(magic));

   if (std::string(magic, sizeof(magic)) == "!<arch>\n")
   {
      format = "ar";
      return true;
   }
   else if (std::string(magic, sizeof(magic)) == "!<thin>\n")
   {
      format = "thin";
      return true;
   }

   format = "Unknown";
   return false;
}

bool ArchiveExtractor::Impl::readArchiveHeader(const std::string& filePath, ArchiveHeader& header,
                                               std::streampos offset) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   file.seekg(offset);
   file.read(reinterpret_cast<char*>(&header), sizeof(header));

   return file.good();
}

std::string ArchiveExtractor::Impl::trimString(const std::string& str) const
{
   size_t start = str.find_first_not_of(" \t\r\n");
   if (start == std::string::npos)
   {
      return "";
   }
   size_t end = str.find_last_not_of(" \t\r\n");
   return str.substr(start, end - start + 1);
}

uint64_t ArchiveExtractor::Impl::parseOctal(const std::string& str) const
{
   std::string trimmed = trimString(str);
   if (trimmed.empty())
   {
      return 0;
   }

   try
   {
      return std::stoull(trimmed, nullptr, 8);
   }
   catch (...)
   {
      return 0;
   }
}

std::string ArchiveExtractor::Impl::parseString(const std::string& str) const
{
   return trimString(str);
}

bool ArchiveExtractor::Impl::isSymbolTable(const std::string& memberName) const
{
   return (memberName == "/" || memberName == "__.SYMDEF" || memberName == "__.SYMDEF SORTED");
}

bool ArchiveExtractor::Impl::isLongNameTable(const std::string& memberName) const
{
   return (memberName == "//" || memberName == "__.SYMDEF_64" ||
           memberName == "__.SYMDEF_64 SORTED");
}

bool ArchiveExtractor::Impl::isThinArchiveMagic(const std::string& filePath) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   char magic[8];
   file.read(magic, sizeof(magic));

   return (std::string(magic, sizeof(magic)) == "!<thin>\n");
}

}  // namespace heimdall