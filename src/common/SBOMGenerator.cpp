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
    std::string spdxVersion = "2.3";                            ///< SPDX specification version (default to 2.3 for compatibility)
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
     * @brief Generate SPDX document content (version-specific)
     * @return The SPDX document as a string
     */
    std::string generateSPDXDocument();

    /**
     * @brief Generate SPDX 3.0.0 JSON document content
     * @return The SPDX 3.0.0 JSON document as a string
     */
    std::string generateSPDX3_0_0_Document();

    /**
     * @brief Generate SPDX 3.0.1 JSON document content
     * @return The SPDX 3.0.1 JSON document as a string
     */
    std::string generateSPDX3_0_1_Document();

    /**
     * @brief Generate SPDX 2.3 tag-value document content
     * @return The SPDX 2.3 document as a string
     */
    std::string generateSPDX2_3_Document();

    /**
     * @brief Generate CycloneDX document content
     * @return The CycloneDX document as a string
     */
    std::string generateCycloneDXDocument();

    /**
     * @brief Generate SPDX component entry (version-specific)
     * @param component The component to generate entry for
     * @return The SPDX component entry as a string
     */
    std::string generateSPDXComponent(const ComponentInfo& component);

    /**
     * @brief Generate SPDX 3.0.0 JSON component entry
     * @param component The component to generate entry for
     * @return The SPDX 3.0.0 JSON component entry as a string
     */
    std::string generateSPDX3_0_0_Component(const ComponentInfo& component);

    /**
     * @brief Generate SPDX 3.0.1 JSON component entry
     * @param component The component to generate entry for
     * @return The SPDX 3.0.1 JSON component entry as a string
     */
    std::string generateSPDX3_0_1_Component(const ComponentInfo& component);

    /**
     * @brief Generate SPDX 2.3 tag-value component entry
     * @param component The component to generate entry for
     * @return The SPDX 2.3 component entry as a string
     */
    std::string generateSPDX2_3_Component(const ComponentInfo& component);

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
     * @brief Generate document namespace for SPDX
     * @return The document namespace string
     */
    std::string generateDocumentNamespace();

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

    /**
     * @brief Generate SPDX 3.x creation info
     * @return The creation info object as a string
     */
    std::string generateSPDX3CreationInfo();

    /**
     * @brief Generate SPDX license identifier (validated)
     * @param license The license string to validate
     * @return Valid SPDX license identifier
     */
    std::string generateSPDXLicenseId(const std::string& license);

    /**
     * @brief Generate element ID for SPDX 3.x (namespace-aware)
     * @param name The element name
     * @return The generated element ID with namespace
     */
    std::string generateSPDXElementId(const std::string& name);

    /**
     * @brief Generate SPDX 3.x relationships array
     * @return The relationships array as a string
     */
    std::string generateSPDX3Relationships();
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
    if (pImpl->format == "spdx" || pImpl->format == "spdx-2.3") {
        // Check if SPDX version is 3.0 or higher for JSON format
        if (pImpl->spdxVersion == "3.0" || pImpl->spdxVersion == "3.0.0" || pImpl->spdxVersion == "3.0.1") {
            success = pImpl->generateSPDX3JSON(pImpl->outputPath);
        } else {
            success = pImpl->generateSPDX(pImpl->outputPath);
        }
    } else if (pImpl->format == "spdx-3.0" || pImpl->format == "spdx-3.0.0") {
        pImpl->spdxVersion = "3.0.0";
        success = pImpl->generateSPDX3JSON(pImpl->outputPath);
    } else if (pImpl->format == "spdx-3.0.1") {
        pImpl->spdxVersion = "3.0.1";
        success = pImpl->generateSPDX3JSON(pImpl->outputPath);
    } else if (pImpl->format == "cyclonedx" || pImpl->format.find("cyclonedx-") == 0) {
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
    
    if (spdxVersion == "3.0.1") {
        document = generateSPDX3_0_1_Document();
    } else if (spdxVersion == "3.0.0" || spdxVersion == "3.0") {
        document = generateSPDX3_0_0_Document();
    } else {
        // Default to SPDX 2.3 tag-value format
        document = generateSPDX2_3_Document();
    }

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        Utils::errorPrint("Could not open output file: " + outputPath);
        return false;
    }

    file << document;
    return true;
}

