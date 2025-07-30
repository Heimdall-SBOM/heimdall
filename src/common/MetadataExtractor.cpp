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
 * @file MetadataExtractor.cpp
 * @brief Implementation of the MetadataExtractor using modular components
 * @author Trevor Bakker
 * @date 2025
 */

#include "MetadataExtractor.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../detectors/LicenseDetector.hpp"
#include "../detectors/PackageManagerDetector.hpp"
#include "../detectors/VersionDetector.hpp"
#include "../extractors/AdaExtractor.hpp"
#include "../factories/BinaryFormatFactory.hpp"
#include "../interfaces/IBinaryExtractor.hpp"
#include "../utils/FileUtils.hpp"
#include "Utils.hpp"
namespace heimdall
{

class MetadataExtractor::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   bool        verbose             = false;
   bool        extractDebugInfo    = true;
   bool        suppressWarnings    = false;
   double      confidenceThreshold = 0.7;
   std::string lastError;

   // Component instances
   std::unique_ptr<PackageManagerDetector> packageManagerDetector;
   std::unique_ptr<LicenseDetector>        licenseDetector;
   std::unique_ptr<VersionDetector>        versionDetector;

   // Helper methods
   bool                              initializeComponentsImpl();
   std::unique_ptr<IBinaryExtractor> createExtractorImpl(const std::string& filePath);
   bool extractDirectoryMetadataImpl(const std::string& directoryPath, ComponentInfo& component);
   void mergeMetadataImpl(ComponentInfo& component, const ComponentInfo& additionalMetadata);
   bool validateMetadataImpl(const ComponentInfo& component);
   void postProcessMetadataImpl(ComponentInfo& component);
   void setLastError(const std::string& error);
};

// MetadataExtractor implementation
MetadataExtractor::MetadataExtractor() : pImpl(std::make_unique<Impl>())
{
   initializeComponents();
}

MetadataExtractor::~MetadataExtractor() = default;

MetadataExtractor::MetadataExtractor(const MetadataExtractor& other)
   : pImpl(std::make_unique<Impl>())
{
   // Copy settings but not the unique_ptr members
   pImpl->verbose             = other.pImpl->verbose;
   pImpl->extractDebugInfo    = other.pImpl->extractDebugInfo;
   pImpl->suppressWarnings    = other.pImpl->suppressWarnings;
   pImpl->confidenceThreshold = other.pImpl->confidenceThreshold;
   pImpl->lastError           = other.pImpl->lastError;

   // Re-initialize components
   pImpl->initializeComponentsImpl();
}

MetadataExtractor::MetadataExtractor(MetadataExtractor&& other) noexcept
   : pImpl(std::move(other.pImpl))
{
}

MetadataExtractor& MetadataExtractor::operator=(const MetadataExtractor& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>();
      // Copy settings but not the unique_ptr members
      pImpl->verbose             = other.pImpl->verbose;
      pImpl->extractDebugInfo    = other.pImpl->extractDebugInfo;
      pImpl->suppressWarnings    = other.pImpl->suppressWarnings;
      pImpl->confidenceThreshold = other.pImpl->confidenceThreshold;
      pImpl->lastError           = other.pImpl->lastError;

      // Re-initialize components
      pImpl->initializeComponentsImpl();
   }
   return *this;
}

