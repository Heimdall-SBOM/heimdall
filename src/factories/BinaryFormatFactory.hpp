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
 * @file BinaryFormatFactory.hpp
 * @brief Factory for creating binary format extractors
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file defines the BinaryFormatFactory class that creates appropriate
 * binary format extractors based on detected file format. It implements
 * the Factory pattern to provide a unified interface for different binary
 * formats (ELF, Mach-O, PE, Archives).
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

// Forward declaration
class IBinaryExtractor;

/**
 * @brief Factory for creating binary format extractors
 *
 * This factory class is responsible for:
 * - Detecting binary file formats
 * - Creating appropriate extractors for detected formats
 * - Managing extractor registration and priority
 * - Providing a unified interface for format detection
 */
class BinaryFormatFactory
{
   public:
   /**
    * @brief Enumeration of supported binary formats
    */
   enum class Format
   {
      ELF,      // Linux executables/libraries
      MachO,    // macOS executables/libraries
      PE,       // Windows executables/libraries
      Archive,  // Static libraries (.a, .lib)
      Java,     // Java class files and JAR archives
      Wasm,     // WebAssembly files
      Unknown   // Unrecognized format
   };

   /**
    * @brief Detect binary format from file
    *
    * @param filePath Path to the binary file
    * @return Detected format or Format::Unknown if not recognized
    */
   static Format detectFormat(const std::string& filePath);

   /**
    * @brief Create appropriate extractor for detected format
    *
    * @param format The detected binary format
    * @return Unique pointer to the appropriate extractor, or nullptr if not supported
    */
   static std::unique_ptr<IBinaryExtractor> createExtractor(Format format);

   /**
    * @brief Create extractor directly from file (auto-detect)
    *
    * @param filePath Path to the binary file
    * @return Unique pointer to the appropriate extractor, or nullptr if not supported
    */
   static std::unique_ptr<IBinaryExtractor> createExtractor(const std::string& filePath);

   /**
    * @brief Get all available extractors that can handle the given file
    *
    * @param filePath Path to the binary file
    * @return Vector of extractors that can handle the file, sorted by priority
    */
   static std::vector<std::unique_ptr<IBinaryExtractor>> getAvailableExtractors(
      const std::string& filePath);

   /**
    * @brief Register a custom extractor with the factory
    *
    * @param extractor Unique pointer to the extractor to register
    * @return true if registration was successful
    * @return false if registration failed (e.g., duplicate format)
    */
   static bool registerExtractor(std::unique_ptr<IBinaryExtractor> extractor);

   /**
    * @brief Get the name of a format as a string
    *
    * @param format The format enum value
    * @return String representation of the format
    */
   static std::string getFormatName(Format format);

   /**
    * @brief Get the file extension associated with a format
    *
    * @param format The format enum value
    * @return Vector of common file extensions for this format
    */
   static std::vector<std::string> getFormatExtensions(Format format);

   /**
    * @brief Check if a file extension is associated with a format
    *
    * @param extension File extension (with or without dot)
    * @param format The format to check against
    * @return true if the extension is associated with the format
    * @return false otherwise
    */
   static bool isExtensionForFormat(const std::string& extension, Format format);

   /**
    * @brief Get all supported formats
    *
    * @return Vector of all supported format enums
    */
   static std::vector<Format> getSupportedFormats();

   /**
    * @brief Check if a format is supported by the factory
    *
    * @param format The format to check
    * @return true if the format is supported
    * @return false otherwise
    */
   static bool isFormatSupported(Format format);

   /**
    * @brief Get the number of registered extractors
    *
    * @return Number of currently registered extractors
    */
   static size_t getRegisteredExtractorCount();

   /**
    * @brief Clear all registered extractors
    *
    * This is useful for testing or when you want to reset the factory state
    */
   static void clearRegisteredExtractors();

   private:
   /**
    * @brief Private constructor to prevent instantiation
    */
   BinaryFormatFactory() = delete;

   /**
    * @brief Private destructor
    */
   ~BinaryFormatFactory() = delete;

   /**
    * @brief Private copy constructor
    */
   BinaryFormatFactory(const BinaryFormatFactory&) = delete;

   /**
    * @brief Private assignment operator
    */
   BinaryFormatFactory& operator=(const BinaryFormatFactory&) = delete;
};

}  // namespace heimdall