bool SBOMGenerator::Impl::generateSPDX3JSON(const std::string& outputPath) {
    std::string document;
    
    if (spdxVersion == "3.0.1") {
        document = generateSPDX3_0_1_Document();
    } else {
        document = generateSPDX3_0_0_Document();
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
    if (spdxVersion == "3.0.1") {
        return generateSPDX3_0_1_Document();
    } else if (spdxVersion == "3.0.0" || spdxVersion == "3.0") {
        return generateSPDX3_0_0_Document();
    } else {
        return generateSPDX2_3_Document();
    }
}

std::string SBOMGenerator::Impl::generateSPDX2_3_Document() {
    std::stringstream ss;

    // SPDX 2.3 Document Header (tag-value format)
    ss << "SPDXVersion: SPDX-2.3\n";
    ss << "DataLicense: CC0-1.0\n";
    ss << "SPDXID: SPDXRef-DOCUMENT\n";
    ss << "DocumentName: Heimdall Generated SBOM\n";
    ss << "DocumentNamespace: " << generateDocumentNamespace() << "\n";
    ss << "Creators: Tool: Heimdall SBOM Generator-2.0.0\n";
    ss << "Created: " << getCurrentTimestamp() << "\n";
    ss << "\n";

    // Package information (required in SPDX 2.3)
    ss << "PackageName: " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\n";
    ss << "SPDXID: " << generateSPDXId("Package") << "\n";
    ss << "PackageVersion: " << (buildInfo.buildId.empty() ? "Unknown" : buildInfo.buildId) << "\n";
    ss << "PackageFileName: " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\n";
    ss << "PackageDownloadLocation: NOASSERTION\n";
    ss << "FilesAnalyzed: true\n";
    ss << "PackageVerificationCode: " << generateVerificationCode() << "\n";
    ss << "PackageLicenseConcluded: NOASSERTION\n";
    ss << "PackageLicenseInfoFromFiles: NOASSERTION\n";
    ss << "PackageLicenseDeclared: NOASSERTION\n";
    ss << "PackageCopyrightText: NOASSERTION\n";
    ss << "PackageDescription: Software Bill of Materials generated by Heimdall\n";
    ss << "\n";

    // File information for each component
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << generateSPDX2_3_Component(component);
    }

    // Relationships (SPDX 2.3 format)
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << "Relationship: " << generateSPDXId("Package") << " CONTAINS " << generateSPDXId(component.name) << "\n";
    }

    // Add relationships between binaries and their source files
    for (const auto& pair : components) {
        const auto& component = pair.second;
        
        if (component.fileType != FileType::Source && component.containsDebugInfo && !component.sourceFiles.empty()) {
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

std::string SBOMGenerator::Impl::generateSPDX3_0_0_Document() {
    std::stringstream ss;

    ss << "{\n";
    ss << "  \"@context\": \"https://raw.githubusercontent.com/spdx/spdx-spec/v3.0.0/rdf/context.json\",\n";
    ss << "  \"@type\": \"SpdxDocument\",\n";
    ss << "  \"spdxId\": \"" << generateSPDXElementId("DOCUMENT") << "\",\n";
    ss << "  \"name\": \"Heimdall Generated SBOM\",\n";
    ss << "  \"dataLicense\": \"CC0-1.0\",\n";
    ss << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
    ss << "  \"documentNamespace\": \"" << generateDocumentNamespace() << "\",\n";
    ss << generateSPDX3CreationInfo() << ",\n";
    
    // Elements (SPDX 3.0 uses element-based model)
    ss << "  \"element\": [\n";
    
    // Package element
    ss << "    {\n";
    ss << "      \"@type\": \"software_Package\",\n";
    ss << "      \"spdxId\": \"" << generateSPDXElementId("Package") << "\",\n";
    ss << "      \"name\": \"" << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\",\n";
    if (!buildInfo.buildId.empty()) {
        ss << "      \"versionInfo\": \"" << buildInfo.buildId << "\",\n";
    }
    ss << "      \"downloadLocation\": \"NOASSERTION\",\n";
    ss << "      \"filesAnalyzed\": true,\n";
    ss << "      \"verificationCode\": {\n";
    ss << "        \"packageVerificationCodeValue\": \"" << generateVerificationCode() << "\"\n";
    ss << "      },\n";
    ss << "      \"licenseConcluded\": \"NOASSERTION\",\n";
    ss << "      \"licenseInfoFromFiles\": [\"NOASSERTION\"],\n";
    ss << "      \"licenseDeclared\": \"NOASSERTION\",\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"comment\": \"Software Bill of Materials generated by Heimdall\"\n";
    ss << "    }";

    // File elements for each component
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << ",\n" << generateSPDX3_0_0_Component(component);
    }

    ss << "\n  ],\n";
    ss << generateSPDX3Relationships();
    ss << "\n}\n";

    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Document() {
    std::stringstream ss;

    ss << "{\n";
    ss << "  \"@context\": \"https://raw.githubusercontent.com/spdx/spdx-spec/v3.0.1/rdf/context.json\",\n";
    ss << "  \"@type\": \"SpdxDocument\",\n";
    ss << "  \"spdxId\": \"" << generateSPDXElementId("DOCUMENT") << "\",\n";
    ss << "  \"name\": \"Heimdall Generated SBOM\",\n";
    ss << "  \"dataLicense\": \"CC0-1.0\",\n";
    ss << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
    ss << "  \"documentNamespace\": \"" << generateDocumentNamespace() << "\",\n";
    ss << generateSPDX3CreationInfo() << ",\n";
    
    // Elements (SPDX 3.0.1 refined element model)
    ss << "  \"element\": [\n";
    
    // Package element
    ss << "    {\n";
    ss << "      \"@type\": \"software_Package\",\n";
    ss << "      \"spdxId\": \"" << generateSPDXElementId("Package") << "\",\n";
    ss << "      \"name\": \"" << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\",\n";
    if (!buildInfo.buildId.empty()) {
        ss << "      \"versionInfo\": \"" << buildInfo.buildId << "\",\n";
    }
    ss << "      \"downloadLocation\": \"NOASSERTION\",\n";
    ss << "      \"filesAnalyzed\": true,\n";
    ss << "      \"verificationCode\": {\n";
    ss << "        \"packageVerificationCodeValue\": \"" << generateVerificationCode() << "\"\n";
    ss << "      },\n";
    ss << "      \"licenseConcluded\": \"NOASSERTION\",\n";
    ss << "      \"licenseInfoFromFiles\": [\"NOASSERTION\"],\n";
    ss << "      \"licenseDeclared\": \"NOASSERTION\",\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"summary\": \"Software Bill of Materials generated by Heimdall\",\n";
    ss << "      \"description\": \"SBOM for " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName) << "\"\n";
    ss << "    }";

    // File elements for each component  
    for (const auto& pair : components) {
        const auto& component = pair.second;
        ss << ",\n" << generateSPDX3_0_1_Component(component);
    }

    ss << "\n  ],\n";
    ss << generateSPDX3Relationships();
    ss << "\n}\n";

    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXComponent(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "FileName: " << Utils::getFileName(component.filePath) << "\n";
    ss << "SPDXID: " << generateSPDXId(component.name) << "\n";
    ss << "FileChecksum: SHA256: " << (component.checksum.empty() ? "da39a3ee5e6b4b0d3255bfef95601890afd80709" : component.checksum) << "\n";
    if (component.fileSize > 0) {
        ss << "FileSize: " << component.fileSize << "\n";
    }
    ss << "LicenseConcluded: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "LicenseInfoInFile: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "FileCopyrightText: NOASSERTION\n";
    ss << "FileComment: " << component.getFileTypeString() << " file\n";
    
    // Add file type classification
    ss << "FileType: ";
    switch (component.fileType) {
        case FileType::Source:
            ss << "SOURCE";
            break;
        case FileType::Executable:
            ss << "BINARY";
            break;
        case FileType::StaticLibrary:
            ss << "ARCHIVE";
            break;
        default:
            ss << "OTHER";
            break;
    }
    ss << "\n";
    
    // Add annotations for debug information in SPDX 2.3
    if (component.containsDebugInfo) {
        ss << "Annotation: Tool: Heimdall-2.0.0\n";
        ss << "AnnotationDate: " << getCurrentTimestamp() << "\n";
        ss << "AnnotationType: OTHER\n";
        ss << "AnnotationComment: Contains debug information\n";
        
        if (!component.sourceFiles.empty()) {
            ss << "Annotation: Tool: Heimdall-2.0.0\n";
            ss << "AnnotationDate: " << getCurrentTimestamp() << "\n";
            ss << "AnnotationType: OTHER\n";
            ss << "AnnotationComment: Source files: " << Utils::join(component.sourceFiles, ", ") << "\n";
        }
    }
    
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
    // Map fileType to valid CycloneDX type
    std::string cyclonedxType = "library";
    switch (component.fileType) {
        case FileType::Source:
            cyclonedxType = "file";
            break;
        case FileType::Executable:
            cyclonedxType = "application";
            break;
        case FileType::StaticLibrary:
        case FileType::SharedLibrary:
        case FileType::Object:
            cyclonedxType = "library";
            break;
        default:
            cyclonedxType = "library";
            break;
    }
    ss << "      \"type\": \"" << cyclonedxType << "\",\n";
    ss << "      \"bom-ref\": \"component-" << generateSPDXId(component.name) << "\",\n";
    ss << "      \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
    
    // Version field is required in 1.3, optional in 1.4+
    if (cyclonedxVersion == "1.3" || !component.version.empty()) {
        ss << "      \"version\": "
           << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version)
           << ",\n";
    }
    
    // Description field (available in all versions)
    ss << "      \"description\": "
       << Utils::formatJsonValue(component.getFileTypeString() + " component") << ",\n";
    
    // Supplier field structure: 1.4+ uses organizational entity object, 1.3 uses string
    if (cyclonedxVersion >= "1.4") {
        ss << "      \"supplier\": {\n";
        ss << "        \"name\": "
           << Utils::formatJsonValue(component.supplier.empty() ? "UNKNOWN" : component.supplier) << "\n";
        ss << "      },\n";
    } else {
        // CycloneDX 1.3 uses string format for supplier
        ss << "      \"supplier\": "
           << Utils::formatJsonValue(component.supplier.empty() ? "UNKNOWN" : component.supplier) << ",\n";
    }
    
    // Hashes: always use valid SHA-256 hex string
    std::string hash = component.checksum;
    if (hash.empty() || hash.length() != 64 || hash.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"; // SHA-256 of empty
    }
    ss << "      \"hashes\": [\n";
    ss << "        {\n";
    ss << "          \"alg\": \"SHA-256\",\n";
    ss << "          \"content\": \"" << hash << "\"\n";
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

    // Add debug properties (available in all versions)
    std::string debugProperties = generateDebugProperties(component);
    
    // Add evidence field only for 1.5+ (not available in 1.3/1.4)
    std::string evidenceField = "";
    if (cyclonedxVersion >= "1.5") {
        evidenceField = generateEvidenceField(component);
    }
    
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
    std::replace(id.begin(), id.end(), '.', '-');
    std::replace(id.begin(), id.end(), ':', '-');
    return id;
}

std::string SBOMGenerator::Impl::generateSPDXElementId(const std::string& name) {
    std::string baseNamespace = generateDocumentNamespace();
    std::string cleanName = name;
    
    // Clean name for element ID
    std::replace(cleanName.begin(), cleanName.end(), ' ', '-');
    std::replace(cleanName.begin(), cleanName.end(), '/', '-');
    std::replace(cleanName.begin(), cleanName.end(), '\\', '-');
    std::replace(cleanName.begin(), cleanName.end(), '.', '-');
    std::replace(cleanName.begin(), cleanName.end(), ':', '-');
    
    return baseNamespace + "#SPDXRef-" + cleanName;
}

std::string SBOMGenerator::Impl::generateDocumentNamespace() {
    // Generate unique document namespace for SPDX
    std::string timestamp = getCurrentTimestamp();
    std::replace(timestamp.begin(), timestamp.end(), ':', '-');
    std::replace(timestamp.begin(), timestamp.end(), 'T', '-');
    std::replace(timestamp.begin(), timestamp.end(), 'Z', '-');
    
    return "https://spdx.org/spdxdocs/heimdall-" + timestamp + "-" + Utils::generateUUID();
}

std::string SBOMGenerator::Impl::generateVerificationCode() {
    // Generate a verification code based on component checksums
    std::vector<std::string> checksums;
    for (const auto& pair : components) {
        if (!pair.second.checksum.empty()) {
            checksums.push_back(pair.second.checksum);
        }
    }
    
    // Sort checksums for consistent verification code
    std::sort(checksums.begin(), checksums.end());
    
    std::string allChecksums = Utils::join(checksums, "");
    
    // Generate SHA-1 hash of concatenated checksums (SPDX standard)
    return Utils::calculateSHA256(allChecksums).substr(0, 40);
}

std::string SBOMGenerator::Impl::generateSPDXLicenseId(const std::string& license) {
    if (license.empty()) {
        return "NOASSERTION";
    }
    
    // Validate against common SPDX license identifiers
    std::vector<std::string> spdxLicenses = {
        "MIT", "Apache-2.0", "GPL-2.0", "GPL-3.0", "BSD-2-Clause", "BSD-3-Clause",
        "LGPL-2.1", "LGPL-3.0", "MPL-2.0", "ISC", "Unlicense", "WTFPL"
    };
    
    for (const auto& spdxLicense : spdxLicenses) {
        if (license.find(spdxLicense) != std::string::npos) {
            return spdxLicense;
        }
    }
    
    // If not found, return as custom license expression
    return "LicenseRef-" + license;
}

std::string SBOMGenerator::Impl::generateSPDX3CreationInfo() {
    std::stringstream ss;
    ss << "  \"creationInfo\": {\n";
    ss << "    \"@type\": \"CreationInfo\",\n";
    ss << "    \"specVersion\": \"" << spdxVersion << "\",\n";
    ss << "    \"created\": \"" << getCurrentTimestamp() << "\",\n";
    ss << "    \"creators\": [\n";
    ss << "      \"Tool: Heimdall SBOM Generator-2.0.0\"\n";
    ss << "    ],\n";
    ss << "    \"licenseListVersion\": \"3.21\",\n";
    ss << "    \"comment\": \"Generated by Heimdall SBOM Generator with enhanced multi-version SPDX support\"\n";
    ss << "  }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3Relationships() {
    std::stringstream ss;
    ss << "  \"relationships\": [\n";
    
    bool firstRel = true;
    
    // Document describes package relationship
    ss << "    {\n";
    ss << "      \"@type\": \"Relationship\",\n";
    ss << "      \"from\": \"" << generateSPDXElementId("DOCUMENT") << "\",\n";
    ss << "      \"to\": \"" << generateSPDXElementId("Package") << "\",\n";
    ss << "      \"relationshipType\": \"describes\"\n";
    ss << "    }";
    firstRel = false;
    
    // Package contains file relationships
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!firstRel) ss << ",\n";
        ss << "    {\n";
        ss << "      \"@type\": \"Relationship\",\n";
        ss << "      \"from\": \"" << generateSPDXElementId("Package") << "\",\n";
        ss << "      \"to\": \"" << generateSPDXElementId(component.name) << "\",\n";
        ss << "      \"relationshipType\": \"contains\"\n";
        ss << "    }";
        firstRel = false;
    }

    // Add relationships between binaries and their source files
    for (const auto& pair : components) {
        const auto& component = pair.second;
        
        if (component.fileType != FileType::Source && component.containsDebugInfo && !component.sourceFiles.empty()) {
            for (const auto& sourceFile : component.sourceFiles) {
                std::string sourceKey = "source:" + sourceFile;
                auto sourceIt = components.find(sourceKey);
                if (sourceIt != components.end()) {
                    ss << ",\n";
                    ss << "    {\n";
                    ss << "      \"@type\": \"Relationship\",\n";
                    ss << "      \"from\": \"" << generateSPDXElementId(component.name) << "\",\n";
                    ss << "      \"to\": \"" << generateSPDXElementId(sourceIt->second.name) << "\",\n";
                    ss << "      \"relationshipType\": \"generatedFrom\"\n";
                    ss << "    }";
                }
            }
        }
    }
    
    ss << "\n  ]";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_0_Component(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"@type\": \"software_File\",\n";
    ss << "      \"spdxId\": \"" << generateSPDXElementId(component.name) << "\",\n";
    ss << "      \"name\": \"" << Utils::getFileName(component.filePath) << "\",\n";
    ss << "      \"integrityMethod\": [\n";
    ss << "        {\n";
    ss << "          \"@type\": \"Hash\",\n";
    ss << "          \"algorithm\": \"sha256\",\n";
    ss << "          \"hashValue\": \"" << (component.checksum.empty() ? "da39a3ee5e6b4b0d3255bfef95601890afd80709" : component.checksum) << "\"\n";
    ss << "        }\n";
    ss << "      ],\n";
    if (component.fileSize > 0) {
        ss << "      \"size\": " << component.fileSize << ",\n";
    }
    ss << "      \"licenseConcluded\": \"" << generateSPDXLicenseId(component.license) << "\",\n";
    ss << "      \"licenseInfoFromFiles\": [\"" << generateSPDXLicenseId(component.license) << "\"],\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"comment\": \"" << component.getFileTypeString() << " file\",\n";
    ss << "      \"fileKind\": \"";
    
    // Map file types to SPDX 3.0 file kinds
    switch (component.fileType) {
        case FileType::Source:
            ss << "source";
            break;
        case FileType::Executable:
            ss << "binary";
            break;
        case FileType::StaticLibrary:
            ss << "archive";
            break;
        default:
            ss << "other";
            break;
    }
    ss << "\",\n";
        ss << "      \"contentType\": \"application/octet-stream\"";

    // Add security and AI/ML extensions for SPDX 3.0
    if (component.containsDebugInfo) {
        ss << ",\n      \"extension\": {\n";
        ss << "        \"@type\": \"Extension\",\n";
        ss << "        \"extensionType\": \"software\",\n";
        ss << "        \"comment\": \"Contains debug information\"\n";
        ss << "      }";
    }
    
    ss << "\n    }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Component(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"@type\": \"software_File\",\n";
    ss << "      \"spdxId\": \"" << generateSPDXElementId(component.name) << "\",\n";
    ss << "      \"name\": \"" << Utils::getFileName(component.filePath) << "\",\n";
    ss << "      \"integrityMethod\": [\n";
    ss << "        {\n";
    ss << "          \"@type\": \"Hash\",\n";
    ss << "          \"algorithm\": \"sha256\",\n";
    ss << "          \"hashValue\": \"" << (component.checksum.empty() ? "da39a3ee5e6b4b0d3255bfef95601890afd80709" : component.checksum) << "\"\n";
    ss << "        }\n";
    ss << "      ],\n";
    if (component.fileSize > 0) {
        ss << "      \"size\": " << component.fileSize << ",\n";
    }
    ss << "      \"licenseConcluded\": \"" << generateSPDXLicenseId(component.license) << "\",\n";
    ss << "      \"licenseInfoFromFiles\": [\"" << generateSPDXLicenseId(component.license) << "\"],\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"comment\": \"" << component.getFileTypeString() << " file\",\n";
    ss << "      \"description\": \"" << component.getFileTypeString() << " component from " << component.filePath << "\",\n";
    ss << "      \"fileKind\": \"";
    
    // Map file types to SPDX 3.0.1 file kinds (refined from 3.0.0)
    switch (component.fileType) {
        case FileType::Source:
            ss << "source";
            break;
        case FileType::Executable:
            ss << "binary";
            break;
        case FileType::StaticLibrary:
            ss << "archive";
            break;
        default:
            ss << "other";
            break;
    }
    ss << "\",\n";
        ss << "      \"contentType\": \"application/octet-stream\"";

    // Enhanced extensions for SPDX 3.0.1
    if (component.containsDebugInfo || !component.sourceFiles.empty()) {
        ss << ",\n      \"extension\": {\n";
        ss << "        \"@type\": \"software_Extension\",\n";
        ss << "        \"extensionType\": \"debug_information\",\n";
        ss << "        \"comment\": \"Contains debug information";
        
        if (!component.sourceFiles.empty()) {
            ss << " with source files: " << Utils::join(component.sourceFiles, ", ");
        }
        
        ss << "\"\n";
        ss << "      }";
    }
    
    ss << "\n    }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX2_3_Component(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "FileName: " << Utils::getFileName(component.filePath) << "\n";
    ss << "SPDXID: " << generateSPDXId(component.name) << "\n";
    ss << "FileChecksum: SHA256: " << (component.checksum.empty() ? "da39a3ee5e6b4b0d3255bfef95601890afd80709" : component.checksum) << "\n";
    if (component.fileSize > 0) {
        ss << "FileSize: " << component.fileSize << "\n";
    }
    ss << "LicenseConcluded: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "LicenseInfoInFile: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "FileCopyrightText: NOASSERTION\n";
    ss << "FileComment: " << component.getFileTypeString() << " file\n";
    
    // Add file type classification
    ss << "FileType: ";
    switch (component.fileType) {
        case FileType::Source:
            ss << "SOURCE";
            break;
        case FileType::Executable:
            ss << "BINARY";
            break;
        case FileType::StaticLibrary:
            ss << "ARCHIVE";
            break;
        default:
            ss << "OTHER";
            break;
    }
    ss << "\n";
    
    // Add annotations for debug information in SPDX 2.3
    if (component.containsDebugInfo) {
        ss << "Annotation: Tool: Heimdall-2.0.0\n";
        ss << "AnnotationDate: " << getCurrentTimestamp() << "\n";
        ss << "AnnotationType: OTHER\n";
        ss << "AnnotationComment: Contains debug information\n";
        
        if (!component.sourceFiles.empty()) {
            ss << "Annotation: Tool: Heimdall-2.0.0\n";
            ss << "AnnotationDate: " << getCurrentTimestamp() << "\n";
            ss << "AnnotationType: OTHER\n";
            ss << "AnnotationComment: Source files: " << Utils::join(component.sourceFiles, ", ") << "\n";
        }
    }
    
    ss << "\n";
    return ss.str();
}

