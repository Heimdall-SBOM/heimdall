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
 * @file BinaryFormatFactory.cpp
 * @brief Implementation of BinaryFormatFactory
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file implements the BinaryFormatFactory class that creates appropriate
 * binary format extractors based on detected file format.
 */

#include "BinaryFormatFactory.hpp"
#include "../extractors/ArchiveExtractor.hpp"
#include "../extractors/ELFExtractor.hpp"
#include "../extractors/LazySymbolExtractor.hpp"
#include "../extractors/LightweightDWARFParser.hpp"
#include "../extractors/MachOExtractor.hpp"

// Conditionally include DWARFExtractor for C++17+
#if __cplusplus >= 201703L
#include "../extractors/DWARFExtractor.hpp"
#endif
#include "../extractors/PEExtractor.hpp"
#include "../interfaces/IBinaryExtractor.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace heimdall
{

namespace
{

// Magic numbers for different binary formats
constexpr uint32_t ELF_MAGIC       = 0x7F454C46;   // "\x7fELF"
constexpr uint32_t MACHO_MAGIC_32  = 0xFEEDFACE;   // Mach-O 32-bit
constexpr uint32_t MACHO_MAGIC_64  = 0xFEEDFACF;   // Mach-O 64-bit
constexpr uint32_t MACHO_MAGIC_FAT = 0xCAFEBABE;   // Mach-O Universal Binary
constexpr uint32_t PE_MAGIC        = 0x00004550;   // PE signature
constexpr char     ARCHIVE_MAGIC[] = "!<arch>\n";  // Unix archive magic

// Additional magic numbers for extended format support
constexpr uint32_t JAVA_CLASS_MAGIC = 0xCAFEBABE;  // Java class file
constexpr uint32_t WASM_MAGIC       = 0x6D736100;  // WebAssembly "\0asm"
constexpr uint32_t COFF_MAGIC       = 0x0000;      // COFF object file
constexpr uint32_t MZ_MAGIC         = 0x5A4D;      // DOS MZ executable

// File extensions for different formats
const std::map<BinaryFormatFactory::Format, std::vector<std::string>> FORMAT_EXTENSIONS = {
   {BinaryFormatFactory::Format::ELF, {".so", ".o", ".a", ".ko", ".bin", ""}},
   {BinaryFormatFactory::Format::MachO, {".dylib", ".bundle", ".o", ".a", ".app", ""}},
   {BinaryFormatFactory::Format::PE, {".exe", ".dll", ".sys", ".drv", ".ocx", ".o", ".a", ""}},
   {BinaryFormatFactory::Format::Archive, {".a", ".lib", ".ar"}},
   {BinaryFormatFactory::Format::Java, {".class", ".jar"}},
   {BinaryFormatFactory::Format::Wasm, {".wasm", ".wat"}},
};

// Registered extractors (will be populated when extractors are implemented)
static std::vector<std::unique_ptr<IBinaryExtractor>> registeredExtractors;
static std::mutex                                     registeredExtractorsMutex;

/**
 * @brief Read magic number from file with improved error handling
 *
 * @param filePath Path to the file
 * @param magic Output magic number
 * @param bytesToRead Number of bytes to read (default 4)
 * @param bigEndian Whether to read as big-endian (true) or little-endian (false)
 * @return true if magic number was read successfully
 * @return false if file could not be read
 */
bool readMagicNumber(const std::string& filePath, uint32_t& magic, size_t bytesToRead = 4,
                     bool bigEndian = true)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Reset magic to 0
   magic = 0;

   // Read the specified number of bytes
   std::vector<char> buffer(bytesToRead);
   file.read(buffer.data(), bytesToRead);

   if (file.gcount() != static_cast<std::streamsize>(bytesToRead))
   {
      return false;
   }

   // Convert to magic number based on byte order
   if (bigEndian)
   {
      // Big-endian (most significant byte first)
      for (size_t i = 0; i < bytesToRead && i < 4; ++i)
      {
         magic |= (static_cast<uint32_t>(static_cast<unsigned char>(buffer[i])) << ((3 - i) * 8));
      }
   }
   else
   {
      // Little-endian (least significant byte first)
      for (size_t i = 0; i < bytesToRead && i < 4; ++i)
      {
         magic |= (static_cast<uint32_t>(static_cast<unsigned char>(buffer[i])) << (i * 8));
      }
   }

   return true;
}

/**
 * @brief Read magic number as string for text-based formats
 *
 * @param filePath Path to the file
 * @param magic Output magic string
 * @param length Length of magic string to read
 * @return true if magic string was read successfully
 * @return false if file could not be read
 */
bool readMagicString(const std::string& filePath, std::string& magic, size_t length)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   magic.resize(length);
   file.read(&magic[0], length);
   return file.gcount() == static_cast<std::streamsize>(length);
}

