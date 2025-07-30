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
 * @brief Ada ALI file extractor implementation
 * @author Trevor Bakker
 * @date 2025
 * @version 1.0.0
 *
 * This file provides the implementation of the AdaExtractor class which
 * implements the IBinaryExtractor interface for extracting metadata from
 * Ada ALI (Ada Library Information) files.
 */

#include "AdaExtractor.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <thread>
#include "../compat/compatibility.hpp"
#include "../utils/FileUtils.hpp"

namespace heimdall
{

// Implementation class
class AdaExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   bool                     verbose                 = false;
   bool                     excludeRuntimePackages  = false;
   bool                     extractEnhancedMetadata = false;
   std::vector<std::string> runtimePackages;
   std::set<std::string>    securityFlags;
   std::set<std::string>    optimizationFlags;
   static bool              testMode;

   // Initialize runtime packages and flags
   void initializeRuntimePackages()
   {
      runtimePackages = {"ada",
                         "system",
                         "interfaces",
                         "text_io",
                         "calendar",
                         "direct_io",
                         "sequential_io",
                         "io_exceptions",
                         "unchecked_conversion",
                         "unchecked_deallocation",
                         "machine_code",
                         "system.storage_elements",
                         "system.address_to_access_conversions",
                         "system.storage_pools",
                         "system.finalization_masters",
                         "system.finalization_root",
                         "system.finalization_implementation",
                         "system.traceback",
                         "system.traceback_entries",
                         "system.traceback_symbolic",
                         "system.exception_traces",
                         "system.exceptions",
                         "system.exception_table",
                         "system.soft_links",
                         "system.secondary_stack",
                         "system.task_info",
                         "system.task_primitives",
                         "system.tasking",
                         "system.tasking.rendezvous",
                         "system.tasking.entry_calls",
                         "system.tasking.initialization",
                         "system.tasking.protected_objects",
                         "system.tasking.protected_objects.entries",
                         "system.tasking.protected_objects.operations",
                         "system.tasking.queuing",
                         "system.tasking.restricted",
                         "system.tasking.restricted.stages",
                         "system.tasking.utilities",
                         "system.tasking.debug",
                         "system.tasking.debug.operations",
                         "system.tasking.debug.rendezvous",
                         "system.tasking.debug.entry_calls",
                         "system.tasking.debug.protected_objects",
                         "system.tasking.debug.utilities",
                         "system.tasking.debug.operations",
                         "system.tasking.debug.rendezvous",
                         "system.tasking.debug.entry_calls",
                         "system.tasking.debug.protected_objects",
                         "system.tasking.debug.utilities"};

      securityFlags = {"-fstack-protector",
                       "-fstack-protector-strong",
                       "-fstack-protector-all",
                       "-fPIE",
                       "-fPIC",
                       "-Wl,-z,relro",
                       "-Wl,-z,now",
                       "-Wl,-z,noexecstack",
                       "-Wl,-z,stack-size",
                       "-D_FORTIFY_SOURCE=2",
                       "-fstack-check",
                       "-fstack-clash-protection",
                       "-fcf-protection",
                       "-fcf-protection=full",
                       "-fcf-protection=branch",
                       "-fcf-protection=return",
                       "-fcf-protection=check"};

      optimizationFlags = {"-O0",
                           "-O1",
                           "-O2",
                           "-O3",
                           "-Os",
                           "-Og",
                           "-Ofast",
                           "-ffast-math",
                           "-fno-math-errno",
                           "-fno-trapping-math",
                           "-fno-signaling-nans",
                           "-fno-rounding-math",
                           "-fno-signed-zeros",
                           "-fno-math-errno",
                           "-fno-trapping-math",
                           "-fno-signaling-nans",
                           "-fno-rounding-math",
                           "-fno-signed-zeros",
                           "-fno-math-errno",
                           "-fno-trapping-math"};
   }
};

// Static member initialization
bool AdaExtractor::Impl::testMode = false;

// Constructor and destructor
AdaExtractor::AdaExtractor() : pImpl(std::make_unique<Impl>())
{
   pImpl->initializeRuntimePackages();
}

AdaExtractor::~AdaExtractor() = default;

AdaExtractor::AdaExtractor(const AdaExtractor& other) : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

