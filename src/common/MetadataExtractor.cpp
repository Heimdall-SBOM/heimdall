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
 * @file MetadataExtractor.cpp
 * @brief Implementation of the MetadataExtractor class for comprehensive binary metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This file implements the MetadataExtractor class, which provides comprehensive
 * metadata extraction capabilities for various binary file formats including ELF,
 * Mach-O, PE, and archive files. It supports extraction of version information,
 * license details, symbol tables, section information, debug data, and dependency
 * analysis.
 *
 * The implementation uses the PIMPL idiom to hide implementation details and
 * provides a clean public interface. It includes platform-specific code for
 * Linux (ELF), macOS (Mach-O), and Windows (PE) binary formats.
 *
 * Key features:
 * - Multi-format binary file support (ELF, Mach-O, PE, archives)
 * - Version detection from multiple sources
 * - License information extraction
 * - Symbol and section analysis
 * - Debug information extraction
 * - Dependency analysis
 * - Package manager metadata detection
 *
 * @see MetadataExtractor.hpp
 * @see ComponentInfo.hpp
 * @see DWARFExtractor.hpp
 */
#include "MetadataExtractor.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdlib>  // for std::getenv
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include "../compat/compatibility.hpp"
#include "AdaExtractor.hpp"
#include "ComponentInfo.hpp"
#include "Utils.hpp"
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
#include <filesystem>
#endif

#if LLVM_DWARF_AVAILABLE
#include "DWARFExtractor.hpp"
#endif

#ifdef __linux__
#include <elf.h>
#include <libelf.h>
#endif

#ifdef __APPLE__
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

// Additional Mach-O constants (only define if not already defined)
#ifndef LC_SOURCE_VERSION
#define LC_SOURCE_VERSION 0x2A
#endif

#ifndef LC_BUILD_VERSION
#define LC_BUILD_VERSION 0x32
#endif

#ifndef LC_VERSION_MIN_MACOSX
#define LC_VERSION_MIN_MACOSX 0x24
#endif

#ifndef LC_VERSION_MIN_IPHONEOS
#define LC_VERSION_MIN_IPHONEOS 0x25
#endif

#ifndef MH_HARDENED_RUNTIME
#define MH_HARDENED_RUNTIME 0x800000
#endif

#ifndef PLATFORM_MACOS
#define PLATFORM_MACOS 1
#endif

#ifndef PLATFORM_IOS
#define PLATFORM_IOS 2
#endif

#ifndef PLATFORM_TVOS
#define PLATFORM_TVOS 3
#endif

#ifndef PLATFORM_WATCHOS
#define PLATFORM_WATCHOS 4
#endif

#endif

namespace heimdall
{

// Thread-safe test mode detection
static std::atomic<bool> g_test_mode{false};
static std::mutex        env_mutex;

/**
 * @brief Private implementation class for MetadataExtractor using PIMPL idiom
 *
 * This class encapsulates the private implementation details of the MetadataExtractor,
 * providing a clean separation between the public interface and internal implementation.
 * It handles file format detection and configuration options.
 */
class MetadataExtractor::Impl
{
   public:
   bool verbose          = false;  ///< Enable verbose output for debugging
   bool extractDebugInfo = true;   ///< Whether to extract debug information (enabled by default)
   bool suppressWarnings = false;

   /**
    * @brief Detect the file format of the given binary file
    * @param filePath Path to the file to analyze
    * @return true if format was successfully detected, false otherwise
    *
    * This method examines the file header to determine if it's an ELF, Mach-O,
    * PE, or archive file. The detected format is stored in the fileFormat member.
    */
   bool        detectFileFormat(const std::string& filePath);

   std::string fileFormat;  ///< Detected file format (ELF, Mach-O, PE, etc.)
};

/**
 * @brief Default constructor for MetadataExtractor
 *
 * Initializes a new MetadataExtractor instance with default settings.
 * Debug information extraction is enabled by default, and verbose output
 * is disabled.
 */
MetadataExtractor::MetadataExtractor() : pImpl(heimdall::compat::make_unique<Impl>()) {}

/**
 * @brief Destructor for MetadataExtractor
 *
 * Cleanly destroys the MetadataExtractor instance and its private implementation.
 * Uses default destructor due to std::unique_ptr automatic cleanup.
 */
MetadataExtractor::~MetadataExtractor() = default;

/**
 * @brief Extract comprehensive metadata from a binary component.
 *
 * This is the main entry point for metadata extraction. It performs a comprehensive
 * analysis of the binary file, extracting all available metadata including:
 *   - File format detection (ELF, Mach-O, PE, archives)
 *   - Version information from multiple sources
 *   - License information
 *   - Symbol table analysis
 *   - Section information
 *   - Debug information (if enabled)
 *   - Dependency analysis
 *   - Package manager metadata detection
 *
 * The method uses a multi-stage approach:
 *   1. File existence and accessibility validation
 *   2. File format detection
 *   3. Basic metadata extraction (version, license, symbols, sections)
 *   4. Debug information extraction (if enabled)
 *   5. Dependency analysis
 *   6. Package manager metadata detection with fallback strategies
 *
 * Package manager detection follows this priority order:
 *   - RPM (Red Hat Package Manager)
 *   - Debian packages
 *   - Conan C++ package manager
 *   - vcpkg package manager
 *   - Spack package manager
 *   - Generic system package detection
 *
 * @param component The ComponentInfo object to populate with extracted metadata.
 * @return true if metadata extraction was successful, false otherwise.
 *
 * @note This method modifies the provided ComponentInfo object in-place.
 * @note Filesystem errors are caught and logged, but don't prevent other extraction attempts.
 * @see ComponentInfo
 * @see MetadataHelpers
 */
bool MetadataExtractor::extractMetadata(ComponentInfo& component)
{
   try
   {
      if (!heimdall::Utils::fileExists(component.filePath))
      {
         heimdall::Utils::errorPrint("File does not exist: " + component.filePath);
         return false;
      }

      // Detect file format
      if (!pImpl->detectFileFormat(component.filePath))
      {
         if (!pImpl->suppressWarnings)
         {
            heimdall::Utils::warningPrint("Could not detect file format for: " +
                                          component.filePath);
         }
      }

      bool success = true;

      // Extract basic metadata (essential for SBOM generation)
      success &= extractVersionInfo(component);
      success &= extractLicenseInfo(component);
      success &= extractSymbolInfo(component);
      success &= extractSectionInfo(component);

      // Extract debug info only if explicitly requested (expensive operation)
      if (pImpl->extractDebugInfo)
      {
         success &= extractDebugInfo(component);
      }

      // Extract dependency info (essential for SBOM)
      success &= extractDependencyInfo(component);

      // Enhanced Mach-O metadata extraction
      if (isMachO(component.filePath))
      {
         heimdall::Utils::debugPrint(
            "MetadataExtractor: Detected Mach-O file, calling enhanced extraction: " +
            component.filePath);
         success &= extractEnhancedMachOMetadata(component);
      }

      // Optimized package manager detection - use path-based detection first
      std::string packageManager = heimdall::Utils::detectPackageManager(component.filePath);
      bool packageManagerDetected = false;

      if (packageManager == "rpm")
      {
         if (heimdall::MetadataHelpers::detectRpmMetadata(component))
         {
            packageManagerDetected = true;
            heimdall::Utils::debugPrint("Detected RPM package metadata");
         }
      }
      else if (packageManager == "deb")
      {
         if (heimdall::MetadataHelpers::detectDebMetadata(component))
         {
            packageManagerDetected = true;
            heimdall::Utils::debugPrint("Detected Debian package metadata");
         }
      }
      else if (packageManager == "conan")
      {
         extractConanMetadata(component);
         packageManagerDetected = true;
         heimdall::Utils::debugPrint("Detected Conan package metadata");
      }
      else if (packageManager == "vcpkg")
      {
         extractVcpkgMetadata(component);
         packageManagerDetected = true;
         heimdall::Utils::debugPrint("Detected vcpkg package metadata");
      }
      else if (packageManager == "spack")
      {
         if (heimdall::MetadataHelpers::detectSpackMetadata(component))
         {
            packageManagerDetected = true;
            heimdall::Utils::debugPrint("Detected Spack package metadata");
         }
      }
      else if (packageManager == "system")
      {
         // For system libraries, just set basic metadata without expensive operations
         component.setPackageManager("system");
         component.markAsSystemLibrary();
         component.setSupplier("system-package-manager");
         packageManagerDetected = true;
      }

      // Try Ada metadata extraction only if Ada files might be present
      std::vector<std::string> aliFiles;

      // Quick check: only search for ALI files if we're in a directory that might contain Ada files
      bool                     shouldSearchForAda = false;
      std::vector<std::string> searchPaths;

#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
      // Use std::filesystem for C++17+
      std::filesystem::path filePath(component.filePath);
      searchPaths = {filePath.parent_path().string(), filePath.parent_path().parent_path().string(),
                     std::filesystem::current_path().string()};
#else
      // Manual path parsing for C++11 compatibility
      std::string filePath  = component.filePath;
      size_t      lastSlash = filePath.find_last_of("/\\");
      if (lastSlash != std::string::npos)
      {
         std::string dirPath = filePath.substr(0, lastSlash);
         searchPaths.push_back(dirPath);

         // Parent directory
         size_t parentSlash = dirPath.find_last_of("/\\");
         if (parentSlash != std::string::npos)
         {
            searchPaths.push_back(dirPath.substr(0, parentSlash));
         }
      }

      // Current directory
      searchPaths.push_back(".");
#endif

      // Performance optimization: Skip Ada file detection by default to avoid directory scanning
      // This was causing exponential slowdown when processing many components
      
      // Only enable Ada detection if explicitly requested or in specific environments
      // This can be controlled via environment variable or configuration
      const char* enableAdaDetection = std::getenv("HEIMDALL_ENABLE_ADA_DETECTION");
      const char* aliFilePath = std::getenv("HEIMDALL_ALI_FILE_PATH");
      
      if (enableAdaDetection && (strcmp(enableAdaDetection, "1") == 0 || 
                                strcmp(enableAdaDetection, "true") == 0))
      {
         // Quick heuristic: check if any Ada-related files exist in the search paths
         for (const auto& searchPath : searchPaths)
         {
            if (searchPath.empty())
               continue;

            // Check for common Ada file patterns in the directory
            bool hasAdaFiles = false;
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
            try
            {
               for (const auto& entry : std::filesystem::directory_iterator(searchPath))
               {
                  if (entry.is_regular_file())
                  {
                     std::string fname = entry.path().filename().string();
                     if (fname.size() > 4 && (fname.substr(fname.size() - 4) == ".adb" ||
                                              fname.substr(fname.size() - 4) == ".ads" ||
                                              fname.substr(fname.size() - 4) == ".ali"))
                     {
                        hasAdaFiles = true;
                        break;
                     }
                  }
               }
            }
            catch (const std::filesystem::filesystem_error&)
            {
               // Ignore filesystem errors, continue with next path
            }
#else
            DIR* dir = opendir(searchPath.c_str());
            if (dir)
            {
               struct dirent* entry;
               while ((entry = readdir(dir)) != nullptr)
               {
                  std::string fname = entry->d_name;
                  if (fname.size() > 4 && (fname.substr(fname.size() - 4) == ".adb" ||
                                           fname.substr(fname.size() - 4) == ".ads" ||
                                           fname.substr(fname.size() - 4) == ".ali"))
                  {
                     hasAdaFiles = true;
                     break;
                  }
               }
               closedir(dir);
            }
#endif
            if (hasAdaFiles)
            {
               shouldSearchForAda = true;
               break;
            }
         }
      }

      // Only search for ALI files if Ada files are present
      if (shouldSearchForAda)
      {
         bool adaMetadataFound = false;
         
         // If a specific ALI file path is provided, search only there
         if (aliFilePath)
         {
            if (findAdaAliFiles(aliFilePath, aliFiles))
            {
               if (extractAdaMetadata(component, aliFiles))
               {
                  if (!packageManagerDetected)
                  {
                     packageManagerDetected = true;
                  }
                  heimdall::Utils::debugPrint("Detected Ada metadata from ALI files in: " +
                                              std::string(aliFilePath));
                  adaMetadataFound = true;
               }
            }
         }
         else
         {
            // Search in the default search paths
            for (const auto& searchPath : searchPaths)
            {
               if (findAdaAliFiles(searchPath, aliFiles))
               {
                  if (extractAdaMetadata(component, aliFiles))
                  {
                     if (!packageManagerDetected)
                     {
                        packageManagerDetected = true;
                     }
                     heimdall::Utils::debugPrint("Detected Ada metadata from ALI files in: " +
                                                 searchPath);
                     adaMetadataFound = true;
                     break;
                  }
               }
            }
         }
      }

      // Enhanced metadata extraction
      extractEnhancedPackageInfo(component);
      
      // Generate description - only if not already set
      if (component.description.empty()) {
         std::string description = generateComponentDescription(component);
         if (!description.empty()) {
            component.setDescription(description);
         }
      }
      
      // Determine scope - only if not already set
      if (component.scope.empty()) {
         std::string scope = determineComponentScope(component);
         if (!scope.empty()) {
            component.setScope(scope);
         }
      }
      
      // Determine MIME type - only if not already set
      if (component.mimeType.empty()) {
         std::string mimeType = determineMimeType(component);
         if (!mimeType.empty()) {
            component.setMimeType(mimeType);
         }
      }
      
      // Extract copyright information (optimized - only reads first 1MB for performance)
      if (component.copyright.empty()) {
         std::string copyright = extractCopyrightInfo(component);
         if (!copyright.empty()) {
            component.setCopyright(copyright);
         }
      }
      
      // Add component evidence
      addComponentEvidence(component);

      return success;
   }
   catch (const std::exception& e)
   {
      heimdall::Utils::errorPrint(std::string("Exception in extractMetadata: ") + e.what());
      return false;
   }
}

bool MetadataExtractor::extractVersionInfo(ComponentInfo& component)
{
   // Try to extract version from file content
   std::string version = MetadataHelpers::detectVersionFromFile(component.filePath);
   if (!version.empty())
   {
      component.setVersion(version);
      return true;
   }

   // Try to extract version from path
   version = MetadataHelpers::detectVersionFromPath(component.filePath);
   if (!version.empty())
   {
      component.setVersion(version);
      return true;
   }

   // Try to extract version from symbols
   version = MetadataHelpers::detectVersionFromSymbols(component.symbols);
   if (!version.empty())
   {
      component.setVersion(version);
      return true;
   }

   return false;
}

bool MetadataExtractor::extractLicenseInfo(ComponentInfo& component)
{
   // Try to detect license from file content
   std::string license = MetadataHelpers::detectLicenseFromFile(component.filePath);
   if (!license.empty())
   {
      component.setLicense(license);
      return true;
   }

   // Try to detect license from path
   license = MetadataHelpers::detectLicenseFromPath(component.filePath);
   if (!license.empty())
   {
      component.setLicense(license);
      return true;
   }

   // Try to detect license from symbols
   license = MetadataHelpers::detectLicenseFromSymbols(component.symbols);
   if (!license.empty())
   {
      component.setLicense(license);
      return true;
   }

   return false;
}

bool MetadataExtractor::extractSymbolInfo(ComponentInfo& component)
{
   // Use lazy symbol extraction with caching
   static LazySymbolExtractor lazyExtractor;

   component.symbols = lazyExtractor.getSymbols(component.filePath);

   // Handle archive members for additional metadata
   if (isArchive(component.filePath))
   {
      std::vector<std::string> members;
      if (MetadataHelpers::extractArchiveMembers(component.filePath, members))
      {
         for (const auto& member : members)
         {
            component.addSourceFile(member);  // Use source files to store member names
         }
         Utils::debugPrint("Extracted " + std::to_string(members.size()) + " archive members");
      }
   }

   return !component.symbols.empty();
}

bool MetadataExtractor::extractSectionInfo(ComponentInfo& component)
{
   if (isELF(component.filePath))
   {
      return MetadataHelpers::extractELFSections(component.filePath, component.sections);
   }
   else if (isMachO(component.filePath))
   {
      return MetadataHelpers::extractMachOSections(component.filePath, component.sections);
   }
   else if (isPE(component.filePath))
   {
      return MetadataHelpers::extractPESections(component.filePath, component.sections);
   }

   return false;
}

bool MetadataExtractor::extractDebugInfo(ComponentInfo& component)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   heimdall::Utils::debugPrint("MetadataExtractor: extractDebugInfo called for " +
                               component.filePath);
#endif
   return MetadataHelpers::extractDebugInfo(component.filePath, component);
}