/**
 * @brief Check if file starts with archive magic
 *
 * @param filePath Path to the file
 * @return true if file is an archive
 * @return false otherwise
 */
bool isArchiveFile(const std::string& filePath)
{
   std::string magic;
   if (!readMagicString(filePath, magic, sizeof(ARCHIVE_MAGIC) - 1))
   {
      return false;
   }

   return magic == std::string(ARCHIVE_MAGIC, sizeof(ARCHIVE_MAGIC) - 1);
}

/**
 * @brief Check if file is a Java class file
 *
 * @param filePath Path to the file
 * @return true if file is a Java class file
 * @return false otherwise
 */
bool isJavaClassFile(const std::string& filePath)
{
   uint32_t magic;
   if (!readMagicNumber(filePath, magic, 4, true))  // Java uses big-endian
   {
      return false;
   }

   return magic == JAVA_CLASS_MAGIC;
}

/**
 * @brief Check if file is a WebAssembly file
 *
 * @param filePath Path to the file
 * @return true if file is a WebAssembly file
 * @return false otherwise
 */
bool isWasmFile(const std::string& filePath)
{
   uint32_t magic;
   if (!readMagicNumber(filePath, magic, 4, false))  // WebAssembly uses little-endian
   {
      return false;
   }

   return magic == WASM_MAGIC;
}

/**
 * @brief Check if file is a DOS MZ executable
 *
 * @param filePath Path to the file
 * @return true if file is a DOS MZ executable
 * @return false otherwise
 */
bool isMZFile(const std::string& filePath)
{
   uint16_t      magic;
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   return file.gcount() == sizeof(magic) && magic == MZ_MAGIC;
}

/**
 * @brief Get file extension from path
 *
 * @param filePath Path to the file
 * @return File extension (including dot)
 */
std::string getFileExtension(const std::string& filePath)
{
   size_t lastDot = filePath.find_last_of('.');
   if (lastDot == std::string::npos)
   {
      return "";
   }
   return filePath.substr(lastDot);
}

/**
 * @brief Check if file exists and is readable
 *
 * @param filePath Path to the file
 * @return true if file exists and is readable
 * @return false otherwise
 */
bool isFileReadable(const std::string& filePath)
{
   std::ifstream file(filePath);
   return file.good();
}

}  // anonymous namespace

BinaryFormatFactory::Format BinaryFormatFactory::detectFormat(const std::string& filePath)
{
   // Check if file exists and is readable
   if (!isFileReadable(filePath))
   {
      return Format::Unknown;
   }

   // Check file extension first for quick filtering
   std::string extension = getFileExtension(filePath);

   // Check for archive format first (has specific magic)
   if (isArchiveFile(filePath))
   {
      return Format::Archive;
   }

   // Check for Java class files
   if (isJavaClassFile(filePath))
   {
      return Format::Java;
   }

   // Check for WebAssembly files
   if (isWasmFile(filePath))
   {
      return Format::Wasm;
   }

   // Read magic number for binary format detection with appropriate byte order
   uint32_t magic;

   // Try ELF first (big-endian)
   if (readMagicNumber(filePath, magic, 4, true) && magic == ELF_MAGIC)
   {
      return Format::ELF;
   }

   // Try Mach-O (little-endian on x86)
   if (readMagicNumber(filePath, magic, 4, false) &&
       (magic == MACHO_MAGIC_32 || magic == MACHO_MAGIC_64 || magic == MACHO_MAGIC_FAT))
   {
      return Format::MachO;
   }

   // Try PE (little-endian)
   if (readMagicNumber(filePath, magic, 4, false) && magic == PE_MAGIC)
   {
      return Format::PE;
   }

   // Check for DOS MZ executables (PE files often start with MZ)
   if (isMZFile(filePath))
   {
      return Format::PE;
   }

   // Fallback to extension-based detection
   for (const auto& format_extensions : FORMAT_EXTENSIONS)
   {
      const auto& format = format_extensions.first;
      const auto& extensions = format_extensions.second;
      if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end())
      {
         return format;
      }
   }

   return Format::Unknown;
}

std::unique_ptr<IBinaryExtractor> BinaryFormatFactory::createExtractor(Format format)
{
   switch (format)
   {
      case Format::ELF:
         return std::make_unique<ELFExtractor>();
      case Format::MachO:
         return std::make_unique<MachOExtractor>();
      case Format::PE:
         return std::make_unique<PEExtractor>();
      case Format::Archive:
         return std::make_unique<ArchiveExtractor>();
      case Format::Java:
      case Format::Wasm:
         // TODO: Implement Java and WebAssembly extractors
         return nullptr;
      case Format::Unknown:
      default:
         return nullptr;
   }
}