AdaExtractor::AdaExtractor(AdaExtractor&& other) noexcept : pImpl(std::move(other.pImpl)) {}

AdaExtractor& AdaExtractor::operator=(const AdaExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

AdaExtractor& AdaExtractor::operator=(AdaExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool AdaExtractor::extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   std::vector<AdaFunctionInfo> functions;
   std::ifstream                file(filePath);
   if (!file.is_open())
   {
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string content = buffer.str();

   if (!extractFunctions(content, functions))
   {
      return false;
   }

   // Convert AdaFunctionInfo to SymbolInfo
   symbols.clear();
   for (const auto& func : functions)
   {
      SymbolInfo symbol;
      symbol.name      = func.name;
      symbol.section   = func.package;
      symbol.isDefined = true;
      symbol.isGlobal  = func.isPublic;
      symbol.isWeak    = false;
      symbol.address   = 0;  // ALI files don't contain address information
      symbol.size      = 0;  // ALI files don't contain size information
      symbols.push_back(symbol);
   }

   return !symbols.empty();
}

bool AdaExtractor::extractSections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   AdaPackageInfo packageInfo;
   if (!parseAliFile(filePath, packageInfo))
   {
      return false;
   }

   // Convert AdaPackageInfo to SectionInfo
   sections.clear();
   SectionInfo section;
   section.name    = packageInfo.name;
   section.type    = packageInfo.isSpecification ? "specification" : "body";
   section.address = 0;  // ALI files don't contain address information
   section.size    = 0;  // ALI files don't contain size information
   section.flags   = 0;  // ALI files don't contain flags
   sections.push_back(section);

   return !sections.empty();
}

bool AdaExtractor::extractVersion(const std::string& filePath, std::string& version)
{
   if (!canHandle(filePath))
   {
      return false;
   }

   AdaBuildInfo  buildInfo;
   std::ifstream file(filePath);
   if (!file.is_open())
   {
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string content = buffer.str();

   if (!extractBuildInfo(content, buildInfo))
   {
      return false;
   }

   version = buildInfo.compilerVersion;
   return !version.empty();
}

std::vector<std::string> AdaExtractor::extractDependencies(const std::string& filePath)
{
   std::vector<std::string> dependencies;
   if (!canHandle(filePath))
   {
      return dependencies;
   }

   std::ifstream file(filePath);
   if (!file.is_open())
   {
      return dependencies;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string content = buffer.str();

   extractDependencies(content, dependencies);
   return dependencies;
}

bool AdaExtractor::extractFunctions(const std::string&        filePath,
                                    std::vector<std::string>& functions)
{
   // AdaExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;   // Suppress unused parameter warning
   (void)functions;  // Suppress unused parameter warning
   return false;
}

bool AdaExtractor::extractCompileUnits(const std::string&        filePath,
                                       std::vector<std::string>& compileUnits)
{
   // AdaExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;      // Suppress unused parameter warning
   (void)compileUnits;  // Suppress unused parameter warning
   return false;
}

bool AdaExtractor::extractSourceFiles(const std::string&        filePath,
                                      std::vector<std::string>& sourceFiles)
{
   // AdaExtractor doesn't support DWARF extraction by default
   // This would need to be implemented with DWARF parsing libraries
   (void)filePath;     // Suppress unused parameter warning
   (void)sourceFiles;  // Suppress unused parameter warning
   return false;
}

bool AdaExtractor::canHandle(const std::string& filePath) const
{
   return isAliFile(filePath);
}

std::string AdaExtractor::getFormatName() const
{
   return "Ada ALI";
}

int AdaExtractor::getPriority() const
{
   return 50;  // Medium priority for Ada ALI files
}

// Ada-specific methods
bool AdaExtractor::extractAdaMetadata(ComponentInfo&                  component,
                                      const std::vector<std::string>& aliFiles)
{
   bool                               success = false;
   std::vector<std::string>           allFunctions;
   std::vector<std::string>           allDependencies;
   std::vector<std::string>           allSourceFiles;
   std::vector<std::string>           allTypes;
   std::map<std::string, std::string> allProperties;

   for (const auto& aliFile : aliFiles)
   {
      AdaPackageInfo packageInfo;
      if (parseAliFile(aliFile, packageInfo))
      {
         success = true;

         // Skip runtime packages if configured to exclude them
         if (pImpl->excludeRuntimePackages && packageInfo.isRuntime)
         {
            continue;
         }

         // Merge functions
         allFunctions.insert(allFunctions.end(), packageInfo.functions.begin(),
                             packageInfo.functions.end());

         // Merge dependencies (include runtime packages by default, like ELF extractor)
         for (const auto& dep : packageInfo.dependencies)
         {
            // Include all dependencies by default, including runtime packages
            // Only filter out runtime packages if explicitly configured to exclude them
            if (!pImpl->excludeRuntimePackages || !isRuntimePackage(dep))
            {
               allDependencies.push_back(dep);
            }
         }

         // Merge source files
         if (!packageInfo.sourceFile.empty())
         {
            allSourceFiles.push_back(packageInfo.sourceFile);
         }

         // Merge types
         allTypes.insert(allTypes.end(), packageInfo.types.begin(), packageInfo.types.end());

         // Add package-specific properties
         allProperties["ada.package." + packageInfo.name + ".source_file"] = packageInfo.sourceFile;
         allProperties["ada.package." + packageInfo.name + ".checksum"]    = packageInfo.checksum;
         allProperties["ada.package." + packageInfo.name + ".timestamp"]   = packageInfo.timestamp;
         allProperties["ada.package." + packageInfo.name + ".is_specification"] =
            packageInfo.isSpecification ? "true" : "false";
         allProperties["ada.package." + packageInfo.name + ".is_runtime"] =
            packageInfo.isRuntime ? "true" : "false";
      }
   }

   if (success)
   {
      // Update component with extracted metadata
      component.functions    = allFunctions;
      component.dependencies = allDependencies;
      component.sourceFiles  = allSourceFiles;
      component.properties.insert(allProperties.begin(), allProperties.end());

      // Set file type
      component.fileType = FileType::Source;

      // Set package manager
      component.packageManager = "GNAT";

      // Set description if not already set
      if (component.description.empty())
      {
         component.description =
            "Ada application with " + std::to_string(aliFiles.size()) + " ALI files";
      }
   }

   return success;
}

bool AdaExtractor::parseAliFile(const std::string& aliFilePath, AdaPackageInfo& packageInfo)
{
   std::ifstream file(aliFilePath);
   if (!file.is_open())
   {
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string content = buffer.str();

   // Validate that the file contains valid ALI content
   if (content.empty() || content.find("V ") == std::string::npos)
   {
      return false;  // Empty or invalid ALI file
   }

   // Extract basic package information
   packageInfo.aliFile    = aliFilePath;
   packageInfo.name       = extractPackageName(aliFilePath);
   packageInfo.sourceFile = extractSourceFilePath(aliFilePath);
   packageInfo.isRuntime  = isRuntimePackage(packageInfo.name);

   // Extract detailed information
   extractDependencies(content, packageInfo.dependencies);
   extractSourceFilesFromContent(content, packageInfo.sourceFile);

   // Extract functions and build info properly
   std::vector<AdaFunctionInfo> functions;
   extractFunctions(content, functions);
   for (const auto& func : functions)
   {
      packageInfo.functions.push_back(func.name);
   }

   AdaBuildInfo buildInfo;
   extractBuildInfo(content, buildInfo);

   return true;
}

bool AdaExtractor::findAliFiles(const std::string& directory, std::vector<std::string>& aliFiles)
{
   if (pImpl->testMode)
   {
      if (pImpl->verbose)
      {
         std::cerr << "AdaExtractor: Skipping Ada ALI file search in test mode for: " << directory
                   << std::endl;
      }
      return false;
   }

   aliFiles.clear();
   auto       startTime = std::chrono::steady_clock::now();
   const auto timeout   = std::chrono::seconds(30);  // 30 second timeout

   try
   {
      if (!std::filesystem::exists(directory))
      {
         return false;
      }

      for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
      {
         // Check for timeout
         if (std::chrono::steady_clock::now() - startTime > timeout)
         {
            if (pImpl->verbose)
            {
               std::cerr << "AdaExtractor: Timeout searching for ALI files in: " << directory
                         << std::endl;
            }
            break;
         }

         if (entry.is_regular_file() && isAliFile(entry.path().string()))
         {
            aliFiles.push_back(entry.path().string());
            if (pImpl->verbose)
            {
               std::cerr << "AdaExtractor: Found ALI file: " << entry.path().string() << std::endl;
            }
         }
      }
   }
   catch (const std::exception& e)
   {
      if (pImpl->verbose)
      {
         std::cerr << "AdaExtractor: Error searching for ALI files in: " << directory << ": "
                   << e.what() << std::endl;
      }
      return false;
   }

   return !aliFiles.empty();
}

bool AdaExtractor::isAliFile(const std::string& filePath) const
{
   std::filesystem::path path(filePath);
   return path.extension() == ".ali";
}

bool AdaExtractor::extractSourceFilesFromContent(const std::string& content, std::string& sourceFile)
{
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      // Handle "W " lines (with-clause dependencies) - format: W package_name%spec_or_body source_file.ads/adb source_file.ali
      if (line.find("W ") == 0)
      {
         std::string depLine = line.substr(2);
         // Remove leading/trailing whitespace
         depLine.erase(0, depLine.find_first_not_of(" \t"));
         depLine.erase(depLine.find_last_not_of(" \t") + 1);
         
         if (!depLine.empty())
         {
            // Parse the source file from the W line format
            std::istringstream lineStream(depLine);
            std::string packagePart, sourceFilePart, aliFile;
            if (lineStream >> packagePart >> sourceFilePart >> aliFile)
            {
               // Extract just the filename from the source file path
               std::filesystem::path sourcePath(sourceFilePart);
               sourceFile = sourcePath.filename().string();
               return true;  // Found the first source file
            }
         }
      }
   }

   return false;
}

bool AdaExtractor::extractDependencies(const std::string&        content,
                                       std::vector<std::string>& dependencies)
{
   dependencies.clear();
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      // Handle "W " lines (with-clause dependencies) - format: W package_name%spec_or_body source_file.ads/adb source_file.ali
      if (line.find("W ") == 0)
      {
         std::string depLine = line.substr(2);
         // Remove leading/trailing whitespace
         depLine.erase(0, depLine.find_first_not_of(" \t"));
         depLine.erase(depLine.find_last_not_of(" \t") + 1);
         
         if (!depLine.empty())
         {
            // Parse the package name from the W line format
            std::istringstream lineStream(depLine);
            std::string packagePart, sourceFile, aliFile;
            if (lineStream >> packagePart >> sourceFile >> aliFile)
            {
               // Extract package name before the % symbol
               size_t percentPos = packagePart.find('%');
               if (percentPos != std::string::npos)
               {
                  std::string packageName = packagePart.substr(0, percentPos);
                  // Only add if not already present (avoid duplicates)
                  if (std::find(dependencies.begin(), dependencies.end(), packageName) == dependencies.end())
                  {
                     dependencies.push_back(packageName);
                  }
               }
            }
         }
      }
      // Handle "D " lines (direct dependencies) - legacy format
      else if (line.find("D ") == 0)
      {
         std::string dep = line.substr(2);
         // Remove leading/trailing whitespace
         dep.erase(0, dep.find_first_not_of(" \t"));
         dep.erase(dep.find_last_not_of(" \t") + 1);
         if (!dep.empty())
         {
            dependencies.push_back(dep);
         }
      }
   }

   return !dependencies.empty();
}

bool AdaExtractor::extractFunctions(const std::string&            content,
                                    std::vector<AdaFunctionInfo>& functions)
{
   functions.clear();
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("P ") == 0)
      {
         if (parseFunctionLine(line, functions))
         {
            // Function was added to the vector by parseFunctionLine
         }
      }
   }

   return !functions.empty();
}

