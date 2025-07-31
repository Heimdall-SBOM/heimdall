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
 * @file LightweightDWARFParser.hpp
 * @brief Lightweight DWARF parser for C++11/14 compatibility
 * @author Trevor Bakker
 * @date 2025
 * @version 2.0.0
 *
 * This file provides the LightweightDWARFParser class which implements the IBinaryExtractor
 * interface for extracting DWARF debug information from ELF files without using LLVM's
 * problematic template features.
 *
 * Features:
 * - Extract source files from DWARF debug information
 * - Extract compile units from DWARF debug information
 * - Extract function names from DWARF debug information
 * - Fallback to symbol table extraction when DWARF is not available
 * - Thread-safe implementation
 * - C++11 compatible (no LLVM dependency for C++11 builds)
 * - Integration with binary format factory
 *
 * Dependencies:
 * - IBinaryExtractor interface
 * - ComponentInfo structures
 * - ELF headers (Linux only)
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"

#ifdef __linux__
#include <elf.h>
#endif

namespace heimdall
{

/**
 * @brief Lightweight DWARF parser for C++11/14 compatibility
 *
 * This class provides DWARF parsing capabilities without using LLVM's
 * problematic template features. It implements a subset of DWARF parsing
 * that covers the most common use cases for SBOM generation.
 *
 * The parser implements the IBinaryExtractor interface and specializes in
 * extracting debug information from ELF files.
 *
 * Features:
 * - Extract source files from DWARF debug information
 * - Extract compile units from DWARF debug information
 * - Extract function names from DWARF debug information
 * - Fallback to symbol table extraction when DWARF is not available
 * - Thread-safe implementation
 * - C++11 compatible (no LLVM dependency for C++11 builds)
 */
class LightweightDWARFParser : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   LightweightDWARFParser();

   /**
    * @brief Destructor
    */
   ~LightweightDWARFParser() override;

   /**
    * @brief Copy constructor
    * @param other The LightweightDWARFParser to copy from
    */
   LightweightDWARFParser(const LightweightDWARFParser& other);