bool MetadataExtractor::extractDependencyInfo(ComponentInfo& component)
{
   std::vector<std::string> deps = MetadataHelpers::detectDependencies(component.filePath);
   for (const auto& dep : deps)
   {
      component.addDependency(dep);
   }
   component.markAsProcessed();
   return !deps.empty();
}

bool MetadataExtractor::extractEnhancedMachOMetadata(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      heimdall::Utils::debugPrint("Enhanced Mach-O extraction: File is not a Mach-O binary: " +
                                  component.filePath);
      return false;
   }

   heimdall::Utils::debugPrint("Enhanced Mach-O extraction: Starting extraction for: " +
                               component.filePath);
   bool anySuccess = false;

   // Extract all enhanced Mach-O metadata
   bool codeSignSuccess = extractMachOCodeSignInfo(component);
   anySuccess |= codeSignSuccess;

   bool buildConfigSuccess = extractMachOBuildConfig(component);
   if (buildConfigSuccess)
   {
      std::cerr << "[DEBUG] Build config - targetPlatform: " << component.buildConfig.targetPlatform
                << std::endl;
      std::cerr << "[DEBUG] Build config - minOSVersion: " << component.buildConfig.minOSVersion
                << std::endl;
      std::cerr << "[DEBUG] Build config - sdkVersion: " << component.buildConfig.sdkVersion
                << std::endl;
      std::cerr << "[DEBUG] Build config - buildVersion: " << component.buildConfig.buildVersion
                << std::endl;
      std::cerr << "[DEBUG] Build config - sourceVersion: " << component.buildConfig.sourceVersion
                << std::endl;
   }
   anySuccess |= buildConfigSuccess;

   bool platformSuccess = extractMachOPlatformInfo(component);
   std::cerr << "[DEBUG] Platform info extraction: " << (platformSuccess ? "success" : "failed")
             << std::endl;
   anySuccess |= platformSuccess;

   bool entitlementsSuccess = extractMachOEntitlements(component);
   std::cerr << "[DEBUG] Entitlements extraction: " << (entitlementsSuccess ? "success" : "failed")
             << std::endl;
   anySuccess |= entitlementsSuccess;

   bool architecturesSuccess = extractMachOArchitectures(component);

   anySuccess |= architecturesSuccess;

   bool frameworksSuccess = extractMachOFrameworks(component);

   anySuccess |= frameworksSuccess;

   // Update component name and version from enhanced Mach-O metadata
   if (anySuccess)
   {
      // Try to get a better name from the file path (for macOS apps)
      std::string fileName = heimdall::Utils::getFileName(component.filePath);
      if (!fileName.empty() && fileName != component.name)
      {
         // For macOS apps, try to extract the app name from the bundle path
         std::string filePath = component.filePath;
         if (filePath.find(".app/Contents/MacOS/") != std::string::npos)
         {
            // Extract app name from bundle path
            size_t appPos = filePath.find(".app/");
            if (appPos != std::string::npos)
            {
               size_t lastSlash = filePath.rfind('/', appPos);
               if (lastSlash != std::string::npos)
               {
                  std::string appName = filePath.substr(lastSlash + 1, appPos - lastSlash - 1);
                  if (!appName.empty())
                  {
                     component.name = appName;
                  }
               }
            }
         }
      }

      // First priority: Try to extract version from Info.plist for macOS apps
      std::string originalVersion = component.version;

      bool appBundleSuccess        = extractMacOSAppBundleMetadata(component);
      bool versionSetFromInfoPlist = false;

      // If Info.plist parsing was successful and returned a non-empty version, consider it set from
      // Info.plist
      if (appBundleSuccess && !component.version.empty())
      {
         versionSetFromInfoPlist = true;
      }

      // Only use fallback versions if Info.plist didn't provide one
      if (!versionSetFromInfoPlist)
      {
         // Fallback: Try to set version from build config if Info.plist didn't provide one
         if (!component.buildConfig.sourceVersion.empty())
         {
            component.version = component.buildConfig.sourceVersion;
         }
         else if (!component.buildConfig.buildVersion.empty())
         {
            component.version = component.buildConfig.buildVersion;
         }
         else if (!component.buildConfig.minOSVersion.empty())
         {
            // Use minOSVersion as a fallback version only if no Info.plist version was found
            component.version = component.buildConfig.minOSVersion;
         }
      }
   }

   heimdall::Utils::debugPrint("Enhanced Mach-O extraction: Completed with success: " +
                               std::to_string(anySuccess));
   return anySuccess;
}

bool MetadataExtractor::extractMachOCodeSignInfo(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOCodeSignInfo(component.filePath, component.codeSignInfo);
}

bool MetadataExtractor::extractMachOBuildConfig(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOBuildConfig(component.filePath, component.buildConfig);
}

bool MetadataExtractor::extractMachOPlatformInfo(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOPlatformInfo(component.filePath, component.platformInfo);
}

bool MetadataExtractor::extractMachOEntitlements(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOEntitlements(component.filePath, component.entitlements);
}

bool MetadataExtractor::extractMachOArchitectures(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOArchitectures(component.filePath, component.architectures);
}

bool MetadataExtractor::extractMachOFrameworks(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   return MetadataHelpers::extractMachOFrameworks(component.filePath, component.frameworks);
}

bool MetadataExtractor::isELF(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   char magic[4] = {0};  // Initialize to zero
   file.read(magic, 4);

   // Check if we actually read 4 bytes
   if (file.gcount() != 4)
   {
      return false;
   }

   return (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

bool MetadataExtractor::isMachO(const std::string& filePath)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }
   uint32_t magic = 0;  // Initialize to zero
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

   // Check if we actually read the expected number of bytes
   if (file.gcount() != sizeof(magic))
   {
      return false;
   }

   return (magic == MH_MAGIC || magic == MH_MAGIC_64 || magic == MH_CIGAM || magic == MH_CIGAM_64 ||
           magic == FAT_MAGIC || magic == FAT_CIGAM);
#else
   return false;
#endif
}

bool MetadataExtractor::isPE(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check for PE magic number (MZ)
   char magic[2] = {0};  // Initialize to zero
   file.read(magic, 2);
   return file.gcount() == 2 && magic[0] == 'M' && magic[1] == 'Z';
}

bool MetadataExtractor::isArchive(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check for Unix archive magic number (!<arch>)
   char magic[8] = {0};  // Initialize to zero
   file.read(magic, 8);
   return file.gcount() == 8 && strncmp(magic, "!<arch>", 7) == 0;
}

bool MetadataExtractor::extractConanMetadata(heimdall::ComponentInfo& component)
{
   component.setPackageManager("conan");

   // Extract version from path (conan typically includes version in path)
   std::string version = heimdall::Utils::extractVersionFromPath(component.filePath);
   if (!version.empty())
   {
      component.setVersion(version);
   }

   // Extract package name
   std::string packageName = heimdall::Utils::extractPackageName(component.filePath);
   if (!packageName.empty())
   {
      component.setSupplier("conan-center");
      component.setDownloadLocation("https://conan.io/center/" + packageName);
   }

   return true;
}

bool MetadataExtractor::extractVcpkgMetadata(heimdall::ComponentInfo& component)
{
   component.setPackageManager("vcpkg");

   // Extract version from path
   std::string version = heimdall::Utils::extractVersionFromPath(component.filePath);
   if (!version.empty())
   {
      component.setVersion(version);
   }

   // Extract package name
   std::string packageName = heimdall::Utils::extractPackageName(component.filePath);
   if (!packageName.empty())
   {
      component.setSupplier("vcpkg");
      component.setDownloadLocation("https://github.com/microsoft/vcpkg");
   }

   return true;
}

bool MetadataExtractor::extractSystemMetadata(heimdall::ComponentInfo& component)
{
   component.setPackageManager("system");
   component.markAsSystemLibrary();

   // Try Linux package managers first
#ifdef __linux__
   if (heimdall::MetadataHelpers::detectRpmMetadata(component))
   {
      return true;
   }
   if (heimdall::MetadataHelpers::detectDebMetadata(component))
   {
      return true;
   }
   if (heimdall::MetadataHelpers::detectPacmanMetadata(component))
   {
      return true;
   }
#endif

   // Try other package managers
   if (heimdall::MetadataHelpers::detectConanMetadata(component))
   {
      return true;
   }
   if (heimdall::MetadataHelpers::detectVcpkgMetadata(component))
   {
      return true;
   }
   if (heimdall::MetadataHelpers::detectSpackMetadata(component))
   {
      return true;
   }

         // Try to extract version from package manager
      std::string packageName = heimdall::Utils::extractPackageName(component.filePath);
      if (!packageName.empty())
      {
         component.setSupplier("system-package-manager");
      }

      // Enhanced metadata extraction
      extractEnhancedPackageInfo(component);
      
      // Generate description - only if not already set
      if (component.description.empty()) {
         std::string description = generateComponentDescription(component);
         if (!description.empty()) {
            component.setDescription(description);
         }
      }
      
      // Determine scope - only if not already set
      if (component.scope.empty()) {
         std::string scope = determineComponentScope(component);
         if (!scope.empty()) {
            component.setScope(scope);
         }
      }
      
      // Determine MIME type - only if not already set
      if (component.mimeType.empty()) {
         std::string mimeType = determineMimeType(component);
         if (!mimeType.empty()) {
            component.setMimeType(mimeType);
         }
      }
      
      // Extract copyright information (optimized - only reads first 1MB for performance)
      std::string copyright = extractCopyrightInfo(component);
      if (!copyright.empty()) {
         component.setCopyright(copyright);
      }
      
      // Add component evidence
      addComponentEvidence(component);

      return true;
}

