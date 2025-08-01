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
 * @file SPDX3_0_0Handler.cpp
 * @brief SPDX 3.0.0 format handler implementation
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

// SPDX3_0_0Handler implementation
SPDX3_0_0Handler::SPDX3_0_0Handler() : BaseSPDXHandler("3.0.0") {}

std::string SPDX3_0_0Handler::generateSBOM(
   const std::unordered_map<std::string, ComponentInfo>& components,
   const std::map<std::string, std::string>&             metadata)
{
   std::stringstream sbom;

   // Generate SPDX 3.0.0 document structure
   sbom << generateSPDX3Document(metadata);

   return sbom.str();
}

std::string SPDX3_0_0Handler::generateComponentEntry(const ComponentInfo& component)
{
   std::stringstream entry;

   // Generate SPDX 3.0.0 element format
   entry << generateSPDX3Element(component);

   return entry.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Document(
   const std::map<std::string, std::string>& metadata) const
{
   std::stringstream doc;

   doc << "{\n";
   doc << "  \"spdxVersion\": \"SPDX-3.0\",\n";
   doc << "  \"dataLicense\": \"CC0-1.0\",\n";
   doc << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
   doc << "  \"name\": \"Heimdall SBOM Document\",\n";
   doc << "  \"documentNamespace\": \"" << generateDocumentNamespace() << "\",\n";
   doc << "  \"creationInfo\": " << generateSPDX3CreationInfo() << ",\n";
   doc << "  \"elements\": []\n";
   doc << "}\n";

   return doc.str();
}

std::string SPDX3_0_0Handler::generateSPDX3CreationInfo() const
{
   std::stringstream info;

   info << "{\n";
   info << "  \"creators\": [\"Tool: Heimdall-SBOM-Generator\"],\n";
   info << "  \"created\": \"" << getCurrentTimestamp() << "\"\n";
   info << "}";

   return info.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Elements(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream elements;

   elements << "[\n";
   bool first = true;
   for (const auto& pair : components)
   {
      if (!first)
         elements << ",\n";
      elements << generateSPDX3Element(pair.second);
      first = false;
   }
   elements << "\n]";

   return elements.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Element(const ComponentInfo& component) const
{
   std::stringstream element;

   element << "{\n";
   element << "  \"SPDXID\": \"" << generateSPDXId(component.name) << "\",\n";
   element << "  \"elementType\": \"Package\",\n";
   element << "  \"name\": \"" << Utils::escapeJsonString(component.name) << "\"";

   if (!component.version.empty())
   {
      element << ",\n  \"versionInfo\": \"" << Utils::escapeJsonString(component.version) << "\"";
   }

   if (!component.description.empty())
   {
      element << ",\n  \"description\": \"" << Utils::escapeJsonString(component.description)
              << "\"";
   }

   if (!component.license.empty())
   {
      element << ",\n  \"licenseConcluded\": \"" << Utils::escapeJsonString(component.license)
              << "\"";
   }

   element << "\n}";

   return element.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Relationships(
   const std::unordered_map<std::string, ComponentInfo>& components) const
{
   std::stringstream relationships;

   relationships << "[\n";
   bool first = true;

   for (const auto& pair : components)
   {
      for (const auto& dep : pair.second.dependencies)
      {
         if (!first)
            relationships << ",\n";
         relationships << "{\n";
         relationships << "  \"spdxElementId\": \"" << generateSPDXId(pair.first) << "\",\n";
         relationships << "  \"relatedSpdxElement\": \"" << generateSPDXId(dep) << "\",\n";
         relationships << "  \"relationshipType\": \"DEPENDS_ON\"\n";
         relationships << "}";
         first = false;
      }
   }

   relationships << "\n]";

   return relationships.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Properties(const ComponentInfo& component) const
{
   std::stringstream properties;

   properties << "[\n";
   bool first = true;

   if (!component.packageManager.empty())
   {
      if (!first)
         properties << ",\n";
      properties << "{\n";
      properties << "  \"propertyName\": \"PackageManager\",\n";
      properties << "  \"propertyValue\": \"" << Utils::escapeJsonString(component.packageManager)
                 << "\"\n";
      properties << "}";
      first = false;
   }

   properties << "\n]";

   return properties.str();
}

std::string SPDX3_0_0Handler::generateSPDX3Evidence(const ComponentInfo& component) const
{
   // SPDX 3.0.0 evidence generation (simplified)
   std::stringstream evidence;

   evidence << "{\n";
   evidence << "  \"evidenceType\": \"source\",\n";
   evidence << "  \"evidence\": \"Generated by Heimdall SBOM Generator\"\n";
   evidence << "}";

   return evidence.str();
}

// Override parse methods for SPDX 3.0.0
std::vector<ComponentInfo> SPDX3_0_0Handler::parseContent(const std::string& content)
{
   // Use base class implementation for now
   return BaseSPDXHandler::parseContent(content);
}

std::vector<ComponentInfo> SPDX3_0_0Handler::parseFile(const std::string& filePath)
{
   // Use base class implementation for now
   return BaseSPDXHandler::parseFile(filePath);
}

}  // namespace heimdall