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
 * @file AdaExtractor.cpp
 * @brief Implementation of Ada ALI file parser for extracting metadata
 * @author Trevor Bakker
 * @date 2025
 */

#include "AdaExtractor.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include "Utils.hpp"
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
#include <filesystem>
#endif

namespace heimdall
{

static std::atomic<bool> g_test_mode{false};
static std::mutex        env_mutex;

void                     setTestMode(bool enabled)
{
   g_test_mode.store(enabled, std::memory_order_release);
}

bool isTestMode()
{
   return g_test_mode.load(std::memory_order_acquire);
}

AdaExtractor::AdaExtractor()
{
   // Initialize known runtime packages
   runtimePackages = {"ada", "system", "interfaces", "gnat", "a-", "s-", "i-"};

   // Initialize known security-related flags
   securityFlags = {
      "NO_EXCEPTION_HANDLERS",   "NO_EXCEPTIONS",          "NO_DEFAULT_INITIALIZATION",
      "NO_IMPLICIT_DEREFERENCE", "NO_IMPLICIT_CONVERSION", "NO_IMPLICIT_OVERRIDE",
      "NO_IMPLICIT_RETURN",      "NO_IMPLICIT_OVERRIDE",   "NO_IMPLICIT_OVERRIDE",
      "NO_IMPLICIT_OVERRIDE",    "NO_IMPLICIT_OVERRIDE",   "NO_IMPLICIT_OVERRIDE"};

   // Initialize known optimization flags
   optimizationFlags = {"O0", "O1", "O2", "O3", "Os", "Ofast", "Og", "O0", "O1", "O2", "O3"};
}

AdaExtractor::~AdaExtractor() = default;

bool AdaExtractor::extractAdaMetadata(ComponentInfo&                  component,
                                      const std::vector<std::string>& aliFiles)
{
   if (aliFiles.empty())
   {
      Utils::errorPrint("No ALI files provided for Ada metadata extraction");
      return false;
   }

   std::vector<AdaPackageInfo>        packages;
   AdaBuildInfo                       buildInfo;
   std::vector<AdaFunctionInfo>       allFunctions;
   std::vector<AdaCrossReference>     allCrossRefs;
   std::vector<AdaTypeInfo>           allTypes;
   std::vector<std::string>           allSecurityFlags;
   std::map<std::string, std::string> allTimestamps;
   std::map<std::string, std::string> allChecksums;

   bool                               hasValidFiles = false;
   // Parse each ALI file
   for (const auto& aliFile : aliFiles)
   {
      AdaPackageInfo packageInfo;
      if (parseAliFile(aliFile, packageInfo))
      {
         packages.push_back(packageInfo);
         hasValidFiles = true;

         // Extract enhanced metadata from this ALI file
         std::ifstream file(aliFile);
         if (file.is_open())
         {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            file.close();

            // Extract functions
            std::vector<AdaFunctionInfo> functions;
            if (extractFunctions(content, functions))
            {
               allFunctions.insert(allFunctions.end(), functions.begin(), functions.end());
            }

            // Extract enhanced metadata if enabled
            if (extractEnhancedMetadata)
            {
               // Extract cross-references
               std::vector<AdaCrossReference> crossRefs;
               if (extractCrossReferences(content, crossRefs))
               {
                  allCrossRefs.insert(allCrossRefs.end(), crossRefs.begin(), crossRefs.end());
               }

               // Extract type information
               std::vector<AdaTypeInfo> types;
               if (extractTypeInfo(content, types))
               {
                  allTypes.insert(allTypes.end(), types.begin(), types.end());
               }

               // Extract security flags
               std::vector<std::string> securityFlags;
               if (extractSecurityFlags(content, securityFlags))
               {
                  allSecurityFlags.insert(allSecurityFlags.end(), securityFlags.begin(),
                                          securityFlags.end());
               }

               // Extract file timestamps and checksums
               std::map<std::string, std::string> timestamps, checksums;
               if (extractFileInfo(content, timestamps, checksums))
               {
                  allTimestamps.insert(timestamps.begin(), timestamps.end());
                  allChecksums.insert(checksums.begin(), checksums.end());
               }
            }
         }
      }
   }

   // Extract build info from the first ALI file
   if (!aliFiles.empty())
   {
      std::ifstream file(aliFiles[0]);
      if (file.is_open())
      {
         std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
         file.close();
         extractBuildInfo(content, buildInfo);
      }
   }

   // Update component with Ada-specific metadata
   component.setPackageManager("GNAT");

   // Add Ada functions to component
   for (const auto& func : allFunctions)
   {
      component.functions.push_back(func.name + "(" + func.signature + ")");
   }

   // Add Ada packages as dependencies
   for (const auto& pkg : packages)
   {
      for (const auto& dep : pkg.dependencies)
      {
         component.addDependency(dep);
      }
      if (!pkg.sourceFile.empty())
      {
         component.addSourceFile(pkg.sourceFile);
      }
   }

   // Also add source files from dependencies mentioned in ALI files
   std::set<std::string> allSourceFiles;
   for (const auto& aliFile : aliFiles)
   {
      std::ifstream file(aliFile);
      if (file.is_open())
      {
         std::string line;
         while (std::getline(file, line))
         {
            // Parse W and Z lines for source files
            if ((line.substr(0, 2) == "W " || line.substr(0, 2) == "Z ") &&
                !isRuntimePackage(extractPackageName(aliFile)))
            {
               std::istringstream lineStream(line.substr(2));
               std::string        packagePart, sourceFile, aliFile;
               if (lineStream >> packagePart >> sourceFile >> aliFile)
               {
                  if (!sourceFile.empty())
                  {
                     allSourceFiles.insert(sourceFile);
                  }
               }
            }
         }
         file.close();
      }
   }

   // Add unique source files to component
   for (const auto& sourceFile : allSourceFiles)
   {
      component.addSourceFile(sourceFile);
   }

   // Set version from build info
   if (!buildInfo.compilerVersion.empty())
   {
      component.setVersion(buildInfo.compilerVersion);
   }

   // Add enhanced metadata to component properties
   if (extractEnhancedMetadata)
   {
      // Add security flags
      if (!allSecurityFlags.empty())
      {
         std::string securityFlagsStr;
         for (const auto& flag : allSecurityFlags)
         {
            if (!securityFlagsStr.empty())
               securityFlagsStr += ", ";
            securityFlagsStr += flag;
         }
         component.addProperty("security.buildFlags", securityFlagsStr);
      }

      // Add function call graph
      if (!allCrossRefs.empty())
      {
         std::string callGraph = generateCallGraph(allCrossRefs);
         component.addProperty("functions.calls", callGraph);
      }

      // Add type system information
      if (!allTypes.empty())
      {
         std::string typesStr;
         for (const auto& type : allTypes)
         {
            if (!typesStr.empty())
               typesStr += ", ";
            typesStr += type.name + "{" + type.baseType + "}";
         }
         component.addProperty("types.variables", typesStr);
      }

      // Add build timestamps
      if (!allTimestamps.empty())
      {
         std::string timestampsStr;
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
         for (const auto& [file, timestamp] : allTimestamps)
         {
#else
         for (const auto& timestampEntry : allTimestamps)
         {
            const auto& file      = timestampEntry.first;
            const auto& timestamp = timestampEntry.second;
#endif
            if (!timestampsStr.empty())
               timestampsStr += ", ";
            timestampsStr += file + ": " + timestamp;
         }
         component.addProperty("build.timestamps", timestampsStr);
      }

      // Add build checksums
      if (!allChecksums.empty())
      {
         std::string checksumsStr;
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
         for (const auto& [file, checksum] : allChecksums)
         {
#else
         for (const auto& checksumEntry : allChecksums)
         {
            const auto& file     = checksumEntry.first;
            const auto& checksum = checksumEntry.second;
#endif
            if (!checksumsStr.empty())
               checksumsStr += ", ";
            checksumsStr += file + ": " + checksum;
         }
         component.addProperty("build.checksums", checksumsStr);
      }

      // Add runtime flags
      if (!buildInfo.runtimeFlags.empty())
      {
         std::string runtimeFlagsStr;
         for (const auto& flag : buildInfo.runtimeFlags)
         {
            if (!runtimeFlagsStr.empty())
               runtimeFlagsStr += ", ";
            runtimeFlagsStr += flag;
         }
         component.addProperty("security.runtimeFlags", runtimeFlagsStr);
      }

      // Add compiler version
      if (!buildInfo.compilerVersion.empty())
      {
         component.addProperty("security.compilerVersion", buildInfo.compilerVersion);
      }
   }

   Utils::infoPrint("Extracted Ada metadata: " + std::to_string(packages.size()) + " packages, "
               + std::to_string(allFunctions.size()) + " functions");
   if (extractEnhancedMetadata)
   {
      Utils::infoPrint("Enhanced Extracted Metadata: " + std::to_string(allCrossRefs.size()) 
               + " cross-references, " + std::to_string(allTypes.size()) + "types," + std::to_string(allSecurityFlags.size()) + " security flags");
   }
   return hasValidFiles;
}

bool AdaExtractor::parseAliFile(const std::string& aliFilePath, AdaPackageInfo& packageInfo)
{
   std::ifstream file(aliFilePath);
   if (!file.is_open())
   {
      Utils::errorPrint("Failed to open ALI file: " + aliFilePath);
      return false;
   }

   packageInfo.aliFile    = aliFilePath;
   packageInfo.name       = extractPackageName(aliFilePath);
   packageInfo.sourceFile = extractSourceFilePath(aliFilePath);

   bool        hasValidContent = false;
   std::string line;
   while (std::getline(file, line))
   {
      // Parse version line
      if (line.substr(0, 2) == "V ")
      {
         AdaBuildInfo buildInfo;
         parseVersionLine(line, buildInfo);
         hasValidContent = true;
      }
      // Parse dependency lines (W lines)
      else if (line.substr(0, 2) == "W ")
      {
         parseDependencyLine(line, packageInfo);
         hasValidContent = true;
      }
      // Parse runtime dependency lines (Z lines)
      else if (line.substr(0, 2) == "Z ")
      {
         parseDependencyLine(line, packageInfo);
         hasValidContent = true;
      }
      // Parse function lines (X lines with V*)
      else if (line.substr(0, 2) == "X " && line.find("V*") != std::string::npos)
      {
         std::vector<AdaFunctionInfo> functions;
         parseFunctionLine(line, functions);
         for (const auto& func : functions)
         {
            packageInfo.functions.push_back(func.name);
         }
         hasValidContent = true;
      }
      // Parse variable lines (X lines with a*)
      else if (line.substr(0, 2) == "X " && line.find("a*") != std::string::npos)
      {
         std::vector<std::string> variables;
         parseVariableLine(line, variables);
         packageInfo.variables.insert(packageInfo.variables.end(), variables.begin(),
                                      variables.end());
         hasValidContent = true;
      }
      // Parse type lines (X lines with i*)
      else if (line.substr(0, 2) == "X " && line.find("i*") != std::string::npos)
      {
         std::vector<std::string> types;
         parseTypeLine(line, types);
         packageInfo.types.insert(packageInfo.types.end(), types.begin(), types.end());
         hasValidContent = true;
      }
   }

   file.close();
   packageInfo.isRuntime       = isRuntimePackage(packageInfo.name);
   packageInfo.isSpecification = packageInfo.sourceFile.find(".ads") != std::string::npos;

   return hasValidContent;
}

bool AdaExtractor::extractDependencies(const std::string&        content,
                                       std::vector<std::string>& dependencies)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "W ")
      {
         // Parse dependency line: W package_name%spec_or_body source_file.ads/adb source_file.ali
         std::istringstream lineStream(line.substr(2));
         std::string        packagePart;
         lineStream >> packagePart;

         // Extract package name (before the %)
         size_t percentPos = packagePart.find('%');
         if (percentPos != std::string::npos)
         {
            std::string packageName = packagePart.substr(0, percentPos);
            dependencies.push_back(packageName);
         }
      }
   }