bool MetadataExtractor::extractMacOSAppBundleMetadata(ComponentInfo& component)
{
   std::string appVersion, bundleName;

   if (MetadataHelpers::extractMacOSAppBundleInfo(component.filePath, appVersion, bundleName))
   {
      if (!appVersion.empty())
      {
         component.version = appVersion;
      }
      if (!bundleName.empty() && component.name != bundleName)
      {
         component.name = bundleName;
      }
      return true;
   }

   return false;
}

void MetadataExtractor::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
}

void MetadataExtractor::setExtractDebugInfo(bool extract)
{
   pImpl->extractDebugInfo = extract;
}

void MetadataExtractor::setSuppressWarnings(bool suppress)
{
   pImpl->suppressWarnings = suppress;
}

bool MetadataExtractor::extractAdaMetadata(ComponentInfo&                  component,
                                           const std::vector<std::string>& aliFiles)
{
   AdaExtractor adaExtractor;
   adaExtractor.setVerbose(pImpl->verbose);
   adaExtractor.setExtractEnhancedMetadata(true);  // Enable enhanced metadata extraction
   return adaExtractor.extractAdaMetadata(component, aliFiles);
}

bool MetadataExtractor::isAdaAliFile(const std::string& filePath)
{
   return filePath.length() > 4 && filePath.substr(filePath.length() - 4) == ".ali";
}

bool MetadataExtractor::findAdaAliFiles(const std::string&        directory,
                                        std::vector<std::string>& aliFiles)
{
   try
   {
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
      // Use custom directory scanner with timeout protection to avoid hanging
      try
      {
         time_t                   start_time      = time(nullptr);
         const int                timeout_seconds = 30;

         std::vector<std::string> dirs_to_scan = {directory};

         while (!dirs_to_scan.empty())
         {
            // Check timeout
            if (time(nullptr) - start_time > timeout_seconds)
            {
               return false;
            }

            std::string current_dir = dirs_to_scan.back();
            dirs_to_scan.pop_back();

            try
            {
               for (const auto& entry : std::filesystem::directory_iterator(current_dir))
               {
                  // Check timeout for each entry
                  if (time(nullptr) - start_time > timeout_seconds)
                  {
                     return false;
                  }

                  if (entry.is_regular_file() && isAdaAliFile(entry.path().string()))
                  {
                     aliFiles.push_back(entry.path().string());
                  }
                  else if (entry.is_directory())
                  {
                     // Add subdirectory for scanning
                     dirs_to_scan.push_back(entry.path().string());
                  }
               }
            }
            catch (const std::filesystem::filesystem_error& e)
            {
               // Skip problematic directories but continue scanning
               std::cerr << "DEBUG: Skipping problematic directory: " << current_dir << ": "
                         << e.what() << std::endl;
               continue;
            }
         }
      }
      catch (const std::exception& e)
      {
         return false;
      }
#else
      // Portable C++11 directory scanning with timeout protection
      try
      {
         time_t    start_time      = time(nullptr);
         const int timeout_seconds = 30;

         if (pImpl->verbose)
         {
            std::cerr << "DEBUG: Starting portable directory scan for: " << directory << std::endl;
         }

         // Use a stack-based approach to avoid recursion depth issues
         std::vector<std::string> dirs_to_scan = {directory};

         while (!dirs_to_scan.empty())
         {
            // Check timeout
            if (time(nullptr) - start_time > timeout_seconds)
            {
               if (pImpl->verbose)
               {
                  std::cerr << "DEBUG: Timeout searching for ALI files in: " << directory
                            << std::endl;
               }
               return false;
            }

            std::string current_dir = dirs_to_scan.back();
            dirs_to_scan.pop_back();

            try
            {
               // Use basic directory operations that work on all platforms
               DIR* dir = opendir(current_dir.c_str());
               if (!dir)
               {
                  if (pImpl->verbose)
                  {
                     std::cerr << "DEBUG: Failed to open directory: " << current_dir << std::endl;
                  }
                  continue;
               }

               struct dirent* entry;
               while ((entry = readdir(dir)) != nullptr)
               {
                  // Check timeout for each entry
                  if (time(nullptr) - start_time > timeout_seconds)
                  {
                     if (pImpl->verbose)
                     {
                        std::cerr << "DEBUG: Timeout searching for ALI files in: " << directory
                                  << std::endl;
                     }
                     closedir(dir);
                     return false;
                  }

                  std::string entry_name = entry->d_name;

                  // Skip . and ..
                  if (entry_name == "." || entry_name == "..")
                  {
                     continue;
                  }

                  std::string full_path = current_dir + "/" + entry_name;

                  // Check if it's a regular file with .ali extension
                  if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN)
                  {
                     // For DT_UNKNOWN, we need to stat the file
                     if (entry->d_type == DT_UNKNOWN)
                     {
                        struct stat st{};
                        if (stat(full_path.c_str(), &st) == 0)
                        {
                           if (!S_ISREG(st.st_mode))
                           {
                              continue;
                           }
                        }
                        else
                        {
                           continue;
                        }
                     }

                     // Check if it's an ALI file
                     if (isAdaAliFile(full_path))
                     {
                        aliFiles.push_back(full_path);
                        if (pImpl->verbose)
                        {
                           std::cerr << "DEBUG: Found ALI file: " << full_path << std::endl;
                        }
                     }
                  }
                  // Check if it's a directory
                  else if (entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN)
                  {
                     // For DT_UNKNOWN, we need to stat the directory
                     if (entry->d_type == DT_UNKNOWN)
                     {
                        struct stat st{};
                        if (stat(full_path.c_str(), &st) == 0)
                        {
                           if (!S_ISDIR(st.st_mode))
                           {
                              continue;
                           }
                        }
                        else
                        {
                           continue;
                        }
                     }

                     // Add subdirectory for scanning
                     dirs_to_scan.push_back(full_path);
                  }
               }

               closedir(dir);
            }
            catch (const std::exception& e)
            {
               // Skip problematic directories but continue scanning
               if (pImpl->verbose)
               {
                  std::cerr << "DEBUG: Skipping problematic directory: " << current_dir << ": "
                            << e.what() << std::endl;
               }
               continue;
            }
         }

         return true;
      }
      catch (const std::exception& e)
      {
         if (pImpl->verbose)
         {
            std::cerr << "DEBUG: Error searching for ALI files in: " << directory << ": "
                      << e.what() << std::endl;
         }
         return false;
      }
#endif
      return true;
   }
   catch (const std::exception& e)
   {
      if (pImpl->verbose)
      {
         std::cerr << "Error searching for ALI files: " << e.what() << std::endl;
      }
      return false;
   }
}

bool MetadataExtractor::extractMetadataBatched(const std::vector<std::string>& filePaths,
                                               std::vector<ComponentInfo>&     components,
                                               size_t                          batch_size)
{
   try
   {
      // Serial fallback: process files one by one
      components.clear();
      size_t total     = filePaths.size();
      size_t completed = 0;
      for (const auto& filePath : filePaths)
      {
         ComponentInfo component;
         component.filePath = filePath;
         extractMetadata(component);
         components.push_back(std::move(component));
         ++completed;
      }
      return !components.empty();
   }
   catch (const std::exception& e)
   {
      heimdall::Utils::errorPrint("Batched metadata extraction failed: " + std::string(e.what()));
      return false;
   }
}

bool MetadataExtractor::Impl::detectFileFormat(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }
   char magic[16] = {0};  // Initialize to zero
   file.read(magic, sizeof(magic));

   // Check if we actually read enough bytes for the smallest magic number check
   if (file.gcount() < 4)
   {
      fileFormat = "Unknown";
      return false;
   }

   if (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
   {
      fileFormat = "ELF";
      return true;
   }
   else if (magic[0] == 'M' && magic[1] == 'Z')
   {
      fileFormat = "PE";
      return true;
   }
   else if (file.gcount() >= 8 && strncmp(magic, "!<arch>", 7) == 0)
   {
      fileFormat = "Archive";
      return true;
   }
#ifdef __APPLE__
   if (file.gcount() >= 4)
   {
      uint32_t* magic32 = reinterpret_cast<uint32_t*>(magic);
      if (*magic32 == MH_MAGIC || *magic32 == MH_MAGIC_64 || *magic32 == MH_CIGAM ||
          *magic32 == MH_CIGAM_64 || *magic32 == FAT_MAGIC || *magic32 == FAT_CIGAM)
      {
         fileFormat = "Mach-O";
         return true;
      }
   }
#endif
   fileFormat = "Unknown";
   return false;
}

// MetadataHelpers implementation
namespace MetadataHelpers
{

/**
 * @brief Utility function to safely open a file for binary reading
 * @param filePath Path to the file to open
 * @param file Reference to ifstream to open
 * @return true if file opened successfully, false otherwise
 */
bool openFileSafely(const std::string& filePath, std::ifstream& file)
{
   file.open(filePath, std::ios::binary);
   return file.is_open();
}

/**
 * @brief Utility function to safely open a file and return empty string on failure
 * @param filePath Path to the file to open
 * @param file Reference to ifstream to open
 * @return Empty string if file cannot be opened, otherwise empty string (caller should check
 * file.is_open())
 */
std::string openFileOrReturnEmpty(const std::string& filePath, std::ifstream& file)
{
   openFileSafely(filePath, file);
   return "";
}

bool isELF(const std::string& filePath)
{
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return false;
   }

   char magic[4] = {0};  // Initialize to zero
   file.read(magic, 4);

   // Check if we actually read 4 bytes
   if (file.gcount() != 4)
   {
      return false;
   }

   return (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

bool isMachO([[maybe_unused]] const std::string& filePath)
{
#ifdef __APPLE__
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return false;
   }
   uint32_t magic = 0;  // Initialize to zero
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

   // Check if we actually read the expected number of bytes
   if (file.gcount() != sizeof(magic))
   {
      return false;
   }

   return (magic == MH_MAGIC || magic == MH_MAGIC_64 || magic == MH_CIGAM || magic == MH_CIGAM_64 ||
           magic == FAT_MAGIC || magic == FAT_CIGAM);
#else
   return false;
#endif
}

bool isPE(const std::string& filePath)
{
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return false;
   }

   // Check for PE magic number (MZ)
   char magic[2] = {0};  // Initialize to zero
   file.read(magic, 2);
   return file.gcount() == 2 && magic[0] == 'M' && magic[1] == 'Z';
}

bool isArchive(const std::string& filePath)
{
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return false;
   }

   // Check for Unix archive magic number (!<arch>)
   char magic[8] = {0};  // Initialize to zero
   file.read(magic, 8);
   return file.gcount() == 8 && strncmp(magic, "!<arch>", 7) == 0;
}

bool extractELFSymbols(const std::string& filePath, std::vector<heimdall::SymbolInfo>& symbols)
{
#ifdef __linux__
   elf_version(EV_CURRENT);
   // Use libelf for comprehensive ELF parsing
   int fd = open(filePath.c_str(), O_RDONLY);
   if (fd < 0)
   {
      std::cerr << "Failed to open ELF file: " << filePath << std::endl;
      heimdall::Utils::debugPrint("Failed to open ELF file: " + filePath);
      return false;
   }

   Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
   if (!elf)
   {
      close(fd);
      std::cerr << "Failed to open ELF file with libelf: " << filePath << std::endl;
      heimdall::Utils::debugPrint("Failed to open ELF file with libelf: " + filePath);
      return false;
   }

   // Get ELF header
   Elf64_Ehdr* ehdr = elf64_getehdr(elf);
   if (!ehdr)
   {
      elf_end(elf);
      close(fd);
      heimdall::Utils::debugPrint("Failed to get ELF header");
      return false;
   }

   heimdall::Utils::debugPrint("ELF file opened successfully, searching for symbol tables...");

   // Find symbol table sections (both .symtab and .dynsym)
   Elf_Scn*    scn               = nullptr;
   Elf64_Shdr* shdr              = nullptr;
   Elf_Data*   data              = nullptr;
   int         symbolTablesFound = 0;
   int         totalSections     = 0;

   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      shdr = elf64_getshdr(scn);
      if (!shdr)
         continue;

      totalSections++;

      if (shdr->sh_type == SHT_SYMTAB || shdr->sh_type == SHT_DYNSYM)
      {
         symbolTablesFound++;
         heimdall::Utils::debugPrint("Found symbol table section, type: " +
                                     std::to_string(shdr->sh_type));

         data = elf_getdata(scn, nullptr);
         if (!data)
         {
            heimdall::Utils::debugPrint("Failed to get symbol table data");
            continue;
         }

         // Get string table
         Elf_Scn* strscn = elf_getscn(elf, shdr->sh_link);
         if (!strscn)
         {
            heimdall::Utils::debugPrint("Failed to get string table section");
            continue;
         }

         Elf64_Shdr* strshdr = elf64_getshdr(strscn);
         if (!strshdr)
         {
            heimdall::Utils::debugPrint("Failed to get string table header");
            continue;
         }

         Elf_Data* strdata = elf_getdata(strscn, nullptr);
         if (!strdata)
         {
            heimdall::Utils::debugPrint("Failed to get string table data");
            continue;
         }

         char*      strtab = static_cast<char*>(strdata->d_buf);
         Elf64_Sym* symtab = static_cast<Elf64_Sym*>(data->d_buf);
         size_t     nsyms  = data->d_size / sizeof(Elf64_Sym);

         heimdall::Utils::debugPrint("Processing " + std::to_string(nsyms) + " symbols");

         for (size_t i = 0; i < nsyms; ++i)
         {
            Elf64_Sym& sym = symtab[i];

            // Skip null symbols
            if (sym.st_name == 0)
               continue;

            // Skip local symbols unless verbose
            if (ELF64_ST_BIND(sym.st_info) == STB_LOCAL)
               continue;

            // Skip debugging symbols unless requested
            if (ELF64_ST_TYPE(sym.st_info) == STT_FILE)
               continue;

            heimdall::SymbolInfo symbol;
            symbol.name    = strtab + sym.st_name;
            symbol.address = sym.st_value;
            symbol.size    = sym.st_size;
            symbol.isDefined =
               (ELF64_ST_TYPE(sym.st_info) != STT_NOTYPE) && (sym.st_shndx != SHN_UNDEF);
            symbol.isGlobal = (ELF64_ST_BIND(sym.st_info) == STB_GLOBAL);
            symbol.isWeak   = (ELF64_ST_BIND(sym.st_info) == STB_WEAK);

            // Get section name
            if (sym.st_shndx < SHN_LORESERVE)
            {
               Elf_Scn* secscn = elf_getscn(elf, sym.st_shndx);
               if (secscn)
               {
                  Elf64_Shdr* secshdr = elf64_getshdr(secscn);
                  if (secshdr)
                  {
                     // Get section name from string table
                     Elf_Scn* shstrscn = elf_getscn(elf, ehdr->e_shstrndx);
                     if (shstrscn)
                     {
                        Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
                        if (shstrshdr)
                        {
                           Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
                           if (shstrdata)
                           {
                              char* shstrtab = static_cast<char*>(shstrdata->d_buf);
                              symbol.section = shstrtab + secshdr->sh_name;
                           }
                        }
                     }
                  }
               }
            }
            else if (sym.st_shndx == SHN_UNDEF)
            {
               symbol.section = "UNDEF";
            }
            else if (sym.st_shndx == SHN_ABS)
            {
               symbol.section = "ABS";
            }
            else if (sym.st_shndx == SHN_COMMON)
            {
               symbol.section = "COMMON";
            }
            else
            {
               symbol.section = "UNKNOWN";
            }

            symbols.push_back(symbol);
         }
      }
   }

   heimdall::Utils::debugPrint("Found " + std::to_string(symbolTablesFound) + " symbol tables");
   heimdall::Utils::debugPrint("Extracted " + std::to_string(symbols.size()) + " symbols");

   elf_end(elf);
   close(fd);

   return !symbols.empty();
