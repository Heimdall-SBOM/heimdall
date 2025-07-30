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
 * @file IBinaryExtractor.hpp
 * @brief Abstract interface for binary format extractors
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file defines the abstract interface that all binary format extractors
 * must implement. It provides a unified contract for extracting metadata
 * from different binary formats (ELF, Mach-O, PE, Archives).
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

// Forward declarations
struct SymbolInfo;
struct SectionInfo;
struct DependencyInfo;

/**
 * @brief Abstract interface for binary format extractors
 *
 * This interface defines the contract that all binary format extractors
 * must implement. It provides a unified way to extract metadata from
 * different binary formats while maintaining platform independence.
 *
 * Implementations should handle:
 * - Symbol extraction
 * - Section information
 * - Version information
 * - Dependency analysis
 * - Platform-specific metadata
 */
class IBinaryExtractor
{
   public:
   /**
    * @brief Virtual destructor for proper cleanup
    */
   virtual ~IBinaryExtractor() = default;

   /**
    * @brief Extract symbol information from binary file
    *
    * @param filePath Path to the binary file
    * @param symbols Output vector to store extracted symbols
    * @return true if symbols were successfully extracted
    * @return false if extraction failed or no symbols found
    */
   virtual bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) = 0;

   /**
    * @brief Extract section information from binary file
    *
    * @param filePath Path to the binary file
    * @param sections Output vector to store extracted sections
    * @return true if sections were successfully extracted
    * @return false if extraction failed or no sections found
    */
   virtual bool extractSections(const std::string&        filePath,
                                std::vector<SectionInfo>& sections) = 0;

   /**
    * @brief Extract version information from binary file
    *
    * @param filePath Path to the binary file
    * @param version Output string to store version information
    * @return true if version was successfully extracted
    * @return false if extraction failed or no version found
    */
   virtual bool extractVersion(const std::string& filePath, std::string& version) = 0;

   /**
    * @brief Extract dependency information from binary file
    *
    * @param filePath Path to the binary file
    * @return Vector of dependency strings (library names, etc.)
    */
   virtual std::vector<std::string> extractDependencies(const std::string& filePath) = 0;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   virtual bool extractFunctions(const std::string&        filePath,
                                 std::vector<std::string>& functions) = 0;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   virtual bool extractCompileUnits(const std::string&        filePath,
                                    std::vector<std::string>& compileUnits) = 0;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   virtual bool extractSourceFiles(const std::string&        filePath,
                                   std::vector<std::string>& sourceFiles) = 0;

   /**
    * @brief Check if the extractor can handle the given file format
    *
    * @param filePath Path to the binary file
    * @return true if this extractor can process the file
    * @return false if the file format is not supported
    */
   virtual bool canHandle(const std::string& filePath) const = 0;

   /**
    * @brief Get the name of the binary format this extractor handles
    *
    * @return String identifier for the binary format (e.g., "ELF", "Mach-O", "PE")
    */
   virtual std::string getFormatName() const = 0;

   /**
    * @brief Get the priority of this extractor (lower numbers = higher priority)
    *
    * Used when multiple extractors might be able to handle the same file.
    *
    * @return Priority value (0 = highest priority)
    */
   virtual int getPriority() const = 0;
};

}  // namespace heimdall