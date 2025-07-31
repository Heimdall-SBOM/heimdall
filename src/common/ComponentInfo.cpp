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
 * @file ComponentInfo.cpp
 * @brief Implementation of ComponentInfo data structures and utilities
 * @author Trevor Bakker
 * @date 2025
 */

#include "ComponentInfo.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <openssl/sha.h>
#include <sys/stat.h>

namespace heimdall
{

/**
 * @brief Calculate SHA256 hash of a file
 * @param filePath Path to the file
 * @return SHA256 hash as a hex string
 */
std::string calculateSHA256(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file)
   {
      return "";
   }

   SHA256_CTX sha256;
   SHA256_Init(&sha256);

   char buffer[4096];
   while (file.read(buffer, sizeof(buffer)))
   {
      SHA256_Update(&sha256, buffer, file.gcount());
   }
   SHA256_Update(&sha256, buffer, file.gcount());

   unsigned char hash[SHA256_DIGEST_LENGTH];
   SHA256_Final(hash, &sha256);

   std::stringstream ss;
   for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
   {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
   }

   return ss.str();
}

/**
 * @brief Get file size in bytes
 * @param filePath Path to the file
 * @return File size in bytes, or 0 if error
 */
uint64_t getFileSize(const std::string& filePath)
{
   struct stat st;
   if (stat(filePath.c_str(), &st) == 0)
   {
      return static_cast<uint64_t>(st.st_size);
   }
   return 0;
}

/**
 * @brief Detect ELF file type
 * @param filePath Path to the ELF file
 * @return FileType enum value
 */
FileType detectElfFileType(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file)
   {
      return FileType::Unknown;
   }

   // Read ELF header
   char header[64];
   file.read(header, sizeof(header));
   if (file.gcount() < 64)
   {
      return FileType::Unknown;
   }

   // Check ELF magic number
   if (header[0] != 0x7f || header[1] != 'E' || header[2] != 'L' || header[3] != 'F')
   {
      return FileType::Unknown;
   }

   // Check file class (32-bit vs 64-bit)
   if (header[4] == 1)  // 32-bit
   {
      // Check type from 16-bit value at offset 16
      uint16_t type = static_cast<uint8_t>(header[16]) | (static_cast<uint8_t>(header[17]) << 8);
      switch (type)
      {
         case 1:
            return FileType::Object;  // ET_REL
         case 2:
            return FileType::Executable;  // ET_EXEC
         case 3:
            return FileType::SharedLibrary;  // ET_DYN
         default:
            return FileType::Unknown;
      }
   }
   else if (header[4] == 2)  // 64-bit
   {
      // Check type from 16-bit value at offset 16
      uint16_t type = static_cast<uint8_t>(header[16]) | (static_cast<uint8_t>(header[17]) << 8);
      switch (type)
      {
         case 1:
            return FileType::Object;  // ET_REL
         case 2:
            return FileType::Executable;  // ET_EXEC
         case 3:
            return FileType::SharedLibrary;  // ET_DYN
         default:
            return FileType::Unknown;
      }
   }

   return FileType::Unknown;
}

/**
 * @brief Detect Mach-O file type
 * @param filePath Path to the Mach-O file
 * @return FileType enum value
 */
FileType detectMachOFileType(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file)
   {
      return FileType::Unknown;
   }

   // Read Mach-O header
   uint32_t magic;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   if (file.gcount() < sizeof(magic))
   {
      return FileType::Unknown;
   }

   // Check Mach-O magic numbers
   if (magic == 0xFEEDFACE)  // 32-bit Mach-O
   {
      uint32_t filetype;
      file.read(reinterpret_cast<char*>(&filetype), sizeof(filetype));
      if (file.gcount() < sizeof(filetype))
      {
         return FileType::Unknown;
      }

      switch (filetype)
      {
         case 1:
            return FileType::Object;  // MH_OBJECT
         case 2:
            return FileType::Executable;  // MH_EXECUTE
         case 6:
            return FileType::SharedLibrary;  // MH_DYLIB
         case 8:
            return FileType::StaticLibrary;  // MH_DYLINKER
         default:
            return FileType::Unknown;
      }
   }
   else if (magic == 0xFEEDFACF)  // 64-bit Mach-O
   {
      uint32_t filetype;
      file.read(reinterpret_cast<char*>(&filetype), sizeof(filetype));
      if (file.gcount() < sizeof(filetype))
      {
         return FileType::Unknown;
      }

      switch (filetype)
      {
         case 1:
            return FileType::Object;  // MH_OBJECT
         case 2:
            return FileType::Executable;  // MH_EXECUTE
         case 6:
            return FileType::SharedLibrary;  // MH_DYLIB
         case 8:
            return FileType::StaticLibrary;  // MH_DYLINKER
         default:
            return FileType::Unknown;
      }
   }
   else if (magic == 0xCAFEBABE)  // Universal binary
   {
      // For universal binaries, we'll treat them as executables for now
      return FileType::Executable;
   }

   return FileType::Unknown;
}

/**
 * @brief Determine file type based on file extension and content
 * @param filePath Path to the file
 * @return FileType enum value
 */
