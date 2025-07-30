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
 * @file SBOMValidator.hpp
 * @brief SBOM validation framework using format handlers
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a comprehensive validation framework for Software Bill of Materials (SBOM)
 * documents using the new modular format handlers. It includes:
 * - Abstract base class for SBOM validators
 * - Unified validator that uses format handlers
 * - Factory pattern for creating validators
 * - Validation result structure with error handling
 *
 * Supported formats:
 * - SPDX 2.3, 3.0.0, and 3.0.1
 * - CycloneDX 1.4, 1.5, and 1.6
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "SBOMFormats.hpp"

namespace heimdall
{

/**
 * @brief Validation result structure for SBOM validation operations
 *
 * This structure encapsulates the result of an SBOM validation operation,
 * including validation status, errors, warnings, and metadata.
 */
struct ValidationResult
{
   bool                               isValid = true;  ///< Whether the SBOM is valid
   std::vector<std::string>           errors;          ///< List of validation errors
   std::vector<std::string>           warnings;        ///< List of validation warnings
   std::map<std::string, std::string> metadata;        ///< Additional metadata from validation

   /**
    * @brief Default constructor
    */
   ValidationResult() = default;

   /**
    * @brief Copy constructor
    */
   ValidationResult(const ValidationResult&) = default;

   /**
    * @brief Copy assignment operator
    */
   ValidationResult& operator=(const ValidationResult&) = default;

   /**
    * @brief Move constructor
    * @param other The ValidationResult to move from
    */
   ValidationResult(ValidationResult&& other) noexcept
      : isValid(other.isValid),
        errors(std::move(other.errors)),
        warnings(std::move(other.warnings)),
        metadata(std::move(other.metadata))
   {
   }

   /**
    * @brief Move assignment operator
    * @param other The ValidationResult to move from
    */
   ValidationResult& operator=(ValidationResult&& other) noexcept = default;

   /**
    * @brief Add an error message to the validation result
    * @param error The error message to add
    */
   void addError(const std::string& error)
   {
      errors.push_back(error);
      isValid = false;
   }

   /**
    * @brief Add a warning message to the validation result
    * @param warning The warning message to add
    */
   void addWarning(const std::string& warning)
   {
      warnings.push_back(warning);
   }

   /**
    * @brief Add metadata key-value pair to the validation result
    * @param key The metadata key
    * @param value The metadata value
    */
   void addMetadata(const std::string& key, const std::string& value)
   {
      metadata[key] = value;
   }
};

/**
 * @brief Abstract base class for SBOM validators
 *
 * This class defines the interface for SBOM validators. The unified validator
 * uses format handlers to perform validation.
 */
class SBOMValidator
{
   public:
   /**
    * @brief Virtual destructor
    */
   virtual ~SBOMValidator() = default;

   /**
    * @brief Validate an SBOM file
    * @param filePath Path to the SBOM file to validate
    * @return ValidationResult containing validation status and any errors/warnings
    */
   virtual ValidationResult validate(const std::string& filePath) = 0;

   /**
    * @brief Validate SBOM content
    * @param content SBOM content to validate
    * @return ValidationResult containing validation status and any errors/warnings
    */
   virtual ValidationResult validateContent(const std::string& content) = 0;

   /**
    * @brief Get the name of the validator
    * @return Validator name
    */
   virtual std::string getName() const = 0;
};

/**
 * @brief Unified SBOM validator that uses format handlers
 *
 * This validator uses the SBOMFormatFactory to create appropriate format handlers
 * and delegates validation to them. It automatically detects the format and version
 * from the content.
 */
class UnifiedSBOMValidator : public SBOMValidator
{
   public:
   /**
    * @brief Default constructor
    */
   UnifiedSBOMValidator() = default;

   /**
    * @brief Constructor with format specification
    * @param format The format this validator handles (spdx, cyclonedx)
    */
   UnifiedSBOMValidator(const std::string& format) : format_(format) {}

   /**
    * @brief Destructor
    */
   ~UnifiedSBOMValidator() override = default;

   /**
    * @brief Validate an SBOM file
    * @param filePath Path to the SBOM file to validate
    * @return ValidationResult containing validation status and any errors/warnings
    */
   ValidationResult validate(const std::string& filePath) override;

   /**
    * @brief Validate SBOM content
    * @param content SBOM content to validate
    * @return ValidationResult containing validation status and any errors/warnings
    */
   ValidationResult validateContent(const std::string& content) override;

   /**
    * @brief Validate SBOM content with specific format and version
    * @param content SBOM content to validate
    * @param format Format name (spdx, cyclonedx)
    * @param version Format version
    * @return ValidationResult containing validation status and any errors/warnings
    */
   ValidationResult validateContent(const std::string& content, const std::string& format,
                                    const std::string& version);

   /**
    * @brief Get the name of the validator
    * @return Validator name
    */
   std::string getName() const override
   {
      if (format_ == "spdx")
      {
         return "SPDX Validator";
      }
      else if (format_ == "cyclonedx")
      {
         return "CycloneDX Validator";
      }
      else
      {
         return "UnifiedSBOMValidator";
      }
   }

   private:
   std::string format_;  ///< The format this validator handles

   /**
    * @brief Detect format from content
    * @param content SBOM content to analyze
    * @return Detected format name
    */
   std::string detectFormat(const std::string& content) const;

   /**
    * @brief Extract version from content
    * @param content SBOM content to analyze
    * @param format Detected format
    * @return Extracted version
    */
   std::string extractVersion(const std::string& content, const std::string& format) const;

   /**
    * @brief Validate SPDX content
    * @param content SPDX content to validate
    * @param version SPDX version
    * @return ValidationResult
    */
   ValidationResult validateSPDX(const std::string& content, const std::string& version);

   /**
    * @brief Validate CycloneDX content
    * @param content CycloneDX content to validate
    * @param version CycloneDX version
    * @return ValidationResult
    */
   ValidationResult validateCycloneDX(const std::string& content, const std::string& version);
};

/**
 * @brief Factory for creating SBOM validators
 *
 * This factory provides a unified interface for creating validators.
 * Currently, it creates UnifiedSBOMValidator instances.
 */
class SBOMValidatorFactory
{
   public:
   /**
    * @brief Create a validator for the specified format
    * @param format Format name (spdx, cyclonedx, or empty for auto-detection)
    * @return Unique pointer to the created validator
    */
   static std::unique_ptr<SBOMValidator> createValidator(const std::string& format = "");

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
