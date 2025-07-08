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
 * @file SBOMGenerator.cpp
 * @brief Implementation of Software Bill of Materials (SBOM) generator
 * @author Trevor Bakker
 * @date 2025
 */

#include "SBOMGenerator.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "MetadataExtractor.hpp"
#include "Utils.hpp"

namespace heimdall {

/**
 * @brief Implementation class for SBOMGenerator
 */
class SBOMGenerator::Impl {
public:
    std::unordered_map<std::string, ComponentInfo> components;  ///< Map of processed components
    std::string outputPath;                                     ///< Output file path
    std::string format = "spdx";                                ///< Output format
    std::string cyclonedxVersion = "1.6";                       ///< CycloneDX specification version
    std::string spdxVersion = "3.0";                            ///< SPDX specification version
    std::unique_ptr<MetadataExtractor> metadataExtractor;       ///< Metadata extractor instance
    BuildInfo buildInfo;                                        ///< Build information

    /**
     * @brief Generate SBOM in SPDX format
     * @param outputPath The output file path
     * @return true if generation was successful
     */
    bool generateSPDX(const std::string& outputPath);

    /**
     * @brief Generate SBOM in SPDX 3.0 JSON format
     * @param outputPath The output file path
     * @return true if generation was successful
     */
    bool generateSPDX3JSON(const std::string& outputPath);

    /**
     * @brief Generate SBOM in CycloneDX format
     * @param outputPath The output file path
     * @return true if generation was successful
     */
    bool generateCycloneDX(const std::string& outputPath);

    /**
     * @brief Generate SPDX document content
     * @return The SPDX document as a string
     */
    std::string generateSPDXDocument();

    /**
     * @brief Generate SPDX 3.0 JSON document content
     * @return The SPDX 3.0 JSON document as a string
     */
    std::string generateSPDX3JSONDocument();

    /**
     * @brief Generate CycloneDX document content
     * @return The CycloneDX document as a string
     */
    std::string generateCycloneDXDocument();

    /**
     * @brief Generate SPDX component entry
     * @param component The component to generate entry for
     * @return The SPDX component entry as a string
     */
    std::string generateSPDXComponent(const ComponentInfo& component);

    /**
     * @brief Generate SPDX 3.0 JSON component entry
     * @param component The component to generate entry for
     * @return The SPDX 3.0 JSON component entry as a string
     */
    std::string generateSPDX3JSONComponent(const ComponentInfo& component);

    /**
     * @brief Generate CycloneDX component entry
     * @param component The component to generate entry for
     * @return The CycloneDX component entry as a string
     */
    std::string generateCycloneDXComponent(const ComponentInfo& component);

    /**
     * @brief Get current timestamp in ISO format
     * @return Current timestamp string
     */
    std::string getCurrentTimestamp();

    /**
     * @brief Generate SPDX ID for a component
     * @param name The component name
     * @return The generated SPDX ID
     */
    std::string generateSPDXId(const std::string& name);

    /**
     * @brief Generate verification code for SPDX
     * @return The verification code string
     */
    std::string generateVerificationCode();

    /**
     * @brief Generate Package URL (PURL) for a component
     * @param component The component to generate PURL for
     * @return The generated PURL string
     */
    std::string generatePURL(const ComponentInfo& component);

    /**
     * @brief Generate debug properties for CycloneDX component
     * @param component The component to generate properties for
     * @return The debug properties as a string
     */
    std::string generateDebugProperties(const ComponentInfo& component);

