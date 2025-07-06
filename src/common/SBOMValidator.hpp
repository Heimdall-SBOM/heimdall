#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace heimdall {

/**
 * @brief Validation result for SBOM validation
 */
struct ValidationResult {
    bool isValid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::map<std::string, std::string> metadata;
    
    void addError(const std::string& error) {
        errors.push_back(error);
        isValid = false;
    }
    
    void addWarning(const std::string& warning) {
        warnings.push_back(warning);
    }
    
    void addMetadata(const std::string& key, const std::string& value) {
        metadata[key] = value;
    }
};

/**
 * @brief Abstract base class for SBOM validators
 */
class SBOMValidator {
public:
    virtual ~SBOMValidator() = default;
    
    /**
     * @brief Validate an SBOM file
     * @param filePath Path to the SBOM file
     * @return Validation result
     */
    virtual ValidationResult validate(const std::string& filePath) = 0;
    
    /**
     * @brief Validate SBOM content from string
     * @param content SBOM content as string
     * @return Validation result
     */
    virtual ValidationResult validateContent(const std::string& content) = 0;
    
    /**
     * @brief Get validator name
     * @return Validator name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief SPDX validator implementation
 */
class SPDXValidator : public SBOMValidator {
public:
    ValidationResult validate(const std::string& filePath) override;
    ValidationResult validateContent(const std::string& content) override;
    std::string getName() const override { return "SPDX Validator"; }
    
private:
    ValidationResult validateSPDX2_3(const std::string& content);
    ValidationResult validateSPDX3_0(const std::string& content);
    bool isValidSPDXIdentifier(const std::string& identifier);
    bool isValidSPDXLicenseExpression(const std::string& license);
};

/**
 * @brief CycloneDX validator implementation
 */
class CycloneDXValidator : public SBOMValidator {
public:
    ValidationResult validate(const std::string& filePath) override;
    ValidationResult validateContent(const std::string& content) override;
    std::string getName() const override { return "CycloneDX Validator"; }
    
private:
    ValidationResult validateCycloneDX1_4(const std::string& content);
    ValidationResult validateCycloneDX1_5(const std::string& content);
    ValidationResult validateCycloneDX1_6(const std::string& content);
    bool isValidCycloneDXVersion(const std::string& version);
    bool isValidUUID(const std::string& uuid);
};

/**
 * @brief Factory for creating SBOM validators
 */
class SBOMValidatorFactory {
public:
    /**
     * @brief Create validator for given format
     * @param format SBOM format ("spdx" or "cyclonedx")
     * @return Validator instance
     */
    static std::unique_ptr<SBOMValidator> createValidator(const std::string& format);
    
    /**
     * @brief Get supported formats
     * @return List of supported formats
     */
    static std::vector<std::string> getSupportedFormats();
};

} // namespace heimdall 