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
 * @file SBOMFormats.hpp
 * @brief Clean interface for SBOM format handlers
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a clean, modular interface for handling different SBOM formats.
 * It separates concerns between SPDX and CycloneDX formats and provides a unified
 * interface for SBOM generation, validation, and comparison.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ComponentInfo.hpp"

// Forward declaration
namespace heimdall
{
struct ValidationResult;
}

namespace heimdall
{

/**
 * @brief Abstract base class for SBOM format handlers
 */
class ISBOMFormatHandler
{
   public:
   virtual ~ISBOMFormatHandler() = default;

   /**
    * @brief Generate SBOM content from components
    * @param components Map of components to include in SBOM
    * @param metadata Additional metadata for the SBOM
    * @return Generated SBOM content as string
    */
   virtual std::string generateSBOM(
      const std::unordered_map<std::string, ComponentInfo>& components,
      const std::map<std::string, std::string>&             metadata = {}) = 0;

   /**
    * @brief Validate SBOM content
    * @param content SBOM content to validate
    * @return Validation result
    */
   virtual ValidationResult validateContent(const std::string& content) = 0;

   /**
    * @brief Get format name
    * @return Format name (e.g., "SPDX", "CycloneDX")
    */
   virtual std::string getFormatName() const = 0;

   /**
    * @brief Get format version
    * @return Format version (e.g., "2.3", "1.6")
    */
   virtual std::string getFormatVersion() const = 0;

   /**
    * @brief Get file extension for this format
    * @return File extension (e.g., ".spdx", ".json")
    */
   virtual std::string getFileExtension() const = 0;

   /**
    * @brief Check if format supports a specific feature
    * @param feature Feature name to check
    * @return true if feature is supported
    */
   virtual bool supportsFeature(const std::string& feature) const = 0;

   /**
    * @brief Parse SBOM content and extract components
    * @param content SBOM content to parse
    * @return Vector of ComponentInfo objects extracted from the SBOM
    */
   virtual std::vector<ComponentInfo> parseContent(const std::string& content) = 0;

   /**
    * @brief Parse SBOM file and extract components
    * @param filePath Path to the SBOM file
    * @return Vector of ComponentInfo objects extracted from the SBOM
    */
   virtual std::vector<ComponentInfo> parseFile(const std::string& filePath) = 0;
};

/**
 * @brief SPDX format handler interface
 */
class ISPDXHandler : public ISBOMFormatHandler
{
   public:
   /**
    * @brief Set SPDX version
    * @param version SPDX version (2.3, 3.0.0, 3.0.1)
    */
   virtual void setVersion(const std::string& version) = 0;

   /**
    * @brief Get supported SPDX versions
    * @return Vector of supported versions
    */
   virtual std::vector<std::string> getSupportedVersions() const = 0;

   /**
    * @brief Generate SPDX component entry
    * @param component Component to generate entry for
    * @return SPDX component entry
    */
   virtual std::string generateComponentEntry(const ComponentInfo& component) = 0;
};

/**
 * @brief CycloneDX format handler interface
 */
class ICycloneDXHandler : public ISBOMFormatHandler
{
   public:
   /**
    * @brief Set CycloneDX version
    * @param version CycloneDX version (1.4, 1.5, 1.6)
    */
   virtual void setVersion(const std::string& version) = 0;

   /**
    * @brief Get supported CycloneDX versions
    * @return Vector of supported versions
    */
   virtual std::vector<std::string> getSupportedVersions() const = 0;

   /**
    * @brief Generate CycloneDX component entry
    * @param component Component to generate entry for
    * @return CycloneDX component entry
    */
   virtual std::string generateComponentEntry(const ComponentInfo& component) = 0;
};

/**
 * @brief Factory for creating SBOM format handlers
 */
class SBOMFormatFactory
{
   public:
   /**
    * @brief Create SPDX handler
    * @param version SPDX version
    * @return SPDX handler instance
    */
   static std::unique_ptr<ISPDXHandler> createSPDXHandler(const std::string& version = "2.3");

   /**
    * @brief Create CycloneDX handler
    * @param version CycloneDX version
    * @return CycloneDX handler instance
    */
   static std::unique_ptr<ICycloneDXHandler> createCycloneDXHandler(
      const std::string& version = "1.6");

   /**
    * @brief Create handler by format name
    * @param format Format name ("spdx", "cyclonedx")
    * @param version Format version
    * @return SBOM format handler
    */
   static std::unique_ptr<ISBOMFormatHandler> createHandler(const std::string& format,
                                                            const std::string& version = "");

   /**
    * @brief Get supported formats
    * @return Vector of supported format names
    */
   static std::vector<std::string> getSupportedFormats();

   /**
    * @brief Get supported versions for a format
    * @param format Format name
    * @return Vector of supported versions
    */
   static std::vector<std::string> getSupportedVersions(const std::string& format);
};

}  // namespace heimdall