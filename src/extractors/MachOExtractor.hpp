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
 * @file MachOExtractor.hpp
 * @brief Mach-O (Mach Object) binary extractor implementation
 * @author Heimdall Development Team
 * @date 2025-07-29
 * @version 1.0.0
 *
 * This file provides the MachOExtractor class which implements the IBinaryExtractor
 * interface for extracting metadata from Mach-O binary files. It supports both
 * 32-bit and 64-bit Mach-O files, as well as universal binaries (fat files).
 *
 * Features:
 * - Symbol table extraction (LC_SYMTAB)
 * - Load command information extraction
 * - Version information extraction
 * - Build ID extraction (LC_UUID)
 * - Dynamic dependency extraction (LC_LOAD_DYLIB)
 * - Architecture detection
 * - File format validation
 * - Universal binary support
 *
 * Dependencies:
 * - mach-o/loader.h (macOS only)
 * - mach-o/nlist.h (macOS only)
 * - IBinaryExtractor interface
 * - BinaryReader utility
 * - FileUtils utility
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"
#include "../utils/BinaryReader.hpp"
#include "../utils/FileUtils.hpp"

#ifdef __APPLE__
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

namespace heimdall
{

/**
 * @brief Mach-O binary format extractor implementation
 *
 * This class provides comprehensive Mach-O binary analysis capabilities,
 * implementing the IBinaryExtractor interface. It supports extraction
 * of symbols, load commands, version information, and dependencies from
 * Mach-O files on macOS systems.
 *
 * The extractor handles both single-architecture and universal binaries,
 * providing detailed analysis of Mach-O file structures and metadata.
 */
class MachOExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   MachOExtractor();

   /**
    * @brief Destructor
    */
   ~MachOExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The MachOExtractor to copy from
    */
   MachOExtractor(const MachOExtractor& other);

   /**
    * @brief Move constructor
    * @param other The MachOExtractor to move from
    */
   MachOExtractor(MachOExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The MachOExtractor to copy from
    * @return Reference to this MachOExtractor
    */
   MachOExtractor& operator=(const MachOExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The MachOExtractor to move from
    * @return Reference to this MachOExtractor
    */
   MachOExtractor& operator=(MachOExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param symbols Vector to populate with extracted symbols
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param sections Vector to populate with extracted sections
    * @return true if extraction was successful, false otherwise
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param version String to populate with version information
    * @return true if extraction was successful, false otherwise
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from Mach-O file
    *
    * @param filePath Path to the Mach-O file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the Mach-O file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the Mach-O file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the Mach-O file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if this extractor can handle the given file
    * @param filePath Path to the file to check
    * @return true if the file is a Mach-O file, false otherwise
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

   // Mach-O specific methods
   /**
    * @brief Extract build ID (UUID) from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param buildId String to populate with build ID
    * @return true if extraction was successful, false otherwise
    */
   bool extractBuildId(const std::string& filePath, std::string& buildId);

   /**
    * @brief Get the architecture of the Mach-O file
    * @param filePath Path to the Mach-O file
    * @return Architecture string (e.g., "x86_64", "arm64")
    */
   std::string getArchitecture(const std::string& filePath);

   /**
    * @brief Check if the Mach-O file is 64-bit
    * @param filePath Path to the Mach-O file
    * @return true if 64-bit, false if 32-bit
    */
   bool is64Bit(const std::string& filePath);

   /**
    * @brief Get the file type of the Mach-O file
    * @param filePath Path to the Mach-O file
    * @return File type string (e.g., "MH_EXECUTE", "MH_DYLIB")
    */
   std::string getFileType(const std::string& filePath);

   /**
    * @brief Check if the file is a universal binary
    * @param filePath Path to the Mach-O file
    * @return true if universal binary, false otherwise
    */
   bool isUniversalBinary(const std::string& filePath);

   /**
    * @brief Get architectures in a universal binary
    * @param filePath Path to the Mach-O file
    * @return Vector of architecture strings
    */
   std::vector<std::string> getUniversalArchitectures(const std::string& filePath);

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Validate Mach-O header
    * @param filePath Path to the Mach-O file
    * @return true if valid Mach-O header, false otherwise
    */
   bool validateMachOHeader(const std::string& filePath) const;

   /**
    * @brief Process symbol table from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param symbols Vector to populate with symbols
    * @return true if processing was successful, false otherwise
    */
   bool processSymbolTable(const std::string& filePath, std::vector<SymbolInfo>& symbols);

   /**
    * @brief Process load commands from Mach-O file
    * @param filePath Path to the Mach-O file
    * @param sections Vector to populate with sections
    * @return true if processing was successful, false otherwise
    */
   bool processLoadCommands(const std::string& filePath, std::vector<SectionInfo>& sections);

   /**
    * @brief Extract dependencies from load commands
    * @param filePath Path to the Mach-O file
    * @return Vector of dependency library names
    */
   std::vector<std::string> extractDependenciesFromLoadCommands(const std::string& filePath);
};

}  // namespace heimdall