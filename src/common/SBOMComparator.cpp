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

#include "SBOMComparator.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include "../compat/compatibility.hpp"
#include "Utils.hpp"

namespace heimdall
{

// SBOMComponent implementation

std::string SBOMComponent::getHash() const
{
   // Simple hash based on key component fields
   std::string hash = name + ":" + version + ":" + type + ":" + purl;
   return hash;
}

bool SBOMComponent::operator==(const SBOMComponent& other) const
{
   return getHash() == other.getHash();
}

bool SBOMComponent::operator!=(const SBOMComponent& other) const
{
   return !(*this == other);
}

// UnifiedSBOMComparator Implementation

std::vector<SBOMDifference> UnifiedSBOMComparator::compare(const std::string& oldSBOM,
                                                           const std::string& newSBOM)
{
   std::vector<SBOMDifference> differences;

   try
   {
      // Read old SBOM file
      std::ifstream oldFile(oldSBOM);
      if (!oldFile.is_open())
      {
         std::cerr << "Cannot open old SBOM file: " << oldSBOM << std::endl;
         return differences;
      }

      std::stringstream oldBuffer;
      oldBuffer << oldFile.rdbuf();
      std::string oldContent = oldBuffer.str();

      // Read new SBOM file
      std::ifstream newFile(newSBOM);
      if (!newFile.is_open())
      {
         std::cerr << "Cannot open new SBOM file: " << newSBOM << std::endl;
         return differences;
      }

      std::stringstream newBuffer;
      newBuffer << newFile.rdbuf();
      std::string newContent = newBuffer.str();

      return compareContent(oldContent, newContent);
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error comparing SBOM files: " << e.what() << std::endl;
   }

   return differences;
}

std::vector<SBOMDifference> UnifiedSBOMComparator::compareContent(const std::string& oldContent,
                                                                  const std::string& newContent)
{
   std::vector<SBOMDifference> differences;

   try
   {
      // Detect format from content
      std::string format = detectFormatFromContent(oldContent);
      if (format.empty())
      {
         std::cerr << "Unable to detect format from old SBOM content" << std::endl;
         return differences;
      }

      // Create format handler
      auto handler = createHandler(format);
      if (!handler)
      {
         std::cerr << "Failed to create handler for format: " << format << std::endl;
         return differences;
      }

      // Parse components from both SBOMs
      std::vector<ComponentInfo> oldComponents = handler->parseContent(oldContent);
      std::vector<ComponentInfo> newComponents = handler->parseContent(newContent);

      // Compare components
      return compareComponents(oldComponents, newComponents);
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error comparing SBOM content: " << e.what() << std::endl;
   }

   return differences;
}

std::string UnifiedSBOMComparator::merge(const std::vector<std::string>& sbomFiles,
                                         const std::string&              outputFormat,
                                         const std::string&              outputVersion)
{
   try
   {
      std::vector<std::vector<ComponentInfo>> allComponents;

      // Parse all SBOM files
      for (const auto& filePath : sbomFiles)
      {
         std::ifstream file(filePath);
         if (!file.is_open())
         {
            std::cerr << "Cannot open SBOM file: " << filePath << std::endl;
            continue;
         }

         std::stringstream buffer;
         buffer << file.rdbuf();
         std::string content = buffer.str();

         // Detect format and create handler
         std::string format = detectFormatFromContent(content);
         if (format.empty())
         {
            std::cerr << "Unable to detect format from file: " << filePath << std::endl;
            continue;
         }

         auto handler = createHandler(format);
         if (!handler)
         {
            std::cerr << "Failed to create handler for format: " << format << std::endl;
            continue;
         }

         // Parse components
         std::vector<ComponentInfo> components = handler->parseContent(content);
         allComponents.push_back(components);
      }

      // Merge components
      std::vector<ComponentInfo> mergedComponents = mergeComponents(allComponents);

      // Generate output
      if (outputFormat == "spdx")
      {
         return generateSPDXOutput(mergedComponents, outputFormat, outputVersion);
      }
      else if (outputFormat == "cyclonedx")
      {
         return generateCycloneDXOutput(mergedComponents, outputFormat, outputVersion);
      }
      else
      {
         std::cerr << "Unsupported output format: " << outputFormat << std::endl;
         return "";
      }
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error merging SBOM files: " << e.what() << std::endl;
   }

   return "";
}

std::string UnifiedSBOMComparator::generateDiffReport(
   const std::vector<SBOMDifference>& differences, const std::string& format)
{
   if (format == "json")
   {
      return generateJSONReport(differences);
   }
   else if (format == "csv")
   {
      return generateCSVReport(differences);
   }
   else
   {
      return generateTextReport(differences);
   }
}

std::map<std::string, int> UnifiedSBOMComparator::getDiffStatistics(
   const std::vector<SBOMDifference>& differences)
{
   std::map<std::string, int> stats;
   stats["added"]     = 0;
   stats["removed"]   = 0;
   stats["modified"]  = 0;
   stats["unchanged"] = 0;

   for (const auto& diff : differences)
   {
      switch (diff.type)
      {
         case SBOMDifference::Type::ADDED:
            stats["added"]++;
            break;
         case SBOMDifference::Type::REMOVED:
            stats["removed"]++;
            break;
         case SBOMDifference::Type::MODIFIED:
            stats["modified"]++;
            break;
         case SBOMDifference::Type::UNCHANGED:
            stats["unchanged"]++;
            break;
      }
   }

   return stats;
}

// Private helper methods

std::string UnifiedSBOMComparator::detectFormatFromFile(const std::string& filePath) const
{
   try
   {
      std::ifstream file(filePath);
      if (!file.is_open())
      {
         return "";
      }

      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string content = buffer.str();

      return detectFormatFromContent(content);
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error detecting format from file: " << e.what() << std::endl;
   }

   return "";
}

std::string UnifiedSBOMComparator::detectFormatFromContent(const std::string& content) const
{
   // Convert to lowercase for case-insensitive comparison
   std::string lowerContent = content;
   std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

   // Check for CycloneDX format (handle whitespace variations)
   if (lowerContent.find("\"bomformat\":") != std::string::npos ||
       lowerContent.find("\"bomformat\": ") != std::string::npos)
   {
      // Check if it's followed by "cyclonedx" (case insensitive)
      size_t pos = lowerContent.find("\"bomformat\":");
      if (pos != std::string::npos)
      {
         std::string afterColon = lowerContent.substr(pos + 12); // length of "bomformat":
         if (afterColon.find("cyclonedx") != std::string::npos)
         {
            return "cyclonedx";
         }
      }
   }

   // Check for SPDX format (handle whitespace variations)
   if (lowerContent.find("spdxversion:") != std::string::npos ||
       lowerContent.find("spdxversion: ") != std::string::npos ||
       lowerContent.find("\"spdxversion\"") != std::string::npos ||
       lowerContent.find("\"spdxVersion\"") != std::string::npos ||
       lowerContent.find("@context") != std::string::npos)
   {
      return "spdx";
   }

   return "";
}

std::unique_ptr<ISBOMFormatHandler> UnifiedSBOMComparator::createHandler(
   const std::string& format) const
{
   return SBOMFormatFactory::createHandler(format);
}

std::vector<SBOMDifference> UnifiedSBOMComparator::compareComponents(
   const std::vector<ComponentInfo>& oldComponents, const std::vector<ComponentInfo>& newComponents)
{
   std::vector<SBOMDifference> differences;

   // Create maps for efficient lookup by name
   std::map<std::string, ComponentInfo> oldComponentMap;
   std::map<std::string, ComponentInfo> newComponentMap;

   for (const auto& component : oldComponents)
   {
      oldComponentMap[component.name] = component;
   }

   for (const auto& component : newComponents)
   {
      newComponentMap[component.name] = component;
   }

   // Find added components (components in new but not in old)
   for (const auto& [name, newComponent] : newComponentMap)
   {
      if (oldComponentMap.find(name) == oldComponentMap.end())
      {
         SBOMComponent sbomComponent = convertToSBOMComponent(newComponent);
         differences.emplace_back(SBOMDifference::Type::ADDED, sbomComponent);
      }
   }

   // Find removed components (components in old but not in new)
   for (const auto& [name, oldComponent] : oldComponentMap)
   {
      if (newComponentMap.find(name) == newComponentMap.end())
      {
         SBOMComponent sbomComponent = convertToSBOMComponent(oldComponent);
         differences.emplace_back(SBOMDifference::Type::REMOVED, sbomComponent);
      }
   }

   // Find modified components (components with same name but different versions)
   for (const auto& [name, newComponent] : newComponentMap)
   {
      auto oldIt = oldComponentMap.find(name);
      if (oldIt != oldComponentMap.end())
      {
         const auto& oldComponent = oldIt->second;

         // Check if component was modified (different version or other properties)
         if (oldComponent.version != newComponent.version || oldComponent != newComponent)
         {
            SBOMComponent newSBOMComponent = convertToSBOMComponent(newComponent);
            SBOMComponent oldSBOMComponent = convertToSBOMComponent(oldComponent);
            differences.emplace_back(SBOMDifference::Type::MODIFIED, newSBOMComponent,
                                     oldSBOMComponent);
         }
         else
         {
            SBOMComponent sbomComponent = convertToSBOMComponent(newComponent);
            differences.emplace_back(SBOMDifference::Type::UNCHANGED, sbomComponent);
         }
      }
   }

   return differences;
}

std::vector<ComponentInfo> UnifiedSBOMComparator::mergeComponents(
   const std::vector<std::vector<ComponentInfo>>& componentLists)
{
   std::vector<ComponentInfo> mergedComponents;
   std::set<std::string>      seenComponents;

   for (const auto& componentList : componentLists)
   {
      for (const auto& component : componentList)
      {
         std::string key = component.name + ":" + component.version;

         if (seenComponents.find(key) == seenComponents.end())
         {
            mergedComponents.push_back(component);
            seenComponents.insert(key);
         }
      }
   }

   return mergedComponents;
}

SBOMComponent UnifiedSBOMComparator::convertToSBOMComponent(const ComponentInfo& component) const
{
   return SBOMComponent(
      component.name,                                                          // id
      component.name + "-" + component.version,                                // bomRef
      component.name,                                                          // name
      component.version,                                                       // version
      component.fileType == FileType::Executable ? "application" : "library",  // type
      component.packageManager.empty() ? ""
                                       : "pkg:" + component.packageManager + "/" + component.name +
                                            "@" + component.version,  // purl
      component.license,                                              // license
      component.description,                                          // description
      component.scope,                                                // scope
      component.group,                                                // group
      component.mimeType,                                             // mimeType
      component.copyright,                                            // copyright
      component.cpe,                                                  // cpe
      component.supplier,                                             // supplier
      component.manufacturer,                                         // manufacturer
      component.publisher,                                            // publisher
      component.properties,                                           // properties
      component.dependencies,                                         // dependencies
      {}  // externalReferences (not directly mapped)
   );
}

ComponentInfo UnifiedSBOMComparator::convertToComponentInfo(const SBOMComponent& component) const
{
   ComponentInfo info;
   info.name         = component.name;
   info.version      = component.version;
   info.description  = component.description;
   info.scope        = component.scope;
   info.group        = component.group;
   info.mimeType     = component.mimeType;
   info.copyright    = component.copyright;
   info.cpe          = component.cpe;
   info.supplier     = component.supplier;
   info.manufacturer = component.manufacturer;
   info.publisher    = component.publisher;
   info.license      = component.license;
   info.properties   = component.properties;
   info.dependencies = component.dependencies;

   // Set file type based on component type
   if (component.type == "application")
   {
      info.fileType = FileType::Executable;
   }
   else
   {
      info.fileType = FileType::SharedLibrary;
   }

   // Extract package manager from PURL
   if (!component.purl.empty() && component.purl.find("pkg:") == 0)
   {
      size_t start = 4;  // Skip "pkg:"
      size_t end   = component.purl.find('/', start);
      if (end != std::string::npos)
      {
         info.packageManager = component.purl.substr(start, end - start);
      }
   }

   return info;
}

std::string UnifiedSBOMComparator::generateSPDXOutput(const std::vector<ComponentInfo>& components,
                                                      const std::string&                format,
                                                      const std::string&                version)
{
   try
   {
      auto handler = SBOMFormatFactory::createSPDXHandler(version);
      if (!handler)
      {
         std::cerr << "Failed to create SPDX handler for version: " << version << std::endl;
         return "";
      }

      std::unordered_map<std::string, ComponentInfo> componentMap;
      for (const auto& component : components)
      {
         componentMap[component.name] = component;
      }

      std::map<std::string, std::string> metadata;
      metadata["document_name"] = "Merged SBOM";
      metadata["creator"]       = "Heimdall SBOM Comparator";

      return handler->generateSBOM(componentMap, metadata);
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error generating SPDX output: " << e.what() << std::endl;
   }

   return "";
}

std::string UnifiedSBOMComparator::generateCycloneDXOutput(
   const std::vector<ComponentInfo>& components, const std::string& format,
   const std::string& version)
{
   try
   {
      auto handler = SBOMFormatFactory::createCycloneDXHandler(version);
      if (!handler)
      {
         std::cerr << "Failed to create CycloneDX handler for version: " << version << std::endl;
         return "";
      }

      std::unordered_map<std::string, ComponentInfo> componentMap;
      for (const auto& component : components)
      {
         componentMap[component.name] = component;
      }

      std::map<std::string, std::string> metadata;
      metadata["document_name"] = "Merged SBOM";
      metadata["creator"]       = "Heimdall SBOM Comparator";

      return handler->generateSBOM(componentMap, metadata);
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error generating CycloneDX output: " << e.what() << std::endl;
   }

   return "";
}

std::string UnifiedSBOMComparator::generateJSONReport(
   const std::vector<SBOMDifference>& differences)
{
   std::stringstream ss;
   ss << "{\n";
   ss << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
   ss << "  \"differences\": [\n";

   for (size_t i = 0; i < differences.size(); ++i)
   {
      const auto& diff = differences[i];
      ss << "    {\n";
      ss << "      \"type\": \"" << getDifferenceTypeStringLowercase(diff.type) << "\",\n";
      ss << "      \"component\": {\n";
      ss << "        \"name\": \"" << diff.component.name << "\",\n";
      ss << "        \"version\": \"" << diff.component.version << "\",\n";
      ss << "        \"type\": \"" << diff.component.type << "\"\n";
      ss << "      }\n";
      ss << "    }";

      if (i + 1 < differences.size())
      {
         ss << ",";
      }
      ss << "\n";
   }

   ss << "  ]\n";
   ss << "}";

   return ss.str();
}

std::string UnifiedSBOMComparator::generateCSVReport(const std::vector<SBOMDifference>& differences)
{
   std::stringstream ss;
   ss << "Type,Name,Version,Type,License,Description\n";

   for (const auto& diff : differences)
   {
      ss << getDifferenceTypeStringLowercase(diff.type) << ","
         << "\"" << diff.component.name << "\","
         << "\"" << diff.component.version << "\","
         << "\"" << diff.component.type << "\","
         << "\"" << diff.component.license << "\","
         << "\"" << diff.component.description << "\"\n";
   }

   return ss.str();
}

std::string UnifiedSBOMComparator::generateTextReport(
   const std::vector<SBOMDifference>& differences)
{
   std::stringstream ss;
   ss << "SBOM Comparison Report\n";
   ss << "Generated: " << getCurrentTimestamp() << "\n\n";

   std::map<std::string, int> stats = getDiffStatistics(differences);
   ss << "Summary:\n";
   ss << "  Added: " << stats["added"] << "\n";
   ss << "  Removed: " << stats["removed"] << "\n";
   ss << "  Modified: " << stats["modified"] << "\n";
   ss << "  Unchanged: " << stats["unchanged"] << "\n\n";

   ss << "Details:\n";
   if (differences.empty())
   {
      ss << "No differences found\n";
   }
   else
   {
      for (const auto& diff : differences)
      {
         ss << "[" << getDifferenceTypeString(diff.type) << "] " << diff.component.name << " "
            << diff.component.version << " (" << diff.component.type << ")\n";
      }
   }

   return ss.str();
}

std::string UnifiedSBOMComparator::getDifferenceTypeString(SBOMDifference::Type type)
{
   switch (type)
   {
      case SBOMDifference::Type::ADDED:
         return "ADDED";
      case SBOMDifference::Type::REMOVED:
         return "REMOVED";
      case SBOMDifference::Type::MODIFIED:
         return "MODIFIED";
      case SBOMDifference::Type::UNCHANGED:
         return "UNCHANGED";
      default:
         return "UNKNOWN";
   }
}

std::string UnifiedSBOMComparator::getDifferenceTypeStringLowercase(SBOMDifference::Type type)
{
   switch (type)
   {
      case SBOMDifference::Type::ADDED:
         return "added";
      case SBOMDifference::Type::REMOVED:
         return "removed";
      case SBOMDifference::Type::MODIFIED:
         return "modified";
      case SBOMDifference::Type::UNCHANGED:
         return "unchanged";
      default:
         return "unknown";
   }
}

std::string UnifiedSBOMComparator::getCurrentTimestamp()
{
   auto now = std::chrono::system_clock::now();
   auto time_t = std::chrono::system_clock::to_time_t(now);

   auto* tm_ptr = std::gmtime(&time_t);
   std::stringstream ss;
   ss << std::setfill('0') << std::setw(4) << tm_ptr->tm_year + 1900 << "-"
      << std::setw(2) << tm_ptr->tm_mon + 1 << "-"
      << std::setw(2) << tm_ptr->tm_mday << "T"
      << std::setw(2) << tm_ptr->tm_hour << ":"
      << std::setw(2) << tm_ptr->tm_min << ":"
      << std::setw(2) << tm_ptr->tm_sec << "Z";
   return ss.str();
}

// SBOMComparatorFactory Implementation

std::unique_ptr<UnifiedSBOMComparator> SBOMComparatorFactory::createComparator(
   const std::string& format)
{
   // Currently, we always return a UnifiedSBOMComparator
   // The format parameter is kept for future extensibility
   (void)format;  // Suppress unused parameter warning
   return std::make_unique<UnifiedSBOMComparator>();
}

std::vector<std::string> SBOMComparatorFactory::getSupportedFormats()
{
   return {"spdx", "cyclonedx"};
}

std::vector<std::string> SBOMComparatorFactory::getSupportedVersions(const std::string& format)
{
   if (format == "spdx")
   {
      return {"2.3", "3.0.0", "3.0.1"};
   }
   else if (format == "cyclonedx")
   {
      return {"1.4", "1.5", "1.6"};
   }
   else
   {
      return {};
   }
}

}  // namespace heimdall