    /**
     * @brief Generate evidence field for CycloneDX 1.6+ component
     * @param component The component to generate evidence for
     * @return The evidence field as a string
     */
    std::string generateEvidenceField(const ComponentInfo& component);
};

/**
 * @brief Default constructor
 */
SBOMGenerator::SBOMGenerator() : pImpl(std::make_unique<Impl>()) {
    pImpl->metadataExtractor = std::make_unique<MetadataExtractor>();
}

/**
 * @brief Destructor
 */
SBOMGenerator::~SBOMGenerator() = default;

/**
 * @brief Process a component and add it to the SBOM
 * @param component The component to process
 */
void SBOMGenerator::processComponent(const ComponentInfo& component) {
    std::string key = component.name + ":" + component.filePath;

    if (pImpl->components.find(key) == pImpl->components.end()) {
        // New component, extract metadata
        ComponentInfo processedComponent = component;

        if (pImpl->metadataExtractor) {
            pImpl->metadataExtractor->extractMetadata(processedComponent);
        }

        pImpl->components[key] = processedComponent;
        Utils::debugPrint("Processed component: " + component.name);

        // Add source files as separate components if DWARF info is available
        if (processedComponent.containsDebugInfo && !processedComponent.sourceFiles.empty()) {
            for (const auto& sourceFile : processedComponent.sourceFiles) {
                // Create a unique key for source file component
                std::string sourceKey = "source:" + sourceFile;
                
                if (pImpl->components.find(sourceKey) == pImpl->components.end()) {
                    ComponentInfo sourceComponent(Utils::getFileName(sourceFile), sourceFile);
                    sourceComponent.fileType = FileType::Source;
                    sourceComponent.license = Utils::detectLicenseFromPath(sourceFile);
                    sourceComponent.supplier = "Organization: UNKNOWN";
                    sourceComponent.downloadLocation = "NOASSERTION";
                    sourceComponent.homepage = "N/A";
                    sourceComponent.version = "UNKNOWN";
                    
                    // Calculate checksum for source file if it exists
                    if (std::filesystem::exists(sourceFile)) {
                        sourceComponent.checksum = Utils::calculateSHA256(sourceFile);
                        sourceComponent.fileSize = std::filesystem::file_size(sourceFile);
                    }
                    
                    pImpl->components[sourceKey] = sourceComponent;
                    Utils::debugPrint("Added source file component: " + sourceFile);
                }
            }
        }
    } else {
        // Update existing component
        ComponentInfo& existing = pImpl->components[key];

        // Merge information
        for (const auto& symbol : component.symbols) {
            existing.addSymbol(symbol);
        }

        for (const auto& section : component.sections) {
            existing.addSection(section);
        }

        for (const auto& dep : component.dependencies) {
            existing.addDependency(dep);
        }

        for (const auto& source : component.sourceFiles) {
            existing.addSourceFile(source);
        }

        Utils::debugPrint("Updated component: " + component.name);
    }
}

/**
 * @brief Generate the SBOM in the specified format
 */
void SBOMGenerator::generateSBOM() {
    if (pImpl->outputPath.empty()) {
        Utils::errorPrint("No output path specified for SBOM generation");
        return;
    }

    Utils::debugPrint("Generating SBOM with " + std::to_string(pImpl->components.size()) +
                      " components");

    bool success = false;
    if (pImpl->format == "spdx" || pImpl->format == "spdx-2.3" || pImpl->format == "spdx-3.0") {
        success = pImpl->generateSPDX(pImpl->outputPath);
    } else if (pImpl->format == "cyclonedx" || pImpl->format == "cyclonedx-1.4") {
        success = pImpl->generateCycloneDX(pImpl->outputPath);
    } else {
        Utils::errorPrint("Unsupported SBOM format: " + pImpl->format);
        return;
    }

    if (success) {
        Utils::debugPrint("SBOM generated successfully: " + pImpl->outputPath);
    } else {
        Utils::errorPrint("Failed to generate SBOM");
    }
}

/**
 * @brief Set the output path for the SBOM
 * @param path The output file path
 */
void SBOMGenerator::setOutputPath(const std::string& path) {
    pImpl->outputPath = path;
}

/**
 * @brief Set the output format for the SBOM
 * @param fmt The format (e.g., "spdx", "cyclonedx")
 */
void SBOMGenerator::setFormat(const std::string& fmt) {
    pImpl->format = Utils::toLower(fmt);
}

/**
 * @brief Set the CycloneDX specification version
 * @param version The CycloneDX version (e.g., "1.4", "1.5", "1.6")
 */
void SBOMGenerator::setCycloneDXVersion(const std::string& version) {
    pImpl->cyclonedxVersion = version;
}

/**
 * @brief Set the SPDX specification version
 * @param version The SPDX version (e.g., "2.3", "3.0")
 */
void SBOMGenerator::setSPDXVersion(const std::string& version) {
    pImpl->spdxVersion = version;
}

/**
 * @brief Get the number of components in the SBOM
 * @return Number of components
 */
size_t SBOMGenerator::getComponentCount() const {
    return pImpl->components.size();
}

/**
 * @brief Check if a component exists in the SBOM
 * @param name The component name to check
 * @return true if the component exists
 */
bool SBOMGenerator::hasComponent(const std::string& name) const {
    for (const auto& pair : pImpl->components) {
        if (pair.second.name == name) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Print statistics about the SBOM
 */
void SBOMGenerator::printStatistics() const {
    std::cout << "SBOM Generator Statistics:" << std::endl;
    std::cout << "  Total components: " << pImpl->components.size() << std::endl;

    size_t objects = 0, staticLibs = 0, sharedLibs = 0, executables = 0;
    size_t systemLibs = 0, withDebugInfo = 0, stripped = 0;

    for (const auto& pair : pImpl->components) {
        const auto& component = pair.second;

        switch (component.fileType) {
            case FileType::Object:
                objects++;
                break;
            case FileType::StaticLibrary:
                staticLibs++;
                break;
            case FileType::SharedLibrary:
                sharedLibs++;
                break;
            case FileType::Executable:
                executables++;
                break;
            default:
                break;
        }

        if (component.isSystemLibrary)
            systemLibs++;
        if (component.containsDebugInfo)
            withDebugInfo++;
        if (component.isStripped)
            stripped++;
    }

    std::cout << "  Object files: " << objects << std::endl;
    std::cout << "  Static libraries: " << staticLibs << std::endl;
    std::cout << "  Shared libraries: " << sharedLibs << std::endl;
    std::cout << "  Executables: " << executables << std::endl;
    std::cout << "  System libraries: " << systemLibs << std::endl;
    std::cout << "  With debug info: " << withDebugInfo << std::endl;
    std::cout << "  Stripped: " << stripped << std::endl;
}

/**
 * @brief Generate SBOM in SPDX format
 * @param outputPath The output file path
 * @return true if generation was successful
 */
bool SBOMGenerator::Impl::generateSPDX(const std::string& outputPath) {
    std::string document;
    
    if (spdxVersion == "3.0") {
        document = generateSPDX3JSONDocument();
    } else {
        document = generateSPDXDocument();
    }

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        Utils::errorPrint("Could not open output file: " + outputPath);
        return false;
    }

    file << document;
    return true;
}

bool SBOMGenerator::Impl::generateCycloneDX(const std::string& outputPath) {
    std::string document = generateCycloneDXDocument();

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        Utils::errorPrint("Could not open output file: " + outputPath);
        return false;
    }

    file << document;
    return true;
}

std::string SBOMGenerator::Impl::generateSPDXDocument() {
    std::stringstream ss;

    // SPDX Document Header
    ss << "SPDXVersion: SPDX-2.3\n";
    ss << "DataLicense: CC0-1.0\n";
    ss << "SPDXID: SPDXRef-DOCUMENT\n";
    ss << "DocumentName: Heimdall Generated SBOM\n";
    ss << "DocumentNamespace: https://spdx.org/spdxdocs/heimdall-" << getCurrentTimestamp() << "\n";
    ss << "Creator: Tool: Heimdall-1.0.0\n";
    ss << "Created: " << getCurrentTimestamp() << "\n";
    ss << "\n";

    // Package information
    ss << "PackageName: " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)
       << "\n";
    ss << "SPDXID: " << generateSPDXId("Package") << "\n";
    ss << "PackageVersion: " << (buildInfo.buildId.empty() ? "Unknown" : buildInfo.buildId) << "\n";
    ss << "PackageFileName: " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)
       << "\n";
    ss << "PackageDownloadLocation: NOASSERTION\n";
    ss << "FilesAnalyzed: true\n";
    ss << "PackageVerificationCode: " << generateVerificationCode() << "\n";
    ss << "PackageLicenseConcluded: NOASSERTION\n";
    ss << "PackageLicenseInfoFromFiles: NOASSERTION\n";
    ss << "PackageLicenseDeclared: NOASSERTION\n";
    ss << "PackageCopyrightText: NOASSERTION\n";
    ss << "PackageSummary: Software Bill of Materials generated by Heimdall\n";
    ss << "PackageDescription: SBOM for "
       << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\n";
    ss << "\n";

    // File information for each component
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << generateSPDXComponent(component);
    }

    // Relationships
    ss << "Relationship: " << generateSPDXId("Package") << " CONTAINS ";
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << generateSPDXId(component.name) << " ";
    }
    ss << "\n";

    // Add relationships between binaries and their source files
    for (const auto& pair : components) {
        const auto& component = pair.second;
        
        // Skip source file components themselves
        if (component.fileType == FileType::Source) {
            continue;
        }
        
        // Add relationships to source files if this component has debug info
        if (component.containsDebugInfo && !component.sourceFiles.empty()) {
            for (const auto& sourceFile : component.sourceFiles) {
                std::string sourceKey = "source:" + sourceFile;
                auto sourceIt = components.find(sourceKey);
                if (sourceIt != components.end()) {
                    ss << "Relationship: " << generateSPDXId(component.name) 
                       << " GENERATED_FROM " << generateSPDXId(sourceIt->second.name) << "\n";
                }
            }
        }
    }

    return ss.str();
}

