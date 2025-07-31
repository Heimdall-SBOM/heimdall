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
 * @file DWARFExtractor.hpp
 * @brief Robust DWARF debug information extractor using LLVM libraries
 * @author Trevor Bakker
 * @date 2025
 * @version 2.0.0
 *
 * This file provides the DWARFExtractor class which implements the IBinaryExtractor
 * interface for extracting DWARF debug information from binary files using LLVM libraries.
 *
 * Features:
 * - Extract source files from DWARF debug information
 * - Extract compile units from DWARF debug information
 * - Extract function names from DWARF debug information
 * - Extract line information from DWARF debug information
 * - Fallback to symbol table extraction when DWARF is not available
 * - Integration with binary format factory
 * - Support for multiple binary formats
 *
 * IMPORTANT THREAD-SAFETY WARNING:
 * This class is NOT thread-safe due to LLVM's DWARF library limitations.
 * Do not create multiple DWARFExtractor instances simultaneously or use
 * from different threads. See heimdall-limitations.md for details.
 *
 * Dependencies:
 * - IBinaryExtractor interface
 * - ComponentInfo structures
 * - LLVM DWARF libraries (when available)
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"

#ifdef LLVM_DWARF_AVAILABLE
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#endif

namespace heimdall
{

#ifdef LLVM_DWARF_AVAILABLE
// Global LLVM objects to prevent premature destruction
extern std::unique_ptr<llvm::MemoryBuffer>       g_buffer;
extern std::unique_ptr<llvm::object::ObjectFile> g_objectFile;
extern std::unique_ptr<llvm::DWARFContext>       g_context;
#endif

/**
 * @brief Robust DWARF debug information extractor using LLVM libraries
 *
 * This class provides comprehensive DWARF parsing capabilities using LLVM's
 * DWARF libraries, which are the most mature and standards-compliant
 * implementation available. It implements the IBinaryExtractor interface
 * for integration with the binary format factory.
 *
 * @warning This class is NOT thread-safe. Do not use multiple instances
 *          simultaneously or from different threads. See heimdall-limitations.md
 *          for detailed thread-safety information.
 */
class DWARFExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   DWARFExtractor();

   /**
    * @brief Destructor
    */
   ~DWARFExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The DWARFExtractor to copy from
    */
   DWARFExtractor(const DWARFExtractor& other);

   /**
    * @brief Move constructor
    * @param other The DWARFExtractor to move from
    */
   DWARFExtractor(DWARFExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The DWARFExtractor to copy from
    * @return Reference to this DWARFExtractor
    */
   DWARFExtractor& operator=(const DWARFExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The DWARFExtractor to move from
    * @return Reference to this DWARFExtractor
    */
   DWARFExtractor& operator=(DWARFExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from a binary file
    * @param filePath Path to the binary file
    * @param symbols Vector to populate with extracted symbols
    * @return true if extraction was successful, false otherwise
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information (not supported by this extractor)
    * @param filePath Path to the binary file
    * @param sections Vector to populate with extracted sections
    * @return Always false
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information (not supported by this extractor)
    * @param filePath Path to the binary file
    * @param version String to populate with version information
    * @return Always false
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependencies (not supported by this extractor)
    * @param filePath Path to the binary file
    * @return Empty vector
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Check if this extractor can handle the given file
    * @param filePath Path to the binary file
    * @return true if the file has DWARF debug information, false otherwise
    */
   bool canHandle(const std::string& filePath) const override;

   /**
    * @brief Get the name of the format handled by this extractor
    * @return "DWARF"
    */
   std::string getFormatName() const override;

   /**
    * @brief Get the priority of this extractor
    * @return Priority value (higher means more specific)
    */
   int getPriority() const override;

   // DWARF-specific methods
   /**
    * @brief Extract source files from DWARF debug information
    * @param filePath Path to the binary file
    * @param sourceFiles Vector to populate with source file paths
    * @return true if extraction was successful, false otherwise
    */
   bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Extract compile units from DWARF debug information
    * @param filePath Path to the binary file
    * @param compileUnits Vector to populate with compile unit names
    * @return true if extraction was successful, false otherwise
    */
   bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract function names from DWARF debug information
    * @param filePath Path to the binary file
    * @param functions Vector to populate with function names
    * @return true if extraction was successful, false otherwise
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract line information from DWARF debug information
    * @param filePath Path to the binary file
    * @param lineInfo Vector to populate with line information
    * @return true if extraction was successful, false otherwise
    */
   bool extractLineInfo(const std::string& filePath, std::vector<std::string>& lineInfo);

   /**
    * @brief Check if the file has DWARF debug information
    * @param filePath Path to the binary file
    * @return true if DWARF debug information is present, false otherwise
    */
   bool hasDWARFInfo(const std::string& filePath) const;

   /**
    * @brief Get the actual DWARF file path (handles .dSYM files on macOS)
    * @param filePath Path to the binary file
    * @return Path to the actual DWARF file (may be in .dSYM bundle)
    */
   std::string getDWARFFilePath(const std::string& filePath) const;

   /**
    * @brief Extract all debug information from DWARF
    * @param filePath Path to the binary file
    * @param sourceFiles Vector to populate with source file paths
    * @param compileUnits Vector to populate with compile unit names
    * @param functions Vector to populate with function names
    * @param lineInfo Vector to populate with line information
    * @return true if extraction was successful, false otherwise
    */
   bool extractAllDebugInfo(const std::string& filePath, std::vector<std::string>& sourceFiles,
                            std::vector<std::string>& compileUnits,
                            std::vector<std::string>& functions,
                            std::vector<std::string>& lineInfo);

   /**
    * @brief Ensure LLVM is properly initialized
    */
   static void ensureLLVMInitialized();

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Private helper methods
   bool extractSourceFilesHeuristic(const std::string&        filePath,
                                    std::vector<std::string>& sourceFiles);

   bool extractFunctionsFromSymbolTable(const std::string&        filePath,
                                        std::vector<std::string>& functions);

#ifdef LLVM_DWARF_AVAILABLE
   void extractSourceFilesFromDie(const llvm::DWARFDie& die, std::vector<std::string>& sourceFiles);

   void extractCompileUnitFromDie(const llvm::DWARFDie&     die,
                                  std::vector<std::string>& compileUnits);

   void extractFunctionsFromDie(const llvm::DWARFDie& die, std::vector<std::string>& functions);
#endif
};

}  // namespace heimdall