#else
   heimdall::Utils::debugPrint("ELF symbol extraction not supported on this platform");
   return false;
#endif
}

bool extractELFSections(const std::string& filePath, std::vector<heimdall::SectionInfo>& sections)
{
#ifdef __linux__
   elf_version(EV_CURRENT);
   // Use libelf for comprehensive ELF section extraction
   int fd = open(filePath.c_str(), O_RDONLY);
   if (fd < 0)
   {
      heimdall::Utils::debugPrint("Failed to open ELF file: " + filePath);
      return false;
   }

   Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
   if (!elf)
   {
      close(fd);
      heimdall::Utils::debugPrint("Failed to open ELF file with libelf: " + filePath);
      return false;
   }

   // Get ELF header
   Elf64_Ehdr* ehdr = elf64_getehdr(elf);
   if (!ehdr)
   {
      elf_end(elf);
      close(fd);
      return false;
   }

   // Get section header string table
   Elf_Scn* strscn = elf_getscn(elf, ehdr->e_shstrndx);
   if (!strscn)
   {
      elf_end(elf);
      close(fd);
      return false;
   }

   Elf64_Shdr* strshdr = elf64_getshdr(strscn);
   if (!strshdr)
   {
      elf_end(elf);
      close(fd);
      return false;
   }

   Elf_Data* strdata = elf_getdata(strscn, nullptr);
   if (!strdata)
   {
      elf_end(elf);
      close(fd);
      return false;
   }

   char* strtab = static_cast<char*>(strdata->d_buf);

   // Iterate through all sections
   Elf_Scn*    scn  = nullptr;
   Elf64_Shdr* shdr = nullptr;

   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      shdr = elf64_getshdr(scn);
      if (!shdr)
         continue;

      // Skip null sections
      if (shdr->sh_type == SHT_NULL)
         continue;

      heimdall::SectionInfo section;
      section.name    = strtab + shdr->sh_name;
      section.address = shdr->sh_addr;
      section.size    = shdr->sh_size;
      section.type    = std::to_string(shdr->sh_type);
      section.flags   = shdr->sh_flags;

      // Only add meaningful sections
      if (shdr->sh_size > 0 || shdr->sh_type == SHT_NOBITS)
      {
         sections.push_back(section);
      }
   }

   elf_end(elf);
   close(fd);

   return !sections.empty();
#else
   heimdall::Utils::debugPrint("ELF section extraction not supported on this platform");
   return false;
#endif
}

bool extractELFVersion(const std::string& filePath, std::string& version)
{
   // Try to extract version from file content
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Read file content and look for version patterns
   std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

   std::regex  versionRegex(R"((\d+\.\d+\.\d+))");
   std::smatch match;

   if (std::regex_search(content, match, versionRegex))
   {
      version = match[1].str();
      return true;
   }

   return false;
}

bool extractELFBuildId(const std::string& filePath, std::string& buildId)
{
#ifdef __linux__
   elf_version(EV_CURRENT);

   int fd = open(filePath.c_str(), O_RDONLY);
   if (fd < 0)
   {
      heimdall::Utils::debugPrint("Failed to open ELF file for build ID extraction: " + filePath);
      return false;
   }

   Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
   if (!elf)
   {
      close(fd);
      heimdall::Utils::debugPrint("Failed to open ELF file with libelf for build ID: " + filePath);
      return false;
   }

   // Get ELF header
   Elf64_Ehdr* ehdr = elf64_getehdr(elf);
   if (!ehdr)
   {
      elf_end(elf);
      close(fd);
      heimdall::Utils::debugPrint("Failed to get ELF header for build ID extraction");
      return false;
   }

   // Iterate through sections to find .note.gnu.build-id
   Elf_Scn*    scn  = nullptr;
   Elf64_Shdr* shdr = nullptr;

   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      shdr = elf64_getshdr(scn);
      if (!shdr)
         continue;

      if (shdr->sh_type == SHT_NOTE)
      {
         Elf_Data* data = elf_getdata(scn, nullptr);
         if (!data)
            continue;

         // Get section name to check if it's .note.gnu.build-id
         Elf_Scn* shstrscn = elf_getscn(elf, ehdr->e_shstrndx);
         if (!shstrscn)
            continue;

         Elf64_Shdr* shstrshdr = elf64_getshdr(shstrscn);
         if (!shstrshdr)
            continue;

         Elf_Data* shstrdata = elf_getdata(shstrscn, nullptr);
         if (!shstrdata)
            continue;

         char*       shstrtab    = static_cast<char*>(shstrdata->d_buf);
         std::string sectionName = shstrtab + shdr->sh_name;

         if (sectionName == ".note.gnu.build-id")
         {
            // Parse note section to extract build ID
            char*  noteData = static_cast<char*>(data->d_buf);
            size_t noteSize = data->d_size;
            size_t offset   = 0;

            while (offset < noteSize)
            {
               if (offset + 12 > noteSize)
                  break;  // Need at least note header

               // Note header: namesz, descsz, type
               uint32_t namesz = *reinterpret_cast<uint32_t*>(noteData + offset);
               uint32_t descsz = *reinterpret_cast<uint32_t*>(noteData + offset + 4);
               uint32_t type   = *reinterpret_cast<uint32_t*>(noteData + offset + 8);

               // Check if this is a GNU build ID note
               if (type == NT_GNU_BUILD_ID)
               {
                  offset += 12;  // Skip header

                  // Skip name (should be "GNU")
                  offset += (namesz + 3) & ~3;  // Align to 4 bytes

                  // Extract build ID
                  if (offset + descsz <= noteSize)
                  {
                     buildId.reserve(descsz * 2);  // Hex string is 2x size
                     for (uint32_t i = 0; i < descsz; ++i)
                     {
                        char hex[3];
                        snprintf(hex, sizeof(hex), "%02x",
                                 static_cast<unsigned char>(noteData[offset + i]));
                        buildId += hex;
                     }

                     elf_end(elf);
                     close(fd);
                     heimdall::Utils::debugPrint("Extracted build ID: " + buildId);
                     return true;
                  }
               }

               // Move to next note
               offset += 12;                 // Header
               offset += (namesz + 3) & ~3;  // Name (aligned)
               offset += (descsz + 3) & ~3;  // Description (aligned)
            }
         }
      }
   }

   elf_end(elf);
   close(fd);
   heimdall::Utils::debugPrint("No build ID found in ELF file: " + filePath);
   return false;
#else
   heimdall::Utils::debugPrint("ELF build ID extraction not supported on this platform");
   return false;
#endif
}

std::vector<std::string> extractELFDependencies(const std::string& filePath)
{
#ifdef __linux__
   std::vector<std::string> dependencies;

   elf_version(EV_CURRENT);

   int fd = open(filePath.c_str(), O_RDONLY);
   if (fd < 0)
   {
      Utils::debugPrint("Failed to open ELF file for dependency extraction: " + filePath);
      return dependencies;
   }

   Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
   if (!elf)
   {
      close(fd);
      Utils::debugPrint("Failed to open ELF file with libelf for dependencies: " + filePath);
      return dependencies;
   }

   // Get ELF header
   Elf64_Ehdr* ehdr = elf64_getehdr(elf);
   if (!ehdr)
   {
      elf_end(elf);
      close(fd);
      Utils::debugPrint("Failed to get ELF header for dependency extraction");
      return dependencies;
   }

   // Find dynamic section
   Elf_Scn*    scn  = nullptr;
   Elf64_Shdr* shdr = nullptr;

   while ((scn = elf_nextscn(elf, scn)) != nullptr)
   {
      shdr = elf64_getshdr(scn);
      if (!shdr)
         continue;

      if (shdr->sh_type == SHT_DYNAMIC)
      {
         Elf_Data* data = elf_getdata(scn, nullptr);
         if (!data)
            continue;

         // Get string table for dynamic section
         Elf_Scn* strscn = elf_getscn(elf, shdr->sh_link);
         if (!strscn)
            continue;

         Elf64_Shdr* strshdr = elf64_getshdr(strscn);
         if (!strshdr)
            continue;

         Elf_Data* strdata = elf_getdata(strscn, nullptr);
         if (!strdata)
            continue;

         char*      strtab = static_cast<char*>(strdata->d_buf);
         Elf64_Dyn* dyn    = static_cast<Elf64_Dyn*>(data->d_buf);
         size_t     ndyn   = data->d_size / sizeof(Elf64_Dyn);

         for (size_t i = 0; i < ndyn; ++i)
         {
            if (dyn[i].d_tag == DT_NEEDED)
            {
               std::string libName = strtab + dyn[i].d_un.d_val;
               dependencies.push_back(libName);
               Utils::debugPrint("Found dependency: " + libName);
            }
         }
         break;
      }
   }

   elf_end(elf);
   close(fd);

   Utils::debugPrint("Extracted " + std::to_string(dependencies.size()) +
                     " dependencies from ELF file");
   return dependencies;
#else
   Utils::debugPrint("ELF dependency extraction not supported on this platform");
   return std::vector<std::string>();
#endif
}

