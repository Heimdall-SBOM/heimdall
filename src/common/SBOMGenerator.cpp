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
#include <set>
#include <sstream>
#include "../compat/compatibility.hpp"
#include "MetadataExtractor.hpp"
#include "Utils.hpp"

namespace heimdall
{

/**
 * @brief Implementation class for SBOMGenerator
 */
class SBOMGenerator::Impl
{
   public:
   std::unordered_map<std::string, ComponentInfo> components;       ///< Map of processed components
   std::string                                    outputPath;       ///< Output file path
   std::string                                    format = "spdx";  ///< Output format
   std::string cyclonedxVersion = "1.6";  ///< CycloneDX specification version
   std::string spdxVersion =
      "2.3";  ///< SPDX specification version (default to 2.3 for compatibility)
   std::unique_ptr<MetadataExtractor> metadataExtractor;  ///< Metadata extractor instance
   BuildInfo                          buildInfo;          ///< Build information
   bool transitiveDependencies = true;  ///< Whether to include transitive dependencies

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
   std::string generateAllProperties(const ComponentInfo& component);

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
    * @brief Process dependencies recursively
    * @param component The component whose dependencies to process
    * @param processedKeys Set of already processed component keys to avoid cycles
    */
   void processDependenciesRecursively(const ComponentInfo&   component,
                                       std::set<std::string>& processedKeys);
};

/**
 * @brief Default constructor
 */
SBOMGenerator::SBOMGenerator() : pImpl(heimdall::compat::make_unique<Impl>())
{
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
void SBOMGenerator::processComponent(const ComponentInfo& component)
{
   // Resolve library paths to canonical absolute paths for consistent key generation
   std::string canonicalPath = Utils::resolveLibraryPath(component.filePath);
   std::string key           = canonicalPath;  // Use canonical file path as unique key



   if (pImpl->components.find(key) == pImpl->components.end())
   {
      // New component, extract metadata
      ComponentInfo processedComponent = component;

      if (pImpl->metadataExtractor)
      {
         pImpl->metadataExtractor->extractMetadata(processedComponent);
      }

      pImpl->components[key] = processedComponent;
      Utils::debugPrint("Processed component: " + component.name);

      // Process dependencies based on transitiveDependencies setting
      if (pImpl->transitiveDependencies)
      {
         // Process dependencies recursively
         std::set<std::string> processedKeys;
         pImpl->processDependenciesRecursively(processedComponent, processedKeys);
      }
      else
      {
         // Process only direct dependencies
         for (const auto& depPath : processedComponent.dependencies)
         {
            std::string resolvedPath = depPath;

            // Handle @rpath dependencies (resolve relative to app bundle)
            if (depPath.find("@rpath/") == 0)
            {
               std::string appDir    = processedComponent.filePath;
               size_t      lastSlash = appDir.rfind('/');
               if (lastSlash != std::string::npos)
               {
                  appDir       = appDir.substr(0, lastSlash);
                  resolvedPath = appDir + "/" + depPath.substr(7);  // Remove "@rpath/"
               }
            }

            // Resolve library paths to canonical absolute paths for consistent key generation
            std::string canonicalPath = Utils::resolveLibraryPath(resolvedPath);
            std::string depKey        = canonicalPath;  // Use canonical file path as unique key

            // Skip if this dependency is already processed
            if (pImpl->components.find(depKey) != pImpl->components.end())
            {
               continue;
            }

            // Create a new ComponentInfo for the dependency
            ComponentInfo depComponent(Utils::getFileName(resolvedPath), resolvedPath);
            // Let the constructor determine the file type instead of hardcoding it
            // This allows executables to be properly detected even when processed as dependencies

            // Check if it's a system library
            if (resolvedPath.find("/usr/lib/") == 0 || resolvedPath.find("/System/Library/") == 0)
            {
               depComponent.isSystemLibrary = true;
               depComponent.packageManager  = "system";
            }

            // Preserve the checksum that was calculated in the constructor
            std::string originalChecksum = depComponent.checksum;

            // Try to extract metadata for the dependency if it exists
            if (Utils::fileExists(resolvedPath) && pImpl->metadataExtractor)
            {
               pImpl->metadataExtractor->extractMetadata(depComponent);
            }
            else
            {
               // For non-existent files (like system libraries), set basic info
               depComponent.version = "system";
#if defined(__APPLE__)
               depComponent.supplier = "Apple Inc.";
#else
               depComponent.supplier = "NOASSERTION";
#endif
            }

            // Always restore the checksum if it was lost during metadata extraction
            if (depComponent.checksum.empty() && !originalChecksum.empty())
            {
               depComponent.checksum = originalChecksum;
            }

            pImpl->components[depKey] = depComponent;
            Utils::debugPrint("Added dependency component: " + depComponent.name + " at " +
                              resolvedPath);
         }
      }
   }
   else
   {
      // Update existing component
      ComponentInfo& existing = pImpl->components[key];

      // Merge information
      for (const auto& symbol : component.symbols)
      {
         existing.addSymbol(symbol);
      }

      for (const auto& section : component.sections)
      {
         existing.addSection(section);
      }

      for (const auto& dep : component.dependencies)
      {
         existing.addDependency(dep);
      }

      for (const auto& source : component.sourceFiles)
      {
         existing.addSourceFile(source);
      }

      Utils::debugPrint("Updated component: " + component.name);
   }
}

/**
 * @brief Generate the SBOM in the specified format
 */
void SBOMGenerator::generateSBOM()
{
   if (pImpl->outputPath.empty())
   {
      Utils::errorPrint("No output path specified for SBOM generation");
      return;
   }

   Utils::debugPrint("Generating SBOM with " + std::to_string(pImpl->components.size()) +
                     " components");

   bool success = false;
   // --- BEGIN PATCH ---
   // For SPDX, select output type based on spdxVersion, not just format string
   if (pImpl->format == "spdx" || pImpl->format == "spdx-2.3" || pImpl->format == "spdx-3.0" ||
       pImpl->format == "spdx-3.0.0" || pImpl->format == "spdx-3.0.1")
   {
      // If version is 2.3, use tag-value; if 3.0/3.0.0/3.0.1, use JSON
      if (pImpl->spdxVersion == "2.3")
      {
         success = pImpl->generateSPDX(pImpl->outputPath);
      }
      else if (pImpl->spdxVersion == "3.0" || pImpl->spdxVersion == "3.0.0" ||
               pImpl->spdxVersion == "3.0.1")
      {
         success = pImpl->generateSPDX3JSON(pImpl->outputPath);
      }
      else
      {
         // Fall back to default version instead of failing
         Utils::warningPrint("Unsupported SPDX version: " + pImpl->spdxVersion +
                             ", falling back to 2.3");
         pImpl->spdxVersion = "2.3";
         success            = pImpl->generateSPDX(pImpl->outputPath);
      }
   }
   else if (pImpl->format == "cyclonedx" || pImpl->format == "cyclonedx-1.4" ||
            pImpl->format == "cyclonedx-1.6")
   {
      success = pImpl->generateCycloneDX(pImpl->outputPath);
   }
   else
   {
      // Fall back to default format instead of failing
      Utils::warningPrint("Unsupported SBOM format: " + pImpl->format + ", falling back to spdx");
      pImpl->format      = "spdx";
      pImpl->spdxVersion = "2.3";
      success            = pImpl->generateSPDX(pImpl->outputPath);
   }
   // --- END PATCH ---

   if (success)
   {
      Utils::debugPrint("SBOM generated successfully: " + pImpl->outputPath);
   }
   else
   {
      Utils::errorPrint("Failed to generate SBOM");
   }
}

/**
 * @brief Set the output path for the SBOM
 * @param path The output file path
 */
void SBOMGenerator::setOutputPath(const std::string& path)
{
   pImpl->outputPath = path;
}

/**
 * @brief Set the output format for the SBOM
 * @param fmt The format (e.g., "spdx", "cyclonedx")
 */
void SBOMGenerator::setFormat(const std::string& fmt)
{
   pImpl->format = Utils::toLower(fmt);
}

/**
 * @brief Set the SPDX version for the SBOM
 * @param version The SPDX version (e.g., "2.3", "3.0.0", "3.0.1")
 */
void SBOMGenerator::setSPDXVersion(const std::string& version)
{
   pImpl->spdxVersion = version;
}

/**
 * @brief Set the CycloneDX version for the SBOM
 * @param version The CycloneDX version (e.g., "1.4", "1.6")
 */
void SBOMGenerator::setCycloneDXVersion(const std::string& version)
{
   pImpl->cyclonedxVersion = version;
}

/**
 * @brief Set suppress warnings flag for metadata extraction
 * @param suppress True to suppress warnings, false to show them
 */
void SBOMGenerator::setSuppressWarnings(bool suppress)
{
   if (pImpl && pImpl->metadataExtractor)
   {
      pImpl->metadataExtractor->setSuppressWarnings(suppress);
   }
}

/**
 * @brief Set whether to include transitive dependencies in the SBOM
 * @param include True to include, false to exclude
 */
void SBOMGenerator::setTransitiveDependencies(bool include)
{
   pImpl->transitiveDependencies = include;
}

/**
 * @brief Get the number of components in the SBOM
 * @return Number of components
 */
size_t SBOMGenerator::getComponentCount() const
{
   return pImpl->components.size();
}

/**
 * @brief Check if a component exists in the SBOM
 * @param name The component name to check
 * @return true if the component exists
 */
bool SBOMGenerator::hasComponent(const std::string& name) const
{
   for (const auto& pair : pImpl->components)
   {
      if (pair.second.name == name)
      {
         return true;
      }
   }
   return false;
}

/**
 * @brief Print statistics about the SBOM
 */
void SBOMGenerator::printStatistics() const
{
   std::cout << "SBOM Generator Statistics:" << std::endl;
   std::cout << "  Total components: " << pImpl->components.size() << std::endl;

   size_t objects = 0, staticLibs = 0, sharedLibs = 0, executables = 0;
   size_t systemLibs = 0, withDebugInfo = 0, stripped = 0;

   for (const auto& pair : pImpl->components)
   {
      const auto& component = pair.second;

      switch (component.fileType)
      {
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
bool SBOMGenerator::Impl::generateSPDX(const std::string& outputPath)
{
   std::string document;

   if (spdxVersion == "3.0.1")
   {
      document = generateSPDX3_0_1_Document();
   }
   else if (spdxVersion == "3.0.0" || spdxVersion == "3.0")
   {
      document = generateSPDX3_0_0_Document();
   }
   else
   {
      document = generateSPDX2_3_Document();
   }

   std::ofstream file(outputPath);
   if (!file.is_open())
   {
      Utils::errorPrint("Could not open output file: " + outputPath);
      return false;
   }

   file << document;
   return true;
}

bool SBOMGenerator::Impl::generateSPDX3JSON(const std::string& outputPath)
{
   std::string document;

   if (spdxVersion == "3.0.1")
   {
      document = generateSPDX3_0_1_Document();
   }
   else
   {
      document = generateSPDX3_0_0_Document();
   }

   std::ofstream file(outputPath);
   if (!file.is_open())
   {
      Utils::errorPrint("Could not open output file: " + outputPath);
      return false;
   }

   file << document;
   return true;
}

bool SBOMGenerator::Impl::generateCycloneDX(const std::string& outputPath)
{
   std::string   document = generateCycloneDXDocument();

   std::ofstream file(outputPath);
   if (!file.is_open())
   {
      Utils::errorPrint("Could not open output file: " + outputPath);
      return false;
   }

   file << document;
   return true;
}

std::string SBOMGenerator::Impl::generateSPDXDocument()
{
   std::stringstream ss;
   // DEBUG: Print all keys and file paths in the components map
   ss << "# DEBUG: Components map keys and file paths\n";
   for (const auto& pair : components)
   {
      ss << "# key: '" << pair.first << "' filePath: '" << pair.second.filePath << "' name: '"
         << pair.second.name << "'\n";
   }
   ss << "\n";
   if (spdxVersion == "3.0.1")
   {
      return generateSPDX3_0_1_Document();
   }
   else if (spdxVersion == "3.0.0" || spdxVersion == "3.0")
   {
      return generateSPDX3_0_0_Document();
   }
   else
   {
      return generateSPDX2_3_Document();
   }
}

// SPDX 2.3 Tag-Value Document Generation (fully schema-compliant)
std::string SBOMGenerator::Impl::generateSPDX2_3_Document()
{
   std::stringstream ss;
   // Document header
   ss << "SPDXVersion: SPDX-2.3\n";
   ss << "DataLicense: CC0-1.0\n";
   ss << "SPDXID: SPDXRef-DOCUMENT\n";
   ss << "DocumentName: "
      << (buildInfo.targetName.empty() ? "Heimdall Generated SBOM" : buildInfo.targetName) << "\n";
   ss << "DocumentNamespace: " << generateDocumentNamespace() << "\n";
   ss << "Creator: Tool: Heimdall SBOM Generator-2.0.0\n";
   ss << "Created: " << getCurrentTimestamp() << "\n\n";
   // Package section (required)
   ss << "PackageName: " << (buildInfo.targetName.empty() ? "Unknown" : buildInfo.targetName)
      << "\n";
   ss << "SPDXID: SPDXRef-Package\n";
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
   ss << "PackageDescription: Software Bill of Materials generated by Heimdall\n\n";
   // File section for each component
   for (const auto& pair : components)
   {
      const auto& component    = pair.second;
      std::string sha1Checksum = Utils::getFileSHA1Checksum(component.filePath);

      ss << "FileName: " << Utils::getFileName(component.filePath) << "\n";
      ss << "SPDXID: " << generateSPDXId(component.name) << "\n";
      ss << "FileType: " << component.getFileTypeString("2.3") << "\n";
      ss << "FileChecksum: SHA1: " << (sha1Checksum.empty() ? "UNKNOWN" : sha1Checksum) << "\n";
      ss << "FileChecksum: SHA256: "
         << (component.checksum.empty() ? "UNKNOWN" : component.checksum) << "\n";
      ss << "LicenseConcluded: " << generateSPDXLicenseId(component.license) << "\n";
      ss << "LicenseInfoInFile: " << generateSPDXLicenseId(component.license) << "\n";
      ss << "FileCopyrightText: NOASSERTION\n";
      // Build enhanced comment with source files and Ada properties
      std::stringstream comment;
      if (!component.sourceFiles.empty())
      {
         comment << "Source files: ";
         for (size_t i = 0; i < component.sourceFiles.size(); ++i)
         {
            comment << component.sourceFiles[i];
            if (i + 1 < component.sourceFiles.size())
               comment << ", ";
         }
      }

      // Add enhanced Ada properties to comment
      if (!component.properties.empty())
      {
         if (!comment.str().empty())
            comment << "; ";
         comment << "Enhanced metadata: ";
         bool first = true;
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
         for (const auto& [key, value] : component.properties)
         {
#else
         for (const auto& property : component.properties)
         {
            const auto& key   = property.first;
            const auto& value = property.second;
#endif
            if (!first)
               comment << ", ";
            comment << key << "=" << value;
            first = false;
         }
      }

      if (!comment.str().empty())
      {
         ss << "FileComment: " << comment.str() << "\n";
      }
      else
      {
         ss << "FileComment: " << component.getFileTypeString("2.3") << " file\n";
      }
      ss << "\n";
   }
   // Relationships
   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      ss << "Relationship: SPDXRef-Package CONTAINS " << generateSPDXId(component.name) << "\n";

      // Add DEPENDS_ON relationships for all dependencies
      for (const auto& dep : component.dependencies)
      {
         // Find the component that matches this dependency
         // Resolve library paths to canonical absolute paths for consistent key generation
         std::string canonicalPath = Utils::resolveLibraryPath(dep);
         std::string depKey        = canonicalPath;  // Use canonical file path as unique key
         if (components.find(depKey) != components.end())
         {
            ss << "Relationship: " << generateSPDXId(component.name) << " DEPENDS_ON "
               << generateSPDXId(Utils::getFileName(dep)) << "\n";
         }
      }
   }
   // Note: Source file relationships removed to avoid validation errors
   // Source files are referenced in FileComment instead
   return ss.str();
}
// SPDX 3.0.x JSON Document Generation (fully schema-compliant)
std::string SBOMGenerator::Impl::generateSPDX3_0_0_Document()
{
   std::stringstream ss;
   ss << "{\n";
   ss << "  \"@context\": \"https://spdx.org/rdf/3.0.0/spdx-context.jsonld\",\n";
   ss << "  \"@graph\": [\n";
   // SBOM Document
   ss << "    {\n";
   ss << "      \"spdxId\": \"spdx:SPDXRef-DOCUMENT\",\n";
   ss << "      \"type\": \"SpdxDocument\",\n";
   ss << "      \"specVersion\": \"SPDX-3.0.0\",\n";
   ss << "      \"name\": "
      << Utils::formatJsonValue(buildInfo.targetName.empty() ? "Heimdall Generated SBOM"
                                                             : buildInfo.targetName)
      << ",\n";
   ss << "      \"documentNamespace\": " << Utils::formatJsonValue(generateDocumentNamespace())
      << ",\n";
   ss << "      \"creationInfo\": {\n";
   ss << "        \"spdxId\": \"spdx:CreationInfo-1\",\n";
   ss << "        \"type\": \"CreationInfo\",\n";
   ss << "        \"created\": " << Utils::formatJsonValue(getCurrentTimestamp()) << ",\n";
   ss << "        \"createdBy\": [\n";
   ss << "          {\n";
   ss << "            \"type\": \"Tool\",\n";
   ss << "            \"name\": " << Utils::formatJsonValue("Heimdall SBOM Generator-2.0.0")
      << "\n";
   ss << "          }\n";
   ss << "        ]\n";
   ss << "      },\n";
   ss << "      \"dataLicense\": " << Utils::formatJsonValue("CC0-1.0") << ",\n";
   ss << "      \"files\": [\n";
   // Files
   bool first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"@id\": \"spdx:" << generateSPDXElementId(component.name) << "\",\n";
      ss << "          \"type\": \"software_File\",\n";
      ss << "          \"fileName\": " << Utils::formatJsonValue(component.filePath) << ",\n";
      ss << "          \"checksums\": [\n";
      ss << "            {\n";
      ss << "              \"type\": \"Checksum\",\n";
      ss << "              \"algorithm\": \"SHA256\",\n";
      ss << "              \"checksumValue\": "
         << Utils::formatJsonValue(component.checksum.empty() ? "NOASSERTION" : component.checksum)
         << "\n";
      ss << "            }\n";
      ss << "          ]\n";
      ss << "        }";
   }
   ss << "\n      ],\n";
   ss << "      \"packages\": [\n";
   // Packages (if any)
   first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"@id\": \"spdx:" << generateSPDXElementId(component.name) << "\",\n";
      ss << "          \"type\": \"software_Package\",\n";
      ss << "          \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
      ss << "          \"versionInfo\": "
         << Utils::formatJsonValue(component.version.empty() ? "NOASSERTION" : component.version)
         << "\n";
      ss << "        }";
   }
   ss << "\n      ],\n";
   ss << "      \"relationships\": [\n";
   // Relationships (example: document CONTAINS files)
   first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"type\": \"Relationship\",\n";
      ss << "          \"relationshipType\": \"CONTAINS\",\n";
      ss << "          \"relatedSpdxElement\": \"spdx:" << generateSPDXElementId(component.name)
         << "\"\n";
      ss << "        }";
   }

   // Add DEPENDS_ON relationships for all dependencies
   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      for (const auto& dep : component.dependencies)
      {
         // Find the component that matches this dependency
         std::string depKey = Utils::getFileName(dep) + ":" + dep;
         if (components.find(depKey) != components.end())
         {
            ss << ",\n        {\n";
            ss << "          \"type\": \"Relationship\",\n";
            ss << "          \"relationshipType\": \"DEPENDS_ON\",\n";
            ss << "          \"spdxElementId\": \"spdx:" << generateSPDXElementId(component.name)
               << "\",\n";
            ss << "          \"relatedSpdxElement\": \"spdx:"
               << generateSPDXElementId(Utils::getFileName(dep)) << "\"\n";
            ss << "        }";
         }
      }
   }
   ss << "\n      ]\n";
   ss << "    }\n";
   ss << "  ]\n";
   ss << "}\n";
   return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Document()
{
   std::stringstream ss;
   ss << "{\n";
   ss << "  \"@context\": \"https://spdx.org/rdf/3.0.1/spdx-context.jsonld\",\n";
   ss << "  \"@graph\": [\n";
   // SBOM Document
   ss << "    {\n";
   ss << "      \"spdxId\": \"spdx:SPDXRef-DOCUMENT\",\n";
   ss << "      \"type\": \"SpdxDocument\",\n";
   ss << "      \"spdxVersion\": \"SPDX-3.0.1\",\n";
   ss << "      \"name\": "
      << Utils::formatJsonValue(buildInfo.targetName.empty() ? "Heimdall Generated SBOM"
                                                             : buildInfo.targetName)
      << ",\n";
   ss << "      \"documentNamespace\": " << Utils::formatJsonValue(generateDocumentNamespace())
      << ",\n";
   ss << "      \"creationInfo\": {\n";
   ss << "        \"spdxId\": \"spdx:CreationInfo-1\",\n";
   ss << "        \"type\": \"CreationInfo\",\n";
   ss << "        \"created\": " << Utils::formatJsonValue(getCurrentTimestamp()) << ",\n";
   ss << "        \"createdBy\": [\n";
   ss << "          {\n";
   ss << "            \"type\": \"Tool\",\n";
   ss << "            \"name\": " << Utils::formatJsonValue("Heimdall SBOM Generator-2.0.0")
      << "\n";
   ss << "          }\n";
   ss << "        ]\n";
   ss << "      },\n";
   ss << "      \"dataLicense\": " << Utils::formatJsonValue("CC0-1.0") << ",\n";
   ss << "      \"files\": [\n";
   // Files
   bool first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"@id\": \"spdx:" << generateSPDXElementId(component.name) << "\",\n";
      ss << "          \"type\": \"File\",\n";
      ss << "          \"fileName\": " << Utils::formatJsonValue(component.filePath) << ",\n";
      ss << "          \"checksums\": [\n";
      ss << "            {\n";
      ss << "              \"type\": \"Checksum\",\n";
      ss << "              \"algorithm\": \"SHA256\",\n";
      ss << "              \"checksumValue\": "
         << Utils::formatJsonValue(component.checksum.empty() ? "NOASSERTION" : component.checksum)
         << "\n";
      ss << "            }\n";
      ss << "          ]\n";
      ss << "        }";
   }
   ss << "\n      ],\n";
   ss << "      \"packages\": [\n";
   // Packages (if any)
   first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"@id\": \"spdx:" << generateSPDXElementId(component.name) << "\",\n";
      ss << "          \"type\": \"Package\",\n";
      ss << "          \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
      ss << "          \"versionInfo\": "
         << Utils::formatJsonValue(component.version.empty() ? "NOASSERTION" : component.version)
         << "\n";
      ss << "        }";
   }
   ss << "\n      ],\n";
   ss << "      \"relationships\": [\n";
   // Relationships (example: document CONTAINS files)
   first = true;
   for (const auto& pair : components)
   {
      if (!first)
         ss << ",\n";
      first                 = false;
      const auto& component = pair.second;
      ss << "        {\n";
      ss << "          \"type\": \"Relationship\",\n";
      ss << "          \"relationshipType\": \"CONTAINS\",\n";
      ss << "          \"relatedSpdxElement\": \"spdx:" << generateSPDXElementId(component.name)
         << "\"\n";
      ss << "        }";
   }

