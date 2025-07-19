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
 */

#include "LazySymbolExtractor.hpp"
#include "Utils.hpp"
#include "MetadataExtractor.hpp"
#include "../compat/compatibility.hpp"

namespace heimdall {

LazySymbolExtractor::LazySymbolExtractor() {
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Constructor called");
#endif
}

LazySymbolExtractor::~LazySymbolExtractor() {
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Destructor called");
    auto stats = getCacheStats();
    heimdall::Utils::debugPrint("LazySymbolExtractor: Cache stats - Hits: " + 
                                std::to_string(stats.first) + ", Misses: " + 
                                std::to_string(stats.second));
#endif
}

std::vector<SymbolInfo> LazySymbolExtractor::getSymbols(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: getSymbols called for " + filePath);
#endif
    
    // Check if symbols are already cached
    auto it = symbolCache.find(filePath);
    if (it != symbolCache.end()) {
        cacheHits++;
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("LazySymbolExtractor: Cache HIT for " + filePath + 
                                    " (" + std::to_string(it->second.size()) + " symbols)");
#endif
        return it->second;
    }
    
    cacheMisses++;
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Cache MISS for " + filePath);
#endif
    
    // Extract symbols from file
    std::vector<SymbolInfo> symbols = extractSymbols(filePath);
    
    // Cache the symbols if appropriate
    if (shouldCache(filePath) && symbols.size() >= MIN_SYMBOLS_TO_CACHE) {
        // Check cache size limit
        if (symbolCache.size() >= MAX_CACHE_SIZE) {
            // Remove oldest entry (simple LRU - remove first entry)
            symbolCache.erase(symbolCache.begin());
#ifdef HEIMDALL_DEBUG_ENABLED
            heimdall::Utils::debugPrint("LazySymbolExtractor: Cache full, removed oldest entry");
#endif
        }
        
        symbolCache[filePath] = symbols;
#ifdef HEIMDALL_DEBUG_ENABLED
        heimdall::Utils::debugPrint("LazySymbolExtractor: Cached " + std::to_string(symbols.size()) + 
                                    " symbols for " + filePath);
#endif
    }
    
    return symbols;
}

void LazySymbolExtractor::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Clearing cache");
#endif
    
    symbolCache.clear();
    cacheHits = 0;
    cacheMisses = 0;
}

std::pair<size_t, size_t> LazySymbolExtractor::getCacheStats() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return {cacheHits, cacheMisses};
}

size_t LazySymbolExtractor::getCacheSize() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return symbolCache.size();
}

std::vector<SymbolInfo> LazySymbolExtractor::extractSymbols(const std::string& filePath) {
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Extracting symbols from " + filePath);
#endif
    
    std::vector<SymbolInfo> symbols;
    
    // Use the existing symbol extraction logic
    if (MetadataHelpers::isELF(filePath)) {
        MetadataHelpers::extractELFSymbols(filePath, symbols);
    } else if (MetadataHelpers::isMachO(filePath)) {
        MetadataHelpers::extractMachOSymbols(filePath, symbols);
    } else if (MetadataHelpers::isPE(filePath)) {
        MetadataHelpers::extractPESymbols(filePath, symbols);
    } else if (MetadataHelpers::isArchive(filePath)) {
        MetadataHelpers::extractArchiveSymbols(filePath, symbols);
    }
    
#ifdef HEIMDALL_DEBUG_ENABLED
    heimdall::Utils::debugPrint("LazySymbolExtractor: Extracted " + std::to_string(symbols.size()) + 
                                " symbols from " + filePath);
#endif
    
    return symbols;
}

bool LazySymbolExtractor::shouldCache(const std::string& filePath) const {
    // Cache system libraries and large files
    if (filePath.find("/usr/lib") != std::string::npos ||
        filePath.find("/lib") != std::string::npos ||
        filePath.find("libc.so") != std::string::npos ||
        filePath.find("libstdc++") != std::string::npos) {
        return true;
    }
    
    // Cache files with .so extension (shared libraries)
    if (filePath.find(".so") != std::string::npos) {
        return true;
    }
    
    // Don't cache small files or executables
    if (filePath.find(".exe") != std::string::npos ||
        filePath.find(".bin") != std::string::npos) {
        return false;
    }
    
    return true;
}

}  // namespace heimdall 