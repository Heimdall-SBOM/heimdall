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
 * @file PEExtractor.hpp
 * @brief PE (Portable Executable) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 *
 * This file provides the PEExtractor class which implements the IBinaryExtractor
 * interface for extracting metadata from PE binary files. It supports both
 * 32-bit and 64-bit PE files, as well as various PE variants.
 *
 * Features:
 * - Symbol table extraction (COFF symbols)
 * - Section information extraction
 * - Version information extraction (VS_VERSIONINFO)
 * - Build ID extraction
 * - Dynamic dependency extraction (Import Address Table)
 * - Architecture detection
 * - File format validation
 * - Resource extraction
 *
 * Dependencies:
 * - winnt.h (Windows only)
 * - IBinaryExtractor interface
 * - BinaryReader utility
 * - FileUtils utility
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"
#include "../utils/BinaryReader.hpp"
#include "../utils/FileUtils.hpp"

#ifdef _WIN32
#include <winnt.h>
#else
// Define PE constants for non-Windows platforms
#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x00004550
#define IMAGE_FILE_MACHINE_I386 0x014C
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_ARM 0x01C0
#define IMAGE_FILE_MACHINE_ARM64 0xAA64
#define IMAGE_FILE_MACHINE_IA64 0x0200
#define IMAGE_FILE_MACHINE_POWERPC 0x01F0
#define IMAGE_FILE_MACHINE_POWERPCFP 0x01F1

// Basic PE structures for non-Windows platforms
struct IMAGE_DOS_HEADER
{
   uint16_t e_magic;
   uint16_t e_cblp;
   uint16_t e_cp;
   uint16_t e_crlc;
   uint16_t e_cparhdr;
   uint16_t e_minalloc;
   uint16_t e_maxalloc;
   uint16_t e_ss;
   uint16_t e_sp;
   uint16_t e_csum;
   uint16_t e_ip;
   uint16_t e_cs;
   uint16_t e_lfarlc;
   uint16_t e_ovno;
   uint16_t e_res[4];
   uint16_t e_oemid;
   uint16_t e_oeminfo;
   uint16_t e_res2[10];
   uint32_t e_lfanew;
};

struct IMAGE_FILE_HEADER
{
   uint32_t Signature;
   uint16_t Machine;
   uint16_t NumberOfSections;
   uint32_t TimeDateStamp;
   uint32_t PointerToSymbolTable;
   uint32_t NumberOfSymbols;
   uint16_t SizeOfOptionalHeader;
   uint16_t Characteristics;
};
#endif

namespace heimdall
{

/**
 * @brief PE binary format extractor implementation
 *
 * This class provides comprehensive PE binary analysis capabilities,
 * implementing the IBinaryExtractor interface. It supports extraction
 * of symbols, sections, version information, and dependencies from
 * PE files on Windows systems.
 *
 * The extractor handles both 32-bit and 64-bit PE files, providing
 * detailed analysis of PE file structures and metadata.
 */
class PEExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   PEExtractor();

   /**
    * @brief Destructor
    */
   ~PEExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The PEExtractor to copy from
    */
   PEExtractor(const PEExtractor& other);

   /**
    * @brief Move constructor
    * @param other The PEExtractor to move from
    */
   PEExtractor(PEExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The PEExtractor to copy from
    * @return Reference to this PEExtractor
    */
   PEExtractor& operator=(const PEExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The PEExtractor to move from
    * @return Reference to this PEExtractor
    */
   PEExtractor& operator=(PEExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from PE file
    * @param filePath Path to the PE file
    * @param symbols Vector to populate with extracted symbols
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from PE file
    * @param filePath Path to the PE file
    * @param sections Vector to populate with extracted sections
    * @return true if extraction was successful, false otherwise
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from PE file
    * @param filePath Path to the PE file
    * @param version String to populate with version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from PE file
    *
    * @param filePath Path to the PE file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the PE file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the PE file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the PE file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if this extractor can handle the given file
    * @param filePath Path to the file to check
    * @return true if the file is a PE file, false otherwise
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

   // PE specific methods
   /**
    * @brief Extract build ID from PE file
    * @param filePath Path to the PE file
    * @param buildId String to populate with build ID
    * @return true if extraction was successful, false otherwise
    */
   bool extractBuildId(const std::string& filePath, std::string& buildId);

   /**
    * @brief Get the architecture of the PE file
    * @param filePath Path to the PE file
    * @return Architecture string (e.g., "x86", "x64", "ARM", "ARM64")
    */
   std::string getArchitecture(const std::string& filePath);

   /**
    * @brief Check if the PE file is 64-bit
    * @param filePath Path to the PE file
    * @return true if 64-bit, false if 32-bit
    */
   bool is64Bit(const std::string& filePath);

   /**
    * @brief Get the file type of the PE file
    * @param filePath Path to the PE file
    * @return File type string (e.g., "IMAGE_FILE_MACHINE_I386", "IMAGE_FILE_MACHINE_AMD64")
    */
   std::string getFileType(const std::string& filePath);

   /**
    * @brief Extract version information from PE resources
    * @param filePath Path to the PE file
    * @param versionInfo String to populate with detailed version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersionInfo(const std::string& filePath, std::string& versionInfo);

   /**
    * @brief Extract imports from PE file
    * @param filePath Path to the PE file
    * @return Vector of imported function names
    */
   std::vector<std::string> extractImports(const std::string& filePath);

   /**
    * @brief Extract exports from PE file
    * @param filePath Path to the PE file
    * @return Vector of exported function names
    */
   std::vector<std::string> extractExports(const std::string& filePath);

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Validate PE header
    * @param filePath Path to the PE file
    * @return true if valid PE header, false otherwise
    */
   bool validatePEHeader(const std::string& filePath) const;

   /**
    * @brief Process COFF symbol table from PE file
    * @param filePath Path to the PE file
    * @param symbols Vector to populate with symbols
    * @return true if processing was successful, false otherwise
    */
   bool processCOFFSymbolTable(const std::string& filePath, std::vector<SymbolInfo>& symbols);

   /**
    * @brief Process PE sections
    * @param filePath Path to the PE file
    * @param sections Vector to populate with sections
    * @return true if processing was successful, false otherwise
    */
   bool processPESections(const std::string& filePath, std::vector<SectionInfo>& sections);

   /**
    * @brief Extract dependencies from Import Address Table
    * @param filePath Path to the PE file
    * @return Vector of dependency library names
    */
   std::vector<std::string> extractDependenciesFromIAT(const std::string& filePath);

   /**
    * @brief Extract version information from PE resources
    * @param filePath Path to the PE file
    * @param versionInfo String to populate with version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersionInfoFromResources(const std::string& filePath, std::string& versionInfo);
};

}  // namespace heimdall