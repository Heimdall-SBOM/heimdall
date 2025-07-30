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
 * @file LazySymbolExtractor.hpp
 * @brief Lazy symbol loading with caching for performance optimization
 * @author Trevor Bakker
 * @date 2025
 * @version 2.0.0
 *
 * This file provides the LazySymbolExtractor class which implements the IBinaryExtractor
 * interface for extracting symbols from binary files with lazy loading and caching
 * for performance optimization.
 *
 * Features:
 * - Lazy symbol loading with intelligent caching
 * - Thread-safe implementation with mutex protection
 * - Performance optimization for large libraries
 * - Integration with binary format factory
 * - Support for multiple binary formats
 *
 * Dependencies:
 * - IBinaryExtractor interface
 * - ComponentInfo structures
 * - Binary format extractors (ELF, Mach-O, PE, etc.)
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"

namespace heimdall
{

/**
 * @brief Lazy symbol extractor with caching for performance optimization
 *
 * This class implements lazy loading of symbols with caching to avoid
 * repeated extraction of the same symbols from files. This is particularly
 * important for large libraries like libc.so.6 which have thousands of symbols.
 *
 * The extractor implements the IBinaryExtractor interface and can work with
 * any binary format by delegating to appropriate format-specific extractors.
 *
 * Thread-safe implementation with mutex protection for concurrent access.
 */
class LazySymbolExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   LazySymbolExtractor();

   /**
    * @brief Destructor
    */
   ~LazySymbolExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The LazySymbolExtractor to copy from
    */
   LazySymbolExtractor(const LazySymbolExtractor& other);

   /**
    * @brief Move constructor
    * @param other The LazySymbolExtractor to move from
    */
   LazySymbolExtractor(LazySymbolExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The LazySymbolExtractor to copy from
    * @return Reference to this LazySymbolExtractor
    */
   LazySymbolExtractor& operator=(const LazySymbolExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The LazySymbolExtractor to move from
    * @return Reference to this LazySymbolExtractor
    */
   LazySymbolExtractor& operator=(LazySymbolExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from binary file (lazy loading with caching)
    *
    * @param filePath Path to the binary file
    * @param symbols Output vector to store extracted symbols
    * @return true if symbols were successfully extracted
    * @return false if extraction failed or no symbols found
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from binary file
    *
    * @param filePath Path to the binary file
    * @param sections Output vector to store extracted sections
    * @return true if sections were successfully extracted
    * @return false if extraction failed or no sections found
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from binary file
    *
    * @param filePath Path to the binary file
    * @param version Output string to store version information
    * @return true if version was successfully extracted
    * @return false if extraction failed or no version found
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from binary file
    *
    * @param filePath Path to the binary file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the binary file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if the extractor can handle the given file format
    *
    * @param filePath Path to the binary file
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
    * @brief Get symbols for a file (lazy loading with caching)
    *
    * @param filePath Path to the file
    * @return Vector of symbol information
    */
   std::vector<SymbolInfo> getSymbols(const std::string& filePath);

   /**
    * @brief Clear the symbol cache
    */
   void clearCache();

   /**
    * @brief Get cache statistics
    *
    * @return Pair of (cache hits, cache misses)
    */
   std::pair<size_t, size_t> getCacheStats() const;

   /**
    * @brief Get cache size
    *
    * @return Number of cached files
    */
   size_t getCacheSize() const;

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   /**
    * @brief Extract symbols from file (actual implementation)
    *
    * @param filePath Path to the file
    * @return Vector of symbol information
    */
   std::vector<SymbolInfo> extractSymbols(const std::string& filePath);

   /**
    * @brief Check if symbols should be cached for this file
    *
    * @param filePath Path to the file
    * @return true if symbols should be cached, false otherwise
    */
   bool shouldCache(const std::string& filePath) const;

   /**
    * @brief Create appropriate binary extractor for the file format
    *
    * @param filePath Path to the binary file
    * @return Unique pointer to the appropriate extractor
    */
   std::unique_ptr<IBinaryExtractor> createExtractor(const std::string& filePath);
};

}  // namespace heimdall