bool AdaExtractor::extractBuildInfo(const std::string& content, AdaBuildInfo& buildInfo)
{
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("V ") == 0)
      {
         parseVersionLine(line, buildInfo);
      }
      else if (line.find("F ") == 0)
      {
         parseBuildFlagLine(line, buildInfo);
      }
   }

   return true;
}

bool AdaExtractor::extractCrossReferences(const std::string&              content,
                                          std::vector<AdaCrossReference>& crossRefs)
{
   crossRefs.clear();
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("X ") == 0)
      {
         if (parseCrossReferenceLine(line, crossRefs))
         {
            // Cross-reference was added to the vector by parseCrossReferenceLine
         }
      }
   }

   return !crossRefs.empty();
}

bool AdaExtractor::extractTypeInfo(const std::string& content, std::vector<AdaTypeInfo>& types)
{
   types.clear();
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("T ") == 0)
      {
         std::vector<std::string> typeNames;
         if (parseTypeLine(line, typeNames))
         {
            for (const auto& typeName : typeNames)
            {
               AdaTypeInfo type;
               type.name = typeName;
               types.push_back(type);
            }
         }
      }
   }

   return !types.empty();
}

bool AdaExtractor::extractSecurityFlags(const std::string&        content,
                                        std::vector<std::string>& securityFlags)
{
   securityFlags.clear();
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("F ") == 0)
      {
         std::string flag = line.substr(2);
         flag.erase(0, flag.find_first_not_of(" \t"));
         flag.erase(flag.find_last_not_of(" \t") + 1);
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
   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (line.find("I ") == 0)
      {
         parseFileInfoLine(line, timestamps, checksums);
      }
   }

   return true;
}

