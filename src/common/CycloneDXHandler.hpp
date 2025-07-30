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
 * @file CycloneDXHandler.hpp
 * @brief CycloneDX format handler implementation
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a clean implementation of CycloneDX SBOM format handling,
 * supporting versions 1.4, 1.5, and 1.6 with proper separation of concerns.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "SBOMFormats.hpp"
#include "SBOMValidator.hpp"

namespace heimdall
{

/**
 * @brief Base CycloneDX handler with common functionality
 */
class BaseCycloneDXHandler : public ICycloneDXHandler
{
   protected:
   std::string version;

   // Common CycloneDX utilities
   std::string generateBOMRef(const std::string& name, const std::string& version) const;
   std::string getCurrentTimestamp() const;
   std::string generatePURL(const ComponentInfo& component) const;
   std::string generateCPE(const ComponentInfo& component) const;
   std::string generateCycloneDXLicense(const std::string& license) const;
   std::string generateComponentType(const ComponentInfo& component) const;
   std::string generateComponentScope(const ComponentInfo& component) const;
   std::string generateComponentGroup(const ComponentInfo& component) const;
   std::string generateComponentMimeType(const ComponentInfo& component) const;
   std::string generateComponentCopyright(const ComponentInfo& component) const;
   std::string generateComponentSupplier(const ComponentInfo& component) const;
   std::string generateComponentManufacturer(const ComponentInfo& component) const;
   std::string generateComponentPublisher(const ComponentInfo& component) const;
   std::string generateComponentDescription(const ComponentInfo& component) const;
   std::string generateComponentProperties(const ComponentInfo& component) const;
   std::string generateComponentEvidence(const ComponentInfo& component) const;
   std::string generateComponentDependencies(const ComponentInfo& component) const;
   std::string generateComponentExternalReferences(const ComponentInfo& component) const;

   // Parsing helper methods
   ComponentInfo parseComponentFromJson(const std::string& componentJson) const;
   std::string   extractPackageManagerFromPURL(const std::string& purl) const;

   public:
   explicit BaseCycloneDXHandler(const std::string& version);
   ~BaseCycloneDXHandler() override = default;

   // ISBOMFormatHandler interface
   std::string getFormatName() const override
   {
      return "CycloneDX";
   }
   std::string getFormatVersion() const override
   {
      return version;
   }
   std::string getFileExtension() const override
   {
      return ".json";
   }
   bool supportsFeature(const std::string& feature) const override;

   // ICycloneDXHandler interface
   void                     setVersion(const std::string& version) override;
   std::vector<std::string> getSupportedVersions() const override;
   ValidationResult         validateContent(const std::string& content) override;

   // ISBOMFormatHandler parsing interface
   std::vector<ComponentInfo> parseContent(const std::string& content) override;
   std::vector<ComponentInfo> parseFile(const std::string& filePath) override;
};

/**
 * @brief CycloneDX 1.4 format handler
 */
class CycloneDX1_4Handler : public BaseCycloneDXHandler
{
   public:
   CycloneDX1_4Handler();
   ~CycloneDX1_4Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   private:
   std::string generateMetadata(const std::map<std::string, std::string>& metadata) const;
   std::string generateComponents(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateComponent(const ComponentInfo& component) const;
   std::string generateDependencies(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateLicenses(const ComponentInfo& component) const;
   std::string generateHashes(const ComponentInfo& component) const;
   std::string generateExternalReferences(const ComponentInfo& component) const;
};

/**
 * @brief CycloneDX 1.5 format handler
 */
class CycloneDX1_5Handler : public BaseCycloneDXHandler
{
   public:
   CycloneDX1_5Handler();
   ~CycloneDX1_5Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   private:
   std::string generateMetadata(const std::map<std::string, std::string>& metadata) const;
   std::string generateComponents(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateComponent(const ComponentInfo& component) const;
   std::string generateDependencies(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateLicenses(const ComponentInfo& component) const;
   std::string generateHashes(const ComponentInfo& component) const;
   std::string generateExternalReferences(const ComponentInfo& component) const;
   std::string generateVulnerabilities(const ComponentInfo& component) const;
   std::string generateFormulation(const ComponentInfo& component) const;
};

/**
 * @brief CycloneDX 1.6 format handler
 */
class CycloneDX1_6Handler : public BaseCycloneDXHandler
{
   public:
   CycloneDX1_6Handler();
   ~CycloneDX1_6Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   private:
   std::string generateMetadata(const std::map<std::string, std::string>& metadata) const;
   std::string generateComponents(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateComponent(const ComponentInfo& component) const;
   std::string generateDependencies(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateLicenses(const ComponentInfo& component) const;
   std::string generateHashes(const ComponentInfo& component) const;
   std::string generateExternalReferences(const ComponentInfo& component) const;
   std::string generateVulnerabilities(const ComponentInfo& component) const;
   std::string generateFormulation(const ComponentInfo& component) const;
   std::string generateServices(const ComponentInfo& component) const;
   std::string generateAnnotations(const ComponentInfo& component) const;
   std::string generateCompositions(const ComponentInfo& component) const;
};

}  // namespace heimdall