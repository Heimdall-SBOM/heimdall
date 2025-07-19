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
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "ComponentInfo.hpp"
#include "../compat/compatibility.hpp"

namespace heimdall {

/**
 * @brief Lazy symbol extractor with caching for performance optimization
 *
 * This class implements lazy loading of symbols with caching to avoid
 * repeated extraction of the same symbols from files. This is particularly
 * important for large libraries like libc.so.6 which have thousands of symbols.
 *
 * Thread-safe implementation with mutex protection for concurrent access.
 */
class LazySymbolExtractor {
public:
    /**
     * @brief Default constructor
     */
    LazySymbolExtractor();

    /**
     * @brief Destructor
     */
    ~LazySymbolExtractor();

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

private:
    std::unordered_map<std::string, std::vector<SymbolInfo>> symbolCache;
    mutable std::mutex cacheMutex;
    size_t cacheHits{0};
    size_t cacheMisses{0};
    
    // Cache configuration
    static constexpr size_t MAX_CACHE_SIZE = 100;  // Maximum number of cached files
    static constexpr size_t MIN_SYMBOLS_TO_CACHE = 100;  // Minimum symbols to trigger caching
};

}  // namespace heimdall 