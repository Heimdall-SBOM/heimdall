#include "SBOMValidator.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include "Utils.hpp"
#include "../compat/compatibility.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <iostream>

namespace heimdall {

// SPDX Validator Implementation

ValidationResult SPDXValidator::validate(const std::string& filePath) {
    ValidationResult result;
    if (!heimdall::Utils::fileExists(filePath)) {
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

ValidationResult SPDXValidator::validate(const std::string& filePath, const std::string& version) {
    ValidationResult result = validate(filePath);
    if (result.isValid) {
        // Override metadata with specified version
        result.metadata["version"] = version;
    }
    return result;
}

ValidationResult SPDXValidator::validateContent(const std::string& content) {
    ValidationResult result;
    // Detect SPDX version
    if (content.find("SPDXVersion:") != std::string::npos) {
        // SPDX 2.3 tag-value format
        return validateSPDX2_3(content);
    } else if (content.find("\"spdxVersion\"") != std::string::npos) {
        // SPDX 3.0 classic JSON format
        return validateSPDX3_0(content);
    } else if (content.find("@context") != std::string::npos && content.find("@graph") != std::string::npos) {
        // SPDX 3.0 JSON-LD format
        try {
            auto sbom = nlohmann::json::parse(content);
            std::string schema_path;
            if (sbom.contains("@context")) {
                std::string context = sbom["@context"];
                if (context == "https://spdx.org/rdf/3.0.1/spdx-context.jsonld") {
                    schema_path = "./schema/spdx-bom-3.0.1.schema.json";
                } else if (context == "https://spdx.org/rdf/3.0.0/spdx-context.jsonld") {
                    schema_path = "./schema/spdx-bom-3.0.0.schema.json";
                } else {
                    std::cerr << "[WARN] Unknown SPDX @context: '" << context << "', defaulting to 3.0.0 schema.\n";
                    schema_path = "./schema/spdx-bom-3.0.0.schema.json";
                }
            } else {
                std::cerr << "[WARN] No @context found in SPDX JSON-LD, defaulting to 3.0.0 schema.\n";
                schema_path = "./schema/spdx-bom-3.0.0.schema.json";
            }
            std::cerr << "[DEBUG] SPDX JSON-LD: schema_path=" << schema_path << std::endl;
            if (sbom.contains("@context")) {
                std::cerr << "[DEBUG] SPDX JSON-LD: sbom['@context']='" << sbom["@context"] << "'" << std::endl;
            } else {
                std::cerr << "[DEBUG] SPDX JSON-LD: sbom['@context'] not found" << std::endl;
            }
            std::cerr << "[DEBUG] SPDX JSON-LD: first 100 chars of content: '" << content.substr(0, 100) << "'" << std::endl;
            // Debug: print first object in @graph and its keys
            if (sbom.contains("@graph") && sbom["@graph"].is_array() && !sbom["@graph"].empty()) {
                const auto& doc = sbom["@graph"][0];
                std::cerr << "[DEBUG] First @graph object: " << doc.dump(2) << std::endl;
                std::cerr << "[DEBUG] Keys in first @graph object:";
                for (auto it = doc.begin(); it != doc.end(); ++it) {
                    std::cerr << " " << it.key();
                }
                std::cerr << std::endl;
            }
            // --- END DEBUG PRINTS ---
            std::ifstream schema_file(schema_path);
            if (!schema_file.is_open()) {
                result.addError("Could not open SPDX schema file: " + schema_path);
                return result;
            }
            nlohmann::json schema;
            schema_file >> schema;
            nlohmann::json_schema::json_validator validator;
            validator.set_root_schema(schema);
            try {
                validator.validate(sbom);
                result.isValid = true;
            } catch (const std::exception& e) {
                result.isValid = false;
                result.errors.push_back(std::string("SPDX 3.x schema validation failed: ") + e.what());
            }
            // Extract SPDX version from @graph (schema-compliant style)
            std::string version = "3.0.x";
            if (sbom.contains("@graph") && sbom["@graph"].is_array()) {
                for (const auto& obj : sbom["@graph"]) {
                    if (obj.contains("type") && obj["type"].is_string() &&
                        obj["type"] == "SpdxDocument") {
                        if (obj.contains("specVersion")) {
                            version = obj["specVersion"].get<std::string>();
                        }
                        break;
                    }
                }
            }
            // Set format based on version
            std::string format = "SPDX 3.0";
            if (version == "SPDX-3.0.1") format = "SPDX 3.0.1";
            else if (version == "SPDX-3.0") format = "SPDX 3.0";
            result.addMetadata("format", format);
            result.addMetadata("version", version);
        } catch (const std::exception& e) {
            result.addError(std::string("SPDX 3.x JSON-LD parse error: ") + e.what());
        }
        return result;
    } else {
        // Try classic SPDX 3.0.1 JSON (no @context, no @graph)
        try {
            auto sbom = nlohmann::json::parse(content);
            if (sbom.contains("spdxId") && sbom.contains("type") && sbom["type"] == "SpdxDocument" && sbom.contains("specVersion")) {
                std::string schema_path = "./schema/spdx-bom-3.0.1.schema.json";
                std::cerr << "[DEBUG] SPDX classic 3.0.1: schema_path=" << schema_path << std::endl;
                std::cerr << "[DEBUG] SPDX classic 3.0.1: first 100 chars of content: '" << content.substr(0, 100) << "'" << std::endl;
                std::string version = sbom["specVersion"].get<std::string>();
                std::string format = (version == "SPDX-3.0.1") ? "SPDX 3.0.1" : "SPDX 3.0";
                result.addMetadata("format", format);
                result.addMetadata("version", version);
                std::ifstream schema_file(schema_path);
                nlohmann::json schema_json;
                schema_file >> schema_json;
                nlohmann::json_schema::json_validator validator;
                validator.set_root_schema(schema_json);
                try {
                    validator.validate(sbom);
                    result.isValid = true;
                } catch (const std::exception& e) {
                    result.isValid = false;
                    result.errors.push_back(std::string("SPDX 3.x schema validation failed: ") + e.what());
                }
                return result;
            }
        } catch (const std::exception& e) {
            result.isValid = false;
            result.errors.push_back(std::string("SPDX 3.x parse failed: ") + e.what());
            return result;
        }
    }
    result.addError("Cannot determine SPDX format");
    return result;
}

ValidationResult SPDXValidator::validateContent(const std::string& content, const std::string& version) {
    ValidationResult result = validateContent(content);
    if (result.isValid) {
        // Override metadata with specified version
        result.metadata["version"] = version;
    }
    return result;
}

std::string SPDXValidator::trimWhitespace(const std::string& str) {
    std::string result = str;
    result.erase(0, result.find_first_not_of(" \t"));
    result.erase(result.find_last_not_of(" \t") + 1);
    return result;
}

void SPDXValidator::validateRequiredFields(ValidationResult& result, 
                                          const std::map<std::string, bool>& fields) {
    if (!fields.at("SPDXVersion")) result.addError("Missing SPDXVersion field");
    if (!fields.at("DataLicense")) result.addError("Missing DataLicense field");
    if (!fields.at("SPDXID")) result.addError("Missing SPDXID field");
    if (!fields.at("DocumentName")) result.addError("Missing DocumentName field");
    if (!fields.at("DocumentNamespace")) result.addError("Missing DocumentNamespace field");
    if (!fields.at("Creator")) result.addError("Missing Creator field");
    if (!fields.at("Created")) result.addError("Missing Created field");
}

void SPDXValidator::processSPDXLine(const std::string& line, ValidationResult& result,
                                   std::map<std::string, bool>& fields) {
    if (line.find("SPDXVersion:") == 0) {
        fields["SPDXVersion"] = true;
        std::string version = trimWhitespace(line.substr(12));
        if (version != "SPDX-2.3") {
            result.addError("Invalid SPDX version: " + version);
        }
    } else if (line.find("DataLicense:") == 0) {
        fields["DataLicense"] = true;
        std::string license = trimWhitespace(line.substr(12));
        if (!isValidSPDXLicenseExpression(license)) {
            result.addError("Invalid data license: " + license);
        }
    } else if (line.find("SPDXID:") == 0) {
        fields["SPDXID"] = true;
        std::string id = trimWhitespace(line.substr(7));
        if (!isValidSPDXIdentifier(id)) {
            result.addError("Invalid SPDX ID: " + id);
        }
    } else if (line.find("DocumentName:") == 0) {
        fields["DocumentName"] = true;
    } else if (line.find("DocumentNamespace:") == 0) {
        fields["DocumentNamespace"] = true;
    } else if (line.find("Creator:") == 0) {
        fields["Creator"] = true;
    } else if (line.find("Created:") == 0) {
        fields["Created"] = true;
    }
}

ValidationResult SPDXValidator::validateSPDX2_3(const std::string& content) {
    ValidationResult result;
    std::istringstream iss(content);
    std::string line;
    std::map<std::string, bool> fields = {
        {"SPDXVersion", false},
        {"DataLicense", false},
        {"SPDXID", false},
        {"DocumentName", false},
        {"DocumentNamespace", false},
        {"Creator", false},
        {"Created", false}
    };
    
    while (std::getline(iss, line)) {
        line = trimWhitespace(line);
        if (line.empty() || line[0] == '#') continue;
        processSPDXLine(line, result, fields);
    }
    
    validateRequiredFields(result, fields);
    result.addMetadata("format", "SPDX 2.3");
    result.addMetadata("version", "2.3");
    return result;
}

ValidationResult SPDXValidator::validateSPDX3_0(const std::string& content) {
    ValidationResult result;
    try {
        nlohmann::json sbom = nlohmann::json::parse(content);
        std::string version = "3.0";
        
        // Check for specVersion in the @graph array (JSON-LD format)
        if (sbom.contains("@graph") && sbom["@graph"].is_array() && !sbom["@graph"].empty()) {
            auto first_obj = sbom["@graph"][0];
            if (first_obj.contains("specVersion")) {
                std::string v = first_obj["specVersion"].get<std::string>();
                if (v == "SPDX-3.0.1") version = "3.0.1";
                else if (v == "SPDX-3.0.0" || v == "SPDX-3.0") version = "3.0.0";
            }
        }
        // Fallback: check for spdxVersion (legacy format)
        else if (sbom.contains("spdxVersion")) {
            std::string v = sbom["spdxVersion"].get<std::string>();
            if (v == "SPDX-3.0.1") version = "3.0.1";
            else if (v == "SPDX-3.0.0" || v == "SPDX-3.0") version = "3.0.0";
        }
        
        std::string schema_path = "./schema/spdx-bom-3.0.0.schema.json";
        if (version == "3.0.1") schema_path = "./schema/spdx-bom-3.0.1.schema.json";
        std::ifstream schema_file(schema_path);
        if (!schema_file.is_open()) {
            result.addError("Could not open SPDX schema file: " + schema_path);
            return result;
        }
        nlohmann::json schema;
        schema_file >> schema;
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema(schema);
        try {
            validator.validate(sbom);
            // If we get here, validation passed
            result.addMetadata("format", "SPDX 3.0");
            result.addMetadata("version", version);
            std::cerr << "[DEBUG] SPDX 3.0 validation passed, setting metadata: format=SPDX 3.0, version=" << version << std::endl;
        } catch (const std::exception& e) {
            result.addError(std::string("SPDX 3.x schema validation failed: ") + e.what());
            std::cerr << "[DEBUG] SPDX 3.0 validation failed: " << e.what() << std::endl;
            return result;
        }
    } catch (const std::exception& e) {
        result.addError(std::string("SPDX 3.x JSON parse error: ") + e.what());
        std::cerr << "[DEBUG] SPDX 3.0 JSON parse error: " << e.what() << std::endl;
    }
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
    if (!heimdall::Utils::fileExists(filePath)) {
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

ValidationResult CycloneDXValidator::validate(const std::string& filePath, const std::string& version) {
    ValidationResult result = validate(filePath);
    if (result.isValid) {
        // Override metadata with specified version
        result.metadata["version"] = version;
    }
    return result;
}

std::string CycloneDXValidator::extractVersion(const std::string& content) {
    size_t pos = content.find("\"specVersion\"");
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t start = content.find("\"", pos + 13) + 1;
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = content.find("\"", start);
    if (end == std::string::npos) {
        return "";
    }
    
    return content.substr(start, end - start);
}

ValidationResult CycloneDXValidator::validateContent(const std::string& content) {
    ValidationResult result;
    
    if (content.find("\"specVersion\"") == std::string::npos) {
        result.addError("Cannot determine CycloneDX version");
        return result;
    }
    
    std::string version = extractVersion(content);
    if (version.empty()) {
        result.addError("Cannot determine CycloneDX version");
        return result;
    }
    
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

ValidationResult CycloneDXValidator::validateContent(const std::string& content, const std::string& version) {
    ValidationResult result;
    
    if (version == "1.4") {
        result = validateCycloneDX1_4(content);
    } else if (version == "1.5") {
        result = validateCycloneDX1_5(content);
    } else if (version == "1.6") {
        result = validateCycloneDX1_6(content);
    } else {
        result.addError("Unsupported CycloneDX version: " + version);
        return result;
    }
    
    if (result.isValid) {
        // Override metadata with specified version
        result.metadata["version"] = version;
    }
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
    if (format == "spdx" || format == "spdx-2.3" || format == "spdx-3.0" || format == "spdx-3.0.0" || format == "spdx-3.0.1") {
        return heimdall::compat::make_unique<SPDXValidator>();
    } else if (format == "cyclonedx" || format == "cyclonedx-1.4" || format == "cyclonedx-1.5" || format == "cyclonedx-1.6") {
        return heimdall::compat::make_unique<CycloneDXValidator>();
    } else {
        return nullptr;
    }
}

std::vector<std::string> SBOMValidatorFactory::getSupportedFormats() {
    return {"spdx", "cyclonedx"};
}

} // namespace heimdall 