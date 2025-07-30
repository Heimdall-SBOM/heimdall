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
 * @file LazySymbolExtractor.cpp
 * @brief Lazy symbol loading with caching for performance optimization
 * @author Trevor Bakker
 * @date 2025
 * @version 2.0.0
 *
 * This file provides the implementation of the LazySymbolExtractor class which
 * implements the IBinaryExtractor interface for extracting symbols from binary
 * files with lazy loading and caching for performance optimization.
 */

#include "LazySymbolExtractor.hpp"
#include <iostream>
#include "../compat/compatibility.hpp"
#include "../factories/BinaryFormatFactory.hpp"
#include "../utils/FileUtils.hpp"

namespace heimdall
{

// PIMPL implementation
class LazySymbolExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   std::unordered_map<std::string, std::vector<SymbolInfo>> symbolCache;
   mutable std::mutex                                       cacheMutex;
   size_t                                                   cacheHits{0};
   size_t                                                   cacheMisses{0};

   // Cache configuration
   static constexpr size_t MAX_CACHE_SIZE       = 100;  // Maximum number of cached files
   static constexpr size_t MIN_SYMBOLS_TO_CACHE = 100;  // Minimum symbols to trigger caching
};

LazySymbolExtractor::LazySymbolExtractor() : pImpl(heimdall::compat::make_unique<Impl>())
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: Constructor called" << std::endl;
#endif
}

LazySymbolExtractor::~LazySymbolExtractor() = default;

LazySymbolExtractor::LazySymbolExtractor(const LazySymbolExtractor& other)
   : pImpl(heimdall::compat::make_unique<Impl>())
{
   // Copy data members manually since mutex can't be copied
   pImpl->symbolCache = other.pImpl->symbolCache;
   pImpl->cacheHits   = other.pImpl->cacheHits;
   pImpl->cacheMisses = other.pImpl->cacheMisses;
}

LazySymbolExtractor::LazySymbolExtractor(LazySymbolExtractor&& other) noexcept
   : pImpl(std::move(other.pImpl))
{
}

LazySymbolExtractor& LazySymbolExtractor::operator=(const LazySymbolExtractor& other)
{
   if (this != &other)
   {
      pImpl = heimdall::compat::make_unique<Impl>();
      // Copy data members manually since mutex can't be copied
      pImpl->symbolCache = other.pImpl->symbolCache;
      pImpl->cacheHits   = other.pImpl->cacheHits;
      pImpl->cacheMisses = other.pImpl->cacheMisses;
   }
   return *this;
}