std::string AdaExtractor::generateCallGraph(const std::vector<AdaCrossReference>& crossRefs)
{
   std::stringstream ss;
   ss << "digraph CallGraph {\n";

   for (const auto& crossRef : crossRefs)
   {
      ss << "  \"" << crossRef.callerPackage << "." << crossRef.callerFunction << "\" -> ";
      ss << "\"" << crossRef.calledPackage << "." << crossRef.calledFunction << "\";\n";
   }

   ss << "}\n";
   return ss.str();
}

bool AdaExtractor::isRuntimePackage(const std::string& packageName) const
{
   // Check for exact match first
   if (std::find(pImpl->runtimePackages.begin(), pImpl->runtimePackages.end(), packageName) !=
       pImpl->runtimePackages.end())
   {
      return true;
   }
   
   // Check for hierarchical package names (e.g., "ada.strings" should match "ada")
   std::string::size_type pos = packageName.find('.');
   if (pos != std::string::npos)
   {
      std::string rootPackage = packageName.substr(0, pos);
      return std::find(pImpl->runtimePackages.begin(), pImpl->runtimePackages.end(), rootPackage) !=
             pImpl->runtimePackages.end();
   }
   
   return false;
}

std::string AdaExtractor::extractPackageName(const std::string& aliFilePath) const
{
   std::filesystem::path path(aliFilePath);
   std::string           filename = path.stem().string();

   // Remove any suffixes that might be present
   size_t pos = filename.find_last_of('.');
   if (pos != std::string::npos)
   {
      filename = filename.substr(0, pos);
   }

   return filename;
}

