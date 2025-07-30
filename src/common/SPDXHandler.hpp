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
 * @file SPDXHandler.hpp
 * @brief SPDX format handler implementation
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a clean implementation of SPDX SBOM format handling,
 * supporting versions 2.3, 3.0.0, and 3.0.1 with proper separation of concerns.
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
 * @brief Base SPDX handler with common functionality
 */
class BaseSPDXHandler : public ISPDXHandler
{
   protected:
   std::string version;

   // Common SPDX utilities
   std::string generateSPDXId(const std::string& name) const;
   std::string generateDocumentNamespace() const;
   std::string getCurrentTimestamp() const;
   std::string generateVerificationCode() const;
   std::string generatePURL(const ComponentInfo& component) const;
   std::string generateSPDXLicenseId(const std::string& license) const;
   std::string generateSPDXElementId(const std::string& name) const;

   public:
   explicit BaseSPDXHandler(const std::string& version);
   ~BaseSPDXHandler() override = default;

   // ISBOMFormatHandler interface
   std::string getFormatName() const override
   {
      return "SPDX";
   }
   std::string getFormatVersion() const override
   {
      return version;
   }
   std::string getFileExtension() const override
   {
      return ".spdx";
   }
   bool                       supportsFeature(const std::string& feature) const override;
   std::vector<ComponentInfo> parseContent(const std::string& content) override;
   std::vector<ComponentInfo> parseFile(const std::string& filePath) override;

   // ISPDXHandler interface
   void                     setVersion(const std::string& version) override;
   std::vector<std::string> getSupportedVersions() const override;
   ValidationResult         validateContent(const std::string& content) override;
};

/**
 * @brief SPDX 2.3 format handler
 */
class SPDX2_3Handler : public BaseSPDXHandler
{
   public:
   SPDX2_3Handler();
   ~SPDX2_3Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   // Override parse methods
   std::vector<ComponentInfo> parseContent(const std::string& content) override;
   std::vector<ComponentInfo> parseFile(const std::string& filePath) override;

   private:
   std::string generateHeader(const std::map<std::string, std::string>& metadata) const;
   std::string generatePackageInfo(const std::map<std::string, std::string>& metadata) const;
   std::string generateFileInfo(const ComponentInfo& component) const;
   std::string generateRelationships(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateFileChecksums(const ComponentInfo& component) const;
   std::string generateFileComment(const ComponentInfo& component) const;
};

/**
 * @brief SPDX 3.0.0 format handler
 */
class SPDX3_0_0Handler : public BaseSPDXHandler
{
   public:
   SPDX3_0_0Handler();
   ~SPDX3_0_0Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   // Override parse methods
   std::vector<ComponentInfo> parseContent(const std::string& content) override;
   std::vector<ComponentInfo> parseFile(const std::string& filePath) override;

   private:
   std::string generateSPDX3Document(const std::map<std::string, std::string>& metadata) const;
   std::string generateSPDX3CreationInfo() const;
   std::string generateSPDX3Elements(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateSPDX3Element(const ComponentInfo& component) const;
   std::string generateSPDX3Relationships(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateSPDX3Properties(const ComponentInfo& component) const;
   std::string generateSPDX3Evidence(const ComponentInfo& component) const;
};

/**
 * @brief SPDX 3.0.1 format handler
 */
class SPDX3_0_1Handler : public BaseSPDXHandler
{
   public:
   SPDX3_0_1Handler();
   ~SPDX3_0_1Handler() override = default;

   std::string generateSBOM(const std::unordered_map<std::string, ComponentInfo>& components,
                            const std::map<std::string, std::string>& metadata = {}) override;

   std::string generateComponentEntry(const ComponentInfo& component) override;

   // Override parse methods
   std::vector<ComponentInfo> parseContent(const std::string& content) override;
   std::vector<ComponentInfo> parseFile(const std::string& filePath) override;

   private:
   std::string generateSPDX3Document(const std::map<std::string, std::string>& metadata) const;
   std::string generateSPDX3CreationInfo() const;
   std::string generateSPDX3Elements(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateSPDX3Element(const ComponentInfo& component) const;
   std::string generateSPDX3Relationships(
      const std::unordered_map<std::string, ComponentInfo>& components) const;
   std::string generateSPDX3Properties(const ComponentInfo& component) const;
   std::string generateSPDX3Evidence(const ComponentInfo& component) const;
   std::string generateSPDX3Annotations(const ComponentInfo& component) const;
};

}  // namespace heimdall