   return !dependencies.empty();
}

bool AdaExtractor::extractFunctions(const std::string&            content,
                                    std::vector<AdaFunctionInfo>& functions)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "X " && line.find("V*") != std::string::npos)
      {
         parseFunctionLine(line, functions);
      }
   }

   return !functions.empty();
}

bool AdaExtractor::extractBuildInfo(const std::string& content, AdaBuildInfo& buildInfo)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "V ")
      {
         parseVersionLine(line, buildInfo);
      }
      else if (line.substr(0, 3) == "RV ")
      {
         // Runtime flags
         std::string flag = line.substr(3);
         buildInfo.runtimeFlags.push_back(flag);
      }
   }

   return true;
}

bool AdaExtractor::isAliFile(const std::string& filePath)
{
   return filePath.length() > 4 && filePath.substr(filePath.length() - 4) == ".ali";
}

// Thread-safe test mode control
void AdaExtractor::setTestMode(bool enabled)
{
   heimdall::setTestMode(enabled);
}

bool AdaExtractor::isTestMode()
{
   return heimdall::isTestMode();
}

bool AdaExtractor::findAliFiles(const std::string& directory, std::vector<std::string>& aliFiles)
{
   // Skip Ada ALI file search in test environment to avoid hanging
   if (isTestMode())
   {
      Utils::errorPrint("AdaExtractor: Skipping Ada ALI file search in test mode for: " + directory);
      return true;
   }

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
               Utils::errorPrint("AdaExtractor: Timeout searching for ALI files in: " + directory);
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
                     Utils::errorPrint("AdaExtractor: Timeout searching for ALI files in: " + directory);
                     return false;
                  }

                  if (entry.is_regular_file() && isAliFile(entry.path().string()))
                  {
                     aliFiles.push_back(entry.path().string());
                     Utils::infoPrint("AdaExtractor: Found ALI file: " + entry.path().string());
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
               Utils::errorPrint("AdaExtractor: Skipping problematic directory: " + current_dir);
               continue;
            }
         }
      }
      catch (const std::exception& e)
      {
         Utils::errorPrint("AdaExtractor: Error searching for ALI files in: " + directory);
         return false;
      }
