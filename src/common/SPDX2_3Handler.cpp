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
 * @file SPDX2_3Handler.cpp
 * @brief SPDX 2.3 format handler implementation
 * @author Trevor Bakker
 * @date 2025
 */

#include <ctime>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include "SPDXHandler.hpp"
#include "Utils.hpp"

namespace heimdall
{

// BaseSPDXHandler implementation
BaseSPDXHandler::BaseSPDXHandler(const std::string& version) : version(version)
{
   // No validator needed - validation is implemented directly
}

bool BaseSPDXHandler::supportsFeature(const std::string& feature) const
{
   // SPDX 2.3 supports basic features
   if (feature == "tag_value" || feature == "relationships" || feature == "checksums")
   {
      return true;
   }
   return false;
}

void BaseSPDXHandler::setVersion(const std::string& newVersion)
{
   version = newVersion;
}

std::vector<std::string> BaseSPDXHandler::getSupportedVersions() const
{
   return {"2.3", "3.0.0", "3.0.1"};
}

ValidationResult BaseSPDXHandler::validateContent(const std::string& content)
{
   ValidationResult result;
   
   // Check if this is JSON format (SPDX 3.0)
   if (content.find("\"@context\"") != std::string::npos && content.find("\"@graph\"") != std::string::npos)
   {
      // SPDX 3.0 JSON format validation
      if (content.find("\"specVersion\"") == std::string::npos)
      {
         result.addError("Missing required field: specVersion");
      }
      else
      {
         // Extract specVersion
         size_t specVersionPos = content.find("\"specVersion\"");
         size_t valueStart = content.find("\"", specVersionPos + 14);
         if (valueStart != std::string::npos)
         {
            size_t valueEnd = content.find("\"", valueStart + 1);
            if (valueEnd != std::string::npos)
            {
               std::string specVersion = content.substr(valueStart + 1, valueEnd - valueStart - 1);
               if (specVersion == "SPDX-3.0.0")
               {
                  result.addMetadata("format", "SPDX 3.0");
                  result.addMetadata("version", "3.0");
               }
               else
               {
                  result.addError("Invalid SPDX specVersion: " + specVersion);
               }
            }
         }
      }
      
      if (content.find("\"name\"") == std::string::npos)
      {
         result.addError("Missing required field: name");
      }
      
      if (content.find("\"documentNamespace\"") == std::string::npos)
      {
         result.addError("Missing required field: documentNamespace");
      }
      
      if (content.find("\"creationInfo\"") == std::string::npos)
      {
         result.addError("Missing required field: creationInfo");
      }
      
      if (content.find("\"dataLicense\"") == std::string::npos)
      {
         result.addError("Missing required field: dataLicense");
      }
   }
   else
   {
      // SPDX 2.3 tag-value format validation
      std::istringstream iss(content);
      std::string line;
      bool hasSPDXVersion = false;
      bool hasDataLicense = false;
      bool hasDocumentName = false;
      bool hasDocumentNamespace = false;
      bool hasCreator = false;
      bool hasCreated = false;
      
      while (std::getline(iss, line))
      {
         std::string trimmed = line;
         trimmed.erase(0, trimmed.find_first_not_of(" \t"));
         trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
         
         if (trimmed.empty() || trimmed[0] == '#')
            continue;
            
         if (trimmed.find("SPDXVersion:") == 0)
         {
            hasSPDXVersion = true;
            std::string version = trimmed.substr(12);
            version.erase(0, version.find_first_not_of(" \t"));
            if (version == "SPDX-2.3")
            {
               result.addMetadata("format", "SPDX 2.3");
               result.addMetadata("version", "2.3");
            }
            else
            {
               result.addError("Invalid SPDX version: " + version);
            }
         }
         else if (trimmed.find("DataLicense:") == 0)
         {
            hasDataLicense = true;
         }
         else if (trimmed.find("DocumentName:") == 0)
         {
            hasDocumentName = true;
         }
         else if (trimmed.find("DocumentNamespace:") == 0)
         {
            hasDocumentNamespace = true;
         }
         else if (trimmed.find("Creator:") == 0)
         {
            hasCreator = true;
         }
         else if (trimmed.find("Created:") == 0)
         {
            hasCreated = true;
         }
         else if (trimmed.find("SPDXID:") == 0)
         {
            // Validate SPDXID format
            std::string spdxid = trimmed.substr(7);
            spdxid.erase(0, spdxid.find_first_not_of(" \t"));
            spdxid.erase(spdxid.find_last_not_of(" \t") + 1);
            
            // SPDXID should start with "SPDXRef-" and contain only alphanumeric characters, hyphens, and underscores
            if (spdxid.find("SPDXRef-") != 0)
            {
               result.addError("Invalid SPDXID format: must start with 'SPDXRef-'");
            }
            else
            {
               std::string idPart = spdxid.substr(8); // Skip "SPDXRef-"
               for (char c : idPart)
               {
                  if (!std::isalnum(c) && c != '-' && c != '_')
                  {
                     result.addError("Invalid SPDXID format: contains invalid character '" + std::string(1, c) + "'");
                     break;
                  }
               }
            }
         }
      }
      
      // Check required fields
      if (!hasSPDXVersion)
         result.addError("Missing required field: SPDXVersion");
      if (!hasDataLicense)
         result.addError("Missing required field: DataLicense");
      if (!hasDocumentName)
         result.addError("Missing required field: DocumentName");
      if (!hasDocumentNamespace)
         result.addError("Missing required field: DocumentNamespace");
      if (!hasCreator)
         result.addError("Missing required field: Creator");
      if (!hasCreated)
         result.addError("Missing required field: Created");
   }
   
   return result;
}

std::string BaseSPDXHandler::generateSPDXId(const std::string& name) const
{
   std::string id = "SPDXRef-";
   for (char c : name)
   {
      if (std::isalnum(c) || c == '-' || c == '_')
      {
         id += c;
      }
      else
      {
         id += '_';
      }
   }
   return id;
}

std::string BaseSPDXHandler::generateDocumentNamespace() const
{
   return "https://spdx.org/spdxdocs/heimdall-" + getCurrentTimestamp();
}

std::string BaseSPDXHandler::getCurrentTimestamp() const
{
   auto              now    = std::chrono::system_clock::now();
   auto              time_t = std::chrono::system_clock::to_time_t(now);
   std::stringstream ss;
   ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
   return ss.str();
}

std::string BaseSPDXHandler::generateVerificationCode() const
{
   // Simplified verification code generation
   return "d6a770ba38583ed4bb4525bd96e50461655d2758";
}

std::string BaseSPDXHandler::generatePURL(const ComponentInfo& component) const
{
   // Generate Package URL based on component information
   std::string purl = "pkg:generic/";
   purl += component.name;
   if (!component.version.empty())
   {
      purl += "@" + component.version;
   }
   return purl;
}

std::vector<ComponentInfo> BaseSPDXHandler::parseContent(const std::string& content)
{
   // Basic SPDX parsing implementation
   std::vector<ComponentInfo> components;

   // Simple regex-based parsing for SPDX tag-value format
   std::regex         packageRegex(R"(PackageName:\s*(.+))");
   std::regex         versionRegex(R"(PackageVersion:\s*(.+))");
   std::regex         licenseRegex(R"(PackageLicenseConcluded:\s*(.+))");

   std::smatch        match;
   ComponentInfo      component;

   std::istringstream iss(content);
   std::string        line;

   while (std::getline(iss, line))
   {
      if (std::regex_search(line, match, packageRegex))
      {
         if (!component.name.empty())
         {
            components.push_back(component);
            component = ComponentInfo();
         }
         component.name = match[1].str();
      }
      else if (std::regex_search(line, match, versionRegex))
      {
         component.version = match[1].str();
      }
      else if (std::regex_search(line, match, licenseRegex))
      {
         component.license = match[1].str();
      }
   }

   if (!component.name.empty())
   {
      components.push_back(component);
   }

   return components;
}

std::vector<ComponentInfo> BaseSPDXHandler::parseFile(const std::string& filePath)
{
   std::ifstream file(filePath);
   if (!file.is_open())
   {
      return {};
   }

   std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

   return parseContent(content);
}

std::string BaseSPDXHandler::generateSPDXLicenseId(const std::string& license) const
{
   // Map common licenses to SPDX identifiers
   if (license.find("MIT") != std::string::npos)
   {
      return "MIT";
   }
   else if (license.find("Apache") != std::string::npos)
   {
      return "Apache-2.0";
   }
   else if (license.find("GPL") != std::string::npos)
   {
      return "GPL-2.0-only";
   }
   else if (license.find("BSD") != std::string::npos)
   {
      return "BSD-3-Clause";
   }
   else
   {
      return "NOASSERTION";
   }
}

std::string BaseSPDXHandler::generateSPDXElementId(const std::string& name) const
{
   return generateSPDXId(name);
}

// SPDX2_3Handler implementation
SPDX2_3Handler::SPDX2_3Handler() : BaseSPDXHandler("2.3") {}

std::string SPDX2_3Handler::generateSBOM(
   const std::unordered_map<std::string, ComponentInfo>& components,
   const std::map<std::string, std::string>&             metadata)
{
   std::stringstream ss;

   // Generate header
   ss << generateHeader(metadata);

   // Generate package info
   ss << generatePackageInfo(metadata);

   // Generate file information for each component
   for (const auto& pair : components)
   {
      ss << generateFileInfo(pair.second);
   }

   // Generate relationships
   ss << generateRelationships(components);

   return ss.str();
}

std::string SPDX2_3Handler::generateComponentEntry(const ComponentInfo& component)
{
   return generateFileInfo(component);
}

std::string SPDX2_3Handler::generateHeader(const std::map<std::string, std::string>& metadata) const
{
   std::stringstream ss;
   ss << "SPDXVersion: SPDX-2.3\n";
   ss << "DataLicense: CC0-1.0\n";
   ss << "SPDXID: SPDXRef-DOCUMENT\n";

   std::string documentName = "Heimdall Generated SBOM";
   if (metadata.find("document_name") != metadata.end())
   {
      documentName = metadata.at("document_name");
   }
   ss << "DocumentName: " << documentName << "\n";

   ss << "DocumentNamespace: " << generateDocumentNamespace() << "\n";
   ss << "Creator: Tool: Heimdall\n";
   ss << "Created: " << getCurrentTimestamp() << "\n\n";

   return ss.str();
}

std::string SPDX2_3Handler::generatePackageInfo(
   const std::map<std::string, std::string>& metadata) const
{
   std::stringstream ss;
   ss << "PackageName: heimdall-generated-sbom\n";
   ss << "SPDXID: SPDXRef-Package\n";
   ss << "PackageVersion: 1.0.0\n";
   ss << "PackageDownloadLocation: NOASSERTION\n";
   ss << "PackageLicenseConcluded: NOASSERTION\n";
   ss << "PackageLicenseDeclared: NOASSERTION\n";
   ss << "PackageCopyrightText: NOASSERTION\n";
   ss << "PackageVerificationCode: " << generateVerificationCode() << " (excludes: ./)\n\n";

   return ss.str();
}

std::string SPDX2_3Handler::generateFileInfo(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << "FileName: " << component.name << "\n";
   ss << "SPDXID: " << generateSPDXId(component.name) << "\n";
   ss << "FileType: " << component.getFileTypeString("2.3") << "\n";

   // Add checksums
   ss << generateFileChecksums(component);

   // Add comment
   ss << generateFileComment(component);

   return ss.str();
}

std::string SPDX2_3Handler::generateRelationships(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream ss;

   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      ss << "Relationship: SPDXRef-Package CONTAINS " << generateSPDXId(component.name) << "\n";

      // Add dependency relationships
      for (const auto& dep : component.dependencies)
      {
         // Find the component that matches this dependency
         for (const auto& depPair : components)
         {
            if (depPair.second.filePath.find(dep) != std::string::npos)
            {
               ss << "Relationship: " << generateSPDXId(component.name) << " DEPENDS_ON "
                  << generateSPDXId(depPair.second.name) << "\n";
               break;
            }
         }
      }
   }

