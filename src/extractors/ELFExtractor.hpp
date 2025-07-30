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
 * @file ELFExtractor.hpp
 * @brief ELF (Executable and Linkable Format) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 *
 * This file provides the ELFExtractor class which implements the IBinaryExtractor
 * interface for extracting metadata from ELF binary files. It supports both
 * 32-bit and 64-bit ELF files, as well as various ELF variants.
 *
 * Features:
 * - Symbol table extraction (.symtab and .dynsym)
 * - Section information extraction
 * - Version information extraction
 * - Build ID extraction
 * - Dynamic dependency extraction
 * - Architecture detection
 * - File format validation
 *
 * Dependencies:
 * - libelf (Linux only)
 * - elf.h (system header)
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

#ifdef __linux__
#include <elf.h>
#include <libelf.h>
#endif

namespace heimdall
{

/**
 * @brief ELF binary format extractor implementation
 *
 * This class provides comprehensive ELF binary analysis capabilities,
 * implementing the IBinaryExtractor interface. It supports extraction
 * of symbols, sections, version information, and dependencies from
 * ELF files on Linux systems.
 *
 * The extractor uses libelf for robust ELF parsing and provides
 * fallback mechanisms for basic information extraction when libelf
 * is not available.
 */
class ELFExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   ELFExtractor();

   /**
    * @brief Destructor
    */
   ~ELFExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The ELFExtractor to copy from
    */
   ELFExtractor(const ELFExtractor& other);

   /**
    * @brief Move constructor
    * @param other The ELFExtractor to move from
    */
   ELFExtractor(ELFExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The ELFExtractor to copy from
    * @return Reference to this ELFExtractor
    */
   ELFExtractor& operator=(const ELFExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The ELFExtractor to move from
    * @return Reference to this ELFExtractor
    */
   ELFExtractor& operator=(ELFExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from ELF file
    * @param filePath Path to the ELF file
    * @param symbols Vector to populate with extracted symbols
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from ELF file
    * @param filePath Path to the ELF file
    * @param sections Vector to populate with extracted sections
    * @return true if extraction was successful, false otherwise
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from ELF file
    * @param filePath Path to the ELF file
    * @param version String to populate with version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from ELF file
    *
    * @param filePath Path to the ELF file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the ELF file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the ELF file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the ELF file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if the given file is a valid ELF file
    * @param filePath Path to the file to check
    * @return true if the file is a valid ELF file, false otherwise
    */
   bool canHandle(const std::string& filePath) const override;

   /**
    * @brief Get the name of the binary format this extractor handles
    * @return String identifier for the binary format
    */
   std::string getFormatName() const override;

   /**
    * @brief Get the priority of this extractor
    * @return Priority value (0 = highest priority)
    */
   int getPriority() const override;

   // ELF-specific methods
   /**
    * @brief Extract build ID from ELF file
    * @param filePath Path to the ELF file
    * @param buildId String to populate with build ID
    * @return true if extraction was successful, false otherwise
    */
   bool extractBuildId(const std::string& filePath, std::string& buildId);

   /**
    * @brief Get ELF file architecture
    * @param filePath Path to the ELF file
    * @return Architecture string (e.g., "x86_64", "aarch64")
    */
   std::string getArchitecture(const std::string& filePath);

   /**
    * @brief Check if ELF file is 64-bit
    * @param filePath Path to the ELF file
    * @return true if 64-bit, false if 32-bit
    */
   bool is64Bit(const std::string& filePath);

   /**
    * @brief Get ELF file type
    * @param filePath Path to the ELF file
    * @return ELF file type string (e.g., "ET_EXEC", "ET_DYN", "ET_REL")
    */
   std::string getFileType(const std::string& filePath);

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Validate ELF file header
    * @param filePath Path to the ELF file
    * @return true if header is valid, false otherwise
    */
   bool validateELFHeader(const std::string& filePath) const;

#ifdef __linux__
   /**
    * @brief Process ELF symbol table using libelf
    * @param elf ELF file handle
    * @param symbols Vector to populate with symbols
    * @return true if processing was successful, false otherwise
    */
   bool processSymbolTable(Elf* elf, std::vector<SymbolInfo>& symbols);

   /**
    * @brief Process ELF sections using libelf
    * @param elf ELF file handle
    * @param sections Vector to populate with sections
    * @return true if processing was successful, false otherwise
    */
   bool processSections(Elf* elf, std::vector<SectionInfo>& sections);
#endif
};

}  // namespace heimdall