#else
      // Portable C++11 directory scanning with timeout protection
      try
      {
         time_t    start_time      = time(nullptr);
         const int timeout_seconds = 30;

         Utils::infoPrint("AdaExtractor: Starting portable directory scan for: " + directory);

         // Use a stack-based approach to avoid recursion depth issues
         std::vector<std::string> dirs_to_scan = {directory};

         while (!dirs_to_scan.empty())
         {
            // Check timeout
            if (time(nullptr) - start_time > timeout_seconds)
            {
               Utils::errorPrint("AdaExtractor: Timeout searching for ALI files in: " + directory);
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
                  Utils::errorPrint("AdaExtractor: Failed to open directory: " + current_dir);
                  continue;
               }

               struct dirent* entry;
               while ((entry = readdir(dir)) != nullptr)
               {
                  // Check timeout for each entry
                  if (time(nullptr) - start_time > timeout_seconds)
                  {
                     Utils::errorPrint("AdaExtractor: Timeout searching for ALI files in: " + directory);
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
                     if (isAliFile(full_path))
                     {
                        aliFiles.push_back(full_path);
                        Utils::infoPrint("AdaExtractor: Found ALI file: " + full_path);
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
               Utils::errorPrint("AdaExtractor: Skipping problematic directory: " + current_dir);
               continue;
            }
         }

         return true;
      }
      catch (const std::exception& e)
      {
         Utils::errorPrint("AdaExtractor: Error searching for ALI files in: " + directory);
         return false;
      }
#endif
      return true;
   }
   catch (const std::exception& e)
   {
      std::string error_msg = e.what();
      Utils::errorPrint("AdaExtractor: Error searching for ALI files: " + error_msg);
      return false;
   }
}