   return ss.str();
}

std::string SPDX2_3Handler::generateFileChecksums(const ComponentInfo& component) const
{
   std::stringstream ss;

   if (!component.checksum.empty())
   {
      ss << "FileChecksum: SHA1: " << component.checksum << "\n";
   }

   return ss.str();
}

std::string SPDX2_3Handler::generateFileComment(const ComponentInfo& component) const
{
   std::stringstream ss;

   if (!component.sourceFiles.empty())
   {
      ss << "FileComment: Source files: ";
      for (size_t i = 0; i < component.sourceFiles.size(); ++i)
      {
         ss << component.sourceFiles[i];
         if (i + 1 < component.sourceFiles.size())
         {
            ss << ", ";
         }
      }
      ss << "\n";
   }
   else
   {
      ss << "FileComment: " << component.getFileTypeString("2.3") << " file\n";
   }

   ss << "\n";
   return ss.str();
}

// Override parse methods for SPDX 2.3
std::vector<ComponentInfo> SPDX2_3Handler::parseContent(const std::string& content)
{
   // Use base class implementation for now
   return BaseSPDXHandler::parseContent(content);
}

std::vector<ComponentInfo> SPDX2_3Handler::parseFile(const std::string& filePath)
{
   // Use base class implementation for now
   return BaseSPDXHandler::parseFile(filePath);
}

}  // namespace heimdall