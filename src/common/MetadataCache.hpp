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
 * @file MetadataCache.hpp
 * @brief Caching system for metadata to improve performance
 * @author Trevor Bakker
 * @date 2025
 *
 * This module provides a caching system for storing and retrieving metadata
 * for previously processed files, improving performance for repeated operations.
 */

#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "ComponentInfo.hpp"

namespace heimdall
{

/**
 * @brief Cache entry for metadata
 */
struct CacheEntry
{
   ComponentInfo                         component;     ///< Cached component metadata
   std::chrono::system_clock::time_point timestamp;     ///< When the entry was created
   std::string                           fileHash;      ///< Hash of the file for validation
   size_t                                fileSize;      ///< File size for validation
   std::chrono::system_clock::time_point lastModified;  ///< File modification time
   bool                                  valid = true;  ///< Whether the cache entry is valid
};

/**
 * @brief Metadata caching system
 *
 * This class provides a thread-safe caching system for metadata extraction
 * results. It supports automatic invalidation based on file changes,
 * configurable cache size limits, and persistence to disk.
 */
class MetadataCache
{
   public:
   /**
    * @brief Constructor
    * @param maxSize Maximum number of cache entries
    * @param maxAge Maximum age of cache entries in seconds
    */
   MetadataCache(size_t maxSize = 1000, size_t maxAge = 3600);

   /**
    * @brief Destructor
    */
   ~MetadataCache();

   /**
    * @brief Get metadata from cache
    * @param filePath Path to the file
    * @param component Output component info if found
    * @return true if metadata was found and is valid
    */
   bool get(const std::string& filePath, ComponentInfo& component);

   /**
    * @brief Store metadata in cache
    * @param filePath Path to the file
    * @param component Component metadata to cache
    * @return true if successfully cached
    */
   bool put(const std::string& filePath, const ComponentInfo& component);

   /**
    * @brief Remove an entry from cache
    * @param filePath Path to the file
    * @return true if entry was removed
    */
   bool remove(const std::string& filePath);

   /**
    * @brief Clear all cache entries
    */
   void clear();

   /**
    * @brief Check if a file is cached
    * @param filePath Path to the file
    * @return true if the file is cached
    */
   bool contains(const std::string& filePath) const;

   /**
    * @brief Get cache statistics
    * @return Map of cache statistics
    */
   std::map<std::string, size_t> getStatistics() const;

   /**
    * @brief Set maximum cache size
    * @param maxSize Maximum number of entries
    */
   void setMaxSize(size_t maxSize);

   /**
    * @brief Set maximum cache entry age
    * @param maxAge Maximum age in seconds
    */
   void setMaxAge(size_t maxAge);

   /**
    * @brief Enable or disable caching
    * @param enabled Whether to enable caching
    */
   void setEnabled(bool enabled);

   /**
    * @brief Check if caching is enabled
    * @return true if caching is enabled
    */
   bool isEnabled() const;

   /**
    * @brief Load cache from file
    * @param filePath Path to cache file
    * @return true if successfully loaded
    */
   bool loadFromFile(const std::string& filePath);

   /**
    * @brief Save cache to file
    * @param filePath Path to cache file
    * @return true if successfully saved
    */
   bool saveToFile(const std::string& filePath) const;

   /**
    * @brief Invalidate cache entries for a directory
    * @param directoryPath Path to the directory
    * @return Number of entries invalidated
    */
   size_t invalidateDirectory(const std::string& directoryPath);

   /**
    * @brief Clean up expired entries
    * @return Number of entries removed
    */
   size_t cleanup();

   /**
    * @brief Get cache hit rate
    * @return Hit rate as a percentage (0.0-100.0)
    */
   double getHitRate() const;

   /**
    * @brief Reset cache statistics
    */
   void resetStatistics();

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

}  // namespace heimdall