void AdaExtractor::setVerbose(bool verbose)
{
   this->verbose = verbose;
}

void AdaExtractor::setExtractRuntimePackages(bool extract)
{
   this->extractRuntimePackages = extract;
}

bool AdaExtractor::parseVersionLine(const std::string& line, AdaBuildInfo& buildInfo)
{
   // Parse version line: V "GNAT Lib v11"
   if (line.length() < 4)
      return false;

   std::string versionInfo = line.substr(2);
   if (versionInfo.length() > 2 && versionInfo[0] == '"' &&
       versionInfo[versionInfo.length() - 1] == '"')
   {
      buildInfo.compilerVersion = versionInfo.substr(1, versionInfo.length() - 2);
      return true;
   }

   return false;
}

bool AdaExtractor::parseDependencyLine(const std::string& line, AdaPackageInfo& packageInfo)
{
   // Parse dependency line: W package_name%spec_or_body source_file.ads/adb source_file.ali
   std::istringstream lineStream(line.substr(2));
   std::string        packagePart, sourceFile, aliFile;

   if (lineStream >> packagePart >> sourceFile >> aliFile)
   {
      // Extract package name (before the %)
      size_t percentPos = packagePart.find('%');
      if (percentPos != std::string::npos)
      {
         std::string packageName = packagePart.substr(0, percentPos);

         // Only add non-runtime packages as dependencies
         if (!isRuntimePackage(packageName))
         {
            packageInfo.dependencies.push_back(packageName);
         }

         // Store the source file name from ALI file (don't try to find actual file on disk)
         if (!isRuntimePackage(packageName) && !sourceFile.empty())
         {
            packageInfo.sourceFile = sourceFile;  // Just store the filename from ALI
         }
         return true;
      }
   }

   return false;
}