LazySymbolExtractor& LazySymbolExtractor::operator=(LazySymbolExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool LazySymbolExtractor::extractSymbols(const std::string&       filePath,
                                         std::vector<SymbolInfo>& symbols)
{
   std::lock_guard<std::mutex> lock(pImpl->cacheMutex);

#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: extractSymbols called for " << filePath << std::endl;
#endif

   // Check if symbols are already cached
   auto it = pImpl->symbolCache.find(filePath);
   if (it != pImpl->symbolCache.end())
   {
      pImpl->cacheHits++;
      symbols = it->second;
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LazySymbolExtractor: Cache HIT for " << filePath << " (" << symbols.size()
                << " symbols)" << std::endl;
#endif
      return true;
   }

   pImpl->cacheMisses++;
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: Cache MISS for " << filePath << std::endl;
#endif

   // Extract symbols from file using appropriate binary extractor
   symbols = extractSymbols(filePath);

   // Cache the symbols if appropriate
   if (shouldCache(filePath) && symbols.size() >= Impl::MIN_SYMBOLS_TO_CACHE)
   {
      // Check cache size limit
      if (pImpl->symbolCache.size() >= Impl::MAX_CACHE_SIZE)
      {
         // Remove oldest entry (simple LRU - remove first entry)
         pImpl->symbolCache.erase(pImpl->symbolCache.begin());
#ifdef HEIMDALL_DEBUG_ENABLED
         std::cout << "LazySymbolExtractor: Cache full, removed oldest entry" << std::endl;
#endif
      }

      pImpl->symbolCache[filePath] = symbols;
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LazySymbolExtractor: Cached " << symbols.size() << " symbols for " << filePath
                << std::endl;
#endif
   }

   return !symbols.empty();
}

bool LazySymbolExtractor::extractSections(const std::string&        filePath,
                                          std::vector<SectionInfo>& sections)
{
   // Delegate to appropriate binary extractor
   auto extractor = createExtractor(filePath);
   if (extractor)
   {
      return extractor->extractSections(filePath, sections);
   }
   return false;
}

bool LazySymbolExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   // Delegate to appropriate binary extractor
   auto extractor = createExtractor(filePath);
   if (extractor)
   {
      return extractor->extractVersion(filePath, version);
   }
   return false;
}

std::vector<std::string> LazySymbolExtractor::extractDependencies(const std::string& filePath)
{
   // LazySymbolExtractor doesn't extract dependencies
   (void)filePath;  // Suppress unused parameter warning
   return std::vector<std::string>();
}

bool LazySymbolExtractor::extractFunctions(const std::string&        filePath,
                                           std::vector<std::string>& functions)
{
   // LazySymbolExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;   // Suppress unused parameter warning
   (void)functions;  // Suppress unused parameter warning
   return false;
}

bool LazySymbolExtractor::extractCompileUnits(const std::string&        filePath,
                                              std::vector<std::string>& compileUnits)
{
   // LazySymbolExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;      // Suppress unused parameter warning
   (void)compileUnits;  // Suppress unused parameter warning
   return false;
}

bool LazySymbolExtractor::extractSourceFiles(const std::string&        filePath,
                                             std::vector<std::string>& sourceFiles)
{
   // LazySymbolExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;     // Suppress unused parameter warning
   (void)sourceFiles;  // Suppress unused parameter warning
   return false;
}

bool LazySymbolExtractor::canHandle(const std::string& filePath) const
{
   // Can handle any binary file that has a supported extractor
   auto extractor = const_cast<LazySymbolExtractor*>(this)->createExtractor(filePath);
   return extractor != nullptr;
}

std::string LazySymbolExtractor::getFormatName() const
{
   return "Lazy Symbol Extractor";
}

int LazySymbolExtractor::getPriority() const
{
   return 10;  // Lower priority than specific format extractors
}

// Legacy interface methods for backward compatibility
std::vector<SymbolInfo> LazySymbolExtractor::getSymbols(const std::string& filePath)
{
   std::vector<SymbolInfo> symbols;
   extractSymbols(filePath, symbols);
   return symbols;
}

void LazySymbolExtractor::clearCache()
{
   std::lock_guard<std::mutex> lock(pImpl->cacheMutex);

#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: Clearing cache" << std::endl;
#endif

   pImpl->symbolCache.clear();
   pImpl->cacheHits   = 0;
   pImpl->cacheMisses = 0;
}

std::pair<size_t, size_t> LazySymbolExtractor::getCacheStats() const
{
   std::lock_guard<std::mutex> lock(pImpl->cacheMutex);
   return {pImpl->cacheHits, pImpl->cacheMisses};
}

size_t LazySymbolExtractor::getCacheSize() const
{
   std::lock_guard<std::mutex> lock(pImpl->cacheMutex);
   return pImpl->symbolCache.size();
}

// Private methods
std::vector<SymbolInfo> LazySymbolExtractor::extractSymbols(const std::string& filePath)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: Extracting symbols from " << filePath << std::endl;
#endif

   std::vector<SymbolInfo> symbols;

   // Use the binary format factory to get appropriate extractor
   auto extractor = createExtractor(filePath);
   if (extractor)
   {
      extractor->extractSymbols(filePath, symbols);
   }

#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LazySymbolExtractor: Extracted " << symbols.size() << " symbols from " << filePath
             << std::endl;
#endif

   return symbols;
}

bool LazySymbolExtractor::shouldCache(const std::string& filePath) const
{
   // Cache system libraries and large files
   if (filePath.find("/usr/lib") != std::string::npos ||
       filePath.find("/lib") != std::string::npos ||
       filePath.find("libc.so") != std::string::npos ||
       filePath.find("libstdc++") != std::string::npos)
   {
      return true;
   }

   // Cache files with .so extension (shared libraries)
   if (filePath.find(".so") != std::string::npos)
   {
      return true;
   }

   // Don't cache small files or executables
   if (filePath.find(".exe") != std::string::npos || filePath.find(".bin") != std::string::npos)
   {
      return false;
   }

   return true;
}

std::unique_ptr<IBinaryExtractor> LazySymbolExtractor::createExtractor(const std::string& filePath)
{
   // Use the binary format factory to create appropriate extractor
   return BinaryFormatFactory::createExtractor(filePath);
}

}  // namespace heimdall