   /**
    * @brief Move constructor
    * @param other The LightweightDWARFParser to move from
    */
   LightweightDWARFParser(LightweightDWARFParser&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The LightweightDWARFParser to copy from
    * @return Reference to this LightweightDWARFParser
    */
   LightweightDWARFParser& operator=(const LightweightDWARFParser& other);

   /**
    * @brief Move assignment operator
    * @param other The LightweightDWARFParser to move from
    * @return Reference to this LightweightDWARFParser
    */
   LightweightDWARFParser& operator=(LightweightDWARFParser&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from ELF file (functions from DWARF)
    *
    * @param filePath Path to the ELF file
    * @param symbols Output vector to store extracted symbols
    * @return true if symbols were successfully extracted
    * @return false if extraction failed or no symbols found
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from ELF file
    *
    * @param filePath Path to the ELF file
    * @param sections Output vector to store extracted sections
    * @return true if sections were successfully extracted
    * @return false if extraction failed or no sections found
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from ELF file
    *
    * @param filePath Path to the ELF file
    * @param version Output string to store version information
    * @return true if version was successfully extracted
    * @return false if extraction failed or no version found
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from ELF file
    *
    * @param filePath Path to the ELF file
    * @return Vector of dependency strings (library names, etc.)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Check if the extractor can handle the given file format
    *
    * @param filePath Path to the ELF file
    * @return true if this extractor can process the file
    * @return false if the file format is not supported
    */
   bool canHandle(const std::string& filePath) const override;

   /**
    * @brief Get the name of the binary format this extractor handles
    *
    * @return String identifier for the binary format
    */
   std::string getFormatName() const override;

   /**
    * @brief Get the priority of this extractor (lower numbers = higher priority)
    *
    * @return Priority value (0 = highest priority)
    */
   int getPriority() const override;

   // Legacy interface methods for backward compatibility
   /**
    * @brief Extract source files from DWARF debug information
    *
    * @param filePath Path to the ELF file containing DWARF info
    * @param sourceFiles Output vector to store extracted source file paths
    * @return true if extraction was successful, false otherwise
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Extract compile units from DWARF debug information
    *
    * @param filePath Path to the ELF file containing DWARF info
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if extraction was successful, false otherwise
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the ELF file containing DWARF info
    * @param functions Output vector to store extracted function names
    * @return true if extraction was successful, false otherwise
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract all debug information using a single pass
    *
    * This method extracts all debug information (source files, compile units, functions)
    * in a single pass through the DWARF data, avoiding the overhead of multiple
    * context creations.
    *
    * @param filePath Path to the ELF file containing DWARF info
    * @param sourceFiles Output vector to store extracted source file paths
    * @param compileUnits Output vector to store extracted compile unit names
    * @param functions Output vector to store extracted function names
    * @return true if extraction was successful, false otherwise
    */
   bool extractAllDebugInfo(const std::string& filePath, std::vector<std::string>& sourceFiles,
                            std::vector<std::string>& compileUnits,
                            std::vector<std::string>& functions);

   /**
    * @brief Check if DWARF information is available in the file
    *
    * @param filePath Path to the ELF file to check
    * @return true if DWARF info is present, false otherwise
    */
   bool hasDWARFInfo(const std::string& filePath) const;

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   /**
    * @brief Parse DWARF debug info section
    *
    * @param filePath Path to the ELF file
    * @param sourceFiles Output vector for source files
    * @param compileUnits Output vector for compile units
    * @param functions Output vector for functions
    * @return true if parsing was successful, false otherwise
    */
   bool parseDWARFDebugInfo(const std::string& filePath, std::vector<std::string>& sourceFiles,
                            std::vector<std::string>& compileUnits,
                            std::vector<std::string>& functions);

   /**
    * @brief Parse DWARF debug line section
    *
    * @param filePath Path to the ELF file
    * @param sourceFiles Output vector for source files
    * @return true if parsing was successful, false otherwise
    */
   bool parseDWARFDebugLine(const std::string& filePath, std::vector<std::string>& sourceFiles);

   /**
    * @brief Parse DWARF debug info section
    *
    * @param filePath Path to the ELF file
    * @param compileUnits Output vector for compile units
    * @param functions Output vector for functions
    * @return true if parsing was successful, false otherwise
    */
   bool parseDWARFDebugInfo(const std::string& filePath, std::vector<std::string>& compileUnits,
                            std::vector<std::string>& functions);

   /**
    * @brief Fallback to symbol table extraction
    *
    * @param filePath Path to the ELF file
    * @param functions Output vector for functions
    * @return true if extraction was successful, false otherwise
    */
   bool extractFunctionsFromSymbolTable(const std::string&        filePath,
                                        std::vector<std::string>& functions);

   /**
    * @brief Fallback heuristic source file extraction
    *
    * @param filePath Path to the ELF file
    * @param sourceFiles Output vector for source files
    * @return true if extraction was successful, false otherwise
    */
   bool extractSourceFilesHeuristic(const std::string&        filePath,
                                    std::vector<std::string>& sourceFiles);

   /**
    * @brief Find DWARF sections in ELF file
    *
    * @param filePath Path to the ELF file
    * @param debugInfoOffset Output offset of .debug_info section
    * @param debugLineOffset Output offset of .debug_line section
    * @param debugAbbrevOffset Output offset of .debug_abbrev section
    * @return true if sections were found, false otherwise
    */
   bool findDWARFSections(const std::string& filePath, uint64_t& debugInfoOffset,
                          uint64_t& debugLineOffset, uint64_t& debugAbbrevOffset) const;

   /**
    * @brief Read DWARF string from string table
    *
    * @param data Pointer to string table data
    * @param offset Offset in string table
    * @return String at the given offset
    */
   std::string readDWARFString(const char* data, uint32_t offset);

   /**
    * @brief Parse DWARF LEB128 encoded value
    *
    * @param data Pointer to data
    * @param offset Offset to read from (updated after reading)
    * @return Parsed LEB128 value
    */
   uint64_t parseLEB128(const char* data, uint32_t& offset);

   /**
    * @brief Parse DWARF ULEB128 encoded value
    *
    * @param data Pointer to data
    * @param offset Offset to read from (updated after reading)
    * @return Parsed ULEB128 value
    */
   uint64_t parseULEB128(const char* data, uint32_t& offset);

   // ELF file parsing helpers
   struct ELFHeader
   {
      uint8_t  e_ident[16];
      uint16_t e_type;
      uint16_t e_machine;
      uint32_t e_version;
      uint64_t e_entry;
      uint64_t e_phoff;
      uint64_t e_shoff;
      uint32_t e_flags;
      uint16_t e_ehsize;
      uint16_t e_phentsize;
      uint16_t e_phnum;
      uint16_t e_shentsize;
      uint16_t e_shnum;
      uint16_t e_shstrndx;
   };

   struct ELFSectionHeader
   {
      uint32_t sh_name;
      uint32_t sh_type;
      uint64_t sh_flags;
      uint64_t sh_addr;
      uint64_t sh_offset;
      uint64_t sh_size;
      uint32_t sh_link;
      uint32_t sh_info;
      uint64_t sh_addralign;
      uint64_t sh_entsize;
   };
};

}  // namespace heimdall