bool AdaExtractor::parseFunctionLine(const std::string&            line,
                                     std::vector<AdaFunctionInfo>& functions)
{
   // Parse function line: X 11 main.adb 6U11*Main 6b11 15l5 15t9
   // Extract function name from the line
   std::istringstream       lineStream(line.substr(2));
   std::string              token;
   std::vector<std::string> tokens;

   while (lineStream >> token)
   {
      tokens.push_back(token);
   }

   if (tokens.size() >= 3)
   {
      // Look for function name pattern (contains *)
      for (const auto& token : tokens)
      {
         if (token.find('*') != std::string::npos)
         {
            size_t starPos = token.find('*');
            if (starPos > 0)
            {
               AdaFunctionInfo func;
               func.name     = token.substr(0, starPos);
               func.isPublic = true;  // Functions in ALI files are typically public
               functions.push_back(func);
               return true;
            }
         }
      }
   }

   return false;
}

bool AdaExtractor::parseVariableLine(const std::string& line, std::vector<std::string>& variables)
{
   // Parse variable line: X 7a4 Data{string} 14r39
   std::istringstream       lineStream(line.substr(2));
   std::string              token;
   std::vector<std::string> tokens;

   while (lineStream >> token)
   {
      tokens.push_back(token);
   }

   if (tokens.size() >= 2)
   {
      // Look for variable pattern (contains {})
      for (const auto& token : tokens)
      {
         if (token.find('{') != std::string::npos && token.find('}') != std::string::npos)
         {
            size_t braceStart = token.find('{');
            size_t braceEnd   = token.find('}');
            if (braceStart > 0 && braceEnd > braceStart)
            {
               std::string varName = token.substr(0, braceStart);
               variables.push_back(varName);
            }
         }
      }
   }

   return !variables.empty();
}

bool AdaExtractor::parseTypeLine(const std::string& line, std::vector<std::string>& types)
{
   // Parse type line: X i* type_name
   std::istringstream lineStream(line.substr(2));
   std::string        token;

   while (lineStream >> token)
   {
      if (token.find('*') != std::string::npos)
      {
         size_t starPos = token.find('*');
         if (starPos > 0)
         {
            std::string typeName = token.substr(0, starPos);
            types.push_back(typeName);
         }
      }
   }

   return !types.empty();
}

bool AdaExtractor::extractCrossReferences(const std::string&              content,
                                          std::vector<AdaCrossReference>& crossRefs)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "G ")
      {
         parseCrossReferenceLine(line, crossRefs);
      }
   }

   return !crossRefs.empty();
}