std::unique_ptr<IBinaryExtractor> BinaryFormatFactory::createExtractor(const std::string& filePath)
{
   Format format = detectFormat(filePath);
   return createExtractor(format);
}

std::vector<std::unique_ptr<IBinaryExtractor>> BinaryFormatFactory::getAvailableExtractors(
   const std::string& filePath)
{
   std::vector<std::unique_ptr<IBinaryExtractor>> availableExtractors;

   // Create all available extractors
   auto elfExtractor        = std::make_unique<ELFExtractor>();
   auto machoExtractor      = std::make_unique<MachOExtractor>();
   auto peExtractor         = std::make_unique<PEExtractor>();
   auto archiveExtractor    = std::make_unique<ArchiveExtractor>();
   auto lazySymbolExtractor = std::make_unique<LazySymbolExtractor>();
   auto dwarfParser         = std::make_unique<LightweightDWARFParser>();
   
   // Conditionally create DWARFExtractor for C++17+
#if __cplusplus >= 201703L
   auto dwarfExtractor      = std::make_unique<DWARFExtractor>();
#endif

   // Check which extractors can handle the file
   if (elfExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(elfExtractor));
   }
   if (machoExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(machoExtractor));
   }
   if (peExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(peExtractor));
   }
   if (archiveExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(archiveExtractor));
   }
   if (lazySymbolExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(lazySymbolExtractor));
   }
   if (dwarfParser->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(dwarfParser));
   }
   
   // Conditionally check DWARFExtractor for C++17+
#if __cplusplus >= 201703L
   if (dwarfExtractor->canHandle(filePath))
   {
      availableExtractors.push_back(std::move(dwarfExtractor));
   }
#endif

   // Add registered extractors
   {
      std::lock_guard<std::mutex> lock(registeredExtractorsMutex);
      for (const auto& extractor : registeredExtractors)
      {
         if (extractor->canHandle(filePath))
         {
            // Create a copy of the registered extractor
            // Note: This is a simplified approach - in a real implementation,
            // you might want to clone the extractor or use a factory pattern
            availableExtractors.push_back(std::make_unique<ELFExtractor>());  // Placeholder
         }
      }
   }

   // Sort by priority (higher priority first)
   std::sort(
      availableExtractors.begin(), availableExtractors.end(),
      [](const std::unique_ptr<IBinaryExtractor>& a, const std::unique_ptr<IBinaryExtractor>& b)
      { return a->getPriority() > b->getPriority(); });

   return availableExtractors;
}

bool BinaryFormatFactory::registerExtractor(std::unique_ptr<IBinaryExtractor> extractor)
{
   if (!extractor)
   {
      return false;
   }

   std::lock_guard<std::mutex> lock(registeredExtractorsMutex);

   // Check for duplicate format
   for (const auto& existing : registeredExtractors)
   {
      if (existing->getFormatName() == extractor->getFormatName())
      {
         return false;  // Duplicate format
      }
   }

   registeredExtractors.push_back(std::move(extractor));
   return true;
}

std::string BinaryFormatFactory::getFormatName(Format format)
{
   switch (format)
   {
      case Format::ELF:
         return "ELF";
      case Format::MachO:
         return "Mach-O";
      case Format::PE:
         return "PE";
      case Format::Archive:
         return "Archive";
      case Format::Java:
         return "Java";
      case Format::Wasm:
         return "WebAssembly";
      case Format::Unknown:
      default:
         return "Unknown";
   }
}

std::vector<std::string> BinaryFormatFactory::getFormatExtensions(Format format)
{
   auto it = FORMAT_EXTENSIONS.find(format);
   if (it != FORMAT_EXTENSIONS.end())
   {
      return it->second;
   }
   return {};
}

bool BinaryFormatFactory::isExtensionForFormat(const std::string& extension, Format format)
{
   auto extensions = getFormatExtensions(format);
   return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

std::vector<BinaryFormatFactory::Format> BinaryFormatFactory::getSupportedFormats()
{
   return {Format::ELF, Format::MachO, Format::PE, Format::Archive, Format::Java, Format::Wasm};
}

bool BinaryFormatFactory::isFormatSupported(Format format)
{
   switch (format)
   {
      case Format::ELF:
      case Format::MachO:
      case Format::PE:
      case Format::Archive:
      case Format::Java:
      case Format::Wasm:
         return true;
      case Format::Unknown:
      default:
         return false;
   }
}

size_t BinaryFormatFactory::getRegisteredExtractorCount()
{
   std::lock_guard<std::mutex> lock(registeredExtractorsMutex);
   return registeredExtractors.size();
}

void BinaryFormatFactory::clearRegisteredExtractors()
{
   std::lock_guard<std::mutex> lock(registeredExtractorsMutex);
   registeredExtractors.clear();
}

}  // namespace heimdall