bool extractMachOSymbols(const std::string& filePath, std::vector<heimdall::SymbolInfo>& symbols)
{
#ifdef __APPLE__
   // Open file
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for symbol extraction: " + filePath);
      return false;
   }

   // Read magic to check for fat or thin
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      // Fat binary: read first architecture only for now
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;
      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
         if (lc.cmd == LC_SYMTAB)
         {
            struct symtab_command symtab{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));
            // Read string table
            std::vector<char> strtab(symtab.strsize);
            file.seekg(symtab.stroff);
            file.read(strtab.data(), symtab.strsize);
            // Read symbol table
            file.seekg(symtab.symoff);
            for (uint32_t j = 0; j < symtab.nsyms; ++j)
            {
               struct nlist_64 nlsym{};
               file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));
               SymbolInfo sym;
               if (nlsym.n_un.n_strx < symtab.strsize)
                  sym.name = &strtab[nlsym.n_un.n_strx];
               else
                  sym.name = "<badstrx>";
               sym.address   = nlsym.n_value;
               sym.size      = 0;  // Mach-O doesn't store symbol size
               sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
               sym.isGlobal  = (nlsym.n_type & N_EXT);
               sym.section   = std::to_string(nlsym.n_sect);
               symbols.push_back(sym);
            }
            break;
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
         if (lc.cmd == LC_SYMTAB)
         {
            struct symtab_command symtab{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));
            std::vector<char> strtab(symtab.strsize);
            file.seekg(symtab.stroff);
            file.read(strtab.data(), symtab.strsize);
            file.seekg(symtab.symoff);
            for (uint32_t j = 0; j < symtab.nsyms; ++j)
            {
               struct nlist nlsym{};
               file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));
               SymbolInfo sym;
               if (nlsym.n_un.n_strx < symtab.strsize)
                  sym.name = &strtab[nlsym.n_un.n_strx];
               else
                  sym.name = "<badstrx>";
               sym.address   = nlsym.n_value;
               sym.size      = 0;
               sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
               sym.isGlobal  = (nlsym.n_type & N_EXT);
               sym.section   = std::to_string(nlsym.n_sect);
               symbols.push_back(sym);
            }
            break;
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   return !symbols.empty();
#else
   Utils::debugPrint("Mach-O symbol extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOSections(const std::string& filePath, std::vector<heimdall::SectionInfo>& sections)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for section extraction: " + filePath);
      return false;
   }
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);
   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }
   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
         if (lc.cmd == LC_SEGMENT_64)
         {
            struct segment_command_64 seg{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&seg), sizeof(seg));
            for (uint32_t j = 0; j < seg.nsects; ++j)
            {
               struct section_64 sect{};
               file.read(reinterpret_cast<char*>(&sect), sizeof(sect));
               SectionInfo s;
               s.name    = sect.sectname;
               s.address = sect.addr;
               s.size    = sect.size;
               s.flags   = sect.flags;
               s.type    = seg.segname;
               sections.push_back(s);
            }
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
         if (lc.cmd == LC_SEGMENT)
         {
            struct segment_command seg{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&seg), sizeof(seg));
            for (uint32_t j = 0; j < seg.nsects; ++j)
            {
               struct section sect{};
               file.read(reinterpret_cast<char*>(&sect), sizeof(sect));
               SectionInfo s;
               s.name    = sect.sectname;
               s.address = sect.addr;
               s.size    = sect.size;
               s.flags   = sect.flags;
               s.type    = seg.segname;
               sections.push_back(s);
            }
         }
         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   return !sections.empty();
#else
   Utils::debugPrint("Mach-O section extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOVersion(const std::string& filePath, std::string& version)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for version extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool        is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
   std::string dylibVersion, sourceVersion, symbolVersion;

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         // Extract dylib version from LC_ID_DYLIB
         if (lc.cmd == LC_ID_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            uint32_t major = dylib_cmd.dylib.current_version >> 16;
            uint32_t minor = (dylib_cmd.dylib.current_version >> 8) & 0xFF;
            uint32_t patch = dylib_cmd.dylib.current_version & 0xFF;

            dylibVersion =
               std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
         }
         // Extract source version from LC_SOURCE_VERSION
         else if (lc.cmd == LC_SOURCE_VERSION)
         {
            struct source_version_command source_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&source_cmd), sizeof(source_cmd));

            uint64_t version  = source_cmd.version;
            uint32_t major    = (version >> 40) & 0xFFFFFF;
            uint32_t minor    = (version >> 30) & 0x3FF;
            uint32_t patch    = (version >> 20) & 0x3FF;
            uint32_t build    = (version >> 10) & 0x3FF;
            uint32_t revision = version & 0x3FF;

            sourceVersion = std::to_string(major) + "." + std::to_string(minor) + "." +
                            std::to_string(patch) + "." + std::to_string(build) + "." +
                            std::to_string(revision);
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         // Extract dylib version from LC_ID_DYLIB
         if (lc.cmd == LC_ID_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            uint32_t major = dylib_cmd.dylib.current_version >> 16;
            uint32_t minor = (dylib_cmd.dylib.current_version >> 8) & 0xFF;
            uint32_t patch = dylib_cmd.dylib.current_version & 0xFF;

            dylibVersion =
               std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
         }
         // Extract source version from LC_SOURCE_VERSION
         else if (lc.cmd == LC_SOURCE_VERSION)
         {
            struct source_version_command source_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&source_cmd), sizeof(source_cmd));

            uint64_t version  = source_cmd.version;
            uint32_t major    = (version >> 40) & 0xFFFFFF;
            uint32_t minor    = (version >> 30) & 0x3FF;
            uint32_t patch    = (version >> 20) & 0x3FF;
            uint32_t build    = (version >> 10) & 0x3FF;
            uint32_t revision = version & 0x3FF;

            sourceVersion = std::to_string(major) + "." + std::to_string(minor) + "." +
                            std::to_string(patch) + "." + std::to_string(build) + "." +
                            std::to_string(revision);
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   // Try to extract version from symbols as fallback
   std::vector<SymbolInfo> symbols;
   if (extractMachOSymbols(filePath, symbols))
   {
      symbolVersion = detectVersionFromSymbols(symbols);
   }

   // Priority: source version > dylib version > symbol version
   if (!sourceVersion.empty())
   {
      version = sourceVersion;
      return true;
   }
   else if (!dylibVersion.empty())
   {
      version = dylibVersion;
      return true;
   }
   else if (!symbolVersion.empty())
   {
      version = symbolVersion;
      return true;
   }

   return false;
#else
   Utils::debugPrint("Mach-O version extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOUUID(const std::string& filePath, std::string& uuid)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for UUID extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_UUID)
         {
            struct uuid_command uuid_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));

            // Format UUID as string
            char uuid_str[37];
            snprintf(uuid_str, sizeof(uuid_str),
                     "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                     uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                     uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                     uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                     uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
            uuid = uuid_str;
            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_UUID)
         {
            struct uuid_command uuid_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));

            char uuid_str[37];
            snprintf(uuid_str, sizeof(uuid_str),
                     "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                     uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                     uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                     uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                     uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
            uuid = uuid_str;
            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return false;
#else
   Utils::debugPrint("Mach-O UUID extraction not supported on this platform");
   return false;
#endif
}

bool extractPESymbols([[maybe_unused]] const std::string& filePath,
                      std::vector<heimdall::SymbolInfo>&  symbols)
{
   // Implementation would use PE parsing libraries
   Utils::debugPrint("PE symbol extraction not implemented");
   return false;
}

bool extractPESections([[maybe_unused]] const std::string& filePath,
                       std::vector<heimdall::SectionInfo>& sections)
{
   // Implementation would use PE parsing libraries
   Utils::debugPrint("PE section extraction not implemented");
   return false;
}

bool extractPEVersion([[maybe_unused]] const std::string& filePath, std::string& version)
{
   // Implementation would use PE parsing libraries
   Utils::debugPrint("PE version extraction not implemented");
   return false;
}

bool extractPECompanyName([[maybe_unused]] const std::string& filePath, std::string& company)
{
   // Implementation would use PE parsing libraries
   Utils::debugPrint("PE company name extraction not implemented");
   return false;
}

bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open archive file: " + filePath);
      return false;
   }

   // Check for Unix archive magic number
   char magic[8];
   file.read(magic, 8);
   if (file.gcount() != 8)
   {
      Utils::debugPrint("Failed to read archive magic number");
      return false;
   }

   // Check for Unix archive format (!<arch>)
   if (strncmp(magic, "!<arch>", 7) != 0)
   {
      Utils::debugPrint("Not a valid Unix archive format");
      return false;
   }

   members.clear();
   file.seekg(8);  // Skip magic number

   while (file.good())
   {
      // Read archive member header (60 bytes)
      char header[60];
      file.read(header, 60);
      if (file.gcount() != 60)
         break;

      // Parse member name (16 bytes, null-padded)
      std::string memberName(header, 16);
      size_t      nullPos = memberName.find('\0');
      if (nullPos != std::string::npos)
      {
         memberName = memberName.substr(0, nullPos);
      }

      // Remove trailing spaces
      while (!memberName.empty() && memberName.back() == ' ')
      {
         memberName.pop_back();
      }

      if (!memberName.empty() && memberName != "/" && memberName != "//")
      {
         members.push_back(memberName);
      }

      // Parse file size (10 bytes, decimal)
      std::string sizeStr(header + 48, 10);
      size_t      fileSize = std::stoul(sizeStr);

      // Skip to next member (file size + header size, aligned to 2 bytes)
      file.seekg((fileSize + 1) & ~1, std::ios::cur);
   }

   Utils::debugPrint("Extracted " + std::to_string(members.size()) + " archive members");
   return !members.empty();
}

/**
 * @brief Parse archive member header and extract member name
 * @param header The 60-byte archive header
 * @return The parsed member name
 */
std::string parseArchiveMemberName(const char* header)
{
   std::string memberName(header, 16);
   size_t      nullPos = memberName.find('\0');
   if (nullPos != std::string::npos)
   {
      memberName = memberName.substr(0, nullPos);
   }

   // Remove trailing spaces
   while (!memberName.empty() && memberName.back() == ' ')
   {
      memberName.pop_back();
   }

   return memberName;
}

/**
 * @brief Parse archive member size from header
 * @param header The 60-byte archive header
 * @return The member size, or 0 if invalid
 */
size_t parseArchiveMemberSize(const char* header)
{
   try
   {
      std::string sizeStr(header + 48, 10);
      size_t      fileSize = std::stoul(sizeStr);
      if (fileSize > 100000000)
      {  // Sanity check
         Utils::debugPrint("Invalid member size: " + std::to_string(fileSize));
         return 0;
      }
      return fileSize;
   }
   catch (const std::exception& e)
   {
      Utils::debugPrint("Exception parsing member size: " + std::string(e.what()));
      return 0;
   }
}

/**
 * @brief Check if archive member is a symbol table
 * @param memberName The member name to check
 * @return true if it's a symbol table, false otherwise
 */
bool isSymbolTableMember(const std::string& memberName)
{
   return memberName == "/" || memberName == "__.SYMDEF";
}

/**
 * @brief Parse symbol table header and validate
 * @param symbolTable The symbol table data
 * @param symbolTableSize Size of the symbol table
 * @param numSymbols Output parameter for number of symbols
 * @param stringTableSize Output parameter for string table size
 * @return true if header is valid, false otherwise
 */
bool parseSymbolTableHeader(const std::vector<char>& symbolTable, size_t symbolTableSize,
                            uint32_t& numSymbols, uint32_t& stringTableSize)
{
   if (symbolTableSize < 8)
   {
      return false;
   }

   numSymbols      = *reinterpret_cast<const uint32_t*>(symbolTable.data());
   stringTableSize = *reinterpret_cast<const uint32_t*>(symbolTable.data() + 4);

   // Sanity checks
   if (numSymbols > 100000 || stringTableSize > symbolTableSize)
   {
      Utils::debugPrint("Invalid symbol table header");
      return false;
   }

   return true;
}

/**
 * @brief Safely extract symbol name from string table
 * @param symbolTable The symbol table data
 * @param symbolOffset Offset of the symbol name
 * @param stringTableSize Size of the string table
 * @param numSymbols Number of symbols
 * @return The symbol name, or empty string if invalid
 */
std::string extractSymbolName(const std::vector<char>& symbolTable, uint32_t symbolOffset,
                              uint32_t stringTableSize, uint32_t numSymbols)
{
   if (symbolOffset >= stringTableSize)
   {
      return "";
   }

   const char* symbolName = symbolTable.data() + 8 + numSymbols * 4 + symbolOffset;
   if (!symbolName)
   {
      return "";
   }

   // Safe string length calculation with bounds checking
   size_t maxLength  = stringTableSize - symbolOffset;
   size_t nameLength = 0;

   // Find string length safely
   for (size_t i = 0; i < maxLength && i < 1000; ++i)
   {
      if (symbolName[i] == '\0')
      {
         nameLength = i;
         break;
      }
   }

   // Check if we found a valid null-terminated string
   if (nameLength > 0 && nameLength < 1000)
   {
      return std::string(symbolName, nameLength);
   }

   return "";
}

/**
 * @brief Parse symbol table and extract symbols
 * @param symbolTable The symbol table data
 * @param symbolTableSize Size of the symbol table
 * @param symbols Output vector to store extracted symbols
 * @return true if symbols were extracted successfully, false otherwise
 */
bool parseSymbolTable(const std::vector<char>& symbolTable, size_t symbolTableSize,
                      std::vector<heimdall::SymbolInfo>& symbols)
{
   uint32_t numSymbols, stringTableSize;
   if (!parseSymbolTableHeader(symbolTable, symbolTableSize, numSymbols, stringTableSize))
   {
      return false;
   }

   size_t offset = 8;  // Skip header

   // Read symbol offsets
   for (uint32_t i = 0; i < numSymbols && offset + 4 <= symbolTableSize; ++i)
   {
      uint32_t symbolOffset = *reinterpret_cast<const uint32_t*>(symbolTable.data() + offset);
      offset += 4;

      std::string symbolName =
         extractSymbolName(symbolTable, symbolOffset, stringTableSize, numSymbols);
      if (!symbolName.empty())
      {
         heimdall::SymbolInfo symbol;
         symbol.name      = symbolName;
         symbol.address   = symbolOffset;
         symbol.size      = 0;  // Archive symbols don't have size info
         symbol.isDefined = true;
         symbol.isGlobal  = true;
         symbols.push_back(symbol);
      }
   }

   return true;
}