std::string SBOMGenerator::Impl::generateCycloneDXDocument() {
    std::stringstream ss;

    // Generate a unique serial number in UUID format as required by 1.5+ schema
    std::string serialNumber = "urn:uuid:" + Utils::generateUUID();

    ss << "{\n";
    ss << "  \"$schema\": \"http://cyclonedx.org/schema/bom-" << cyclonedxVersion << ".schema.json\",\n";
    ss << "  \"bomFormat\": \"CycloneDX\",\n";
    ss << "  \"specVersion\": \"" << cyclonedxVersion << "\",\n";
    ss << "  \"serialNumber\": \"" << serialNumber << "\",\n";
    ss << "  \"version\": 1,\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    ss << "    \"tools\": {\n";
    ss << "      \"components\": [\n";
    ss << "        {\n";
    ss << "          \"type\": \"application\",\n";
    ss << "          \"bom-ref\": \"heimdall-sbom-generator\",\n";
    ss << "          \"supplier\": {\n";
    ss << "            \"name\": \"Heimdall Project\"\n";
    ss << "          },\n";
    ss << "          \"name\": \"Heimdall SBOM Generator\",\n";
    ss << "          \"version\": \"2.0.0\"\n";
    ss << "        }\n";
    ss << "      ]\n";
    ss << "    },\n";
    ss << "    \"component\": {\n";
    ss << "      \"type\": \"application\",\n";
    ss << "      \"name\": "
       << Utils::formatJsonValue(buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)
       << ",\n";
    ss << "      \"version\": "
       << Utils::formatJsonValue(buildInfo.buildId.empty() ? "Unknown" : buildInfo.buildId) << "\n";
    ss << "    }";
    
    // Add CycloneDX 1.5+ specific metadata fields (lifecycles supported in both 1.5 and 1.6)
    if (cyclonedxVersion >= "1.5") {
        ss << ",\n";
        ss << "    \"lifecycles\": [\n";
        ss << "      {\n";
        ss << "        \"phase\": \"build\"\n";
        ss << "      }\n";
        ss << "    ]";
    }
    
    ss << "\n";
    ss << "  },\n";
    ss << "  \"components\": [\n";

    bool first = true;
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!first)
            ss << ",\n";
        ss << generateCycloneDXComponent(component);
        first = false;
    }

    ss << "\n  ]\n";
    ss << "}\n";

    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXComponent(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "FileName: " << Utils::getFileName(component.filePath) << "\n";
    ss << "SPDXID: " << generateSPDXId(component.name) << "\n";
    ss << "FileChecksum: SHA256: " << (component.checksum.empty() ? "UNKNOWN" : component.checksum)
       << "\n";
    ss << "Supplier: "
       << (component.supplier.empty() ? "Organization: UNKNOWN" : component.supplier) << "\n";
    ss << "DownloadLocation: "
       << (component.downloadLocation.empty() ? "NOASSERTION" : component.downloadLocation) << "\n";
    ss << "Homepage: " << (component.homepage.empty() ? "N/A" : component.homepage) << "\n";
    ss << "Version: " << (component.version.empty() ? "UNKNOWN" : component.version) << "\n";
    ss << "LicenseConcluded: " << (component.license.empty() ? "NOASSERTION" : component.license)
       << "\n";
    ss << "LicenseInfoInFile: " << (component.license.empty() ? "NOASSERTION" : component.license)
       << "\n";
    ss << "FileCopyrightText: NOASSERTION\n";
    ss << "FileComment: " << component.getFileTypeString() << " file\n";
    ss << "\n";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateDebugProperties(const ComponentInfo& component) {
    if (!component.containsDebugInfo) {
        return "";
    }
    
    std::stringstream properties;
    properties << ",\n      \"properties\": [\n";
    
    bool hasPreviousProperty = false;
    
    // Add source files property
    if (!component.sourceFiles.empty()) {
        properties << "        {\n";
        properties << "          \"name\": \"heimdall:source-files\",\n";
        properties << "          \"value\": " << Utils::formatJsonValue(Utils::join(component.sourceFiles, ",")) << "\n";
        properties << "        }";
        hasPreviousProperty = true;
    }
    
    // Add functions property if available
    if (!component.functions.empty()) {
        if (hasPreviousProperty) {
            properties << ",\n";
        }
        properties << "        {\n";
        properties << "          \"name\": \"heimdall:functions\",\n";
        properties << "          \"value\": " << Utils::formatJsonValue(Utils::join(component.functions, ",")) << "\n";
        properties << "        }";
        hasPreviousProperty = true;
    }
    
    // Add compile units property if available
    if (!component.compileUnits.empty()) {
        if (hasPreviousProperty) {
            properties << ",\n";
        }
        properties << "        {\n";
        properties << "          \"name\": \"heimdall:compile-units\",\n";
        properties << "          \"value\": " << Utils::formatJsonValue(Utils::join(component.compileUnits, ",")) << "\n";
        properties << "        }";
        hasPreviousProperty = true;
    }
    
    // Add debug info flag
    if (hasPreviousProperty) {
        properties << ",\n";
    }
    properties << "        {\n";
    properties << "          \"name\": \"heimdall:contains-debug-info\",\n";
    properties << "          \"value\": \"true\"\n";
    properties << "        }";
    
    properties << "\n      ]";
    return properties.str();
}

std::string SBOMGenerator::Impl::generateEvidenceField(const ComponentInfo& component) {
    if (cyclonedxVersion < "1.5") {
        return "";
    }
    
    std::stringstream evidence;
    evidence << "      \"evidence\": {\n";
    
    // Add identity evidence (supported in both 1.5 and 1.6)
    bool hasIdentity = !component.checksum.empty() || !component.filePath.empty();
    if (hasIdentity) {
        evidence << "        \"identity\": {\n";
        evidence << "          \"field\": \"hash\",\n";
        evidence << "          \"confidence\": 1.0,\n";
        evidence << "          \"methods\": [\n";
        evidence << "            {\n";
        evidence << "              \"technique\": \"binary-analysis\",\n";
        evidence << "              \"confidence\": 1.0,\n";
        evidence << "              \"value\": \"File hash verification\"\n";
        evidence << "            }\n";
        evidence << "          ]\n";
        evidence << "        },\n";
    }
    
    // Add occurrence evidence (supported in both 1.5 and 1.6)
    evidence << "        \"occurrences\": [\n";
    evidence << "          {\n";
    evidence << "            \"location\": " << Utils::formatJsonValue(component.filePath) << "\n";
    evidence << "          }\n";
    evidence << "        ]";
    
    // Add callstack evidence if debug info is available (different structure for 1.5 vs 1.6)
    if (component.containsDebugInfo && !component.functions.empty()) {
        evidence << ",\n";
        evidence << "        \"callstack\": {\n";
        evidence << "          \"frames\": [\n";
        bool firstFrame = true;
        int frameCount = 0;
        for (const auto& func : component.functions) {
            if (!firstFrame) evidence << ",\n";
            evidence << "            {\n";
            
            if (cyclonedxVersion >= "1.6") {
                // CycloneDX 1.6 format - function field is optional
                evidence << "              \"function\": " << Utils::formatJsonValue(func) << ",\n";
                evidence << "              \"line\": 1,\n";
                evidence << "              \"column\": 1\n";
            } else {
                // CycloneDX 1.5 format - module field is required
                evidence << "              \"module\": " << Utils::formatJsonValue(Utils::getFileName(component.filePath)) << ",\n";
                evidence << "              \"function\": " << Utils::formatJsonValue(func) << ",\n";
                evidence << "              \"line\": 1,\n";
                evidence << "              \"column\": 1\n";
            }
            
            evidence << "            }";
            firstFrame = false;
            frameCount++;
            if (frameCount >= 3) break; // Limit to first few for brevity
        }
        evidence << "\n          ]\n";
        evidence << "        }";
    }
    
    evidence << "\n      }";
    return evidence.str();
}

std::string SBOMGenerator::Impl::generateCycloneDXComponent(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"type\": \"library\",\n";
    ss << "      \"bom-ref\": \"component-" << generateSPDXId(component.name) << "\",\n";
    ss << "      \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
    ss << "      \"version\": "
       << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version)
       << ",\n";
    ss << "      \"description\": "
       << Utils::formatJsonValue(component.getFileTypeString() + " component") << ",\n";
    
    // Fix supplier to be an organizational entity object
    ss << "      \"supplier\": {\n";
    ss << "        \"name\": "
       << Utils::formatJsonValue(component.supplier.empty() ? "UNKNOWN" : component.supplier) << "\n";
    ss << "      },\n";
    
    ss << "      \"hashes\": [\n";
    ss << "        {\n";
    ss << "          \"alg\": \"SHA-256\",\n";
    ss << "          \"content\": \""
       << (component.checksum.empty() ? "UNKNOWN" : component.checksum) << "\"\n";
    ss << "        }\n";
    ss << "      ],\n";
    ss << "      \"purl\": \"" << generatePURL(component) << "\",\n";
    ss << "      \"externalReferences\": [\n";
    ss << "        {\n";
    ss << "          \"type\": \"distribution\",\n";
    ss << "          \"url\": "
       << Utils::formatJsonValue(component.downloadLocation.empty() ? "NOASSERTION"
                                                                    : component.downloadLocation)
       << "\n";
    ss << "        }\n";
    ss << "      ]";

    // Add debug properties and evidence for 1.6+
    std::string debugProperties = generateDebugProperties(component);
    std::string evidenceField = generateEvidenceField(component);
    
    // Output properties and evidence with correct comma placement
    if (!debugProperties.empty() && !evidenceField.empty()) {
        ss << debugProperties << ",\n" << evidenceField;
    } else if (!debugProperties.empty()) {
        ss << debugProperties;
    } else if (!evidenceField.empty()) {
        ss << ",\n" << evidenceField;
    }
    
    ss << "\n    }";
    return ss.str();
}