bool AdaExtractor::extractTypeInfo(const std::string& content, std::vector<AdaTypeInfo>& types)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "X " && line.find("i*") != std::string::npos)
      {
         // Parse type information from X lines
         std::istringstream       lineStream(line.substr(2));
         std::string              token;
         std::vector<std::string> tokens;

         while (lineStream >> token)
         {
            tokens.push_back(token);
         }

         if (tokens.size() >= 2)
         {
            for (const auto& token : tokens)
            {
               if (token.find('*') != std::string::npos)
               {
                  size_t starPos = token.find('*');
                  if (starPos > 0)
                  {
                     AdaTypeInfo type;
                     type.name     = token.substr(0, starPos);
                     type.baseType = "unknown";  // Default base type
                     types.push_back(type);
                  }
               }
            }
         }
      }
   }

   return !types.empty();
}

bool AdaExtractor::extractSecurityFlags(const std::string&        content,
                                        std::vector<std::string>& securityFlags)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 3) == "RV ")
      {
         std::string flag = line.substr(3);
         if (isSecurityFlag(flag))
         {
            securityFlags.push_back(flag);
         }
      }
   }

   return !securityFlags.empty();
}

bool AdaExtractor::extractFileInfo(const std::string&                  content,
                                   std::map<std::string, std::string>& timestamps,
                                   std::map<std::string, std::string>& checksums)
{
   std::istringstream stream(content);
   std::string        line;

   while (std::getline(stream, line))
   {
      if (line.substr(0, 2) == "D ")
      {
         parseFileInfoLine(line, timestamps, checksums);
      }
   }

   return !timestamps.empty() || !checksums.empty();
}

std::string AdaExtractor::generateCallGraph(const std::vector<AdaCrossReference>& crossRefs)
{
   std::string callGraph;

   for (const auto& crossRef : crossRefs)
   {
      if (!callGraph.empty())
      {
         callGraph += ", ";
      }
      callGraph += "[" + crossRef.callerFunction + " " + crossRef.callerPackage + "] -> ";
      callGraph += "[" + crossRef.calledFunction + " " + crossRef.calledPackage + "]";
   }

   return callGraph;
}

bool AdaExtractor::parseCrossReferenceLine(const std::string&              line,
                                           std::vector<AdaCrossReference>& crossRefs)
{
   // Parse cross-reference line: G r c none [main standard 6 11 none] [read_data_file data_reader 2
   // 13 none]
   if (line.length() < 4)
      return false;

   std::string content = line.substr(2);

   // Look for function call patterns in square brackets
   std::regex               bracketPattern(R"(\[([^\]]+)\])");
   std::sregex_iterator     iter(content.begin(), content.end(), bracketPattern);
   std::sregex_iterator     end;

   std::vector<std::string> functionCalls;
   for (; iter != end; ++iter)
   {
      functionCalls.push_back(iter->str(1));
   }

   // Parse function calls (need at least 2 for a call relationship)
   if (functionCalls.size() >= 2)
   {
      for (size_t i = 0; i < functionCalls.size() - 1; ++i)
      {
         AdaCrossReference crossRef;

         // Parse caller
         std::istringstream callerStream(functionCalls[i]);
         std::string        callerFunc, callerPkg, callerLine, callerCol, callerType;
         if (callerStream >> callerFunc >> callerPkg >> callerLine >> callerCol >> callerType)
         {
            crossRef.callerFunction = callerFunc;
            crossRef.callerPackage  = callerPkg;
            crossRef.callerLine     = callerLine;
         }

         // Parse called function
         std::istringstream calledStream(functionCalls[i + 1]);
         std::string        calledFunc, calledPkg, calledLine, calledCol, calledType;
         if (calledStream >> calledFunc >> calledPkg >> calledLine >> calledCol >> calledType)
         {
            crossRef.calledFunction = calledFunc;
            crossRef.calledPackage  = calledPkg;
            crossRef.calledLine     = calledLine;
         }

         crossRef.relationship = "calls";
         crossRefs.push_back(crossRef);
      }
   }

   return !crossRefs.empty();
}

