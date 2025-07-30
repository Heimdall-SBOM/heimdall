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
 * @file CycloneDXHandler.cpp
 * @brief CycloneDX format handler implementation
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides the implementation of CycloneDX SBOM format handling,
 * supporting versions 1.4, 1.5, and 1.6 with proper separation of concerns.
 */

#include "CycloneDXHandler.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "SBOMValidator.hpp"
#include "Utils.hpp"

namespace heimdall
{

// BaseCycloneDXHandler implementation

BaseCycloneDXHandler::BaseCycloneDXHandler(const std::string& version) : version(version) {}

void BaseCycloneDXHandler::setVersion(const std::string& version)
{
   this->version = version;
}

std::vector<std::string> BaseCycloneDXHandler::getSupportedVersions() const
{
   return {"1.4", "1.5", "1.6"};
}

ValidationResult BaseCycloneDXHandler::validateContent(const std::string& content)
{
   ValidationResult result;
   
   // Basic CycloneDX validation
   if (content.find("\"bomFormat\"") == std::string::npos)
   {
      result.addError("Missing bomFormat field");
   }
   else
   {
      // Check if bomFormat is "CycloneDX"
      size_t bomFormatPos = content.find("\"bomFormat\"");
      size_t valueStart = content.find("\"", bomFormatPos + 12);
      if (valueStart != std::string::npos)
      {
         size_t valueEnd = content.find("\"", valueStart + 1);
         if (valueEnd != std::string::npos)
         {
            std::string bomFormat = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            if (bomFormat != "CycloneDX")
            {
               result.addError("Invalid bomFormat: " + bomFormat + " (expected CycloneDX)");
            }
         }
      }
   }
   
   if (content.find("\"specVersion\"") == std::string::npos)
   {
      result.addError("Missing specVersion field");
   }
   else
   {
      // Extract and validate specVersion
      size_t specVersionPos = content.find("\"specVersion\"");
      size_t valueStart = content.find("\"", specVersionPos + 14);
      if (valueStart != std::string::npos)
      {
         size_t valueEnd = content.find("\"", valueStart + 1);
         if (valueEnd != std::string::npos)
         {
            std::string specVersion = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            result.addMetadata("version", specVersion);
         }
      }
   }
   
   if (content.find("\"version\"") == std::string::npos)
   {
      result.addError("Missing version field");
   }
   
   if (content.find("\"metadata\"") == std::string::npos)
   {
      result.addError("Missing metadata field");
   }
   
   if (content.find("\"components\"") == std::string::npos)
   {
      result.addError("Missing components field");
   }

   result.addMetadata("format", "CycloneDX");
   return result;
}

std::vector<ComponentInfo> BaseCycloneDXHandler::parseContent(const std::string& content)
{
   std::vector<ComponentInfo> components;

   try
   {
      // Basic JSON parsing to extract components
      // This is a simplified implementation - in a real scenario, you'd want more robust JSON
      // parsing

      // Look for the components array
      size_t componentsStart = content.find("\"components\"");
      if (componentsStart == std::string::npos)
      {
         return components;
      }

      // Find the start of the components array
      size_t arrayStart = content.find('[', componentsStart);
      if (arrayStart == std::string::npos)
      {
         return components;
      }

      // Find the end of the components array
      int    braceCount = 0;
      size_t arrayEnd   = arrayStart;
      bool   inString   = false;
      bool   escaped    = false;

      for (size_t i = arrayStart; i < content.length(); ++i)
      {
         char c = content[i];

         if (escaped)
         {
            escaped = false;
            continue;
         }

         if (c == '\\')
         {
            escaped = true;
            continue;
         }

         if (c == '"')
         {
            inString = !inString;
            continue;
         }

         if (!inString)
         {
            if (c == '{')
            {
               braceCount++;
            }
            else if (c == '}')
            {
               braceCount--;
            }
            else if (c == ']' && braceCount == 0)
            {
               arrayEnd = i;
               break;
            }
         }
      }

      if (arrayEnd == arrayStart)
      {
         return components;
      }

      // Extract the components array content
      std::string componentsArray = content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

      // Parse individual components
      size_t pos = 0;
      while (pos < componentsArray.length())
      {
         // Find the start of a component object
         size_t componentStart = componentsArray.find('{', pos);
         if (componentStart == std::string::npos)
         {
            break;
         }

         // Find the end of this component object
         int    componentBraceCount = 0;
         size_t componentEnd        = componentStart;
         bool   inComponentString   = false;
         bool   componentEscaped    = false;

         for (size_t i = componentStart; i < componentsArray.length(); ++i)
         {
            char c = componentsArray[i];

            if (componentEscaped)
            {
               componentEscaped = false;
               continue;
            }

            if (c == '\\')
            {
               componentEscaped = true;
               continue;
            }

            if (c == '"')
            {
               inComponentString = !inComponentString;
               continue;
            }

            if (!inComponentString)
            {
               if (c == '{')
               {
                  componentBraceCount++;
               }
               else if (c == '}')
               {
                  componentBraceCount--;
                  if (componentBraceCount == 0)
                  {
                     componentEnd = i;
                     break;
                  }
               }
            }
         }

         if (componentEnd == componentStart)
         {
            pos = componentStart + 1;
            continue;
         }

         // Extract the component object
         std::string componentJson =
            componentsArray.substr(componentStart, componentEnd - componentStart + 1);

         // Parse the component
         ComponentInfo component = parseComponentFromJson(componentJson);
         if (!component.name.empty())
         {
            components.push_back(component);
         }

         pos = componentEnd + 1;
      }
   }
   catch (const std::exception& e)
   {
      // Log error but don't throw
      std::cerr << "Error parsing CycloneDX content: " << e.what() << std::endl;
   }

   return components;
}

std::vector<ComponentInfo> BaseCycloneDXHandler::parseFile(const std::string& filePath)
{
   std::vector<ComponentInfo> components;

   try
   {
      std::ifstream file(filePath);
      if (!file.is_open())
      {
         return components;
      }

      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string content = buffer.str();

      return parseContent(content);
   }
   catch (const std::exception& e)
   {
      // Log error but don't throw
      std::cerr << "Error parsing CycloneDX file: " << e.what() << std::endl;
   }

   return components;
}

ComponentInfo BaseCycloneDXHandler::parseComponentFromJson(const std::string& componentJson) const
{
   ComponentInfo component;

   try
   {
      // Extract basic fields using regex
      std::regex  nameRegex("\"name\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  versionRegex("\"version\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  typeRegex("\"type\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  purlRegex("\"purl\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  descriptionRegex("\"description\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  supplierRegex("\"supplier\"\\s*:\\s*\\{\\s*\"name\"\\s*:\\s*\"([^\"]+)\"");
      std::regex  licenseRegex("\"license\"\\s*:\\s*\\{\\s*\"id\"\\s*:\\s*\"([^\"]+)\"");

      std::smatch match;

      if (std::regex_search(componentJson, match, nameRegex))
      {
         component.name = match[1].str();
      }

      if (std::regex_search(componentJson, match, versionRegex))
      {
         component.version = match[1].str();
      }

      if (std::regex_search(componentJson, match, typeRegex))
      {
         std::string type = match[1].str();
         if (type == "application")
         {
            component.fileType = FileType::Executable;
         }
         else if (type == "library")
         {
            component.fileType = FileType::SharedLibrary;
         }
      }

      if (std::regex_search(componentJson, match, purlRegex))
      {
         component.packageManager = extractPackageManagerFromPURL(match[1].str());
      }

      if (std::regex_search(componentJson, match, descriptionRegex))
      {
         component.description = match[1].str();
      }

      if (std::regex_search(componentJson, match, supplierRegex))
      {
         component.supplier = match[1].str();
      }

      if (std::regex_search(componentJson, match, licenseRegex))
      {
         component.license = match[1].str();
      }
   }
   catch (const std::exception& e)
   {
      // Log error but don't throw
      std::cerr << "Error parsing component JSON: " << e.what() << std::endl;
   }

   return component;
}

std::string BaseCycloneDXHandler::extractPackageManagerFromPURL(const std::string& purl) const
{
   // Extract package manager from PURL format: pkg:type/name@version
   std::regex  purlRegex("pkg:([^/]+)/");
   std::smatch match;

   if (std::regex_search(purl, match, purlRegex))
   {
      return match[1].str();
   }

   return "";
}

bool BaseCycloneDXHandler::supportsFeature(const std::string& feature) const
{
   // Convert feature to lowercase for case-insensitive comparison
   std::string lowerFeature = feature;
   std::transform(lowerFeature.begin(), lowerFeature.end(), lowerFeature.begin(), ::tolower);

   if (lowerFeature == "vulnerabilities")
   {
      return version == "1.5" || version == "1.6";
   }
   if (lowerFeature == "formulation")
   {
      return version == "1.5" || version == "1.6";
   }
   if (lowerFeature == "services")
   {
      return version == "1.6";
   }
   if (lowerFeature == "annotations")
   {
      return version == "1.6";
   }
   if (lowerFeature == "compositions")
   {
      return version == "1.6";
   }

   return true;  // Most features are supported across all versions
}

// Utility methods

std::string BaseCycloneDXHandler::generateBOMRef(const std::string& name,
                                                 const std::string& version) const
{
   // Generate a unique BOM reference
   std::string ref = name;
   if (!version.empty())
   {
      ref += "@" + version;
   }

   // Replace invalid characters
   std::replace(ref.begin(), ref.end(), ' ', '-');
   std::replace(ref.begin(), ref.end(), '/', '-');
   std::replace(ref.begin(), ref.end(), '\\', '-');

   return ref;
}

std::string BaseCycloneDXHandler::getCurrentTimestamp() const
{
   auto now    = std::chrono::system_clock::now();
   auto time_t = std::chrono::system_clock::to_time_t(now);
   auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

   std::stringstream ss;
   ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
   ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";

   return ss.str();
}

std::string BaseCycloneDXHandler::generatePURL(const ComponentInfo& component) const
{
   std::stringstream ss;

   if (component.packageManager == "conan")
   {
      ss << "pkg:conan/" << component.name << "@" << component.version;
   }
   else if (component.packageManager == "vcpkg")
   {
      ss << "pkg:vcpkg/" << component.name << "@" << component.version;
   }
   else if (component.packageManager == "system")
   {
      ss << "pkg:system/" << component.name << "@" << component.version;
   }
   else
   {
      ss << "pkg:generic/" << component.name << "@" << component.version;
   }

   return ss.str();
}

std::string BaseCycloneDXHandler::generateCPE(const ComponentInfo& component) const
{
   if (!component.cpe.empty())
   {
      return component.cpe;
   }

   // Generate a basic CPE if not provided
   std::string cpe = "cpe:2.3:a:";

   if (!component.supplier.empty())
   {
      cpe += component.supplier + ":";
   }
   else
   {
      cpe += "unknown:";
   }

   cpe += component.name + ":";
   cpe += (component.version.empty() ? "*" : component.version) + ":*:*:*:*:*:*:*";

   return cpe;
}

std::string BaseCycloneDXHandler::generateCycloneDXLicense(const std::string& license) const
{
   if (license.empty())
   {
      return "";
   }

   // Simple license mapping - in a real implementation, you'd want a more comprehensive mapping
   std::string lowerLicense = license;
   std::transform(lowerLicense.begin(), lowerLicense.end(), lowerLicense.begin(), ::tolower);

   if (lowerLicense.find("apache") != std::string::npos)
   {
      return "Apache-2.0";
   }
   else if (lowerLicense.find("mit") != std::string::npos)
   {
      return "MIT";
   }
   else if (lowerLicense.find("gpl") != std::string::npos)
   {
      if (lowerLicense.find("v3") != std::string::npos)
      {
         return "GPL-3.0";
      }
      else if (lowerLicense.find("v2") != std::string::npos)
      {
         return "GPL-2.0";
      }
      return "GPL-3.0";
   }
   else if (lowerLicense.find("bsd") != std::string::npos)
   {
      return "BSD-3-Clause";
   }

   return license;  // Return as-is if no mapping found
}

std::string BaseCycloneDXHandler::generateComponentType(const ComponentInfo& component) const
{
   switch (component.fileType)
   {
      case FileType::Executable:
         return "application";
      case FileType::SharedLibrary:
      case FileType::StaticLibrary:
         return "library";
      case FileType::Object:
         return "file";
      case FileType::Source:
         return "source";
      default:
         return "unknown";
   }
}

std::string BaseCycloneDXHandler::generateComponentScope(const ComponentInfo& component) const
{
   if (component.scope.empty())
   {
      return "required";
   }
   return component.scope;
}

std::string BaseCycloneDXHandler::generateComponentGroup(const ComponentInfo& component) const
{
   return component.group;
}

std::string BaseCycloneDXHandler::generateComponentMimeType(const ComponentInfo& component) const
{
   if (!component.mimeType.empty())
   {
      return component.mimeType;
   }

   // Generate MIME type based on file type
   switch (component.fileType)
   {
      case FileType::Executable:
         return "application/x-executable";
      case FileType::SharedLibrary:
         return "application/x-sharedlib";
      case FileType::StaticLibrary:
         return "application/x-archive";
      case FileType::Object:
         return "application/x-object";
      default:
         return "application/octet-stream";
   }
}

std::string BaseCycloneDXHandler::generateComponentCopyright(const ComponentInfo& component) const
{
   return component.copyright;
}

std::string BaseCycloneDXHandler::generateComponentSupplier(const ComponentInfo& component) const
{
   return component.supplier;
}

std::string BaseCycloneDXHandler::generateComponentManufacturer(
   const ComponentInfo& component) const
{
   return component.manufacturer;
}

std::string BaseCycloneDXHandler::generateComponentPublisher(const ComponentInfo& component) const
{
   return component.publisher;
}

std::string BaseCycloneDXHandler::generateComponentDescription(const ComponentInfo& component) const
{
   return component.description;
}

std::string BaseCycloneDXHandler::generateComponentProperties(const ComponentInfo& component) const
{
   if (component.properties.empty())
   {
      return "";
   }

   std::stringstream ss;
   ss << "\"properties\": [";

   bool first = true;
   for (const auto& [key, value] : component.properties)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << "{\"name\": \"" << key << "\", \"value\": \"" << value << "\"}";
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string BaseCycloneDXHandler::generateComponentEvidence(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "\"evidence\": {";

   if (!component.symbols.empty())
   {
      ss << "\"identity\": {\"field\": \"symbols\", \"confidence\": 0.9}";
   }

   ss << "}";
   return ss.str();
}

std::string BaseCycloneDXHandler::generateComponentDependencies(
   const ComponentInfo& component) const
{
   if (component.dependencies.empty())
   {
      return "";
   }

   std::stringstream ss;
   ss << "\"dependencies\": [";

   for (size_t i = 0; i < component.dependencies.size(); ++i)
   {
      if (i > 0)
      {
         ss << ",";
      }
      ss << "\"" << generateBOMRef(component.dependencies[i], "") << "\"";
   }

   ss << "]";
   return ss.str();
}

std::string BaseCycloneDXHandler::generateComponentExternalReferences(
   const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "\"externalReferences\": [";

   bool hasRefs = false;

   if (!component.homepage.empty())
   {
      if (hasRefs)
         ss << ",";
      ss << "{\"url\": \"" << component.homepage << "\", \"type\": \"website\"}";
      hasRefs = true;
   }

   if (!component.downloadLocation.empty())
   {
      if (hasRefs)
         ss << ",";
      ss << "{\"url\": \"" << component.downloadLocation << "\", \"type\": \"distribution\"}";
      hasRefs = true;
   }

   ss << "]";
   return ss.str();
}

// CycloneDX1_4Handler implementation

CycloneDX1_4Handler::CycloneDX1_4Handler() : BaseCycloneDXHandler("1.4") {}

std::string CycloneDX1_4Handler::generateSBOM(
   const std::unordered_map<std::string, ComponentInfo>& components,
   const std::map<std::string, std::string>&             metadata)
{
   std::stringstream ss;
   ss << "{\n";
   ss << "  \"bomFormat\": \"CycloneDX\",\n";
   ss << "  \"specVersion\": \"1.4\",\n";
   ss << "  \"version\": 1,\n";
   ss << "  \"metadata\": " << generateMetadata(metadata) << ",\n";
   ss << "  \"components\": " << generateComponents(components);
   ss << "\n}";

   return ss.str();
}

std::string CycloneDX1_4Handler::generateComponentEntry(const ComponentInfo& component)
{
   return generateComponent(component);
}

std::string CycloneDX1_4Handler::generateMetadata(
   const std::map<std::string, std::string>& metadata) const
{
   std::stringstream ss;
   ss << "{\n";
   ss << "    \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
   ss << "    \"tools\": [\n";
   ss << "      {\n";
   ss << "        \"vendor\": \"Heimdall Project\",\n";
   ss << "        \"name\": \"Heimdall SBOM Generator\",\n";
   ss << "        \"version\": \"1.0.0\"\n";
   ss << "      }\n";
   ss << "    ],\n";
   ss << "    \"component\": {\n";
   ss << "      \"type\": \"application\",\n";
   ss << "      \"name\": \"Unknown\",\n";
   ss << "      \"version\": \"Unknown\",\n";
   ss << "      \"supplier\": {\n";
   ss << "        \"name\": \"Heimdall Project\"\n";
   ss << "      },\n";
   ss << "      \"copyright\": \"Copyright 2025 Heimdall Project. Licensed under Apache License "
         "2.0.\",\n";
   ss << "      \"licenses\": [\n";
   ss << "        {\n";
   ss << "          \"license\": {\n";
   ss << "            \"id\": \"Apache-2.0\",\n";
   ss << "            \"name\": \"Apache License 2.0\",\n";
   ss << "            \"url\": \"https://www.apache.org/licenses/LICENSE-2.0\",\n";
   ss << "            \"licensing\": {\n";
   ss << "              \"licenseTypes\": [\"perpetual\"]\n";
   ss << "            }\n";
   ss << "          }\n";
   ss << "        }\n";
   ss << "      ]\n";
   ss << "    }";

   if (!metadata.empty())
   {
      ss << ",\n    \"properties\": [";
      bool first = true;
      for (const auto& [key, value] : metadata)
      {
         if (!first)
         {
            ss << ",";
         }
         ss << "\n      {\n";
         ss << "        \"name\": " << Utils::formatJsonValue(key) << ",\n";
         ss << "        \"value\": " << Utils::formatJsonValue(value) << "\n";
         ss << "      }";
         first = false;
      }
      ss << "\n    ]";
   }

   ss << "\n  }";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateComponents(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[\n";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",\n";
      }
      ss << generateComponent(component);
      first = false;
   }

   ss << "\n  ]";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateComponent(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "    {\n";

   // Generate BOM reference based on component name and version
   std::string bomRef = component.name;
   if (!component.version.empty() && component.version != "UNKNOWN")
   {
      bomRef += "-" + component.version;
   }

   ss << "      \"bom-ref\": \"" << bomRef << "\",\n";

   // Determine the correct component type based on file type
   std::string componentType = "library";
   if (component.fileType == FileType::Executable)
   {
      componentType = "application";
   }
   else if (component.fileType == FileType::StaticLibrary)
   {
      componentType = "library";
   }
   else if (component.fileType == FileType::SharedLibrary)
   {
      componentType = "library";
   }
   else if (component.fileType == FileType::Object)
   {
      componentType = "library";
   }

   ss << "      \"type\": \"" << componentType << "\",\n";
   ss << "      \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
   ss << "      \"version\": "
      << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version) << ",\n";

   // Enhanced description - only include if not empty
   if (!component.description.empty())
   {
      ss << "      \"description\": " << Utils::formatJsonValue(component.description) << ",\n";
   }

   // Scope - only include if not empty
   if (!component.scope.empty())
   {
      ss << "      \"scope\": \"" << component.scope << "\",\n";
   }

   // Group - only include if not empty
   if (!component.group.empty())
   {
      ss << "      \"group\": " << Utils::formatJsonValue(component.group) << ",\n";
   }

   // MIME type - only include if not empty
   if (!component.mimeType.empty())
   {
      ss << "      \"mime-type\": " << Utils::formatJsonValue(component.mimeType) << ",\n";
   }

   // Copyright - only include if not empty
   if (!component.copyright.empty())
   {
      ss << "      \"copyright\": " << Utils::formatJsonValue(component.copyright) << ",\n";
   }

   // CPE - only include if not empty
   if (!component.cpe.empty())
   {
      ss << "      \"cpe\": " << Utils::formatJsonValue(component.cpe) << ",\n";
   }

   // Supplier - always include with a default value if empty
   ss << "      \"supplier\": {\n";
   ss << "        \"name\": "
      << Utils::formatJsonValue(component.supplier.empty() ? "Unknown" : component.supplier)
      << "\n";
   ss << "      },\n";

   // Manufacturer - always include with a default value if empty
   ss << "      \"manufacturer\": {\n";
   ss << "        \"name\": "
      << Utils::formatJsonValue(component.manufacturer.empty() ? "Unknown" : component.manufacturer)
      << "\n";
   ss << "      },\n";

   // Publisher - only include if not empty, and as object with name field
   if (!component.publisher.empty())
   {
      ss << "      \"publisher\": {\n";
      ss << "        \"name\": " << Utils::formatJsonValue(component.publisher) << "\n";
      ss << "      },\n";
   }

   // Only include hash if we have a valid checksum
   if (!component.checksum.empty() && component.checksum.length() == 64)
   {
      ss << "      \"hashes\": [\n";
      ss << "        {\n";
      ss << "          \"alg\": \"SHA-256\",\n";
      ss << "          \"content\": \"" << component.checksum << "\"\n";
      ss << "        }\n";
      ss << "      ],\n";
   }
   ss << "      \"purl\": \"" << generatePURL(component) << "\"";

   // External references - only include if we have any
   bool hasExternalRefs = !component.downloadLocation.empty() || !component.homepage.empty() ||
                          std::any_of(component.properties.begin(), component.properties.end(),
                                      [](const std::pair<const std::string, std::string>& prop)
                                      { return prop.first.find("external:") == 0; });

   if (hasExternalRefs)
   {
      ss << ",\n      \"externalReferences\": [\n";

      bool firstRef = true;

      // Download location
      if (!component.downloadLocation.empty())
      {
         ss << "        {\n";
         ss << "          \"type\": \"distribution\",\n";
         ss << "          \"url\": " << Utils::formatJsonValue(component.downloadLocation) << "\n";
         ss << "        }";
         firstRef = false;
      }

      // Homepage
      if (!component.homepage.empty())
      {
         if (!firstRef)
         {
            ss << ",\n";
         }
         ss << "        {\n";
         ss << "          \"type\": \"website\",\n";
         ss << "          \"url\": " << Utils::formatJsonValue(component.homepage) << "\n";
         ss << "        }";
         firstRef = false;
      }

      // Additional external references from properties
      for (const auto& property : component.properties)
      {
         const std::string& key   = property.first;
         const std::string& value = property.second;
         if (key.find("external:") == 0)
         {
            if (!firstRef)
            {
               ss << ",\n";
            }
            ss << "        {\n";
            ss << "          \"type\": \"" << key.substr(9) << "\",\n";
            ss << "          \"url\": " << Utils::formatJsonValue(value) << "\n";
            ss << "        }";
            firstRef = false;
         }
      }

      ss << "\n      ]";
   }

   // Add all component properties (including enhanced Ada metadata and Mach-O metadata)
   if (!component.properties.empty() || component.containsDebugInfo ||
       !component.buildConfig.targetPlatform.empty() ||
       !component.platformInfo.architecture.empty() || component.codeSignInfo.isHardenedRuntime ||
       component.codeSignInfo.isAdHocSigned || !component.codeSignInfo.signer.empty() ||
       !component.codeSignInfo.teamId.empty() || !component.codeSignInfo.certificateHash.empty() ||
       !component.architectures.empty() || !component.entitlements.empty() ||
       !component.frameworks.empty())
   {
      ss << ",\n" << generateComponentProperties(component);
   }

   ss << "\n    }";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateDependencies(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << "{\"ref\": \"" << generateBOMRef(component.name, component.version) << "\",";
      ss << "\"dependsOn\": [";

      for (size_t i = 0; i < component.dependencies.size(); ++i)
      {
         if (i > 0)
         {
            ss << ",";
         }
         ss << "\"" << generateBOMRef(component.dependencies[i], "") << "\"";
      }

      ss << "]}";
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateLicenses(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"license\": {\"id\": \"" << generateCycloneDXLicense(component.license) << "\"}}]";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateHashes(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"alg\": \"SHA-256\", \"content\": \"" << component.checksum << "\"}]";
   return ss.str();
}

std::string CycloneDX1_4Handler::generateExternalReferences(const ComponentInfo& component) const
{
   return generateComponentExternalReferences(component);
}

// CycloneDX1_5Handler implementation

CycloneDX1_5Handler::CycloneDX1_5Handler() : BaseCycloneDXHandler("1.5") {}

std::string CycloneDX1_5Handler::generateSBOM(
   const std::unordered_map<std::string, ComponentInfo>& components,
   const std::map<std::string, std::string>&             metadata)
{
   std::stringstream ss;
   ss << "{";
   ss << "\"bomFormat\": \"CycloneDX\",";
   ss << "\"specVersion\": \"1.5\",";
   ss << "\"serialNumber\": \"urn:uuid:" << generateBOMRef("heimdall", "1.0") << "\",";
   ss << "\"version\": 1,";
   ss << "\"metadata\": " << generateMetadata(metadata) << ",";
   ss << "\"components\": " << generateComponents(components);
   ss << "}";

   return ss.str();
}

std::string CycloneDX1_5Handler::generateComponentEntry(const ComponentInfo& component)
{
   return generateComponent(component);
}

std::string CycloneDX1_5Handler::generateMetadata(
   const std::map<std::string, std::string>& metadata) const
{
   std::stringstream ss;
   ss << "{";
   ss << "\"timestamp\": \"" << getCurrentTimestamp() << "\",";
   ss << "\"tools\": [{";
   ss << "\"vendor\": \"Heimdall\",";
   ss << "\"name\": \"SBOM Generator\",";
   ss << "\"version\": \"1.0\"";
   ss << "}]";

   if (!metadata.empty())
   {
      ss << ",\"properties\": [";
      bool first = true;
      for (const auto& [key, value] : metadata)
      {
         if (!first)
         {
            ss << ",";
         }
         ss << "{\"name\": \"" << key << "\", \"value\": \"" << value << "\"}";
         first = false;
      }
      ss << "]";
   }

   ss << "}";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateComponents(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << generateComponent(component);
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateComponent(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "{";
   ss << "\"type\": \"" << generateComponentType(component) << "\",";
   ss << "\"bom-ref\": \"" << generateBOMRef(component.name, component.version) << "\",";
   ss << "\"name\": \"" << component.name << "\"";

   if (!component.version.empty())
   {
      ss << ",\"version\": \"" << component.version << "\"";
   }

   if (!component.description.empty())
   {
      ss << ",\"description\": \"" << component.description << "\"";
   }

   if (!component.supplier.empty())
   {
      ss << ",\"supplier\": {\"name\": \"" << component.supplier << "\"}";
   }

   if (!component.license.empty())
   {
      ss << ",\"licenses\": " << generateLicenses(component);
   }

   if (!component.cpe.empty())
   {
      ss << ",\"cpe\": \"" << component.cpe << "\"";
   }

   if (!component.packageManager.empty() || !component.name.empty())
   {
      ss << ",\"purl\": \"" << generatePURL(component) << "\"";
   }

   if (!component.checksum.empty())
   {
      ss << ",\"hashes\": " << generateHashes(component);
   }

   if (!component.dependencies.empty())
   {
      ss << ",\"dependencies\": " << generateDependencies({{component.name, component}});
   }

   if (!component.properties.empty())
   {
      ss << "," << generateComponentProperties(component);
   }

   // 1.5 specific features
   ss << "," << generateComponentEvidence(component);

   ss << "}";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateDependencies(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << "{\"ref\": \"" << generateBOMRef(component.name, component.version) << "\",";
      ss << "\"dependsOn\": [";

      for (size_t i = 0; i < component.dependencies.size(); ++i)
      {
         if (i > 0)
         {
            ss << ",";
         }
         ss << "\"" << generateBOMRef(component.dependencies[i], "") << "\"";
      }

      ss << "]}";
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateLicenses(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"license\": {\"id\": \"" << generateCycloneDXLicense(component.license) << "\"}}]";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateHashes(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"alg\": \"SHA-256\", \"content\": \"" << component.checksum << "\"}]";
   return ss.str();
}

std::string CycloneDX1_5Handler::generateExternalReferences(const ComponentInfo& component) const
{
   return generateComponentExternalReferences(component);
}

std::string CycloneDX1_5Handler::generateVulnerabilities(const ComponentInfo& component) const
{
   // Placeholder for vulnerability information
   return "\"vulnerabilities\": []";
}

std::string CycloneDX1_5Handler::generateFormulation(const ComponentInfo& component) const
{
   // Placeholder for formulation information
   return "\"formulation\": null";
}

// CycloneDX1_6Handler implementation

CycloneDX1_6Handler::CycloneDX1_6Handler() : BaseCycloneDXHandler("1.6") {}

std::string CycloneDX1_6Handler::generateSBOM(
   const std::unordered_map<std::string, ComponentInfo>& components,
   const std::map<std::string, std::string>&             metadata)
{
   std::stringstream ss;
   ss << "{";
   ss << "\"bomFormat\": \"CycloneDX\",";
   ss << "\"specVersion\": \"1.6\",";
   ss << "\"serialNumber\": \"urn:uuid:" << generateBOMRef("heimdall", "1.0") << "\",";
   ss << "\"version\": 1,";
   ss << "\"metadata\": " << generateMetadata(metadata) << ",";
   ss << "\"components\": " << generateComponents(components);
   ss << "}";

   return ss.str();
}

std::string CycloneDX1_6Handler::generateComponentEntry(const ComponentInfo& component)
{
   return generateComponent(component);
}

std::string CycloneDX1_6Handler::generateMetadata(
   const std::map<std::string, std::string>& metadata) const
{
   std::stringstream ss;
   ss << "{";
   ss << "\"timestamp\": \"" << getCurrentTimestamp() << "\",";
   ss << "\"tools\": [{";
   ss << "\"vendor\": \"Heimdall\",";
   ss << "\"name\": \"SBOM Generator\",";
   ss << "\"version\": \"1.0\"";
   ss << "}]";

   if (!metadata.empty())
   {
      ss << ",\"properties\": [";
      bool first = true;
      for (const auto& [key, value] : metadata)
      {
         if (!first)
         {
            ss << ",";
         }
         ss << "{\"name\": \"" << key << "\", \"value\": \"" << value << "\"}";
         first = false;
      }
      ss << "]";
   }

   ss << "}";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateComponents(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << generateComponent(component);
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateComponent(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "{";
   ss << "\"type\": \"" << generateComponentType(component) << "\",";
   ss << "\"bom-ref\": \"" << generateBOMRef(component.name, component.version) << "\",";
   ss << "\"name\": \"" << component.name << "\"";

   if (!component.version.empty())
   {
      ss << ",\"version\": \"" << component.version << "\"";
   }

   if (!component.description.empty())
   {
      ss << ",\"description\": \"" << component.description << "\"";
   }

   if (!component.supplier.empty())
   {
      ss << ",\"supplier\": {\"name\": \"" << component.supplier << "\"}";
   }

   if (!component.license.empty())
   {
      ss << ",\"licenses\": " << generateLicenses(component);
   }

   if (!component.cpe.empty())
   {
      ss << ",\"cpe\": \"" << component.cpe << "\"";
   }

   if (!component.packageManager.empty() || !component.name.empty())
   {
      ss << ",\"purl\": \"" << generatePURL(component) << "\"";
   }

   if (!component.checksum.empty())
   {
      ss << ",\"hashes\": " << generateHashes(component);
   }

   if (!component.dependencies.empty())
   {
      ss << ",\"dependencies\": " << generateDependencies({{component.name, component}});
   }

   if (!component.properties.empty())
   {
      ss << "," << generateComponentProperties(component);
   }

   // 1.6 specific features
   ss << "," << generateComponentEvidence(component);

   ss << "}";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateDependencies(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;
   ss << "[";

   bool first = true;
   for (const auto& [name, component] : components)
   {
      if (!first)
      {
         ss << ",";
      }
      ss << "{\"ref\": \"" << generateBOMRef(component.name, component.version) << "\",";
      ss << "\"dependsOn\": [";

      for (size_t i = 0; i < component.dependencies.size(); ++i)
      {
         if (i > 0)
         {
            ss << ",";
         }
         ss << "\"" << generateBOMRef(component.dependencies[i], "") << "\"";
      }

      ss << "]}";
      first = false;
   }

   ss << "]";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateLicenses(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"license\": {\"id\": \"" << generateCycloneDXLicense(component.license) << "\"}}]";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateHashes(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "[{\"alg\": \"SHA-256\", \"content\": \"" << component.checksum << "\"}]";
   return ss.str();
}

std::string CycloneDX1_6Handler::generateExternalReferences(const ComponentInfo& component) const
{
   return generateComponentExternalReferences(component);
}

std::string CycloneDX1_6Handler::generateVulnerabilities(const ComponentInfo& component) const
{
   // Placeholder for vulnerability information
   return "\"vulnerabilities\": []";
}

std::string CycloneDX1_6Handler::generateFormulation(const ComponentInfo& component) const
{
   // Placeholder for formulation information
   return "\"formulation\": null";
}

std::string CycloneDX1_6Handler::generateServices(const ComponentInfo& component) const
{
   // Placeholder for services information
   return "\"services\": []";
}

std::string CycloneDX1_6Handler::generateAnnotations(const ComponentInfo& component) const
{
   // Placeholder for annotations information
   return "\"annotations\": []";
}

std::string CycloneDX1_6Handler::generateCompositions(const ComponentInfo& component) const
{
   // Placeholder for compositions information
   return "\"compositions\": []";
}

}  // namespace heimdall