// CycloneDX methods (keep existing implementation)
std::string SBOMGenerator::Impl::generateCycloneDXDocument() {
    std::stringstream ss;

    ss << "{\n";
    
    // Add $schema field only for 1.4+
    if (cyclonedxVersion >= "1.4") {
        ss << "  \"$schema\": \"http://cyclonedx.org/schema/bom-" << cyclonedxVersion << ".schema.json\",\n";
    }
    
    ss << "  \"bomFormat\": \"CycloneDX\",\n";
    ss << "  \"specVersion\": \"" << cyclonedxVersion << "\",\n";
    
    // Add serialNumber for 1.3+ (optional in 1.3/1.4, required in 1.5+)
    if (cyclonedxVersion >= "1.3") {
        std::string serialNumber = "urn:uuid:" + Utils::generateUUID();
        ss << "  \"serialNumber\": \"" << serialNumber << "\",\n";
    }
    
    ss << "  \"version\": 1,\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    
    // Tools structure varies by version
    if (cyclonedxVersion >= "1.5") {
        // CycloneDX 1.5+ uses tools.components structure
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
    } else {
        // CycloneDX 1.3 and 1.4 use simple tools array
        ss << "    \"tools\": [\n";
        ss << "      {\n";
        ss << "        \"vendor\": \"Heimdall Project\",\n";
        ss << "        \"name\": \"Heimdall SBOM Generator\",\n";
        ss << "        \"version\": \"2.0.0\"\n";
        ss << "      }\n";
        ss << "    ],\n";
    }
    
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



}  // namespace heimdall