   // Add DEPENDS_ON relationships for all dependencies
   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      for (const auto& dep : component.dependencies)
      {
         // Find the component that matches this dependency
         std::string depKey = Utils::getFileName(dep) + ":" + dep;
         if (components.find(depKey) != components.end())
         {
            ss << ",\n        {\n";
            ss << "          \"type\": \"Relationship\",\n";
            ss << "          \"relationshipType\": \"DEPENDS_ON\",\n";
            ss << "          \"spdxElementId\": \"spdx:" << generateSPDXElementId(component.name)
               << "\",\n";
            ss << "          \"relatedSpdxElement\": \"spdx:"
               << generateSPDXElementId(Utils::getFileName(dep)) << "\"\n";
            ss << "        }";
         }
      }
   }
   ss << "\n      ]\n";
   ss << "    }\n";
   ss << "  ]\n";
   ss << "}\n";
   return ss.str();
}

std::string SBOMGenerator::Impl::generateCycloneDXDocument()
{
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
   ss << "      \"name\": ";

   // Try to get a better application name
   std::string appName    = buildInfo.targetName;
   std::string appVersion = buildInfo.buildId;
   


   if (appName.empty() && !components.empty())
   {
      // Find the main executable - prioritize executables, then any component with a good name
      for (const auto& pair : components)
      {
         const auto& component = pair.second;
         if (component.fileType == FileType::Executable)
         {
            appName = component.name;
            if (!component.version.empty())
            {
               appVersion = component.version;
            }
            break;
         }
      }
      
      // If no executable found, look for app bundle executables (macOS)
      if (appName.empty())
      {
         for (const auto& pair : components)
         {
            const auto& component = pair.second;
            if (component.filePath.find(".app/Contents/MacOS/") != std::string::npos &&
                (component.fileType == FileType::Executable ||
                 component.fileType == FileType::Unknown))
            {
               appName = component.name;
               if (!component.version.empty())
               {
                  appVersion = component.version;
               }
               break;
            }
         }
      }
      
      // If still no executable found, use the first component with a non-empty name
      if (appName.empty())
      {
         for (const auto& pair : components)
         {
            const auto& component = pair.second;
            if (!component.name.empty() && component.name != "Unknown")
            {
               appName = component.name;
               if (!component.version.empty())
               {
                  appVersion = component.version;
               }
               break;
            }
         }
      }
   }

   ss << Utils::formatJsonValue(appName.empty() ? "Unknown" : appName) << ",\n";
   ss << "      \"version\": "
      << Utils::formatJsonValue(appVersion.empty() ? "Unknown" : appVersion) << "\n";
   ss << "    }\n";
   ss << "  },\n";
   ss << "  \"components\": [\n";

   bool first = true;

   // Get the main application info for filtering
   std::string mainAppName = buildInfo.targetName;
   std::string mainAppPath;

   if (mainAppName.empty() && !components.empty())
   {
      // Find the main executable - prioritize executables, then any component with a good name
      for (const auto& pair : components)
      {
         const auto& component = pair.second;
         if (component.fileType == FileType::Executable)
         {
            mainAppName = component.name;
            mainAppPath = component.filePath;
            break;
         }
      }
      
      // If no executable found, look for app bundle executables (macOS)
      if (mainAppName.empty())
      {
         for (const auto& pair : components)
         {
            const auto& component = pair.second;
            if (component.filePath.find(".app/Contents/MacOS/") != std::string::npos &&
                (component.fileType == FileType::Executable ||
                 component.fileType == FileType::Unknown))
            {
               mainAppName = component.name;
               mainAppPath = component.filePath;
               break;
            }
         }
      }
      
      // If still no executable found, use the first component with a non-empty name
      if (mainAppName.empty())
      {
         for (const auto& pair : components)
         {
            const auto& component = pair.second;
            if (!component.name.empty() && component.name != "Unknown")
            {
               mainAppName = component.name;
               mainAppPath = component.filePath;
               break;
            }
         }
      }
   }

   for (const auto& pair : components)
   {
      const auto& component = pair.second;

      if (!first)
         ss << ",\n";
      ss << generateCycloneDXComponent(component);
      first = false;
   }

   ss << "\n  ]";

   // Add dependencies section if any component has dependencies
   bool hasDependencies = false;
   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      if (!component.dependencies.empty())
      {
         hasDependencies = true;
         break;
      }
   }

   if (hasDependencies)
   {
      ss << ",\n  \"dependencies\": [\n";
      bool firstDep = true;
      for (const auto& pair : components)
      {
         const auto& component = pair.second;
         if (!component.dependencies.empty())
         {
            if (!firstDep)
            {
               ss << ",\n";
            }
            
            // Generate BOM reference for this component
            std::string bomRef = component.name;
            if (!component.version.empty() && component.version != "UNKNOWN")
            {
               bomRef += "-" + component.version;
            }
            
            ss << "    {\n";
            ss << "      \"ref\": \"" << bomRef << "\",\n";
            ss << "      \"dependsOn\": [\n";
            
            for (size_t j = 0; j < component.dependencies.size(); ++j)
            {
               const auto& dep = component.dependencies[j];
               
               // Try to find the dependency component to get its BOM reference
               std::string depBomRef = dep;  // Default to dependency path
               for (const auto& depPair : components)
               {
                  const auto& depComponent = depPair.second;
                  if (depComponent.filePath == dep || Utils::getFileName(depComponent.filePath) == Utils::getFileName(dep))
                  {
                     depBomRef = depComponent.name;
                     if (!depComponent.version.empty() && depComponent.version != "UNKNOWN")
                     {
                        depBomRef += "-" + depComponent.version;
                     }
                     break;
                  }
               }
               
               ss << "        \"" << depBomRef << "\"";
               if (j < component.dependencies.size() - 1)
               {
                  ss << ",";
               }
               ss << "\n";
            }
            ss << "      ]\n";
            ss << "    }";
            firstDep = false;
         }
      }
      ss << "\n  ]";
   }

   ss << "\n}\n";

   return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX2_3_Component(const ComponentInfo& component)
{
   std::stringstream ss;
   ss << "FileName: " << Utils::getFileName(component.filePath) << "\n";
   ss << "SPDXID: " << generateDocumentNamespace() << "#" << generateSPDXId(component.name) << "\n";
   ss << "FileChecksum: SHA256: " << (component.checksum.empty() ? "UNKNOWN" : component.checksum)
      << "\n";
   ss << "Supplier: " << (component.supplier.empty() ? "Organization: UNKNOWN" : component.supplier)
      << "\n";
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

std::string SBOMGenerator::Impl::generateSPDX3_0_0_Component(const ComponentInfo& component)
{
   std::stringstream ss;
   ss << "    {\n";
   ss << "      \"SPDXID\": "
      << Utils::formatJsonValue(std::string(generateSPDXElementId(component.name))) << ",\n";
   ss << "      \"name\": "
      << Utils::formatJsonValue(std::string(Utils::getFileName(component.filePath))) << ",\n";
   ss << "      \"versionInfo\": "
      << Utils::formatJsonValue(component.version.empty() ? std::string("NOASSERTION")
                                                          : component.version)
      << ",\n";
   ss << "      \"checksums\": [\n";
   ss << "        {\n";
   ss << "          \"algorithm\": \"SHA256\",\n";
   ss << "          \"checksumValue\": "
      << Utils::formatJsonValue(component.checksum.empty() ? std::string("NOASSERTION")
                                                           : component.checksum)
      << "\n";
   ss << "        }\n";
   ss << "      ],\n";
   ss << "      \"licenseConcluded\": "
      << Utils::formatJsonValue(component.license.empty()
                                   ? std::string("NOASSERTION")
                                   : generateSPDXLicenseId(component.license))
      << ",\n";
   ss << "      \"licenseDeclared\": " << Utils::formatJsonValue(std::string("NOASSERTION"))
      << ",\n";
   ss << "      \"copyrightText\": " << Utils::formatJsonValue(std::string("NOASSERTION")) << ",\n";
   ss << "      \"downloadLocation\": " << Utils::formatJsonValue(std::string("NOASSERTION"))
      << ",\n";
   ss << "      \"supplier\": " << Utils::formatJsonValue(std::string("NOASSERTION")) << ",\n";
   ss << "      \"description\": " << Utils::formatJsonValue(std::string("NOASSERTION")) << ",\n";
   ss << "      \"filesAnalyzed\": true,\n";
   ss << "      \"externalRefs\": [\n";
   ss << "        {\n";
   ss << "          \"referenceCategory\": \"PACKAGE-MANAGER\",\n";
   ss << "          \"referenceType\": \"purl\",\n";
   ss << "          \"referenceLocator\": "
      << Utils::formatJsonValue(generatePURL(component).empty() ? std::string("NOASSERTION")
                                                                : generatePURL(component))
      << "\n";
   ss << "        }\n";
   ss << "      ]";
   // Remove sourceFiles field - not part of SPDX 3.0 spec
   ss << "\n    }";
   return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDX3_0_1_Component(const ComponentInfo& component)
{
   return generateSPDX3_0_0_Component(component);  // Simplified for now
}

std::string SBOMGenerator::Impl::generateCycloneDXComponent(const ComponentInfo& component)
{
   std::stringstream ss;
   ss << "    {\n";
   
   // Generate BOM reference based on component name and version
   std::string bomRef = component.name;
   if (!component.version.empty() && component.version != "UNKNOWN")
   {
      bomRef += "-" + component.version;
   }
   
   ss << "      \"bom-ref\": \"" << bomRef << "\",\n";
   
   // Determine the correct component type based on file type
   std::string componentType = "library";
   if (component.fileType == FileType::Executable)
   {
      componentType = "application";
   }
   else if (component.fileType == FileType::StaticLibrary)
   {
      componentType = "library";
   }
   else if (component.fileType == FileType::SharedLibrary)
   {
      componentType = "library";
   }
   else if (component.fileType == FileType::Object)
   {
      componentType = "library";
   }
   
   ss << "      \"type\": \"" << componentType << "\",\n";
   ss << "      \"name\": " << Utils::formatJsonValue(component.name) << ",\n";
   ss << "      \"version\": "
      << Utils::formatJsonValue(component.version.empty() ? "UNKNOWN" : component.version) << ",\n";
   ss << "      \"description\": "
      << Utils::formatJsonValue(component.getFileTypeString() + " component") << ",\n";
   ss << "      \"supplier\": {\n";
   ss << "        \"name\": "
      << Utils::formatJsonValue(component.supplier.empty() ? "system-package-manager"
                                                           : component.supplier)
      << "\n";
   ss << "      },\n";
   // Only include hash if we have a valid checksum
   if (!component.checksum.empty() && component.checksum.length() == 64)
   {
      ss << "      \"hashes\": [\n";
      ss << "        {\n";
      ss << "          \"alg\": \"SHA-256\",\n";
      ss << "          \"content\": \"" << component.checksum << "\"\n";
      ss << "        }\n";
      ss << "      ],\n";
   }
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

   // Add all component properties (including enhanced Ada metadata and Mach-O metadata)
   if (!component.properties.empty() || component.containsDebugInfo ||
       !component.buildConfig.targetPlatform.empty() ||
       !component.platformInfo.architecture.empty() || component.codeSignInfo.isHardenedRuntime ||
       component.codeSignInfo.isAdHocSigned || !component.codeSignInfo.signer.empty() ||
       !component.codeSignInfo.teamId.empty() || !component.codeSignInfo.certificateHash.empty() ||
       !component.architectures.empty() || !component.entitlements.empty() ||
       !component.frameworks.empty())
   {
      ss << ",\n" << generateAllProperties(component);
   }

   // Add evidence field for CycloneDX 1.6+
   if (cyclonedxVersion == "1.6")
   {
      ss << ",\n" << generateEvidenceField(component);
   }

   // Add DWARF/source info for all CycloneDX versions
   if (!component.sourceFiles.empty())
   {
      ss << ",\n      \"sourceFiles\": [";
      for (size_t i = 0; i < component.sourceFiles.size(); ++i)
      {
         ss << Utils::formatJsonValue(component.sourceFiles[i]);
         if (i + 1 < component.sourceFiles.size())
            ss << ", ";
      }
      ss << "]";
   }
   if (!component.functions.empty())
   {
      ss << ",\n      \"functions\": [";
      for (size_t i = 0; i < component.functions.size(); ++i)
      {
         ss << Utils::formatJsonValue(component.functions[i]);
         if (i + 1 < component.functions.size())
            ss << ", ";
      }
      ss << "]";
   }
   if (!component.compileUnits.empty())
   {
      ss << ",\n      \"compileUnits\": [";
      for (size_t i = 0; i < component.compileUnits.size(); ++i)
      {
         ss << Utils::formatJsonValue(component.compileUnits[i]);
         if (i + 1 < component.compileUnits.size())
            ss << ", ";
      }
      ss << "]";
   }

   ss << "\n    }";
   return ss.str();
}

std::string SBOMGenerator::Impl::getCurrentTimestamp()
{
   auto              now    = std::chrono::system_clock::now();
   auto              time_t = std::chrono::system_clock::to_time_t(now);

   std::stringstream ss;
#if defined(_POSIX_VERSION)
   struct tm tm_buf{};
   gmtime_r(&time_t, &tm_buf);
   ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
#else
   // Fallback: not thread-safe
   ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
#endif
   return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXId(const std::string& name)
{
   std::string id = "SPDXRef-" + name;
   // Replace invalid characters for SPDX 2.3 (no dots, underscores allowed)
   std::replace(id.begin(), id.end(), ' ', '-');
   std::replace(id.begin(), id.end(), '/', '-');
   std::replace(id.begin(), id.end(), '\\', '-');
   std::replace(id.begin(), id.end(), '.', '-');
   std::replace(id.begin(), id.end(), '_', '-');

   // Handle consecutive plus signs (replace ++ with single +)
   size_t pos = 0;
   while ((pos = id.find("++", pos)) != std::string::npos)
   {
      id.replace(pos, 2, "+");
      pos += 1;
   }

   // Handle +- sequences (replace +- with -)
   pos = 0;
   while ((pos = id.find("+-", pos)) != std::string::npos)
   {
      id.replace(pos, 2, "-");
      pos += 1;
   }

   return id;
}

std::string SBOMGenerator::Impl::generateDocumentNamespace()
{
   std::stringstream ss;
   ss << "https://spdx.org/spdxdocs/heimdall-" << getCurrentTimestamp();
   return ss.str();
}

std::string SBOMGenerator::Impl::generateSPDXElementId(const std::string& name)
{
   std::string id = "SPDXRef-" + name;
   // Replace invalid characters for SPDX 3.x
   std::replace(id.begin(), id.end(), ' ', '-');
   std::replace(id.begin(), id.end(), '/', '-');
   std::replace(id.begin(), id.end(), '\\', '-');
   std::replace(id.begin(), id.end(), '.', '-');
   return id;
}

std::string SBOMGenerator::Impl::generateVerificationCode()
{
   // Generate a proper SPDX 2.3 verification code
   // Format: <checksum> (excludes: <file1>, <file2>, ...)
   std::string              allChecksums;
   std::vector<std::string> excludedFiles;

   for (const auto& pair : components)
   {
      const auto& component = pair.second;
      if (!component.checksum.empty() && component.checksum != "UNKNOWN")
      {
         allChecksums += component.checksum;
      }
      else
      {
         excludedFiles.push_back(Utils::getFileName(component.filePath));
      }
   }

   // Generate SHA1 hash of all checksums
   std::string hash = Utils::getStringSHA1Checksum(allChecksums);

   std::string result = hash;
   if (!excludedFiles.empty())
   {
      result += " (excludes: ";
      for (size_t i = 0; i < excludedFiles.size(); ++i)
      {
         result += excludedFiles[i];
         if (i + 1 < excludedFiles.size())
            result += ", ";
      }
      result += ")";
   }

   return result;
}

std::string SBOMGenerator::Impl::generatePURL(const ComponentInfo& component)
{
   std::stringstream ss;

   if (component.packageManager == "conan")
   {
      ss << "pkg:conan/" << component.name << "@" << component.version;
   }
   else if (component.packageManager == "vcpkg")
   {
      ss << "pkg:vcpkg/" << component.name << "@" << component.version;
   }
   else if (component.packageManager == "system")
   {
      ss << "pkg:system/" << component.name << "@" << component.version;
   }
   else
   {
      ss << "pkg:generic/" << component.name << "@" << component.version;
   }

   return ss.str();
}

std::string SBOMGenerator::Impl::generateDebugProperties(const ComponentInfo& component)
{
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

std::string SBOMGenerator::Impl::generateAllProperties(const ComponentInfo& component)
{
   std::stringstream ss;
   ss << "      \"properties\": [\n";

   // Add enhanced Ada properties first
   bool firstProperty = true;
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || \
   defined(HEIMDALL_CPP23_AVAILABLE)
   for (const auto& [key, value] : component.properties)
   {
#else
   for (const auto& property : component.properties)
   {
      const auto& key   = property.first;
      const auto& value = property.second;
#endif
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": " << Utils::formatJsonValue(key) << ",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(value) << "\n";
      ss << "        }";
      firstProperty = false;
   }

   // Add debug properties if available
   if (component.containsDebugInfo)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
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
      ss << "        }";
      firstProperty = false;
   }

   // Add enhanced Mach-O metadata if available
   if (!component.buildConfig.targetPlatform.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_target_platform\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.buildConfig.targetPlatform)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.buildConfig.minOSVersion.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_min_os_version\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.buildConfig.minOSVersion)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.buildConfig.sdkVersion.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_sdk_version\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.buildConfig.sdkVersion)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.buildConfig.buildVersion.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_build_version\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.buildConfig.buildVersion)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.buildConfig.sourceVersion.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_source_version\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.buildConfig.sourceVersion)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.buildConfig.isSimulator)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_is_simulator\",\n";
      ss << "          \"value\": \"true\"\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.platformInfo.architecture.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_architecture\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.platformInfo.architecture)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.platformInfo.platform.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_platform\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.platformInfo.platform)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.platformInfo.minVersion > 0)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_platform_min_version\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(std::to_string(component.platformInfo.minVersion)) << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.platformInfo.sdkVersion > 0)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_platform_sdk_version\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(std::to_string(component.platformInfo.sdkVersion)) << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.platformInfo.isSimulator)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_platform_is_simulator\",\n";
      ss << "          \"value\": \"true\"\n";
      ss << "        }";
      firstProperty = false;
   }

   // Add code signing information
   if (!component.codeSignInfo.signer.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_code_signer\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.codeSignInfo.signer)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.codeSignInfo.teamId.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_team_id\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.codeSignInfo.teamId)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.codeSignInfo.certificateHash.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_certificate_hash\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(component.codeSignInfo.certificateHash) << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.codeSignInfo.signingTime.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_signing_time\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(component.codeSignInfo.signingTime)
         << "\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.codeSignInfo.isAdHocSigned)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_ad_hoc_signed\",\n";
      ss << "          \"value\": \"true\"\n";
      ss << "        }";
      firstProperty = false;
   }

   if (component.codeSignInfo.isHardenedRuntime)
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_hardened_runtime\",\n";
      ss << "          \"value\": \"true\"\n";
      ss << "        }";
      firstProperty = false;
   }

   if (!component.architectures.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_architectures\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(std::to_string(component.architectures.size())) << "\n";
      ss << "        }";
      firstProperty = false;
   }

   // Add entitlements information
   if (!component.entitlements.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_entitlements_count\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(std::to_string(component.entitlements.size())) << "\n";
      ss << "        }";
      firstProperty = false;

      // Add individual entitlements as comma-separated string
      if (!component.entitlements.empty())
      {
         std::stringstream entitlementsStr;
         for (size_t i = 0; i < component.entitlements.size(); ++i)
         {
            entitlementsStr << component.entitlements[i];
            if (i + 1 < component.entitlements.size())
               entitlementsStr << ", ";
         }
         ss << ",\n";
         ss << "        {\n";
         ss << "          \"name\": \"macho_entitlements\",\n";
         ss << "          \"value\": " << Utils::formatJsonValue(entitlementsStr.str()) << "\n";
         ss << "        }";
      }
   }

   // Add frameworks information
   if (!component.frameworks.empty())
   {
      if (!firstProperty)
      {
         ss << ",\n";
      }
      ss << "        {\n";
      ss << "          \"name\": \"macho_frameworks_count\",\n";
      ss << "          \"value\": "
         << Utils::formatJsonValue(std::to_string(component.frameworks.size())) << "\n";
      ss << "        }";
      firstProperty = false;

      // Add individual frameworks as comma-separated string
      std::stringstream frameworksStr;
      for (size_t i = 0; i < component.frameworks.size(); ++i)
      {
         frameworksStr << component.frameworks[i];
         if (i + 1 < component.frameworks.size())
            frameworksStr << ", ";
      }
      ss << ",\n";
      ss << "        {\n";
      ss << "          \"name\": \"macho_frameworks\",\n";
      ss << "          \"value\": " << Utils::formatJsonValue(frameworksStr.str()) << "\n";
      ss << "        }";
   }

   ss << "\n      ]";
   return ss.str();
}