std::string AdaExtractor::extractSourceFilePath(const std::string& aliFilePath) const
{
   std::filesystem::path path(aliFilePath);
   std::string           packageName = extractPackageName(aliFilePath);

   // Try to find corresponding .ads or .adb file
   std::filesystem::path parentDir = path.parent_path();
   std::filesystem::path adsFile   = parentDir / (packageName + ".ads");
   std::filesystem::path adbFile   = parentDir / (packageName + ".adb");

   if (std::filesystem::exists(adsFile))
   {
      return adsFile.string();
   }
   else if (std::filesystem::exists(adbFile))
   {
      return adbFile.string();
   }

   return "";
}

bool AdaExtractor::isSecurityFlag(const std::string& flag) const
{
   return pImpl->securityFlags.find(flag) != pImpl->securityFlags.end();
}

bool AdaExtractor::isOptimizationFlag(const std::string& flag) const
{
   return pImpl->optimizationFlags.find(flag) != pImpl->optimizationFlags.end();
}

// Configuration methods
void AdaExtractor::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
}

void AdaExtractor::setExcludeRuntimePackages(bool exclude)
{
   pImpl->excludeRuntimePackages = exclude;
}

void AdaExtractor::setExtractEnhancedMetadata(bool extract)
{
   pImpl->extractEnhancedMetadata = extract;
}

void AdaExtractor::setTestMode(bool enabled)
{
   Impl::testMode = enabled;
}