bool AdaExtractor::parseBuildFlagLine(const std::string& line, AdaBuildInfo& buildInfo)
{
   // Parse build flag line: RV NO_IO
   if (line.length() < 4)
      return false;

   std::string flag = line.substr(3);

   if (isSecurityFlag(flag))
   {
      buildInfo.securityFlags.push_back(flag);
   }
   else if (isOptimizationFlag(flag))
   {
      buildInfo.optimizationFlags.push_back(flag);
   }
   else
   {
      buildInfo.runtimeFlags.push_back(flag);
   }

   return true;
}

bool AdaExtractor::parseFileInfoLine(const std::string&                  line,
                                     std::map<std::string, std::string>& timestamps,
                                     std::map<std::string, std::string>& checksums)
{
   // Parse file info line: D data_reader.ads 20250719161512 b2efb2f5 data_reader%s
   std::istringstream lineStream(line.substr(2));
   std::string        fileName, timestamp, checksum, packageInfo;

   if (lineStream >> fileName >> timestamp >> checksum >> packageInfo)
   {
      timestamps[fileName] = timestamp;
      checksums[fileName]  = checksum;
      return true;
   }

   return false;
}

bool AdaExtractor::isSecurityFlag(const std::string& flag)
{
   return securityFlags.find(flag) != securityFlags.end();
}

bool AdaExtractor::isOptimizationFlag(const std::string& flag)
{
   return optimizationFlags.find(flag) != optimizationFlags.end();
}

void AdaExtractor::setExtractEnhancedMetadata(bool extract)
{
   this->extractEnhancedMetadata = extract;
}

bool AdaExtractor::isRuntimePackage(const std::string& packageName)
{
   for (const auto& runtimePkg : runtimePackages)
   {
      if (packageName.find(runtimePkg) == 0)
      {
         return true;
      }
   }
   return false;
}

std::string AdaExtractor::extractPackageName(const std::string& aliFilePath)
{
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
   // Use std::filesystem for C++17+
   std::filesystem::path path(aliFilePath);
   std::string           filename = path.filename().string();
#else
   // Extract filename manually for C++11 compatibility
   size_t      lastSlash = aliFilePath.find_last_of("/\\");
   std::string filename  = (lastSlash != std::string::npos) ? aliFilePath.substr(lastSlash + 1)
                                                            : aliFilePath;
#endif

   // Remove .ali extension
   if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".ali")
   {
      filename = filename.substr(0, filename.length() - 4);
   }

   return filename;
}

std::string AdaExtractor::extractSourceFilePath(const std::string& aliFilePath)
{
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
   // Use std::filesystem for C++17+
   std::filesystem::path path(aliFilePath);
   std::string           filename = path.filename().string();
#else
   // Extract filename manually for C++11 compatibility
   size_t      lastSlash = aliFilePath.find_last_of("/\\");
   std::string filename  = (lastSlash != std::string::npos) ? aliFilePath.substr(lastSlash + 1)
                                                            : aliFilePath;
#endif

   // Remove .ali extension and construct source file name
   if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".ali")
   {
      std::string baseName = filename.substr(0, filename.length() - 4);

      // Try both .ads and .adb extensions (specification and body)
      std::vector<std::string> extensions = {".ads", ".adb"};

      for (const auto& ext : extensions)
      {
         std::string sourceFileName = baseName + ext;

         // Check if this source file is referenced in the ALI file
         std::ifstream aliFile(aliFilePath);
         if (aliFile.is_open())
         {
            std::string line;
            while (std::getline(aliFile, line))
            {
               // Look for W or Z lines that reference this source file
               if ((line.substr(0, 2) == "W " || line.substr(0, 2) == "Z ") &&
                   line.find(sourceFileName) != std::string::npos)
               {
                  aliFile.close();
                  return sourceFileName;  // Return just the filename, not full path
               }
            }
            aliFile.close();
         }
      }
   }

   return "";
}

}  // namespace heimdall