std::string SBOMGenerator::Impl::generateEvidenceField(const ComponentInfo& component)
{
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

std::string SBOMGenerator::Impl::generateSPDX3CreationInfo()
{
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

std::string SBOMGenerator::Impl::generateSPDXLicenseId(const std::string& license)
{
   if (license.empty() || license == "UNKNOWN")
   {
      return "NOASSERTION";
   }

   // Basic SPDX license validation
   std::string upperLicense = Utils::toUpper(license);
   if (upperLicense.find("APACHE") != std::string::npos)
   {
      return "Apache-2.0";
   }
   else if (upperLicense.find("MIT") != std::string::npos)
   {
      return "MIT";
   }
   else if (upperLicense.find("GPL") != std::string::npos)
   {
      if (upperLicense.find('3') != std::string::npos)
      {
         return "GPL-3.0-only";
      }
      else
      {
         return "GPL-2.0-only";
      }
   }
   else if (upperLicense.find("LGPL") != std::string::npos)
   {
      if (upperLicense.find('3') != std::string::npos)
      {
         return "LGPL-3.0-only";
      }
      else
      {
         return "LGPL-2.1-only";
      }
   }
   else if (upperLicense.find("BSD") != std::string::npos)
   {
      return "BSD-3-Clause";
   }
   else
   {
      return "NOASSERTION";
   }
}

/**
 * @brief Process dependencies recursively
 * @param component The component whose dependencies to process
 * @param processedKeys Set of already processed component keys to avoid cycles
 */
void SBOMGenerator::Impl::processDependenciesRecursively(const ComponentInfo&   component,
                                                         std::set<std::string>& processedKeys)
{
   // Resolve library paths to canonical absolute paths for consistent key generation
   std::string canonicalPath = Utils::resolveLibraryPath(component.filePath);
   std::string key           = canonicalPath;  // Use canonical file path as unique key
   if (processedKeys.count(key) > 0)
   {
      return;  // Already processed this component
   }
   processedKeys.insert(key);

   Utils::debugPrint("Processing dependencies recursively for: " + component.name +
                     " (dependencies: " + std::to_string(component.dependencies.size()) + ")");

   for (const auto& depPath : component.dependencies)
   {
      std::string resolvedPath = depPath;

      // Handle @rpath dependencies (resolve relative to app bundle)
      if (depPath.find("@rpath/") == 0)
      {
         std::string appDir    = component.filePath;
         size_t      lastSlash = appDir.rfind('/');
         if (lastSlash != std::string::npos)
         {
            appDir       = appDir.substr(0, lastSlash);
            resolvedPath = appDir + "/" + depPath.substr(7);  // Remove "@rpath/"
         }
      }

      // Always resolve to canonical absolute path
      std::string canonicalPath = Utils::resolveLibraryPath(resolvedPath);
      if (!canonicalPath.empty())
      {
         resolvedPath = canonicalPath;
      }
      // Resolve library paths to canonical absolute paths for consistent key generation
      std::string depKey = resolvedPath;  // Use canonical file path as unique key

      // Skip if this dependency is already processed
      if (components.find(depKey) != components.end())
      {
         continue;
      }

      // Create a new ComponentInfo for the dependency
      ComponentInfo depComponent(Utils::getFileName(resolvedPath), resolvedPath);
      // Let the constructor determine the file type instead of hardcoding it
      // This allows executables to be properly detected even when processed as dependencies

      // Check if it's a system library
      if (resolvedPath.find("/usr/lib/") == 0 || resolvedPath.find("/System/Library/") == 0)
      {
         depComponent.isSystemLibrary = true;
         depComponent.packageManager  = "system";
      }

      // Preserve the checksum that was calculated in the constructor
      std::string originalChecksum = depComponent.checksum;

      // Try to extract metadata for the dependency if it exists
      if (Utils::fileExists(resolvedPath) && metadataExtractor)
      {
         metadataExtractor->extractMetadata(depComponent);
      }
      else
      {
         // For non-existent files (like system libraries), set basic info
         depComponent.version = "system";
#if defined(__APPLE__)
         depComponent.supplier = "Apple Inc.";
#else
         depComponent.supplier = "NOASSERTION";
#endif
      }

      // Always restore the checksum if it was lost during metadata extraction
      if (depComponent.checksum.empty() && !originalChecksum.empty())
      {
         depComponent.checksum = originalChecksum;
      }

      components[depKey] = depComponent;
      Utils::debugPrint("Added dependency component: " + depComponent.name + " at " + resolvedPath);

      // Recursively process the dependency's dependencies
      processDependenciesRecursively(depComponent, processedKeys);
   }
}

}  // namespace heimdall