bool extractArchiveSymbols(const std::string& filePath, std::vector<heimdall::SymbolInfo>& symbols)
{
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      Utils::debugPrint("Failed to open archive file: " + filePath);
      return false;
   }

   // Check for Unix archive magic number
   char magic[8];
   file.read(magic, 8);
   if (file.gcount() != 8 || strncmp(magic, "!<arch>", 7) != 0)
   {
      Utils::debugPrint("Not a valid Unix archive format");
      return false;
   }

   symbols.clear();
   file.seekg(8);  // Skip magic number

   // Find symbol table (usually first member)
   while (file.good())
   {
      char header[60];
      file.read(header, 60);
      if (file.gcount() != 60)
      {
         break;
      }

      std::string memberName = parseArchiveMemberName(header);

      // Check if this is the symbol table
      if (isSymbolTableMember(memberName))
      {
         try
         {
            // Parse symbol table size
            std::string sizeStr(header + 48, 10);
            size_t      symbolTableSize = std::stoul(sizeStr);

            if (symbolTableSize == 0 || symbolTableSize > 1000000)
            {  // Sanity check
               Utils::debugPrint("Invalid symbol table size: " + std::to_string(symbolTableSize));
               break;
            }

            // Read symbol table
            std::vector<char> symbolTable(symbolTableSize);
            file.read(symbolTable.data(), symbolTableSize);

            if (file.gcount() == symbolTableSize)
            {
               parseSymbolTable(symbolTable, symbolTableSize, symbols);
            }
         }
         catch (const std::exception& e)
         {
            Utils::debugPrint("Exception parsing symbol table: " + std::string(e.what()));
            break;
         }
         break;
      }

      // Skip to next member
      size_t fileSize = parseArchiveMemberSize(header);
      if (fileSize == 0)
      {
         break;
      }
      file.seekg((fileSize + 1) & ~1, std::ios::cur);
   }

   Utils::debugPrint("Extracted " + std::to_string(symbols.size()) + " archive symbols");
   return !symbols.empty();
}

bool extractDebugInfo(const std::string& filePath, heimdall::ComponentInfo& component)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   heimdall::Utils::debugPrint("MetadataHelpers: Starting extractDebugInfo for " + filePath);
#endif

#if LLVM_DWARF_AVAILABLE
   // Use the new robust DWARF extractor with optimized single-context extraction
   heimdall::DWARFExtractor dwarfExtractor;

   // Extract all debug information using a single DWARF context
   std::vector<std::string> sourceFiles;
   std::vector<std::string> compileUnits;
   std::vector<std::string> functions;

#ifdef HEIMDALL_DEBUG_ENABLED
   heimdall::Utils::debugPrint(
      "MetadataHelpers: Calling extractAllDebugInfo (optimized single-context extraction)");
#endif

   if (dwarfExtractor.extractAllDebugInfo(filePath, sourceFiles, compileUnits, functions))
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      heimdall::Utils::debugPrint("MetadataHelpers: extractAllDebugInfo returned true");
      heimdall::Utils::debugPrint("MetadataHelpers: Found " + std::to_string(sourceFiles.size()) +
                                  " source files");
      heimdall::Utils::debugPrint("MetadataHelpers: Found " + std::to_string(compileUnits.size()) +
                                  " compile units");
      heimdall::Utils::debugPrint("MetadataHelpers: Found " + std::to_string(functions.size()) +
                                  " functions");
#endif

      // Add source files to component
      for (const auto& sourceFile : sourceFiles)
      {
         component.addSourceFile(sourceFile);
      }

      // Add compile units to component
      for (const auto& unit : compileUnits)
      {
         component.compileUnits.push_back(unit);
      }

      // Add functions to component
      for (const auto& function : functions)
      {
         component.functions.push_back(function);
      }

      component.setContainsDebugInfo(true);
#ifdef HEIMDALL_DEBUG_ENABLED
      heimdall::Utils::debugPrint("MetadataHelpers: Setting containsDebugInfo to true");
#endif
      return true;
   }
   else
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      heimdall::Utils::debugPrint("MetadataHelpers: extractAllDebugInfo returned false");
#endif
   }
#else
   heimdall::Utils::debugPrint(
      "MetadataHelpers: DWARF support not available, skipping debug info extraction");
#endif

#ifdef HEIMDALL_DEBUG_ENABLED
   heimdall::Utils::debugPrint("MetadataHelpers: No debug info found, returning false");
#endif
   return false;
}

bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles)
{
#if LLVM_DWARF_AVAILABLE
   // Use the new robust DWARF extractor
   heimdall::DWARFExtractor dwarfExtractor;
   return dwarfExtractor.extractSourceFiles(filePath, sourceFiles);
#else
   heimdall::Utils::debugPrint(
      "MetadataHelpers: DWARF support not available, skipping source file extraction");
   return false;
#endif
}

bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& units)
{
#if LLVM_DWARF_AVAILABLE
   // Use the new robust DWARF extractor
   heimdall::DWARFExtractor dwarfExtractor;
   return dwarfExtractor.extractCompileUnits(filePath, units);
#else
   heimdall::Utils::debugPrint(
      "MetadataHelpers: DWARF support not available, skipping compile unit extraction");
   return false;
#endif
}

std::string detectLicenseFromFile(const std::string& filePath)
{
   // Try to find license information in the file
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return "";
   }

   // Read a portion of the file to look for license strings
   std::string content;
   content.resize(4096);
   file.read(&content[0], content.size());
   content.resize(file.gcount());

   // Convert to string for regex search
   std::string textContent(content.begin(), content.end());

   // Look for common license patterns
   std::vector<std::pair<std::regex, std::string>> licensePatterns = {
      {std::regex(R"(GPL|GNU General Public License)", std::regex::icase), "GPL"},
      {std::regex(R"(LGPL|GNU Lesser General Public License)", std::regex::icase), "LGPL"},
      {std::regex(R"(MIT License|MIT)", std::regex::icase), "MIT"},
      {std::regex(R"(Apache License|Apache)", std::regex::icase), "Apache"},
      {std::regex(R"(BSD License|BSD)", std::regex::icase), "BSD"},
      {std::regex(R"(MPL|Mozilla Public License)", std::regex::icase), "MPL"}};

   for (const auto& pattern : licensePatterns)
   {
      if (std::regex_search(textContent, pattern.first))
      {
         return pattern.second;
      }
   }

   return "";
}

std::string detectLicenseFromPath(const std::string& filePath)
{
   // Try to detect license from directory structure
   std::string normalizedPath = heimdall::Utils::normalizePath(filePath);

   if (normalizedPath.find("gpl") != std::string::npos)
   {
      return "GPL";
   }
   else if (normalizedPath.find("lgpl") != std::string::npos)
   {
      return "LGPL";
   }
   else if (normalizedPath.find("mit") != std::string::npos)
   {
      return "MIT";
   }
   else if (normalizedPath.find("apache") != std::string::npos)
   {
      return "Apache";
   }
   else if (normalizedPath.find("bsd") != std::string::npos)
   {
      return "BSD";
   }

   return "";
}

std::string detectLicenseFromSymbols(const std::vector<heimdall::SymbolInfo>& symbols)
{
   // Look for license-related symbols
   for (const auto& symbol : symbols)
   {
      std::string lowerName = heimdall::Utils::toLower(symbol.name);
      if (lowerName.find("gpl") != std::string::npos)
      {
         return "GPL";
      }
      else if (lowerName.find("lgpl") != std::string::npos)
      {
         return "LGPL";
      }
      else if (lowerName.find("mit") != std::string::npos)
      {
         return "MIT";
      }
      else if (lowerName.find("apache") != std::string::npos)
      {
         return "Apache";
      }
      else if (lowerName.find("bsd") != std::string::npos)
      {
         return "BSD";
      }
   }

   return "";
}

std::string detectVersionFromFile(const std::string& filePath)
{
   std::ifstream file;
   if (!openFileSafely(filePath, file))
   {
      return "";
   }

   // Read file content and look for version patterns
   std::string content;
   content.resize(8192);  // Increased buffer size for better coverage
   file.read(&content[0], content.size());
   content.resize(file.gcount());

   // Convert to string for regex search
   std::string textContent(content.begin(), content.end());

   // Enhanced version patterns
   std::vector<std::regex> versionPatterns = {
      std::regex(R"((\d+\.\d+\.\d+))"),                                  // 1.2.3
      std::regex(R"((\d+\.\d+))"),                                       // 1.2
      std::regex(R"(version[:\s]*(\d+\.\d+\.\d+))", std::regex::icase),  // version: 1.2.3
      std::regex(R"(v[:\s]*(\d+\.\d+\.\d+))", std::regex::icase),        // v: 1.2.3
      std::regex(R"((\d+\.\d+\.\d+\.\d+))"),                             // 1.2.3.4
      std::regex(R"(release[:\s]*(\d+\.\d+\.\d+))", std::regex::icase),  // release: 1.2.3
      std::regex(R"(build[:\s]*(\d+\.\d+\.\d+))", std::regex::icase),    // build: 1.2.3
   };

   for (const auto& pattern : versionPatterns)
   {
      std::smatch match;
      if (std::regex_search(textContent, match, pattern))
      {
         return match[1].str();
      }
   }

   return "";
}

std::string detectVersionFromPath(const std::string& filePath)
{
   return heimdall::Utils::extractVersionFromPath(filePath);
}

std::string detectVersionFromSymbols(const std::vector<heimdall::SymbolInfo>& symbols)
{
   // Look for version-related symbols
   for (const auto& symbol : symbols)
   {
      std::string lowerName = heimdall::Utils::toLower(symbol.name);

      // Common version symbol patterns
      std::vector<std::regex> versionPatterns = {
         std::regex(R"((\d+\.\d+\.\d+))"),                                  // 1.2.3
         std::regex(R"((\d+\.\d+))"),                                       // 1.2
         std::regex(R"(version[_\s]*(\d+\.\d+\.\d+))", std::regex::icase),  // version_1.2.3
         std::regex(R"(v[_\s]*(\d+\.\d+\.\d+))", std::regex::icase),        // v_1.2.3
         std::regex(R"(ver[_\s]*(\d+\.\d+\.\d+))", std::regex::icase),      // ver_1.2.3
         std::regex(R"(lib[_\s]*(\d+\.\d+\.\d+))", std::regex::icase),      // lib_1.2.3
      };

      for (const auto& pattern : versionPatterns)
      {
         std::smatch match;
         if (std::regex_search(lowerName, match, pattern))
         {
            return match[1].str();
         }
      }

      // Check for common version symbol names
      if (lowerName.find("version") != std::string::npos ||
          lowerName.find("_ver") != std::string::npos || lowerName.find("_v") != std::string::npos)
      {
         // Extract version from the symbol name
         std::regex  versionExtract(R"((\d+\.\d+\.\d+))");
         std::smatch match;
         if (std::regex_search(lowerName, match, versionExtract))
         {
            return match[1].str();
         }
      }
   }

   return "";
}

// Package Manager Integration Functions
bool detectRpmMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect RPM package information
   std::string filePath = component.filePath;
   std::string fileName = heimdall::Utils::getFileName(filePath);
   std::string dirName  = heimdall::Utils::getDirectory(filePath);

   // Look for RPM-specific paths
   if (dirName.find("/usr/lib/rpm") != std::string::npos ||
       dirName.find("/var/lib/rpm") != std::string::npos)
   {
      // Set package manager to rpm
      component.setPackageManager("rpm");

      // Try to extract version from filename
      std::string version = heimdall::Utils::extractVersionFromPath(fileName);
      if (!version.empty())
      {
         component.setVersion(version);
      }

      // Try to extract package name
      std::regex  rpmPattern(R"(([a-zA-Z0-9_-]+)-(\d+\.\d+\.\d+))");
      std::smatch match;
      if (std::regex_search(fileName, match, rpmPattern))
      {
         component.name = match[1].str();
         if (version.empty())
         {
            component.setVersion(match[2].str());
         }
      }

      return true;
   }

   return false;
}

bool detectDebMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect Debian package information
   std::string filePath = component.filePath;
   std::string fileName = heimdall::Utils::getFileName(filePath);
   std::string dirName  = heimdall::Utils::getDirectory(filePath);

   // Look for Debian-specific paths
   if (dirName.find("/usr/lib/x86_64-linux-gnu") != std::string::npos ||
       dirName.find("/usr/lib/aarch64-linux-gnu") != std::string::npos ||
       dirName.find("/usr/lib/arm-linux-gnueabihf") != std::string::npos)
   {
      // Set package manager to deb
      component.setPackageManager("deb");

      // Try to extract version from filename
      std::string version = heimdall::Utils::extractVersionFromPath(fileName);
      if (!version.empty())
      {
         component.setVersion(version);
      }

      // Try to extract package name
      std::regex  debPattern(R"(([a-zA-Z0-9_-]+)-(\d+\.\d+\.\d+))");
      std::smatch match;
      if (std::regex_search(fileName, match, debPattern))
      {
         component.name = match[1].str();
         if (version.empty())
         {
            component.setVersion(match[2].str());
         }
      }

      return true;
   }

   return false;
}

