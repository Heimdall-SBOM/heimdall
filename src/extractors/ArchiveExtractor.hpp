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
 * @file ArchiveExtractor.hpp
 * @brief Archive (static library) extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 *
 * This file provides the ArchiveExtractor class which implements the IBinaryExtractor
 * interface for extracting metadata from static library archive files. It supports
 * various archive formats including ar, thin archives, and other variants.
 *
 * Features:
 * - Archive member extraction
 * - Symbol table extraction (from archive index)
 * - Member file information extraction
 * - Archive format detection
 * - Cross-reference table parsing
 * - Long filename support
 * - Thin archive support
 *
 * Dependencies:
 * - ar.h (system header)
 * - IBinaryExtractor interface
 * - BinaryReader utility
 * - FileUtils utility
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"
#include "../utils/BinaryReader.hpp"
#include "../utils/FileUtils.hpp"

namespace heimdall
{

/**
 * @brief Archive member information structure
 */
struct ArchiveMember
{
   std::string              name;              ///< Member name
   std::string              longName;          ///< Long name (if different from name)
   uint64_t                 offset;            ///< File offset
   uint64_t                 size;              ///< Member size
   std::string              modificationTime;  ///< Modification time
   std::string              owner;             ///< Owner information
   std::string              group;             ///< Group information
   std::string              mode;              ///< File mode
   std::vector<std::string> symbols;           ///< Symbols defined by this member
};

/**
 * @brief Archive binary format extractor implementation
 *
 * This class provides comprehensive archive analysis capabilities,
 * implementing the IBinaryExtractor interface. It supports extraction
 * of symbols, member information, and dependencies from static library
 * archive files.
 *
 * The extractor handles various archive formats including traditional
 * ar archives, thin archives, and other variants.
 */
class ArchiveExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   ArchiveExtractor();

   /**
    * @brief Destructor
    */
   ~ArchiveExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The ArchiveExtractor to copy from
    */
   ArchiveExtractor(const ArchiveExtractor& other);

   /**
    * @brief Move constructor
    * @param other The ArchiveExtractor to move from
    */
   ArchiveExtractor(ArchiveExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The ArchiveExtractor to copy from
    * @return Reference to this ArchiveExtractor
    */
   ArchiveExtractor& operator=(const ArchiveExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The ArchiveExtractor to move from
    * @return Reference to this ArchiveExtractor
    */
   ArchiveExtractor& operator=(ArchiveExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from archive file
    * @param filePath Path to the archive file
    * @param symbols Vector to populate with extracted symbols
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from archive file
    * @param filePath Path to the archive file
    * @param sections Vector to populate with extracted sections
    * @return true if extraction was successful, false otherwise
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from archive file
    * @param filePath Path to the archive file
    * @param version String to populate with version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from archive file
    *
    * @param filePath Path to the archive file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the archive file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the archive file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the archive file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if this extractor can handle the given file
    * @param filePath Path to the file to check
    * @return true if the file is an archive file, false otherwise
    */
   bool canHandle(const std::string& filePath) const override;

   /**
    * @brief Get the format name for this extractor
    * @return String representation of the format name
    */
   std::string getFormatName() const override;

   /**
    * @brief Get the priority of this extractor
    * @return Priority value (higher values indicate higher priority)
    */
   int getPriority() const override;

   // Archive specific methods
   /**
    * @brief Extract archive members
    * @param filePath Path to the archive file
    * @param members Vector to populate with archive members
    * @return true if extraction was successful, false otherwise
    */
   bool extractMembers(const std::string& filePath, std::vector<ArchiveMember>& members);

   /**
    * @brief Get archive format type
    * @param filePath Path to the archive file
    * @return Archive format string (e.g., "ar", "thin", "bsd")
    */
   std::string getArchiveFormat(const std::string& filePath);

   /**
    * @brief Check if archive is thin (symbolic links)
    * @param filePath Path to the archive file
    * @return true if thin archive, false otherwise
    */
   bool isThinArchive(const std::string& filePath);

   /**
    * @brief Extract symbol table from archive index
    * @param filePath Path to the archive file
    * @param symbolTable Map of symbol names to member names
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbolTable(const std::string&                  filePath,
                           std::map<std::string, std::string>& symbolTable);

   /**
    * @brief Get archive member count
    * @param filePath Path to the archive file
    * @return Number of members in the archive
    */
   size_t getMemberCount(const std::string& filePath);

   /**
    * @brief Extract specific member from archive
    * @param filePath Path to the archive file
    * @param memberName Name of the member to extract
    * @param outputPath Path where to save the extracted member
    * @return true if extraction was successful, false otherwise
    */
   bool extractMember(const std::string& filePath, const std::string& memberName,
                      const std::string& outputPath);

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Validate archive header
    * @param filePath Path to the archive file
    * @return true if valid archive header, false otherwise
    */
   bool validateArchiveHeader(const std::string& filePath) const;

   /**
    * @brief Process archive members
    * @param filePath Path to the archive file
    * @param members Vector to populate with members
    * @return true if processing was successful, false otherwise
    */
   bool processArchiveMembers(const std::string& filePath, std::vector<ArchiveMember>& members);

   /**
    * @brief Process archive symbol table
    * @param filePath Path to the archive file
    * @param symbols Vector to populate with symbols
    * @return true if processing was successful, false otherwise
    */
   bool processArchiveSymbolTable(const std::string& filePath, std::vector<SymbolInfo>& symbols);

   /**
    * @brief Parse archive header
    * @param filePath Path to the archive file
    * @param format String to populate with format information
    * @return true if parsing was successful, false otherwise
    */
   bool parseArchiveHeader(const std::string& filePath, std::string& format);
};

}  // namespace heimdall