std::string SBOMGenerator::Impl::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXId(const std::string& name) {
    std::string id = "SPDXRef-" + name;
    // Replace invalid characters
    std::replace(id.begin(), id.end(), ' ', '-');
    std::replace(id.begin(), id.end(), '/', '-');
    std::replace(id.begin(), id.end(), '\\', '-');
    return id;
}

std::string SBOMGenerator::Impl::generateVerificationCode() {
    // Generate a simple verification code based on component checksums
    std::string allChecksums;
    for (const auto& pair : components) {
        allChecksums += pair.second.checksum;
    }

    // Simple hash of all checksums
    std::hash<std::string> hasher;
    std::stringstream ss;
    ss << std::hex << hasher(allChecksums);
    return ss.str().substr(0, 40);  // SPDX uses 40-character verification codes
}

std::string SBOMGenerator::Impl::generatePURL(const ComponentInfo& component) {
    std::stringstream ss;

    if (component.packageManager == "conan") {
        ss << "pkg:conan/" << component.name << "@" << component.version;
    } else if (component.packageManager == "vcpkg") {
        ss << "pkg:vcpkg/" << component.name << "@" << component.version;
    } else if (component.packageManager == "system") {
        ss << "pkg:system/" << component.name << "@" << component.version;
    } else {
        ss << "pkg:generic/" << component.name << "@" << component.version;
    }

    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3JSONDocument() {
    std::stringstream ss;

    ss << "{\n";
    ss << "  \"spdxVersion\": \"SPDX-3.0\",\n";
    ss << "  \"dataLicense\": \"CC0-1.0\",\n";
    ss << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
    ss << "  \"name\": " << Utils::formatJsonValue("Heimdall Generated SBOM") << ",\n";
    ss << "  \"documentNamespace\": " << Utils::formatJsonValue("https://spdx.org/spdxdocs/heimdall-" + getCurrentTimestamp()) << ",\n";
    ss << "  \"creationInfo\": {\n";
    ss << "    \"creators\": [\n";
    ss << "      \"Tool: Heimdall-2.0.0\"\n";
    ss << "    ],\n";
    ss << "    \"created\": " << Utils::formatJsonValue(getCurrentTimestamp()) << "\n";
    ss << "  },\n";
    ss << "  \"packages\": [\n";
    ss << "    {\n";
    ss << "      \"SPDXID\": " << Utils::formatJsonValue(generateSPDXId("Package")) << ",\n";
    ss << "      \"name\": " << Utils::formatJsonValue(buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << ",\n";
    ss << "      \"versionInfo\": " << Utils::formatJsonValue(buildInfo.buildId.empty() ? "Unknown" : buildInfo.buildId) << ",\n";
    ss << "      \"downloadLocation\": \"NOASSERTION\",\n";
    ss << "      \"filesAnalyzed\": true,\n";
    ss << "      \"verificationCode\": {\n";
    ss << "        \"packageVerificationCodeValue\": " << Utils::formatJsonValue(generateVerificationCode()) << "\n";
    ss << "      },\n";
    ss << "      \"licenseConcluded\": \"NOASSERTION\",\n";
    ss << "      \"licenseInfoFromFiles\": [\"NOASSERTION\"],\n";
    ss << "      \"licenseDeclared\": \"NOASSERTION\",\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"summary\": " << Utils::formatJsonValue("Software Bill of Materials generated by Heimdall") << ",\n";
    ss << "      \"description\": " << Utils::formatJsonValue("SBOM for " + (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)) << "\n";
    ss << "    }\n";
    ss << "  ],\n";
    ss << "  \"files\": [\n";

    bool first = true;
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!first)
            ss << ",\n";
        ss << generateSPDX3JSONComponent(component);
        first = false;
    }

    ss << "\n  ],\n";
    ss << "  \"relationships\": [\n";
    
    // Package contains relationships
    bool firstRel = true;
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!firstRel)
            ss << ",\n";
        ss << "    {\n";
        ss << "      \"spdxElementId\": " << Utils::formatJsonValue(generateSPDXId("Package")) << ",\n";
        ss << "      \"relatedSpdxElement\": " << Utils::formatJsonValue(generateSPDXId(component.name)) << ",\n";
        ss << "      \"relationshipType\": \"CONTAINS\"\n";
        ss << "    }";
        firstRel = false;
    }

    // Add relationships between binaries and their source files
    for (const auto& pair : components) {
        const auto& component = pair.second;
        
        // Skip source file components themselves
        if (component.fileType == FileType::Source) {
            continue;
        }
        
        // Add relationships to source files if this component has debug info
        if (component.containsDebugInfo && !component.sourceFiles.empty()) {
            for (const auto& sourceFile : component.sourceFiles) {
                std::string sourceKey = "source:" + sourceFile;
                auto sourceIt = components.find(sourceKey);
                if (sourceIt != components.end()) {
                    ss << ",\n";
                    ss << "    {\n";
                    ss << "      \"spdxElementId\": " << Utils::formatJsonValue(generateSPDXId(component.name)) << ",\n";
                    ss << "      \"relatedSpdxElement\": " << Utils::formatJsonValue(generateSPDXId(sourceIt->second.name)) << ",\n";
                    ss << "      \"relationshipType\": \"GENERATED_FROM\"\n";
                    ss << "    }";
                }
            }
        }
    }

    ss << "\n  ]\n";
    ss << "}\n";

    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3JSONComponent(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"SPDXID\": " << Utils::formatJsonValue(generateSPDXId(component.name)) << ",\n";
    ss << "      \"fileName\": " << Utils::formatJsonValue(Utils::getFileName(component.filePath)) << ",\n";
    ss << "      \"checksums\": [\n";
    ss << "        {\n";
    ss << "          \"algorithm\": \"SHA256\",\n";
    ss << "          \"checksumValue\": " << Utils::formatJsonValue(component.checksum.empty() ? "UNKNOWN" : component.checksum) << "\n";
    ss << "        }\n";
    ss << "      ],\n";
    ss << "      \"licenseConcluded\": " << Utils::formatJsonValue(component.license.empty() ? "NOASSERTION" : component.license) << ",\n";
    ss << "      \"licenseInfoInFiles\": [" << Utils::formatJsonValue(component.license.empty() ? "NOASSERTION" : component.license) << "],\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"comment\": " << Utils::formatJsonValue(component.getFileTypeString() + " file") << ",\n";
    ss << "      \"fileTypes\": [\n";
    ss << "        " << Utils::formatJsonValue(component.getFileTypeString()) << "\n";
    ss << "      ]";

    // Add DWARF debug information as annotations
    if (component.containsDebugInfo) {
        ss << ",\n      \"annotations\": [\n";
        
        // Add source files annotation
        if (!component.sourceFiles.empty()) {
            ss << "        {\n";
            ss << "          \"annotator\": \"Tool: Heimdall-2.0.0\",\n";
            ss << "          \"annotationDate\": " << Utils::formatJsonValue(getCurrentTimestamp()) << ",\n";
            ss << "          \"annotationType\": \"OTHER\",\n";
            ss << "          \"comment\": " << Utils::formatJsonValue("Source files: " + Utils::join(component.sourceFiles, ", ")) << "\n";
            ss << "        }";
        }
        
        // Add functions annotation if available
        if (!component.functions.empty()) {
            if (!component.sourceFiles.empty()) {
                ss << ",\n";
            }
            ss << "        {\n";
            ss << "          \"annotator\": \"Tool: Heimdall-2.0.0\",\n";
            ss << "          \"annotationDate\": " << Utils::formatJsonValue(getCurrentTimestamp()) << ",\n";
            ss << "          \"annotationType\": \"OTHER\",\n";
            ss << "          \"comment\": " << Utils::formatJsonValue("Functions: " + Utils::join(component.functions, ", ")) << "\n";
            ss << "        }";
        }
        
        // Add compile units annotation if available
        if (!component.compileUnits.empty()) {
            if (!component.sourceFiles.empty() || !component.functions.empty()) {
                ss << ",\n";
            }
            ss << "        {\n";
            ss << "          \"annotator\": \"Tool: Heimdall-2.0.0\",\n";
            ss << "          \"annotationDate\": " << Utils::formatJsonValue(getCurrentTimestamp()) << ",\n";
            ss << "          \"annotationType\": \"OTHER\",\n";
            ss << "          \"comment\": " << Utils::formatJsonValue("Compile units: " + Utils::join(component.compileUnits, ", ")) << "\n";
            ss << "        }";
        }
        
        ss << "\n      ]";
    }
    
    ss << "\n    }";
    return ss.str();
}

}  // namespace heimdall
