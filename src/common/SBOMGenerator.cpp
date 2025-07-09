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
#include "../compat/compatibility.hpp"

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
};

/**
 * @brief Default constructor
 */
SBOMGenerator::SBOMGenerator() : pImpl(heimdall::compat::make_unique<Impl>()) {
    pImpl->metadataExtractor = heimdall::compat::make_unique<MetadataExtractor>();
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
        success = pImpl->generateSPDX(pImpl->outputPath);
    } else if (pImpl->format == "spdx-3.0" || pImpl->format == "spdx-3.0.0" || pImpl->format == "spdx-3.0.1") {
        success = pImpl->generateSPDX3JSON(pImpl->outputPath);
    } else if (pImpl->format == "cyclonedx" || pImpl->format == "cyclonedx-1.4" || pImpl->format == "cyclonedx-1.6") {
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
 * @brief Set the SPDX version for the SBOM
 * @param version The SPDX version (e.g., "2.3", "3.0.0", "3.0.1")
 */
void SBOMGenerator::setSPDXVersion(const std::string& version) {
    pImpl->spdxVersion = version;
}

/**
 * @brief Set the CycloneDX version for the SBOM
 * @param version The CycloneDX version (e.g., "1.4", "1.6")
 */
void SBOMGenerator::setCycloneDXVersion(const std::string& version) {
    pImpl->cyclonedxVersion = version;
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
        
        if (component.fileType != FileType::Unknown && component.containsDebugInfo && !component.sourceFiles.empty()) {
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
    ss << "  \"spdxVersion\": \"SPDX-3.0\",\n";
    ss << "  \"dataLicense\": \"CC0-1.0\",\n";
    ss << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
    ss << "  \"name\": \"Heimdall Generated SBOM\",\n";
    ss << "  \"documentNamespace\": \"" << generateDocumentNamespace() << "\",\n";
    ss << "  \"creationInfo\": " << generateSPDX3CreationInfo() << ",\n";
    ss << "  \"packages\": [\n";
    
    bool first = true;
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!first) ss << ",\n";
        ss << generateSPDX3_0_0_Component(component);
        first = false;
    }
    
    ss << "\n  ],\n";
    ss << "  \"relationships\": [\n";
    
    first = true;
    for (const auto& pair : components) {
        const auto& component = pair.second;
        if (!first) ss << ",\n";
        ss << "    {\n";
        ss << "      \"spdxElementId\": \"" << generateSPDXElementId("Package") << "\",\n";
        ss << "      \"relatedSpdxElement\": \"" << generateSPDXElementId(component.name) << "\",\n";
        ss << "      \"relationshipType\": \"CONTAINS\"\n";
        ss << "    }";
        first = false;
    }
    
    ss << "\n  ]\n";
    ss << "}\n";
    
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Document() {
    // Similar to 3.0.0 but with 3.0.1 specific fields
    return generateSPDX3_0_0_Document(); // Simplified for now
}

std::string SBOMGenerator::Impl::generateCycloneDXDocument() {
    std::stringstream ss;

    ss << "{\n";
    ss << "  \"bomFormat\": \"CycloneDX\",\n";
    ss << "  \"specVersion\": \"" << cyclonedxVersion << "\",\n";
    ss << "  \"version\": 1,\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    ss << "    \"tools\": [\n";
    ss << "      {\n";
    ss << "        \"vendor\": \"Heimdall\",\n";
    ss << "        \"name\": \"SBOM Generator\",\n";
    ss << "        \"version\": \"2.0.0\"\n";
    ss << "      }\n";
    ss << "    ],\n";
    ss << "    \"component\": {\n";
    ss << "      \"type\": \"application\",\n";
    ss << "      \"name\": "
       << Utils::formatJsonValue(buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)
       << ",\n";
    ss << "      \"version\": "
       << Utils::formatJsonValue(buildInfo.buildId.empty() ? "Unknown" : buildInfo.buildId) << "\n";
    ss << "    }\n";
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

std::string SBOMGenerator::Impl::generateSPDX2_3_Component(const ComponentInfo& component) {
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
    ss << "LicenseConcluded: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "LicenseInfoInFile: " << generateSPDXLicenseId(component.license) << "\n";
    ss << "FileCopyrightText: NOASSERTION\n";
    ss << "FileComment: " << component.getFileTypeString() << " file\n";
    ss << "\n";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_0_Component(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"SPDXID\": \"" << generateSPDXElementId(component.name) << "\",\n";
    ss << "      \"name\": " << Utils::formatJsonValue(Utils::getFileName(component.filePath)) << ",\n";
    ss << "      \"version\": " << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version) << ",\n";
    ss << "      \"checksums\": [\n";
    ss << "        {\n";
    ss << "          \"algorithm\": \"SHA256\",\n";
    ss << "          \"checksumValue\": \"" << (component.checksum.empty() ? "UNKNOWN" : component.checksum) << "\"\n";
    ss << "        }\n";
    ss << "      ],\n";
    ss << "      \"licenseConcluded\": \"" << generateSPDXLicenseId(component.license) << "\",\n";
    ss << "      \"copyrightText\": \"NOASSERTION\",\n";
    ss << "      \"comment\": \"" << component.getFileTypeString() << " file\"\n";
    ss << "    }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Component(const ComponentInfo& component) {
    return generateSPDX3_0_0_Component(component); // Simplified for now
}

std::string SBOMGenerator::Impl::generateCycloneDXComponent(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "    {\n";
    ss << "      \"type\": \"library\",\n";
    ss << "      \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
    ss << "      \"version\": "
       << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version)
       << ",\n";
    ss << "      \"description\": "
       << Utils::formatJsonValue(component.getFileTypeString() + " component") << ",\n";
    ss << "      \"supplier\": {\n";
    ss << "        \"name\": " << Utils::formatJsonValue(component.supplier.empty() ? "system-package-manager" : component.supplier) << "\n";
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
    
    // Add debug properties if available
    if (component.containsDebugInfo) {
        ss << ",\n" << generateDebugProperties(component);
    }
    
    // Add evidence field for CycloneDX 1.6+
    if (cyclonedxVersion == "1.6") {
        ss << ",\n" << generateEvidenceField(component);
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

std::string SBOMGenerator::Impl::generateDocumentNamespace() {
    std::stringstream ss;
    ss << "https://spdx.org/spdxdocs/heimdall-" << getCurrentTimestamp();
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXElementId(const std::string& name) {
    std::string id = "SPDXRef-" + name;
    // Replace invalid characters for SPDX 3.x
    std::replace(id.begin(), id.end(), ' ', '-');
    std::replace(id.begin(), id.end(), '/', '-');
    std::replace(id.begin(), id.end(), '\\', '-');
    std::replace(id.begin(), id.end(), '.', '-');
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

std::string SBOMGenerator::Impl::generateDebugProperties(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "      \"properties\": [\n";
    ss << "        {\n";
    ss << "          \"name\": \"debug_info\",\n";
    ss << "          \"value\": \"" << (component.containsDebugInfo ? "true" : "false") << "\"\n";
    ss << "        },\n";
    ss << "        {\n";
    ss << "          \"name\": \"stripped\",\n";
    ss << "          \"value\": \"" << (component.isStripped ? "true" : "false") << "\"\n";
    ss << "        },\n";
    ss << "        {\n";
    ss << "          \"name\": \"system_library\",\n";
    ss << "          \"value\": \"" << (component.isSystemLibrary ? "true" : "false") << "\"\n";
    ss << "        }\n";
    ss << "      ]";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateEvidenceField(const ComponentInfo& component) {
    std::stringstream ss;
    ss << "      \"evidence\": {\n";
    ss << "        \"licenses\": [\n";
    ss << "          {\n";
    ss << "            \"license\": {\n";
    ss << "              \"id\": \"" << generateSPDXLicenseId(component.license) << "\"\n";
    ss << "            }\n";
    ss << "          }\n";
    ss << "        ]\n";
    ss << "      }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3CreationInfo() {
    std::stringstream ss;
    ss << "{\n";
    ss << "    \"creators\": [\n";
    ss << "      {\n";
    ss << "        \"creatorType\": \"Tool\",\n";
    ss << "        \"creator\": \"Heimdall SBOM Generator-2.0.0\"\n";
    ss << "      }\n";
    ss << "    ],\n";
    ss << "    \"created\": \"" << getCurrentTimestamp() << "\"\n";
    ss << "  }";
    return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXLicenseId(const std::string& license) {
    if (license.empty() || license == "UNKNOWN") {
        return "NOASSERTION";
    }
    
    // Basic SPDX license validation
    std::string upperLicense = Utils::toUpper(license);
    if (upperLicense.find("APACHE") != std::string::npos) {
        return "Apache-2.0";
    } else if (upperLicense.find("MIT") != std::string::npos) {
        return "MIT";
    } else if (upperLicense.find("GPL") != std::string::npos) {
        if (upperLicense.find("3") != std::string::npos) {
            return "GPL-3.0-only";
        } else {
            return "GPL-2.0-only";
        }
    } else if (upperLicense.find("LGPL") != std::string::npos) {
        if (upperLicense.find("3") != std::string::npos) {
            return "LGPL-3.0-only";
        } else {
            return "LGPL-2.1-only";
        }
    } else if (upperLicense.find("BSD") != std::string::npos) {
        return "BSD-3-Clause";
    } else {
        return "NOASSERTION";
    }
}

}  // namespace heimdall