bool detectPacmanMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect Pacman package information
   std::string filePath = component.filePath;
   std::string fileName = heimdall::Utils::getFileName(filePath);
   std::string dirName  = heimdall::Utils::getDirectory(filePath);

   // Look for Pacman-specific paths
   if (dirName.find("/usr/lib/pacman") != std::string::npos ||
       dirName.find("/var/lib/pacman") != std::string::npos)
   {
      // Set package manager to pacman
      component.setPackageManager("pacman");

      // Try to extract version from filename
      std::string version = heimdall::Utils::extractVersionFromPath(fileName);
      if (!version.empty())
      {
         component.setVersion(version);
      }

      // Try to extract package name
      std::regex  pacmanPattern(R"(([a-zA-Z0-9_-]+)-(\d+\.\d+\.\d+))");
      std::smatch match;
      if (std::regex_search(fileName, match, pacmanPattern))
      {
         component.name = match[1].str();
         if (version.empty())
         {
            component.setVersion(match[2].str());
         }
      }

      return true;
   }

   return false;
}

bool detectConanMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect Conan package information
   std::string filePath = component.filePath;
   std::string dirName  = heimdall::Utils::getDirectory(filePath);

   // Look for Conan package directories
   if (dirName.find("/.conan") != std::string::npos || dirName.find("conan") != std::string::npos)
   {
      // Parse Conan package path structure
      std::vector<std::string> pathParts = heimdall::Utils::splitPath(dirName);

      for (size_t i = 0; i < pathParts.size(); ++i)
      {
         if (pathParts[i] == "conan" && i + 2 < pathParts.size())
         {
            component.name = pathParts[i + 1];
            component.setVersion(pathParts[i + 2]);
            component.setPackageManager("conan");
            component.setSupplier("conan-center");
            return true;
         }
      }

      // For test cases or simpler conan paths, just detect the presence of "conan"
      if (dirName.find("conan") != std::string::npos)
      {
         component.setPackageManager("conan");
         component.setSupplier("conan-center");
         // Try to extract package name from filename
         std::string fileName    = heimdall::Utils::getFileName(filePath);
         std::string packageName = heimdall::Utils::extractPackageName(filePath);
         if (!packageName.empty())
         {
            component.name = packageName;
         }
         return true;
      }
   }

   return false;
}

bool detectVcpkgMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect vcpkg package information
   std::string filePath = component.filePath;
   std::string dirName  = heimdall::Utils::getDirectory(filePath);

   // Look for vcpkg package directories
   if (dirName.find("vcpkg") != std::string::npos || dirName.find("installed") != std::string::npos)
   {
      // Parse vcpkg package path structure
      std::vector<std::string> pathParts = heimdall::Utils::splitPath(dirName);

      for (size_t i = 0; i < pathParts.size(); ++i)
      {
         if (pathParts[i] == "installed" && i + 1 < pathParts.size())
         {
            std::string packageInfo = pathParts[i + 1];

            // Parse package name and version
            size_t underscorePos = packageInfo.find('_');
            if (underscorePos != std::string::npos)
            {
               component.name = packageInfo.substr(0, underscorePos);
               component.setVersion(packageInfo.substr(underscorePos + 1));
               component.setPackageManager("vcpkg");
               component.setSupplier("vcpkg");
               return true;
            }
         }
      }

      // For test cases or simpler vcpkg paths, just detect the presence of "vcpkg"
      if (dirName.find("vcpkg") != std::string::npos)
      {
         component.setPackageManager("vcpkg");
         component.setSupplier("vcpkg");
         // Try to extract package name from filename
         std::string packageName = heimdall::Utils::extractPackageName(filePath);
         if (!packageName.empty())
         {
            component.name = packageName;
         }
         return true;
      }
   }

   return false;
}

bool detectSpackMetadata(heimdall::ComponentInfo& component)
{
   // Try to detect Spack package information
   std::string filePath = component.filePath;
   std::string dirName  = heimdall::Utils::getDirectory(filePath);
   std::string fileName = heimdall::Utils::getFileName(filePath);

   // Look for Spack package directories
   if (dirName.find("spack") != std::string::npos || dirName.find("opt/spack") != std::string::npos)
   {
      // Parse Spack package path structure
      std::vector<std::string> pathParts = heimdall::Utils::splitPath(dirName);

      for (size_t i = 0; i < pathParts.size(); ++i)
      {
         if (pathParts[i] == "spack" && i + 2 < pathParts.size())
         {
            component.name = pathParts[i + 1];
            component.setVersion(pathParts[i + 2]);
            component.setPackageManager("spack");
            component.setSupplier("spack");
            return true;
         }
      }

      // For test cases or simpler spack paths, just detect the presence of "spack"
      if (dirName.find("spack") != std::string::npos)
      {
         component.setPackageManager("spack");
         component.setSupplier("spack");
         // Try to extract package name from filename
         std::string packageName = heimdall::Utils::extractPackageName(filePath);
         if (!packageName.empty())
         {
            component.name = packageName;
         }
         return true;
      }
   }

   return false;
}

bool isAdaAliFile(const std::string& filePath)
{
   return filePath.length() > 4 && filePath.substr(filePath.length() - 4) == ".ali";
}

// Thread-safe test mode control
void setTestMode(bool enabled)
{
   g_test_mode.store(enabled, std::memory_order_release);
}

bool isTestMode()
{
   return g_test_mode.load(std::memory_order_acquire);
}

bool findAdaAliFiles(const std::string& directory, std::vector<std::string>& aliFiles)
{
   // Skip Ada ALI file search in test environment to avoid hanging
   if (isTestMode())
   {
      std::cerr << "DEBUG: Skipping Ada ALI file search in test mode for: " << directory
                << std::endl;
      return true;
   }

   try
   {
      // Enhanced directory scanning with timeout and error handling
      std::string command =
         "/usr/bin/timeout 30s /usr/bin/find " + directory + " -name \"*.ali\" -type f 2>/dev/null";

      std::cerr << "DEBUG: Starting findAdaAliFiles for directory: " << directory << std::endl;
      std::cerr << "DEBUG: Command: " << command << std::endl;
      std::cerr << "DEBUG: About to call popen with command: " << command << std::endl;

      FILE* pipe = popen(command.c_str(), "r");
      if (!pipe)
      {
         std::cerr << "DEBUG: popen failed for directory: " << directory << " with errno: " << errno
                   << std::endl;
         return false;
      }
      std::cerr << "DEBUG: popen succeeded, pipe fd: " << fileno(pipe) << std::endl;

      // Set non-blocking mode for timeout handling
      int fd    = fileno(pipe);
      int flags = fcntl(fd, F_GETFL, 0);
      fcntl(fd, F_SETFL, flags | O_NONBLOCK);

      char      buffer[1024];
      time_t    start_time      = time(nullptr);
      const int timeout_seconds = 30;

      while (true)
      {
         // Check timeout
         if (time(nullptr) - start_time > timeout_seconds)
         {
            std::cerr << "DEBUG: Timeout searching for ALI files in: " << directory << std::endl;
            pclose(pipe);
            return false;
         }

         // Try to read from pipe
         char* result = fgets(buffer, sizeof(buffer), pipe);
         if (result == nullptr)
         {
            // Check if it's just a temporary error (non-blocking mode)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
               // Sleep briefly and try again
               usleep(10000);  // 10ms
               continue;
            }
            // End of file or error
            break;
         }

         std::string line(buffer);
         // Remove newline
         if (!line.empty() && line[line.length() - 1] == '\n')
         {
            line.erase(line.length() - 1);
         }
         if (!line.empty())
         {
            aliFiles.push_back(line);
            std::cerr << "DEBUG: Found ALI file: " << line << std::endl;
         }
      }

      std::cerr << "DEBUG: About to call pclose on pipe fd: " << fileno(pipe) << std::endl;
      int status = pclose(pipe);
      std::cerr << "DEBUG: pclose returned status: " << status << std::endl;
      if (status != 0)
      {
         std::cerr << "DEBUG: Find command exited with status: " << status << std::endl;
      }

      return true;
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error searching for ALI files: " << e.what() << std::endl;
      return false;
   }
}

bool extractAdaMetadata(const std::vector<std::string>& aliFiles,
                        heimdall::ComponentInfo&        component)
{
   heimdall::AdaExtractor adaExtractor;
   adaExtractor.setExtractEnhancedMetadata(true);  // Enable enhanced metadata extraction
   return adaExtractor.extractAdaMetadata(component, aliFiles);
}

std::vector<std::string> detectDependencies(const std::string& filePath)
{
   std::vector<std::string> dependencies;

   // Extract dynamic dependencies
   auto dynamicDeps = extractDynamicDependencies(filePath);
   dependencies.insert(dependencies.end(), dynamicDeps.begin(), dynamicDeps.end());

   // Extract static dependencies
   auto staticDeps = extractStaticDependencies(filePath);
   dependencies.insert(dependencies.end(), staticDeps.begin(), staticDeps.end());

   return dependencies;
}

std::vector<std::string> extractDynamicDependencies(const std::string& filePath)
{
   std::vector<std::string> dependencies;

   // Use platform-specific extraction
#ifdef __linux__
   if (isELF(filePath))
   {
      dependencies = extractELFDependencies(filePath);
   }
#endif

#ifdef __APPLE__
   if (isMachO(filePath))
   {
      dependencies = MetadataHelpers::extractMachOLinkedLibraries(filePath);
   }
#endif

   return dependencies;
}

std::vector<std::string> extractStaticDependencies(const std::string& filePath)
{
   std::vector<std::string> dependencies;

   // This would parse static library dependencies
   // For now, return empty vector
   heimdall::Utils::debugPrint("Static dependency extraction not implemented");

   return dependencies;
}

std::vector<std::string> extractMachOLinkedLibraries(const std::string& filePath)
{
   std::vector<std::string> libraries;

#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for library extraction: " + filePath);
      return libraries;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         // Check for dynamic library load commands
         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            // Read library name
            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            if (!libName.empty())
            {
               libraries.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            if (!libName.empty())
            {
               libraries.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
#else
   heimdall::Utils::debugPrint("Mach-O library extraction not supported on this platform");
#endif

   return libraries;
}

bool extractMachOCodeSignInfo(const std::string& filePath, CodeSignInfo& codeSignInfo)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for code signing extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_CODE_SIGNATURE)
         {
            struct linkedit_data_command code_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&code_cmd), sizeof(code_cmd));

            // Basic code signing info extraction
            // Note: Full code signing parsing would require parsing the CMS signature
            codeSignInfo.isAdHocSigned     = true;  // Default assumption
            codeSignInfo.isHardenedRuntime = (mh.flags & MH_HARDENED_RUNTIME) != 0;

            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_CODE_SIGNATURE)
         {
            struct linkedit_data_command code_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&code_cmd), sizeof(code_cmd));

            codeSignInfo.isAdHocSigned     = true;
            codeSignInfo.isHardenedRuntime = (mh.flags & MH_HARDENED_RUNTIME) != 0;

            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return false;
#else
   Utils::debugPrint("Mach-O code signing extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOBuildConfig(const std::string& filePath, BuildConfigInfo& buildConfig)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for build config extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_BUILD_VERSION)
         {
            struct build_version_command build_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&build_cmd), sizeof(build_cmd));

            // Extract platform info
            switch (build_cmd.platform)
            {
               case PLATFORM_MACOS:
                  buildConfig.targetPlatform = "macos";
                  break;
               case PLATFORM_IOS:
                  buildConfig.targetPlatform = "ios";
                  break;
               case PLATFORM_TVOS:
                  buildConfig.targetPlatform = "tvos";
                  break;
               case PLATFORM_WATCHOS:
                  buildConfig.targetPlatform = "watchos";
                  break;
               default:
                  buildConfig.targetPlatform = "unknown";
                  break;
            }

            // Extract version info
            uint32_t major = (build_cmd.minos >> 16) & 0xFFFF;
            uint32_t minor = (build_cmd.minos >> 8) & 0xFF;
            uint32_t patch = build_cmd.minos & 0xFF;
            buildConfig.minOSVersion =
               std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

            major = (build_cmd.sdk >> 16) & 0xFFFF;
            minor = (build_cmd.sdk >> 8) & 0xFF;
            patch = build_cmd.sdk & 0xFF;
            buildConfig.sdkVersion =
               std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_VERSION_MIN_MACOSX || lc.cmd == LC_VERSION_MIN_IPHONEOS)
         {
            struct version_min_command version_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&version_cmd), sizeof(version_cmd));

            if (lc.cmd == LC_VERSION_MIN_MACOSX)
            {
               buildConfig.targetPlatform = "macos";
            }
            else
            {
               buildConfig.targetPlatform = "ios";
            }

            uint32_t major = (version_cmd.version >> 16) & 0xFFFF;
            uint32_t minor = (version_cmd.version >> 8) & 0xFF;
            uint32_t patch = version_cmd.version & 0xFF;
            buildConfig.minOSVersion =
               std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

            return true;
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return false;
#else
   Utils::debugPrint("Mach-O build config extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOPlatformInfo(const std::string& filePath, PlatformInfo& platformInfo)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for platform info extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

      // Extract architecture
      switch (mh.cputype)
      {
         case CPU_TYPE_X86_64:
            platformInfo.architecture = "x86_64";
            break;
         case CPU_TYPE_ARM64:
            platformInfo.architecture = "arm64";
            break;
         case CPU_TYPE_ARM:
            platformInfo.architecture = "arm";
            break;
         default:
            platformInfo.architecture = "unknown";
            break;
      }

      // Extract platform from flags
      if (mh.flags & MH_HARDENED_RUNTIME)
      {
         platformInfo.platform = "macos";  // Hardened runtime is typically macOS
      }

      return true;
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

      // Extract architecture
      switch (mh.cputype)
      {
         case CPU_TYPE_X86:
            platformInfo.architecture = "x86";
            break;
         case CPU_TYPE_ARM:
            platformInfo.architecture = "arm";
            break;
         default:
            platformInfo.architecture = "unknown";
            break;
      }

      return true;
   }

   return false;
