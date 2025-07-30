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
 * @file MetadataExtractor.hpp
 * @brief Main class for extracting metadata from binary files and libraries
 * @author Trevor Bakker
 * @date 2025
 * 
 * @deprecated This class is deprecated. Use MetadataExtractor instead.
 * This header provides a compatibility layer for gradual migration.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../compat/compatibility.hpp"
#include "ComponentInfo.hpp"
#include "../extractors/LazySymbolExtractor.hpp"

// Forward declaration to avoid circular includes
namespace heimdall {
    class MetadataExtractor;
}

namespace heimdall
{

/**
 * @brief Main class for extracting metadata from binary files
 *
 * @deprecated This class is deprecated. Use MetadataExtractor instead.
 * This class provides a compatibility layer that delegates to MetadataExtractor.
 *
 * This class provides functionality to extract various types of metadata
 * from binary files including symbols, sections, dependencies, and
 * package manager information.
 */
class MetadataExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   MetadataExtractor();
   /**
    * @brief Destructor
    */
   ~MetadataExtractor();

   /**
    * @brief Extract all metadata from a component
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractMetadata(ComponentInfo& component);

   /**
    * @brief Extract version information from a component
    * @param component The component to extract version from
    * @return true if version was extracted successfully
    */
   bool extractVersionInfo(ComponentInfo& component);

   /**
    * @brief Extract license information from a component
    * @param component The component to extract license from
    * @return true if license was extracted successfully
    */
   bool extractLicenseInfo(ComponentInfo& component);

   /**
    * @brief Extract symbol information from a component
    * @param component The component to extract symbols from
    * @return true if symbols were extracted successfully
    */
   bool extractSymbolInfo(ComponentInfo& component);

   /**
    * @brief Extract section information from a component
    * @param component The component to extract sections from
    * @return true if sections were extracted successfully
    */
   bool extractSectionInfo(ComponentInfo& component);

   /**
    * @brief Extract debug information from a component
    * @param component The component to extract debug info from
    * @return true if debug info was extracted successfully
    */
   bool extractDebugInfo(ComponentInfo& component);

   /**
    * @brief Extract dependency information from a component
    * @param component The component to extract dependencies from
    * @return true if dependencies were extracted successfully
    */
   bool extractDependencyInfo(ComponentInfo& component);

   /**
    * @brief Extract enhanced Mach-O metadata from a component
    * @param component The component to extract enhanced metadata from
    * @return true if enhanced metadata was extracted successfully
    */
   bool extractEnhancedMachOMetadata(ComponentInfo& component);

   /**
    * @brief Extract code signing information from a Mach-O component
    * @param component The component to extract code signing info from
    * @return true if code signing info was extracted successfully
    */
   bool extractMachOCodeSignInfo(ComponentInfo& component);

   /**
    * @brief Extract build configuration from a Mach-O component
    * @param component The component to extract build config from
    * @return true if build configuration was extracted successfully
    */
   bool extractMachOBuildConfig(ComponentInfo& component);

   /**
    * @brief Extract platform information from a Mach-O component
    * @param component The component to extract platform info from
    * @return true if platform info was extracted successfully
    */
   bool extractMachOPlatformInfo(ComponentInfo& component);

   /**
    * @brief Extract entitlements from a Mach-O component
    * @param component The component to extract entitlements from
    * @return true if entitlements were extracted successfully
    */
   bool extractMachOEntitlements(ComponentInfo& component);

   /**
    * @brief Extract architecture information from a Mach-O component
    * @param component The component to extract architecture info from
    * @return true if architecture info was extracted successfully
    */
   bool extractMachOArchitectures(ComponentInfo& component);

   /**
    * @brief Extract framework dependencies from a Mach-O component
    * @param component The component to extract framework dependencies from
    * @return true if framework dependencies were extracted successfully
    */
   bool extractMachOFrameworks(ComponentInfo& component);

   /**
    * @brief Check if a file is in ELF format
    * @param filePath The path to the file
    * @return true if the file is in ELF format
    */
   bool isELF(const std::string& filePath);

   /**
    * @brief Check if a file is in Mach-O format
    * @param filePath The path to the file
    * @return true if the file is in Mach-O format
    */
   bool isMachO(const std::string& filePath);

   /**
    * @brief Check if a file is in PE format
    * @param filePath The path to the file
    * @return true if the file is in PE format
    */
   bool isPE(const std::string& filePath);

   /**
    * @brief Check if a file is an archive
    * @param filePath The path to the file
    * @return true if the file is an archive
    */
   bool isArchive(const std::string& filePath);

   /**
    * @brief Extract Conan package manager metadata
    * @param component The component to extract metadata from
    * @return true if metadata was extracted successfully
    */
   bool extractConanMetadata(ComponentInfo& component);

   /**
    * @brief Extract vcpkg package manager metadata
    * @param component The component to extract metadata from
    * @return true if metadata was extracted successfully
    */
   bool extractVcpkgMetadata(ComponentInfo& component);

   /**
    * @brief Extract system package manager metadata
    * @param component The component to extract metadata from
    * @return true if metadata was extracted successfully
    */
   bool extractSystemMetadata(ComponentInfo& component);

   /**
    * @brief Extract macOS app bundle metadata from Info.plist
    * @param component The component to extract metadata from
    * @return true if metadata was extracted successfully
    */
   bool extractMacOSAppBundleMetadata(ComponentInfo& component);

   /**
    * @brief Detect RPM package manager metadata
    * @param component The component to detect metadata for
    * @return true if RPM metadata was detected
    */
   bool detectRpmMetadata(ComponentInfo& component);

   /**
    * @brief Detect Debian package manager metadata
    * @param component The component to detect metadata for
    * @return true if Debian metadata was detected
    */
   bool detectDebianMetadata(ComponentInfo& component);

   /**
    * @brief Detect Pacman package manager metadata
    * @param component The component to detect metadata for
    * @return true if Pacman metadata was detected
    */
   bool detectPacmanMetadata(ComponentInfo& component);

   /**
    * @brief Extract Ada metadata from ALI files
    * @param component The component to extract metadata from
    * @param aliFiles Vector of ALI file paths
    * @return true if metadata was extracted successfully
    */
   bool extractAdaMetadata(ComponentInfo& component, const std::vector<std::string>& aliFiles);

   /**
    * @brief Check if a file is an Ada ALI file
    * @param filePath The path to the file
    * @return true if the file is an Ada ALI file
    */
   bool isAdaAliFile(const std::string& filePath);

   /**
    * @brief Find Ada ALI files in a directory
    * @param directory The directory to search
    * @param aliFiles Output vector of ALI file paths
    * @return true if ALI files were found
    */
   bool findAdaAliFiles(const std::string& directory, std::vector<std::string>& aliFiles);

   /**
    * @brief Extract metadata from multiple files in batch
    * @param filePaths Vector of file paths to process
    * @param components Output vector of ComponentInfo objects
    * @return true if batch extraction was successful
    */
   bool extractMetadataBatched(const std::vector<std::string>& filePaths,
                               std::vector<ComponentInfo>& components);

   /**
    * @brief Set verbose output mode
    * @param verbose True to enable verbose output
    */
   void setVerbose(bool verbose);

   /**
    * @brief Set whether to extract debug information
    * @param extract True to extract debug information
    */
   void setExtractDebugInfo(bool extract);

   /**
    * @brief Set whether to suppress warnings
    * @param suppress True to suppress warnings
    */
   void setSuppressWarnings(bool suppress);

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

}  // namespace heimdall