FileType determineFileType(const std::string& filePath)
{
   // Check file extension first
   std::string extension;
   size_t      dotPos = filePath.find_last_of('.');
   if (dotPos != std::string::npos)
   {
      extension = filePath.substr(dotPos + 1);
      std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
   }

   // Source files
   if (extension == "c" || extension == "cpp" || extension == "cc" || extension == "cxx" ||
       extension == "h" || extension == "hpp" || extension == "hh" || extension == "hxx" ||
       extension == "m" || extension == "mm" || extension == "s" || extension == "S" ||
       extension == "asm" || extension == "f" || extension == "f90" || extension == "f95")
   {
      return FileType::Source;
   }

   // Object files
   if (extension == "o" || extension == "obj")
   {
      return FileType::Object;
   }

   // Static libraries
   if (extension == "a" || extension == "lib")
   {
      return FileType::StaticLibrary;
   }

   // Shared libraries
   if (extension == "so" || extension == "dylib" || extension == "dll")
   {
      return FileType::SharedLibrary;
   }

   // Executables (no extension or common executable extensions)
   if (extension.empty() || extension == "exe" || extension == "out" || extension == "bin")
   {
      // Try to detect binary format
      FileType elfType = detectElfFileType(filePath);
      if (elfType != FileType::Unknown)
      {
         return elfType;
      }

      FileType machoType = detectMachOFileType(filePath);
      if (machoType != FileType::Unknown)
      {
         return machoType;
      }

      // If no specific format detected but no extension, assume executable
      if (extension.empty())
      {
         return FileType::Executable;
      }
   }

   return FileType::Unknown;
}

// ComponentInfo member function implementations

heimdall::ComponentInfo::ComponentInfo(std::string componentName, const std::string& path)
   : name(std::move(componentName)),
     filePath(path),
     fileType(determineFileType(path)),
     fileSize(getFileSize(path)),
     wasProcessed(false),
     detectedBy(LinkerType::Unknown),
     isSystemLibrary(false),
     containsDebugInfo(false),
     isStripped(false)
{
   checksum = calculateSHA256(path);
}

void heimdall::ComponentInfo::addDependency(const std::string& dependency)
{
   if (std::find(dependencies.begin(), dependencies.end(), dependency) == dependencies.end())
   {
      dependencies.push_back(dependency);
   }
}

void heimdall::ComponentInfo::addSourceFile(const std::string& sourceFile)
{
   if (std::find(sourceFiles.begin(), sourceFiles.end(), sourceFile) == sourceFiles.end())
   {
      sourceFiles.push_back(sourceFile);
   }
}

void heimdall::ComponentInfo::setVersion(const std::string& ver)
{
   version = ver;
}

void heimdall::ComponentInfo::setSupplier(const std::string& sup)
{
   supplier = sup;
}

void heimdall::ComponentInfo::setDownloadLocation(const std::string& location)
{
   downloadLocation = location;
}

void heimdall::ComponentInfo::setHomepage(const std::string& page)
{
   homepage = page;
}

void heimdall::ComponentInfo::setLicense(const std::string& lic)
{
   license = lic;
}

void heimdall::ComponentInfo::setPackageManager(const std::string& pkgMgr)
{
   packageManager = pkgMgr;
}

void ComponentInfo::setDescription(const std::string& desc)
{
   description = desc;
}

void ComponentInfo::setScope(const std::string& s)
{
   scope = s;
}

void ComponentInfo::setGroup(const std::string& g)
{
   group = g;
}

void ComponentInfo::setMimeType(const std::string& mime)
{
   mimeType = mime;
}

void ComponentInfo::setCopyright(const std::string& copy)
{
   copyright = copy;
}

void ComponentInfo::setCPE(const std::string& cpeId)
{
   cpe = cpeId;
}

void ComponentInfo::setManufacturer(const std::string& manu)
{
   manufacturer = manu;
}

void heimdall::ComponentInfo::setPublisher(const std::string& pub)
{
   publisher = pub;
}

void heimdall::ComponentInfo::markAsProcessed()
{
   wasProcessed = true;
}

void heimdall::ComponentInfo::setProcessingError(const std::string& error)
{
   processingError = error;
}

void heimdall::ComponentInfo::setDetectedBy(LinkerType linker)
{
   detectedBy = linker;
}

void heimdall::ComponentInfo::addProperty(const std::string& key, const std::string& value)
{
   properties[key] = value;
}

std::string heimdall::ComponentInfo::getProperty(const std::string& key) const
{
   auto it = properties.find(key);
   return (it != properties.end()) ? it->second : "";
}

void heimdall::ComponentInfo::markAsSystemLibrary()
{
   isSystemLibrary = true;
}

void heimdall::ComponentInfo::setContainsDebugInfo(bool hasDebug)
{
   containsDebugInfo = hasDebug;
}

void heimdall::ComponentInfo::setStripped(bool stripped)
{
   isStripped = stripped;
}

bool heimdall::ComponentInfo::hasSymbol(const std::string& symbolName) const
{
   return std::any_of(symbols.begin(), symbols.end(), [&symbolName](const SymbolInfo& symbol)
                      { return symbol.name == symbolName; });
}

bool heimdall::ComponentInfo::hasSection(const std::string& sectionName) const
{
   return std::any_of(sections.begin(), sections.end(), [&sectionName](const SectionInfo& section)
                      { return section.name == sectionName; });
}

std::string heimdall::ComponentInfo::getFileTypeString(const std::string& spdxVersion) const
{
   switch (fileType)
   {
      case FileType::Object:
         return "Object";
      case FileType::StaticLibrary:
         return "StaticLibrary";
      case FileType::SharedLibrary:
         return "SharedLibrary";
      case FileType::Executable:
         return "Executable";
      case FileType::Source:
         return "Source";
      case FileType::Unknown:
      default:
         return "Unknown";
   }
}

std::string heimdall::ComponentInfo::getLinkerTypeString() const
{
   switch (detectedBy)
   {
      case LinkerType::LLD:
         return "LLD";
      case LinkerType::Gold:
         return "Gold";
      case LinkerType::BFD:
         return "BFD";
      case LinkerType::Unknown:
      default:
         return "Unknown";
   }
}

}  // namespace heimdall