#else
   Utils::debugPrint("Mach-O platform info extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOEntitlements(const std::string& filePath, std::vector<std::string>& entitlements)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for entitlements extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_CODE_SIGNATURE)
         {
            struct linkedit_data_command code_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&code_cmd), sizeof(code_cmd));

            // Note: Full entitlements parsing would require parsing the code signature
            // For now, we'll add basic entitlements based on file flags
            if (mh.flags & MH_HARDENED_RUNTIME)
            {
               entitlements.push_back("com.apple.security.hardened-runtime");
            }

            return !entitlements.empty();
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_CODE_SIGNATURE)
         {
            struct linkedit_data_command code_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&code_cmd), sizeof(code_cmd));

            if (mh.flags & MH_HARDENED_RUNTIME)
            {
               entitlements.push_back("com.apple.security.hardened-runtime");
            }

            return !entitlements.empty();
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   return false;
#else
   Utils::debugPrint("Mach-O entitlements extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOArchitectures(const std::string&             filePath,
                               std::vector<ArchitectureInfo>& architectures)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for architecture extraction: " + filePath);
      return false;
   }

   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);

      for (uint32_t i = 0; i < nfat_arch; ++i)
      {
         struct fat_arch arch{};
         file.read(reinterpret_cast<char*>(&arch), sizeof(arch));

         ArchitectureInfo archInfo;
         archInfo.cpuType    = OSSwapBigToHostInt32(arch.cputype);
         archInfo.cpuSubtype = OSSwapBigToHostInt32(arch.cpusubtype);
         archInfo.offset     = OSSwapBigToHostInt32(arch.offset);
         archInfo.size       = OSSwapBigToHostInt32(arch.size);
         archInfo.align      = OSSwapBigToHostInt32(arch.align);

         // Set architecture name
         switch (archInfo.cpuType)
         {
            case CPU_TYPE_X86_64:
               archInfo.name = "x86_64";
               break;
            case CPU_TYPE_ARM64:
               archInfo.name = "arm64";
               break;
            case CPU_TYPE_ARM:
               archInfo.name = "arm";
               break;
            case CPU_TYPE_X86:
               archInfo.name = "x86";
               break;
            default:
               archInfo.name = "unknown";
               break;
         }

         architectures.push_back(archInfo);
      }

      return !architectures.empty();
   }
   else
   {
      // Single architecture
      bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

      if (is64)
      {
         struct mach_header_64 mh{};
         file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

         ArchitectureInfo archInfo;
         archInfo.cpuType    = mh.cputype;
         archInfo.cpuSubtype = mh.cpusubtype;
         archInfo.offset     = 0;
         archInfo.size       = 0;  // Would need to calculate from file size
         archInfo.align      = 0;

         switch (archInfo.cpuType)
         {
            case CPU_TYPE_X86_64:
               archInfo.name = "x86_64";
               break;
            case CPU_TYPE_ARM64:
               archInfo.name = "arm64";
               break;
            case CPU_TYPE_ARM:
               archInfo.name = "arm";
               break;
            default:
               archInfo.name = "unknown";
               break;
         }

         architectures.push_back(archInfo);
      }
      else
      {
         struct mach_header mh{};
         file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

         ArchitectureInfo archInfo;
         archInfo.cpuType    = mh.cputype;
         archInfo.cpuSubtype = mh.cpusubtype;
         archInfo.offset     = 0;
         archInfo.size       = 0;
         archInfo.align      = 0;

         switch (archInfo.cpuType)
         {
            case CPU_TYPE_X86:
               archInfo.name = "x86";
               break;
            case CPU_TYPE_ARM:
               archInfo.name = "arm";
               break;
            default:
               archInfo.name = "unknown";
               break;
         }

         architectures.push_back(archInfo);
      }

      return !architectures.empty();
   }

   return false;
#else
   Utils::debugPrint("Mach-O architecture extraction not supported on this platform");
   return false;
#endif
}

bool extractMachOFrameworks(const std::string& filePath, std::vector<std::string>& frameworks)
{
#ifdef __APPLE__
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      Utils::debugPrint("Failed to open Mach-O file for framework extraction: " + filePath);
      return false;
   }

   // Handle fat binaries
   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   // Read Mach-O header
   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      // Iterate load commands
      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         // Check for framework load commands
         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            // Read library name
            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            // Check if it's a framework
            if (!libName.empty() && libName.find(".framework") != std::string::npos)
            {
               frameworks.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
      uint32_t ncmds = mh.ncmds;

      for (uint32_t i = 0; i < ncmds; ++i)
      {
         std::streampos      cmdStart = file.tellg();
         struct load_command lc{};
         file.read(reinterpret_cast<char*>(&lc), sizeof(lc));

         if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB ||
             lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB)
         {
            struct dylib_command dylib_cmd{};
            file.seekg(cmdStart);
            file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));

            std::string libName;
            char        ch;
            file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
            while (file.get(ch) && ch != '\0')
            {
               libName += ch;
            }

            if (!libName.empty() && libName.find(".framework") != std::string::npos)
            {
               frameworks.push_back(libName);
            }
         }

         file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
      }
   }

   return !frameworks.empty();
#else
   Utils::debugPrint("Mach-O framework extraction not supported on this platform");
   return false;
#endif
}

bool extractMacOSAppBundleInfo(const std::string& appBundlePath, std::string& version,
                               std::string& bundleName)
{
   std::string infoPlistPath;

   // Detect if we have an executable path within an app bundle
   if (appBundlePath.find(".app/Contents/MacOS/") != std::string::npos)
   {
      // Extract the .app bundle path
      size_t appPos = appBundlePath.find(".app/Contents/MacOS/");
      if (appPos != std::string::npos)
      {
         std::string appBundleRoot = appBundlePath.substr(0, appPos + 4);  // Include ".app"
         infoPlistPath             = appBundleRoot + "/Contents/Info.plist";
      }
   }
   else if (appBundlePath.find(".app") != std::string::npos)
   {
      // Assume it's already the app bundle path
      infoPlistPath = appBundlePath + "/Contents/Info.plist";
   }
   else
   {
      return false;
   }

   std::ifstream file(infoPlistPath);
   if (!file.is_open())
   {
      return false;
   }

   std::string line;
   bool        foundVersionKey = false;
   bool        foundNameKey    = false;

   while (std::getline(file, line))
   {
      // Look for CFBundleShortVersionString key
      if (line.find("<key>CFBundleShortVersionString</key>") != std::string::npos)
      {
         foundVersionKey = true;
         continue;
      }

      // Look for CFBundleName key
      if (line.find("<key>CFBundleName</key>") != std::string::npos)
      {
         foundNameKey = true;
         continue;
      }

      // Extract version value after finding the key
      if (foundVersionKey && line.find("<string>") != std::string::npos)
      {
         size_t start = line.find("<string>") + 8;
         size_t end   = line.find("</string>");
         if (end != std::string::npos && end > start)
         {
            version         = line.substr(start, end - start);
            foundVersionKey = false;
         }
      }

      // Extract bundle name value after finding the key
      if (foundNameKey && line.find("<string>") != std::string::npos)
      {
         size_t start = line.find("<string>") + 8;
         size_t end   = line.find("</string>");
         if (end != std::string::npos && end > start)
         {
            bundleName   = line.substr(start, end - start);
            foundNameKey = false;
         }
      }

      // Stop if we found both
      if (!version.empty() && !bundleName.empty())
      {
         break;
      }
   }

   return !version.empty() || !bundleName.empty();
}

}  // namespace MetadataHelpers

// Enhanced metadata extraction methods

std::string MetadataExtractor::generateComponentDescription(const ComponentInfo& component)
{
   std::stringstream desc;
   desc << component.getFileTypeString() << " file";
   
   if (!component.symbols.empty()) {
      desc << " with " << component.symbols.size() << " symbols";
   }
   
   if (component.containsDebugInfo) {
      desc << ", contains debug information";
   }
   
   if (component.isSystemLibrary) {
      desc << ", system library";
   }
   
   if (!component.packageManager.empty()) {
      desc << ", managed by " << component.packageManager;
   }
   
   if (component.fileSize > 0) {
      desc << " (" << component.fileSize << " bytes)";
   }
   
   return desc.str();
}

std::string MetadataExtractor::determineComponentScope(const ComponentInfo& component)
{
   if (component.isSystemLibrary) {
      return "excluded";  // System libraries are excluded
   }
   
   // Could implement more sophisticated logic based on usage analysis
   return "required";
}

std::string MetadataExtractor::extractComponentGroup(const ComponentInfo& component)
{
   // Extract group from package manager metadata
   if (!component.packageManager.empty()) {
      if (component.packageManager == "rpm") {
         return "rpm";
      } else if (component.packageManager == "deb") {
         return "deb";
      } else if (component.packageManager == "conan") {
         return "conan";
      } else if (component.packageManager == "vcpkg") {
         return "vcpkg";
      }
   }
   
   // Extract from package name patterns
   std::string name = component.name;
   size_t dotPos = name.find('.');
   if (dotPos != std::string::npos) {
      return name.substr(0, dotPos);
   }
   
   return "";
}

std::string MetadataExtractor::determineMimeType(const ComponentInfo& component)
{
   // Determine MIME type based on file type and extension
   switch (component.fileType) {
      case FileType::Executable:
         return "application/x-executable";
      case FileType::SharedLibrary:
         return "application/x-sharedlib";
      case FileType::StaticLibrary:
         return "application/x-archive";
      case FileType::Object:
         return "application/x-object";
      case FileType::Source:
         return "text/x-source";
      default:
         return "application/octet-stream";
   }
}

std::string MetadataExtractor::extractCopyrightInfo(const ComponentInfo& component)
{
   // Try to extract copyright from binary strings (optimized for performance)
   std::ifstream file(component.filePath, std::ios::binary);
   if (!file.is_open()) {
      return "";
   }
   
   // For large files, only read the first 1MB to avoid performance issues
   // Copyright information is typically in headers or early in the file
   const size_t maxReadSize = 1024 * 1024; // 1MB
   
   file.seekg(0, std::ios::end);
   size_t fileSize = file.tellg();
   size_t readSize = std::min(fileSize, maxReadSize);
   
   if (readSize == 0) {
      return "";
   }
   
   std::string content;
   content.resize(readSize);
   file.seekg(0, std::ios::beg);
   file.read(&content[0], readSize);
   
   // Look for copyright patterns
   std::regex copyrightPattern(R"((Copyright|©)\s*[0-9]{4}[-\s]*[0-9]{4}?\s*[^\n\r]*?)", std::regex::icase);
   std::smatch match;
   
   if (std::regex_search(content, match, copyrightPattern)) {
      return match.str();
   }
   
   return "";
}

void MetadataExtractor::extractEnhancedPackageInfo(ComponentInfo& component)
{
   // Extract supplier from package manager - only if not already set
   if (component.supplier.empty()) {
      if (component.packageManager == "rpm") {
         component.setSupplier("Red Hat Package Manager");
      } else if (component.packageManager == "deb") {
         component.setSupplier("Debian Package Manager");
      } else if (component.packageManager == "conan") {
         component.setSupplier("Conan Center");
      } else if (component.packageManager == "vcpkg") {
         component.setSupplier("vcpkg");
      } else if (component.packageManager == "spack") {
         component.setSupplier("Spack");
      }
      // Set supplier for heimdall-sbom executable
      else if (component.fileType == FileType::Executable && 
               (component.name == "heimdall-sbom" || component.filePath.find("heimdall-sbom") != std::string::npos)) {
         component.setSupplier("Heimdall Project");
      }
   }
   
   // Extract group from package name - only if not already set
   if (component.group.empty()) {
      std::string group = extractComponentGroup(component);
      if (!group.empty()) {
         component.setGroup(group);
      }
   }
   
   // Set manufacturer same as supplier - only if not already set
   if (component.manufacturer.empty() && !component.supplier.empty()) {
      component.setManufacturer(component.supplier);
   }
}

void MetadataExtractor::addComponentEvidence(ComponentInfo& component)
{
   // Add identity evidence (removed duplicate hash - already in component.checksum)
   component.addProperty("evidence:identity:symbols", std::to_string(component.symbols.size()));
   component.addProperty("evidence:identity:sections", std::to_string(component.sections.size()));
   
   // Add occurrence evidence
   component.addProperty("evidence:occurrence:location", component.filePath);
   component.addProperty("evidence:occurrence:size", std::to_string(component.fileSize));
   
   // Add file type evidence
   component.addProperty("evidence:identity:fileType", component.getFileTypeString());
   
   // Add debug info evidence
   component.addProperty("evidence:identity:hasDebugInfo", component.containsDebugInfo ? "true" : "false");
   component.addProperty("evidence:identity:isStripped", component.isStripped ? "true" : "false");
}

}  // namespace heimdall