MetadataExtractor& MetadataExtractor::operator=(MetadataExtractor&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

bool MetadataExtractor::extractMetadata(ComponentInfo& component)
{
   try
   {
      // Check if file exists (matching pre-refactored behavior)
      if (!std::filesystem::exists(component.filePath))
      {
         pImpl->setLastError("File does not exist: " + component.filePath);
         return false;
      }

      // Extract binary format metadata
      bool binaryExtractionSuccess = extractBinaryMetadata(component);
      
      // Extract dependency information
      extractDependencyInfo(component);

      // Extract version metadata
      extractVersionMetadata(component);

      // Extract license metadata
      extractLicenseMetadata(component);

      // Extract package manager metadata
      extractPackageManagerMetadata(component);

         // Extract enhanced metadata (Mach-O specific)
   if (isMachO(component.filePath))
   {
      extractEnhancedMachOMetadata(component);
   }

   // Extract enhanced package information
   extractEnhancedPackageInfo(component);

   // Add component evidence (matching pre-refactored behavior)
   addComponentEvidence(component);

   // Post-process and validate metadata
   postProcessMetadata(component);
   validateMetadata(component);

   // Mark the component as processed (it was attempted)
   component.markAsProcessed();

   // Return the success status of binary extraction (matching pre-refactored behavior)
   // The pre-refactored version would return false if any critical extraction failed
   return binaryExtractionSuccess;
   }
   catch (const std::exception& e)
   {
      // Exception handling matching pre-refactored behavior
      pImpl->setLastError(std::string("Exception in extractMetadata: ") + e.what());
      return false;
   }
}

bool MetadataExtractor::extractBinaryMetadata(ComponentInfo& component)
{
   auto extractor = createExtractor(component.filePath);
   if (!extractor)
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   if (!extractor->canHandle(component.filePath))
   {
      pImpl->setLastError("Extractor cannot handle file: " + component.filePath);
      return false;
   }

   bool success = false;  // Track if any extraction succeeded

   // Extract symbols
   std::vector<SymbolInfo> symbols;
   if (extractor->extractSymbols(component.filePath, symbols))
   {
      component.symbols = symbols;
      success = true;  // At least one extraction succeeded
   }

   // Extract sections
   std::vector<SectionInfo> sections;
   if (extractor->extractSections(component.filePath, sections))
   {
      component.sections = sections;
      success = true;  // At least one extraction succeeded
   }

   // Extract version
   std::string version;
   if (extractor->extractVersion(component.filePath, version))
   {
      // Don't set ELF version string for the main application component
      // This is just binary format info, not a real version
      if (version.find("ELF") == 0 && component.fileType == FileType::Executable)
      {
         // For executables, don't use the ELF format version string
         // The version will be set by other means (build info, etc.)
      }
      else
      {
         component.version = version;
      }
      success = true;  // At least one extraction succeeded
   }

   // Extract dependencies
   std::vector<std::string> dependencies = extractor->extractDependencies(component.filePath);
   if (!dependencies.empty())
   {
      component.dependencies = dependencies;
      success = true;  // At least one extraction succeeded
   }

   // Extract DWARF debug information if enabled
   if (pImpl->extractDebugInfo)
   {
      // Extract functions from DWARF
      std::vector<std::string> functions;
      if (extractor->extractFunctions(component.filePath, functions))
      {
         component.functions = functions;
         success = true;  // At least one extraction succeeded
      }

      // Extract compile units from DWARF
      std::vector<std::string> compileUnits;
      if (extractor->extractCompileUnits(component.filePath, compileUnits))
      {
         component.compileUnits = compileUnits;
         success = true;  // At least one extraction succeeded
      }

      // Extract source files from DWARF
      std::vector<std::string> sourceFiles;
      if (extractor->extractSourceFiles(component.filePath, sourceFiles))
      {
         component.sourceFiles = sourceFiles;
         success = true;  // At least one extraction succeeded
      }
   }

   return success;  // Return true only if at least one extraction succeeded
}

bool MetadataExtractor::extractPackageManagerMetadata(ComponentInfo& component)
{
   // First, try path-based package manager detection
   std::string detectedPackageManager = heimdall::Utils::detectPackageManager(component.filePath);
   if (!detectedPackageManager.empty() && detectedPackageManager != "unknown")
   {
      component.packageManager = detectedPackageManager;
      component.properties["package_manager"] = detectedPackageManager;
      return true;
   }

   // Fall back to manifest-based detection
   if (!pImpl->packageManagerDetector)
   {
      pImpl->setLastError("PackageManagerDetector not initialized");
      return false;
   }

   std::string directoryPath = heimdall::FileUtils::getDirectoryPath(component.filePath);
   if (directoryPath.empty())
   {
      return false;
   }

   std::vector<PackageManagerInfo> packageManagers;
   if (pImpl->packageManagerDetector->detectPackageManagers(directoryPath, packageManagers))
   {
      // Store package manager information in component properties
      for (const auto& pm : packageManagers)
      {
         component.packageManager = pm.name;
         component.properties["package_manager"]          = pm.name;
         component.properties["package_manager_version"]  = pm.version;
         component.properties["package_manager_manifest"] = pm.manifestFile;
         component.properties["package_manager_lock"]     = pm.lockFile;

         // Add package information
         std::string packages;
         for (const auto& pkg : pm.packages)
         {
            if (!packages.empty())
               packages += ", ";
            packages += pkg;
         }
         component.properties["packages"] = packages;
      }
      return true;
   }

   return false;
}

bool MetadataExtractor::extractLicenseMetadata(ComponentInfo& component)
{
   if (!pImpl->licenseDetector)
   {
      pImpl->setLastError("LicenseDetector not initialized");
      return false;
   }

   std::string directoryPath = heimdall::FileUtils::getDirectoryPath(component.filePath);
   if (directoryPath.empty())
   {
      return false;
   }

   std::vector<LicenseInfo> licenses;
   if (pImpl->licenseDetector->detectLicenses(directoryPath, licenses))
   {
      // Store license information in component
      if (!licenses.empty())
      {
         const auto& license                  = licenses[0];  // Use highest confidence license
         component.license                    = license.name;
         component.properties["license_spdx"] = license.spdxId;
         component.properties["license_file"] = license.filePath;
         component.properties["license_confidence"] = std::to_string(license.confidence);
         component.properties["license_copyright"]  = license.copyright;
         component.properties["license_author"]     = license.author;
      }
      return true;
   }

   return false;
}

bool MetadataExtractor::extractVersionMetadata(ComponentInfo& component)
{
   if (!pImpl->versionDetector)
   {
      pImpl->setLastError("VersionDetector not initialized");
      return false;
   }

   std::string directoryPath = heimdall::FileUtils::getDirectoryPath(component.filePath);
   if (directoryPath.empty())
   {
      return false;
   }

   std::vector<VersionInfo> versions;
   if (pImpl->versionDetector->detectVersions(directoryPath, versions))
   {
      // Store version information in component
      if (!versions.empty())
      {
         const auto& version                   = versions[0];  // Use highest confidence version
         component.version                     = version.fullVersion;
         component.properties["version_major"] = version.major;
         component.properties["version_minor"] = version.minor;
         component.properties["version_patch"] = version.patch;
         component.properties["version_prerelease"] = version.prerelease;
         component.properties["version_build"]      = version.build;
         component.properties["version_source"]     = version.source;
         component.properties["version_confidence"] = std::to_string(version.confidence);
         component.properties["version_commit"]     = version.commitHash;
         component.properties["version_branch"]     = version.branch;
         component.properties["version_tag"]        = version.tag;
      }
      return true;
   }

   return false;
}

bool MetadataExtractor::extractSymbolMetadata(ComponentInfo& component)
{
   // TODO: Integrate LazySymbolExtractor
   if (pImpl->verbose)
   {
      std::cout << "Symbol metadata extraction not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::extractDebugMetadata(ComponentInfo& component)
{
   // TODO: Integrate DWARFExtractor
   if (pImpl->verbose)
   {
      std::cout << "Debug metadata extraction not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::extractAdaMetadata(ComponentInfo& component)
{
   // Use AdaExtractor from extractors directory
   if (pImpl->verbose)
   {
      std::cout << "Extracting Ada metadata using AdaExtractor..." << std::endl;
   }

   // Find ALI files in the component's directory
   std::vector<std::string> aliFiles;
   AdaExtractor             extractor;
   extractor.setVerbose(pImpl->verbose);
   extractor.setExcludeRuntimePackages(false);  // Include runtime packages by default (like ELF extractor)

   if (extractor.findAliFiles(component.filePath, aliFiles))
   {
      return extractor.extractAdaMetadata(component, aliFiles);
   }

   return false;
}

bool MetadataExtractor::generateSBOM(ComponentInfo& component)
{
   // TODO: Integrate SBOMGenerator
   if (pImpl->verbose)
   {
      std::cout << "SBOM generation not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::compareSBOMs(const ComponentInfo& component1,
                                       const ComponentInfo& component2)
{
   // TODO: Integrate SBOMComparator
   if (pImpl->verbose)
   {
      std::cout << "SBOM comparison not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::signSBOM(ComponentInfo& component)
{
   // TODO: Integrate SBOMSigner
   if (pImpl->verbose)
   {
      std::cout << "SBOM signing not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::validateSBOM(const ComponentInfo& component)
{
   // TODO: Integrate SBOMValidator
   if (pImpl->verbose)
   {
      std::cout << "SBOM validation not yet implemented" << std::endl;
   }
   return false;
}

bool MetadataExtractor::extractMetadataBatched(const std::vector<std::string>& filePaths,
                                                 std::vector<ComponentInfo>&     components)
{
   components.clear();
   bool overallSuccess = true;

   for (const auto& filePath : filePaths)
   {
      ComponentInfo component;
      component.filePath = filePath;

      if (extractMetadata(component))
      {
         components.push_back(component);
      }
      else
      {
         overallSuccess = false;
         if (pImpl->verbose)
         {
            std::cout << "Failed to extract metadata from: " << filePath << std::endl;
         }
      }
   }

   return overallSuccess;
}

void MetadataExtractor::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;

   // Propagate verbose setting to all components
   if (pImpl->packageManagerDetector)
   {
      pImpl->packageManagerDetector->setVerbose(verbose);
   }
   if (pImpl->licenseDetector)
   {
      pImpl->licenseDetector->setVerbose(verbose);
   }
   if (pImpl->versionDetector)
   {
      pImpl->versionDetector->setVerbose(verbose);
   }
}

void MetadataExtractor::setExtractDebugInfo(bool extract)
{
   pImpl->extractDebugInfo = extract;
}

void MetadataExtractor::setSuppressWarnings(bool suppress)
{
   pImpl->suppressWarnings = suppress;
}

void MetadataExtractor::setConfidenceThreshold(double threshold)
{
   pImpl->confidenceThreshold = threshold;

   // Propagate confidence threshold to all detectors
   if (pImpl->licenseDetector)
   {
      pImpl->licenseDetector->setConfidenceThreshold(threshold);
   }
   if (pImpl->versionDetector)
   {
      pImpl->versionDetector->setConfidenceThreshold(threshold);
   }
}

std::string MetadataExtractor::getLastError() const
{
   return pImpl->lastError;
}

bool MetadataExtractor::canProcessFile(const std::string& filePath) const
{
   auto extractor = BinaryFormatFactory::createExtractor(filePath);
   return extractor && extractor->canHandle(filePath);
}

std::vector<std::string> MetadataExtractor::getSupportedFormats() const
{
   return {"ELF", "Mach-O", "PE", "Archive"};
}

std::string MetadataExtractor::generateComponentDescription(const ComponentInfo& component) const
{
   std::stringstream ss;
   ss << component.name;

   if (!component.version.empty())
   {
      ss << " v" << component.version;
   }

   if (!component.license.empty())
   {
      ss << " (" << component.license << ")";
   }

   return ss.str();
}

std::string MetadataExtractor::determineComponentScope(const ComponentInfo& component) const
{
   // Simple heuristic based on file extension and metadata
   std::string extension = heimdall::FileUtils::getFileExtension(component.filePath);

   if (extension == "exe" || extension == "app")
   {
      return "application";
   }
   else if (extension == "so" || extension == "dylib" || extension == "dll")
   {
      return "library";
   }
   else if (extension == "framework")
   {
      return "framework";
   }

   return "unknown";
}

std::string MetadataExtractor::extractComponentGroup(const ComponentInfo& component) const
{
   // Extract group from file path or metadata
   std::string directory = heimdall::FileUtils::getDirectoryPath(component.filePath);
   std::string fileName  = heimdall::FileUtils::getFileName(component.filePath);

   // Simple heuristic: use parent directory name as group
   auto pos = directory.find_last_of("/\\");
   if (pos != std::string::npos)
   {
      return directory.substr(pos + 1);
   }

   return "default";
}

std::string MetadataExtractor::determineMimeType(const ComponentInfo& component) const
{
   std::string extension = heimdall::FileUtils::getFileExtension(component.filePath);

   if (extension == "so")
   {
      return "application/x-sharedlib";
   }
   else if (extension == "exe")
   {
      return "application/x-executable";
   }
   else if (extension == "dylib")
   {
      return "application/x-mach-binary";
   }
   else if (extension == "dll")
   {
      return "application/x-msdownload";
   }
   else if (extension == "a")
   {
      return "application/x-archive";
   }

   return "application/octet-stream";
}

std::string MetadataExtractor::extractCopyrightInfo(const ComponentInfo& component) const
{
   // Try to extract copyright from properties
   auto it = component.properties.find("license_copyright");
   if (it != component.properties.end())
   {
      return it->second;
   }

   return "";
}

void MetadataExtractor::addComponentEvidence(ComponentInfo& component) const
{
   // Add evidence about how metadata was extracted
   component.properties["evidence_extractor_version"] = "2.0";
   component.properties["evidence_extraction_date"]   = std::to_string(std::time(nullptr));
   component.properties["evidence_confidence_threshold"] =
      std::to_string(pImpl->confidenceThreshold);

   // Add identity evidence (matching pre-refactored behavior)
   component.properties["evidence:identity:symbols"] = std::to_string(component.symbols.size());
   component.properties["evidence:identity:sections"] = std::to_string(component.sections.size());

   // Add occurrence evidence
   component.properties["evidence:occurrence:location"] = component.filePath;
   component.properties["evidence:occurrence:size"] = std::to_string(component.fileSize);

   // Add file type evidence
   component.properties["evidence:identity:fileType"] = component.getFileTypeString();

   // Add debug info evidence
   component.properties["evidence:identity:hasDebugInfo"] = component.containsDebugInfo ? "true" : "false";
   component.properties["evidence:identity:isStripped"] = component.isStripped ? "true" : "false";
}

// Private helper methods
bool MetadataExtractor::initializeComponents()
{
   return pImpl->initializeComponentsImpl();
}

std::unique_ptr<IBinaryExtractor> MetadataExtractor::createExtractor(const std::string& filePath)
{
   return pImpl->createExtractorImpl(filePath);
}

bool MetadataExtractor::extractDirectoryMetadata(const std::string& directoryPath,
                                                   ComponentInfo&     component)
{
   return pImpl->extractDirectoryMetadataImpl(directoryPath, component);
}

void MetadataExtractor::mergeMetadata(ComponentInfo&       component,
                                        const ComponentInfo& additionalMetadata)
{
   pImpl->mergeMetadataImpl(component, additionalMetadata);
}

bool MetadataExtractor::validateMetadata(const ComponentInfo& component)
{
   return pImpl->validateMetadataImpl(component);
}

void MetadataExtractor::postProcessMetadata(ComponentInfo& component)
{
   pImpl->postProcessMetadataImpl(component);
}

// Impl implementation
bool MetadataExtractor::Impl::initializeComponentsImpl()
{
   try
   {
      // Initialize detectors
      packageManagerDetector = std::make_unique<PackageManagerDetector>();
      licenseDetector        = std::make_unique<LicenseDetector>();
      versionDetector        = std::make_unique<VersionDetector>();

      // Set verbose mode for all components
      packageManagerDetector->setVerbose(verbose);
      licenseDetector->setVerbose(verbose);
      versionDetector->setVerbose(verbose);

      // Set confidence thresholds
      licenseDetector->setConfidenceThreshold(confidenceThreshold);
      versionDetector->setConfidenceThreshold(confidenceThreshold);

      return true;
   }
   catch (const std::exception& e)
   {
      setLastError("Failed to initialize components: " + std::string(e.what()));
      return false;
   }
}

std::unique_ptr<IBinaryExtractor> MetadataExtractor::Impl::createExtractorImpl(
   const std::string& filePath)
{
   return BinaryFormatFactory::createExtractor(filePath);
}

bool MetadataExtractor::Impl::extractDirectoryMetadataImpl(const std::string& directoryPath,
                                                             ComponentInfo&     component)
{
   // This method can be used to extract metadata that requires directory scanning
   // Currently handled by individual detector methods
   return true;
}

void MetadataExtractor::Impl::mergeMetadataImpl(ComponentInfo&       component,
                                                  const ComponentInfo& additionalMetadata)
{
   // Merge properties from additional source into component
   for (const auto& [key, value] : additionalMetadata.properties)
   {
      if (component.properties.find(key) == component.properties.end())
      {
         component.properties[key] = value;
      }
   }
}

bool MetadataExtractor::Impl::validateMetadataImpl(const ComponentInfo& component)
{
   // Basic validation of extracted metadata
   if (component.filePath.empty())
   {
      setLastError("Component has no file path");
      return false;
   }

   return true;
}

void MetadataExtractor::Impl::postProcessMetadataImpl(ComponentInfo& component)
{
   // Post-process extracted metadata
   if (component.name.empty())
   {
      component.name = heimdall::FileUtils::getFileName(component.filePath);
   }

   // Only set component file type based on file extension if it's currently Unknown
   // This preserves the correct file type detection done by ComponentInfo constructor
   if (component.fileType == FileType::Unknown)
   {
      std::string extension = heimdall::FileUtils::getFileExtension(component.filePath);
      if (extension == "so" || extension == "dylib" || extension == "dll")
      {
         component.fileType = FileType::SharedLibrary;
      }
      else if (extension == "exe" || extension == "app")
      {
         component.fileType = FileType::Executable;
      }
      else if (extension == "a")
      {
         component.fileType = FileType::StaticLibrary;
      }
      // If still unknown, leave it as Unknown (don't overwrite)
   }

   // Component is marked as processed in extractMetadata if extraction succeeds
}

void MetadataExtractor::Impl::setLastError(const std::string& error)
{
   lastError = error;
   if (verbose)
   {
      std::cerr << "MetadataExtractor error: " << error << std::endl;
   }
}

// Format detection methods
bool MetadataExtractor::isELF(const std::string& filePath) const
{
   auto extractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::ELF);
   return extractor && extractor->canHandle(filePath);
}

bool MetadataExtractor::isMachO(const std::string& filePath) const
{
   auto extractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::MachO);
   return extractor && extractor->canHandle(filePath);
}

bool MetadataExtractor::isPE(const std::string& filePath) const
{
   auto extractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::PE);
   return extractor && extractor->canHandle(filePath);
}

bool MetadataExtractor::isArchive(const std::string& filePath) const
{
   auto extractor = BinaryFormatFactory::createExtractor(BinaryFormatFactory::Format::Archive);
   return extractor && extractor->canHandle(filePath);
}

// Individual extraction methods for backward compatibility
bool MetadataExtractor::extractSymbolInfo(ComponentInfo& component)
{
   auto extractor = BinaryFormatFactory::createExtractor(component.filePath);
   if (!extractor)
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   std::vector<SymbolInfo> symbols;
   bool                    result = extractor->extractSymbols(component.filePath, symbols);
   if (result)
   {
      component.symbols = symbols;
   }
   return result;
}

bool MetadataExtractor::extractSectionInfo(ComponentInfo& component)
{
   auto extractor = BinaryFormatFactory::createExtractor(component.filePath);
   if (!extractor)
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   std::vector<SectionInfo> sections;
   bool                     result = extractor->extractSections(component.filePath, sections);
   if (result)
   {
      component.sections = sections;
   }
   return result;
}

bool MetadataExtractor::extractDependencyInfo(ComponentInfo& component)
{
   auto extractor = BinaryFormatFactory::createExtractor(component.filePath);
   if (!extractor)
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   if (!extractor->canHandle(component.filePath))
   {
      pImpl->setLastError("Extractor cannot handle file: " + component.filePath);
      return false;
   }

   std::vector<std::string> dependencies = extractor->extractDependencies(component.filePath);
   component.dependencies                = dependencies;
   return true;  // Dependencies extraction succeeds if extractor was found and can handle the file
}

bool MetadataExtractor::extractVersionInfo(ComponentInfo& component)
{
   return extractVersionMetadata(component);
}

bool MetadataExtractor::extractLicenseInfo(ComponentInfo& component)
{
   return extractLicenseMetadata(component);
}

bool MetadataExtractor::extractDebugInfo(ComponentInfo& component)
{
   return extractDebugMetadata(component);
}

// Enhanced metadata extraction methods
bool MetadataExtractor::extractEnhancedMachOMetadata(ComponentInfo& component)
{
   if (!isMachO(component.filePath))
   {
      return false;
   }

   bool anySuccess = false;

   // Extract all enhanced Mach-O metadata
   bool codeSignSuccess = extractMachOCodeSignInfo(component);
   anySuccess |= codeSignSuccess;

   bool buildConfigSuccess = extractMachOBuildConfig(component);
   anySuccess |= buildConfigSuccess;

   bool platformSuccess = extractMachOPlatformInfo(component);
   anySuccess |= platformSuccess;

   bool entitlementsSuccess = extractMachOEntitlements(component);
   anySuccess |= entitlementsSuccess;

   bool architecturesSuccess = extractMachOArchitectures(component);
   anySuccess |= architecturesSuccess;

   bool frameworksSuccess = extractMachOFrameworks(component);
   anySuccess |= frameworksSuccess;

   // Extract macOS app bundle metadata if applicable
   bool appBundleSuccess = extractMacOSAppBundleMetadata(component);
   anySuccess |= appBundleSuccess;

   return anySuccess;
}

void MetadataExtractor::extractEnhancedPackageInfo(ComponentInfo& component)
{
   // Extract supplier from package manager - only if not already set
   if (component.supplier.empty())
   {
      if (component.packageManager == "rpm")
      {
         component.setSupplier("Red Hat Package Manager");
      }
      else if (component.packageManager == "deb")
      {
         component.setSupplier("Debian Package Manager");
      }
      else if (component.packageManager == "conan")
      {
         component.setSupplier("Conan Center");
      }
      else if (component.packageManager == "vcpkg")
      {
         component.setSupplier("vcpkg");
      }
      else if (component.packageManager == "spack")
      {
         component.setSupplier("Spack");
      }
      // Set supplier for heimdall-sbom executable
      else if (component.fileType == FileType::Executable &&
               (component.name == "heimdall-sbom" || component.filePath.find("heimdall-sbom") != std::string::npos))
      {
         component.setSupplier("Heimdall Project");
      }
   }

   // Extract group from package name - only if not already set
   if (component.group.empty())
   {
      std::string group = extractComponentGroup(component);
      if (!group.empty())
      {
         component.setGroup(group);
      }
   }

   // Set manufacturer same as supplier - only if not already set
   if (component.manufacturer.empty() && !component.supplier.empty())
   {
      component.setManufacturer(component.supplier);
   }
}

bool MetadataExtractor::extractMachOCodeSignInfo(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOBuildConfig(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOPlatformInfo(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOEntitlements(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOArchitectures(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOFrameworks(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMacOSAppBundleMetadata(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

}  // namespace heimdall