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
 * @brief SBOM validation framework for SPDX and CycloneDX formats
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides a comprehensive validation framework for Software Bill of Materials (SBOM)
 * documents in both SPDX and CycloneDX formats. It includes:
 * - Abstract base class for SBOM validators
 * - Concrete implementations for SPDX and CycloneDX
 * - Factory pattern for creating validators
 * - Validation result structure with error handling
 * 
 * Supported formats:
 * - SPDX 2.3 and 3.0
 * - CycloneDX 1.4, 1.5, and 1.6
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace heimdall {

/**
 * @brief Validation result structure for SBOM validation operations
 * 
 * This structure encapsulates the result of an SBOM validation operation,
 * including validation status, errors, warnings, and metadata.
 */
struct ValidationResult {
    bool isValid = true;                                    ///< Whether the SBOM is valid
    std::vector<std::string> errors;                       ///< List of validation errors
    std::vector<std::string> warnings;                     ///< List of validation warnings
    std::map<std::string, std::string> metadata;          ///< Additional metadata from validation

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
        : isValid(other.isValid)
        , errors(std::move(other.errors))
        , warnings(std::move(other.warnings))
        , metadata(std::move(other.metadata))
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
    void addError(const std::string& error) {
        errors.push_back(error);
        isValid = false;
    }
    
    /**
     * @brief Add a warning message to the validation result
     * @param warning The warning message to add
     */
    void addWarning(const std::string& warning) {
        warnings.push_back(warning);
    }
    
    /**
     * @brief Add metadata key-value pair to the validation result
     * @param key The metadata key
     * @param value The metadata value
     */
    void addMetadata(const std::string& key, const std::string& value) {
        metadata[key] = value;
    }
};

/**
 * @brief Abstract base class for SBOM validators
 * 
 * This class defines the interface for SBOM validators. Concrete implementations
 * should inherit from this class and implement the pure virtual methods.
 */
class SBOMValidator {
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
     * @brief Validate SBOM content from string
     * @param content SBOM content as string to validate
     * @return ValidationResult containing validation status and any errors/warnings
     */
    virtual ValidationResult validateContent(const std::string& content) = 0;
    
    /**
     * @brief Get the name of this validator
     * @return String containing the validator name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief SPDX validator implementation
 * 
 * Validates SPDX (Software Package Data Exchange) documents according to
 * the SPDX specification. Supports both SPDX 2.3 and 3.0 formats.
 */
class SPDXValidator : public SBOMValidator {
public:
    /**
     * @brief Validate an SPDX file
     * @param filePath Path to the SPDX file
     * @return ValidationResult with validation status
     */
    ValidationResult validate(const std::string& filePath) override;
    
    /**
     * @brief Validate SPDX content from string
     * @param content SPDX content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateContent(const std::string& content) override;
    
    /**
     * @brief Get the validator name
     * @return "SPDX Validator"
     */
    std::string getName() const override { return "SPDX Validator"; }
    
    /**
     * @brief Validate an SPDX file with specific version
     * @param filePath Path to the SPDX file
     * @param version SPDX version to validate against
     * @return ValidationResult with validation status
     */
    ValidationResult validate(const std::string& filePath, const std::string& version);
    
    /**
     * @brief Validate SPDX content with specific version
     * @param content SPDX content as string
     * @param version SPDX version to validate against
     * @return ValidationResult with validation status
     */
    ValidationResult validateContent(const std::string& content, const std::string& version);
    
private:
    /**
     * @brief Trim whitespace from a string
     * @param str The string to trim
     * @return Trimmed string
     */
    std::string trimWhitespace(const std::string& str);
    
    /**
     * @brief Validate required fields in SPDX document
     * @param result ValidationResult to update
     * @param fields Map of field names to their presence status
     */
    void validateRequiredFields(ValidationResult& result, const std::map<std::string, bool>& fields);
    
    /**
     * @brief Process a single SPDX line
     * @param line The SPDX line to process
     * @param result ValidationResult to update
     * @param fields Map of field names to their presence status
     */
    void processSPDXLine(const std::string& line, ValidationResult& result, std::map<std::string, bool>& fields);
    
    /**
     * @brief Validate SPDX 2.3 content
     * @param content SPDX 2.3 content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateSPDX2_3(const std::string& content);
    
    /**
     * @brief Validate SPDX 3.0 content
     * @param content SPDX 3.0 content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateSPDX3_0(const std::string& content);
    
    /**
     * @brief Check if a string is a valid SPDX identifier
     * @param identifier The identifier to validate
     * @return true if valid, false otherwise
     */
    bool isValidSPDXIdentifier(const std::string& identifier);
    
    /**
     * @brief Check if a string is a valid SPDX license expression
     * @param license The license expression to validate
     * @return true if valid, false otherwise
     */
    bool isValidSPDXLicenseExpression(const std::string& license);
};

/**
 * @brief CycloneDX validator implementation
 * 
 * Validates CycloneDX documents according to the CycloneDX specification.
 * Supports versions 1.4, 1.5, and 1.6.
 */
class CycloneDXValidator : public SBOMValidator {
public:
    /**
     * @brief Validate a CycloneDX file
     * @param filePath Path to the CycloneDX file
     * @return ValidationResult with validation status
     */
    ValidationResult validate(const std::string& filePath) override;
    
    /**
     * @brief Validate CycloneDX content from string
     * @param content CycloneDX content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateContent(const std::string& content) override;
    
    /**
     * @brief Get the validator name
     * @return "CycloneDX Validator"
     */
    std::string getName() const override { return "CycloneDX Validator"; }
    
    /**
     * @brief Validate a CycloneDX file with specific version
     * @param filePath Path to the CycloneDX file
     * @param version CycloneDX version to validate against
     * @return ValidationResult with validation status
     */
    ValidationResult validate(const std::string& filePath, const std::string& version);
    
    /**
     * @brief Validate CycloneDX content with specific version
     * @param content CycloneDX content as string
     * @param version CycloneDX version to validate against
     * @return ValidationResult with validation status
     */
    ValidationResult validateContent(const std::string& content, const std::string& version);
    
private:
    /**
     * @brief Extract version from CycloneDX content
     * @param content CycloneDX content as string
     * @return Version string
     */
    std::string extractVersion(const std::string& content);
    
    /**
     * @brief Validate CycloneDX 1.4 content
     * @param content CycloneDX 1.4 content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateCycloneDX1_4(const std::string& content);
    
    /**
     * @brief Validate CycloneDX 1.5 content
     * @param content CycloneDX 1.5 content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateCycloneDX1_5(const std::string& content);
    
    /**
     * @brief Validate CycloneDX 1.6 content
     * @param content CycloneDX 1.6 content as string
     * @return ValidationResult with validation status
     */
    ValidationResult validateCycloneDX1_6(const std::string& content);
    
    /**
     * @brief Check if a string is a valid CycloneDX version
     * @param version The version string to validate
     * @return true if valid, false otherwise
     */
    bool isValidCycloneDXVersion(const std::string& version);
    
    /**
     * @brief Check if a string is a valid UUID
     * @param uuid The UUID string to validate
     * @return true if valid, false otherwise
     */
    bool isValidUUID(const std::string& uuid);
};

/**
 * @brief Factory class for creating SBOM validators
 * 
 * This class provides a factory pattern for creating appropriate validators
 * based on the SBOM format. It supports SPDX and CycloneDX formats.
 */
class SBOMValidatorFactory {
public:
    /**
     * @brief Create a validator for the given format
     * @param format SBOM format ("spdx" or "cyclonedx")
     * @return Unique pointer to the created validator, or nullptr if format is not supported
     */
    static std::unique_ptr<SBOMValidator> createValidator(const std::string& format);
    
    /**
     * @brief Get list of supported SBOM formats
     * @return Vector of supported format names
     */
    static std::vector<std::string> getSupportedFormats();
};

} // namespace heimdall 