bool AdaExtractor::isTestMode()
{
   return Impl::testMode;
}

// Private helper methods
bool AdaExtractor::parseVersionLine(const std::string& line, AdaBuildInfo& buildInfo)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string version = line.substr(2);
   version.erase(0, version.find_first_not_of(" \t"));
   version.erase(version.find_last_not_of(" \t") + 1);

   buildInfo.compilerVersion = version;
   return true;
}

bool AdaExtractor::parseDependencyLine(const std::string& line, AdaPackageInfo& packageInfo)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string dep = line.substr(2);
   dep.erase(0, dep.find_first_not_of(" \t"));
   dep.erase(dep.find_last_not_of(" \t") + 1);

   if (!dep.empty())
   {
      packageInfo.dependencies.push_back(dep);
   }

   return true;
}

bool AdaExtractor::parseFunctionLine(const std::string&            line,
                                     std::vector<AdaFunctionInfo>& functions)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string funcInfo = line.substr(2);
   funcInfo.erase(0, funcInfo.find_first_not_of(" \t"));
   funcInfo.erase(funcInfo.find_last_not_of(" \t") + 1);

   // Simple parsing - in a real implementation, this would be more sophisticated
   AdaFunctionInfo func;
   func.name        = funcInfo;
   func.isPublic    = true;
   func.isProcedure = false;

   functions.push_back(func);
   return true;
}

bool AdaExtractor::parseVariableLine(const std::string& line, std::vector<std::string>& variables)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string var = line.substr(2);
   var.erase(0, var.find_first_not_of(" \t"));
   var.erase(var.find_last_not_of(" \t") + 1);

   if (!var.empty())
   {
      variables.push_back(var);
   }

   return true;
}

bool AdaExtractor::parseTypeLine(const std::string& line, std::vector<std::string>& types)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string type = line.substr(2);
   type.erase(0, type.find_first_not_of(" \t"));
   type.erase(type.find_last_not_of(" \t") + 1);

   if (!type.empty())
   {
      types.push_back(type);
   }

   return true;
}

bool AdaExtractor::parseCrossReferenceLine(const std::string&              line,
                                           std::vector<AdaCrossReference>& crossRefs)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string crossRefInfo = line.substr(2);
   crossRefInfo.erase(0, crossRefInfo.find_first_not_of(" \t"));
   crossRefInfo.erase(crossRefInfo.find_last_not_of(" \t") + 1);

   // Simple parsing - in a real implementation, this would be more sophisticated
   AdaCrossReference crossRef;
   crossRef.relationship = "calls";

   // Parse the cross-reference information
   std::istringstream       iss(crossRefInfo);
   std::string              token;
   std::vector<std::string> tokens;

   while (iss >> token)
   {
      tokens.push_back(token);
   }

   if (tokens.size() >= 2)
   {
      crossRef.callerFunction = tokens[0];
      crossRef.calledFunction = tokens[1];
      crossRefs.push_back(crossRef);
   }

   return true;
}

bool AdaExtractor::parseBuildFlagLine(const std::string& line, AdaBuildInfo& buildInfo)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string flag = line.substr(2);
   flag.erase(0, flag.find_first_not_of(" \t"));
   flag.erase(flag.find_last_not_of(" \t") + 1);

   if (!flag.empty())
   {
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
         buildInfo.compilationFlags.push_back(flag);
      }
   }

   return true;
}

bool AdaExtractor::parseFileInfoLine(const std::string&                  line,
                                     std::map<std::string, std::string>& timestamps,
                                     std::map<std::string, std::string>& checksums)
{
   if (line.length() < 3)
   {
      return false;
   }

   std::string fileInfo = line.substr(2);
   fileInfo.erase(0, fileInfo.find_first_not_of(" \t"));
   fileInfo.erase(fileInfo.find_last_not_of(" \t") + 1);

   // Simple parsing - in a real implementation, this would be more sophisticated
   std::istringstream iss(fileInfo);
   std::string        filename, timestamp, checksum;

   if (iss >> filename >> timestamp >> checksum)
   {
      timestamps[filename] = timestamp;
      checksums[filename]  = checksum;
   }

   return true;
}

}  // namespace heimdall