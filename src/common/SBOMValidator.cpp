#include "SBOMValidator.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace heimdall {

// SPDX Validator Implementation

ValidationResult SPDXValidator::validate(const std::string& filePath) {
    ValidationResult result;
    if (!std::filesystem::exists(filePath)) {
        result.addError("File does not exist: " + filePath);
        return result;
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.addError("Cannot open file: " + filePath);
        return result;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return validateContent(content);
}

ValidationResult SPDXValidator::validateContent(const std::string& content) {
    ValidationResult result;
    // Detect SPDX version
    if (content.find("SPDXVersion:") != std::string::npos) {
        // SPDX 2.3 tag-value format
        return validateSPDX2_3(content);
    } else if (content.find("\"spdxVersion\"") != std::string::npos) {
        // SPDX 3.0 JSON format
        return validateSPDX3_0(content);
    } else {
        result.addError("Cannot determine SPDX format");
        return result;
    }
}

ValidationResult SPDXValidator::validateSPDX2_3(const std::string& content) {
    ValidationResult result;
    // Basic SPDX 2.3 validation
    std::istringstream iss(content);
    std::string line;
    bool hasSPDXVersion = false;
    bool hasDataLicense = false;
    bool hasSPDXID = false;
    bool hasDocumentName = false;
    bool hasDocumentNamespace = false;
    bool hasCreator = false;
    bool hasCreated = false;
    while (std::getline(iss, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line[0] == '#') continue;
        if (line.find("SPDXVersion:") == 0) {
            hasSPDXVersion = true;
            std::string version = line.substr(12);
            // Trim whitespace
            version.erase(0, version.find_first_not_of(" \t"));
            version.erase(version.find_last_not_of(" \t") + 1);
            if (version != "SPDX-2.3") {
                result.addError("Invalid SPDX version: " + version);
            }
        } else if (line.find("DataLicense:") == 0) {
            hasDataLicense = true;
            std::string license = line.substr(12);
            // Trim whitespace
            license.erase(0, license.find_first_not_of(" \t"));
            license.erase(license.find_last_not_of(" \t") + 1);
            if (!isValidSPDXLicenseExpression(license)) {
                result.addError("Invalid data license: " + license);
            }
        } else if (line.find("SPDXID:") == 0) {
            hasSPDXID = true;
            std::string id = line.substr(7);
            // Trim whitespace
            id.erase(0, id.find_first_not_of(" \t"));
            id.erase(id.find_last_not_of(" \t") + 1);
            if (!isValidSPDXIdentifier(id)) {
                result.addError("Invalid SPDX ID: " + id);
            }
        } else if (line.find("DocumentName:") == 0) {
            hasDocumentName = true;
        } else if (line.find("DocumentNamespace:") == 0) {
            hasDocumentNamespace = true;
        } else if (line.find("Creator:") == 0) {
            hasCreator = true;
        } else if (line.find("Created:") == 0) {
            hasCreated = true;
        }
    }
    // Check required fields
    if (!hasSPDXVersion) result.addError("Missing SPDXVersion field");
    if (!hasDataLicense) result.addError("Missing DataLicense field");
    if (!hasSPDXID) result.addError("Missing SPDXID field");
    if (!hasDocumentName) result.addError("Missing DocumentName field");
    if (!hasDocumentNamespace) result.addError("Missing DocumentNamespace field");
    if (!hasCreator) result.addError("Missing Creator field");
    if (!hasCreated) result.addError("Missing Created field");
    result.addMetadata("format", "SPDX 2.3");
    result.addMetadata("version", "2.3");
    return result;
}

ValidationResult SPDXValidator::validateSPDX3_0(const std::string& content) {
    ValidationResult result;
    // Basic SPDX 3.0 JSON validation
    if (content.find("\"spdxVersion\"") == std::string::npos) {
        result.addError("Missing spdxVersion field");
    }
    if (content.find("\"dataLicense\"") == std::string::npos) {
        result.addError("Missing dataLicense field");
    }
    if (content.find("\"SPDXID\"") == std::string::npos) {
        result.addError("Missing SPDXID field");
    }
    if (content.find("\"name\"") == std::string::npos) {
        result.addError("Missing name field");
    }
    if (content.find("\"documentNamespace\"") == std::string::npos) {
        result.addError("Missing documentNamespace field");
    }
    if (content.find("\"creationInfo\"") == std::string::npos) {
        result.addError("Missing creationInfo field");
    }
    result.addMetadata("format", "SPDX 3.0");
    result.addMetadata("version", "3.0");
    return result;
}

bool SPDXValidator::isValidSPDXIdentifier(const std::string& identifier) {
    // SPDX identifier format: SPDXRef-<idstring>
    std::regex idRegex(R"(^SPDXRef-[A-Za-z0-9\-\.]+$)");
    return std::regex_match(identifier, idRegex);
}

bool SPDXValidator::isValidSPDXLicenseExpression(const std::string& license) {
    // Basic license expression validation
    if (license == "CC0-1.0" || license == "CC-BY-3.0" || license == "CC-BY-SA-3.0") {
        return true;
    }
    // Check for compound expressions
    std::regex licenseRegex(R"(^[A-Za-z0-9\-\.]+(\s+(AND|OR)\s+[A-Za-z0-9\-\.]+)*$)");
    return std::regex_match(license, licenseRegex);
}

// CycloneDX Validator Implementation

ValidationResult CycloneDXValidator::validate(const std::string& filePath) {
    ValidationResult result;
    if (!std::filesystem::exists(filePath)) {
        result.addError("File does not exist: " + filePath);
        return result;
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.addError("Cannot open file: " + filePath);
        return result;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return validateContent(content);
}

ValidationResult CycloneDXValidator::validateContent(const std::string& content) {
    ValidationResult result;
    // Detect CycloneDX version
    if (content.find("\"specVersion\"") != std::string::npos) {
        // Extract version
        size_t pos = content.find("\"specVersion\"");
        if (pos != std::string::npos) {
            size_t start = content.find("\"", pos + 13) + 1;
            size_t end = content.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string version = content.substr(start, end - start);
                if (version == "1.4") {
                    return validateCycloneDX1_4(content);
                } else if (version == "1.5") {
                    return validateCycloneDX1_5(content);
                } else if (version == "1.6") {
                    return validateCycloneDX1_6(content);
                } else {
                    result.addError("Unsupported CycloneDX version: " + version);
                    return result;
                }
            }
        }
    }
    result.addError("Cannot determine CycloneDX version");
    return result;
}

ValidationResult CycloneDXValidator::validateCycloneDX1_4(const std::string& content) {
    ValidationResult result;
    // Basic CycloneDX 1.4 validation
    if (content.find("\"bomFormat\"") == std::string::npos) {
        result.addError("Missing bomFormat field");
    }
    if (content.find("\"specVersion\"") == std::string::npos) {
        result.addError("Missing specVersion field");
    }
    if (content.find("\"version\"") == std::string::npos) {
        result.addError("Missing version field");
    }
    if (content.find("\"metadata\"") == std::string::npos) {
        result.addError("Missing metadata field");
    }
    if (content.find("\"components\"") == std::string::npos) {
        result.addWarning("No components found in SBOM");
    }
    result.addMetadata("format", "CycloneDX");
    result.addMetadata("version", "1.4");
    return result;
}

ValidationResult CycloneDXValidator::validateCycloneDX1_5(const std::string& content) {
    ValidationResult result = validateCycloneDX1_4(content);
    result.metadata["version"] = "1.5";
    // CycloneDX 1.5 specific validations
    return result;
}

ValidationResult CycloneDXValidator::validateCycloneDX1_6(const std::string& content) {
    ValidationResult result = validateCycloneDX1_4(content);
    result.metadata["version"] = "1.6";
    // CycloneDX 1.6 specific validations
    return result;
}

bool CycloneDXValidator::isValidCycloneDXVersion(const std::string& version) {
    return version == "1.4" || version == "1.5" || version == "1.6";
}

bool CycloneDXValidator::isValidUUID(const std::string& uuid) {
    std::regex uuidRegex(R"(^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$)");
    return std::regex_match(uuid, uuidRegex);
}

// SBOM Validator Factory Implementation

std::unique_ptr<SBOMValidator> SBOMValidatorFactory::createValidator(const std::string& format) {
    std::string lowerFormat = format;
    std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);
    if (lowerFormat == "spdx") {
        return std::make_unique<SPDXValidator>();
    } else if (lowerFormat == "cyclonedx" || lowerFormat == "cyclone") {
        return std::make_unique<CycloneDXValidator>();
    } else {
        return nullptr;
    }
}

std::vector<std::string> SBOMValidatorFactory::getSupportedFormats() {
    return {"spdx", "cyclonedx"};
}

} // namespace heimdall 