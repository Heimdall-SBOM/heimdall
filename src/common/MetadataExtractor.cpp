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
#include "../compat/compatibility.hpp"

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#endif

namespace heimdall
{

// Forward declarations
std::string getArchitectureString(uint32_t cputype, uint32_t cpusubtype);
std::string extractComponentName(const std::string& filePath);

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
      if (!heimdall::compat::fs::exists(component.filePath))
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
   // Get all available extractors for this file
   auto availableExtractors = BinaryFormatFactory::getAvailableExtractors(component.filePath);
   if (availableExtractors.empty())
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   // Find the format-specific extractor (ELF, Mach-O, PE, etc.) and DWARF extractor
   std::unique_ptr<IBinaryExtractor> primaryExtractor;
   std::unique_ptr<IBinaryExtractor> dwarfExtractor;

   for (auto& extractor : availableExtractors)
   {
      // Select format-specific extractor as primary (ELF, Mach-O, PE, etc.)
      if (!primaryExtractor && extractor->getFormatName() != "DWARF" &&
          extractor->getFormatName() != "Lightweight DWARF Parser" &&
          extractor->getFormatName() != "Lazy Symbol Extractor")
      {
         primaryExtractor = std::move(extractor);
      }
      else if (extractor->getFormatName() == "DWARF")
      {
         dwarfExtractor = std::move(extractor);
      }
   }

   // If no format-specific extractor found, use the first available one
   if (!primaryExtractor && !availableExtractors.empty())
   {
      for (auto& extractor : availableExtractors)
      {
         if (extractor)
         {
            primaryExtractor = std::move(extractor);
            break;
         }
      }
   }

   // If no primary extractor found, use the first available one
   if (!primaryExtractor && !availableExtractors.empty())
   {
      primaryExtractor = std::move(availableExtractors[0]);
   }

   if (!primaryExtractor)
   {
      pImpl->setLastError("No suitable extractor found for file: " + component.filePath);
      return false;
   }

   if (!primaryExtractor->canHandle(component.filePath))
   {
      pImpl->setLastError("Extractor cannot handle file: " + component.filePath);
      return false;
   }

   bool success = false;  // Track if any extraction succeeded

   // Extract symbols
   std::vector<SymbolInfo> symbols;
   if (primaryExtractor->extractSymbols(component.filePath, symbols))
   {
      component.symbols = symbols;
      success           = true;  // At least one extraction succeeded
   }

   // Extract sections
   std::vector<SectionInfo> sections;
   if (primaryExtractor->extractSections(component.filePath, sections))
   {
      component.sections = sections;
      success            = true;  // At least one extraction succeeded
   }

   // Extract version
   std::string version;
   if (primaryExtractor->extractVersion(component.filePath, version))
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
   std::vector<std::string> dependencies =
      primaryExtractor->extractDependencies(component.filePath);
   if (!dependencies.empty())
   {
      component.dependencies = dependencies;
      success                = true;  // At least one extraction succeeded
   }

   // Extract DWARF debug information if enabled
   if (pImpl->extractDebugInfo)
   {
      bool debugInfoFound = false;

      // Try DWARF extractor first if available
      if (dwarfExtractor)
      {
         // Extract functions from DWARF
         std::vector<std::string> functions;
         if (dwarfExtractor->extractFunctions(component.filePath, functions))
         {
            component.functions = functions;
            debugInfoFound      = true;
            success             = true;  // At least one extraction succeeded
         }

         // Extract compile units from DWARF
         std::vector<std::string> compileUnits;
         if (dwarfExtractor->extractCompileUnits(component.filePath, compileUnits))
         {
            component.compileUnits = compileUnits;
            debugInfoFound         = true;
            success                = true;  // At least one extraction succeeded
         }

         // Extract source files from DWARF
         std::vector<std::string> sourceFiles;
         if (dwarfExtractor->extractSourceFiles(component.filePath, sourceFiles))
         {
            component.sourceFiles = sourceFiles;
            debugInfoFound        = true;
            success               = true;  // At least one extraction succeeded
         }
      }
      else
      {
         // Fallback to primary extractor for DWARF extraction
         // Extract functions from DWARF
         std::vector<std::string> functions;
         if (primaryExtractor->extractFunctions(component.filePath, functions))
         {
            component.functions = functions;
            debugInfoFound      = true;
            success             = true;  // At least one extraction succeeded
         }

         // Extract compile units from DWARF
         std::vector<std::string> compileUnits;
         if (primaryExtractor->extractCompileUnits(component.filePath, compileUnits))
         {
            component.compileUnits = compileUnits;
            debugInfoFound         = true;
            success                = true;  // At least one extraction succeeded
         }

         // Extract source files from DWARF
         std::vector<std::string> sourceFiles;
         if (primaryExtractor->extractSourceFiles(component.filePath, sourceFiles))
         {
            component.sourceFiles = sourceFiles;
            debugInfoFound        = true;
            success               = true;  // At least one extraction succeeded
         }
      }

      // Set the containsDebugInfo flag if any debug info was found
      if (debugInfoFound)
      {
         component.containsDebugInfo = true;
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
      component.packageManager                = detectedPackageManager;
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
         component.packageManager                         = pm.name;
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
   extractor.setExcludeRuntimePackages(
      false);  // Include runtime packages by default (like ELF extractor)

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
   component.properties["evidence:identity:symbols"]  = std::to_string(component.symbols.size());
   component.properties["evidence:identity:sections"] = std::to_string(component.sections.size());

   // Add occurrence evidence
   component.properties["evidence:occurrence:location"] = component.filePath;
   component.properties["evidence:occurrence:size"]     = std::to_string(component.fileSize);

   // Add file type evidence
   component.properties["evidence:identity:fileType"] = component.getFileTypeString();

   // Add debug info evidence
   component.properties["evidence:identity:hasDebugInfo"] = component.containsDebugInfo ? "true"
                                                                                        : "false";
   component.properties["evidence:identity:isStripped"]   = component.isStripped ? "true" : "false";
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
   for (const auto& property : additionalMetadata.properties)
   {
      const auto& key = property.first;
      const auto& value = property.second;
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
      component.name = extractComponentName(component.filePath);
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

   // Update component name and version from enhanced Mach-O metadata
   if (anySuccess)
   {
      // Try to get a better name from the file path (for macOS apps)
      std::string fileName = heimdall::FileUtils::getFileName(component.filePath);
      if (!fileName.empty() && fileName != component.name)
      {
         // For macOS apps, try to extract the app name from the bundle path
         std::string filePath = component.filePath;
         if (filePath.find(".app/Contents/MacOS/") != std::string::npos)
         {
            // Extract app name from bundle path
            size_t appPos = filePath.find(".app/");
            if (appPos != std::string::npos)
            {
               size_t lastSlash = filePath.rfind('/', appPos);
               if (lastSlash != std::string::npos)
               {
                  std::string appName = filePath.substr(lastSlash + 1, appPos - lastSlash - 1);
                  if (!appName.empty())
                  {
                     component.name = appName;
                  }
               }
            }
         }
      }

      // First priority: Try to extract version from Info.plist for macOS apps
      std::string originalVersion = component.version;

      bool        versionSetFromInfoPlist = false;

      // If Info.plist parsing was successful and returned a non-empty version, consider it set from
      // Info.plist
      if (appBundleSuccess && !component.version.empty())
      {
         versionSetFromInfoPlist = true;
      }

      // Only use fallback versions if Info.plist didn't provide one
      if (!versionSetFromInfoPlist)
      {
         // Fallback: Try to set version from build config if Info.plist didn't provide one
         if (!component.buildConfig.sourceVersion.empty())
         {
            component.version = component.buildConfig.sourceVersion;
         }
         else if (!component.buildConfig.buildVersion.empty())
         {
            component.version = component.buildConfig.buildVersion;
         }
         else if (!component.buildConfig.minOSVersion.empty())
         {
            // Use minOSVersion as a fallback version only if no Info.plist version was found
            component.version = component.buildConfig.minOSVersion;
         }
      }
   }

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
               (component.name == "heimdall-sbom" ||
                component.filePath.find("heimdall-sbom") != std::string::npos))
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
#ifdef __APPLE__
   std::ifstream file(component.filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   // Handle fat binaries - use first architecture
   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t        nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
      struct fat_arch arch{};
      file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
      uint32_t offset = OSSwapBigToHostInt32(arch.offset);
      file.seekg(offset);
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.seekg(offset);
   }

   bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

   if (is64)
   {
      struct mach_header_64 mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

      component.platformInfo.architecture = getArchitectureString(mh.cputype, mh.cpusubtype);
      component.platformInfo.platform     = "macos";  // Default for Mach-O files
      component.platformInfo.minVersion   = 0;
      component.platformInfo.sdkVersion   = 0;
      component.platformInfo.isSimulator  = false;

      return true;
   }
   else
   {
      struct mach_header mh{};
      file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

      component.platformInfo.architecture = getArchitectureString(mh.cputype, mh.cpusubtype);
      component.platformInfo.platform     = "macos";  // Default for Mach-O files
      component.platformInfo.minVersion   = 0;
      component.platformInfo.sdkVersion   = 0;
      component.platformInfo.isSimulator  = false;

      return true;
   }
#else
   return false;
#endif
}

bool MetadataExtractor::extractMachOEntitlements(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMachOArchitectures(ComponentInfo& component)
{
#ifdef __APPLE__
   std::ifstream file(component.filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   uint32_t magic = 0;
   file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
   file.seekg(0);

   // Handle fat binaries
   if (magic == FAT_MAGIC || magic == FAT_CIGAM)
   {
      struct fat_header fatHeader{};
      file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
      uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);

      for (uint32_t i = 0; i < nfat_arch; ++i)
      {
         struct fat_arch arch{};
         file.read(reinterpret_cast<char*>(&arch), sizeof(arch));

         // Handle endianness
         if (fatHeader.magic == FAT_CIGAM || fatHeader.magic == FAT_CIGAM_64)
         {
            arch.cputype    = OSSwapBigToHostInt32(arch.cputype);
            arch.cpusubtype = OSSwapBigToHostInt32(arch.cpusubtype);
         }

         ArchitectureInfo archInfo;
         archInfo.name       = getArchitectureString(arch.cputype, arch.cpusubtype);
         archInfo.cpuType    = arch.cputype;
         archInfo.cpuSubtype = arch.cpusubtype;
         archInfo.offset     = OSSwapBigToHostInt32(arch.offset);
         archInfo.size       = OSSwapBigToHostInt32(arch.size);
         archInfo.align      = OSSwapBigToHostInt32(arch.align);

         component.architectures.push_back(archInfo);
      }
      return !component.architectures.empty();
   }
   else
   {
      // Single architecture
      bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);

      if (is64)
      {
         struct mach_header_64 mh{};
         file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

         ArchitectureInfo archInfo;
         archInfo.name       = getArchitectureString(mh.cputype, mh.cpusubtype);
         archInfo.cpuType    = mh.cputype;
         archInfo.cpuSubtype = mh.cpusubtype;
         archInfo.offset     = 0;
         archInfo.size       = 0;
         archInfo.align      = 0;

         component.architectures.push_back(archInfo);
      }
      else
      {
         struct mach_header mh{};
         file.read(reinterpret_cast<char*>(&mh), sizeof(mh));

         ArchitectureInfo archInfo;
         archInfo.name       = getArchitectureString(mh.cputype, mh.cpusubtype);
         archInfo.cpuType    = mh.cputype;
         archInfo.cpuSubtype = mh.cpusubtype;
         archInfo.offset     = 0;
         archInfo.size       = 0;
         archInfo.align      = 0;

         component.architectures.push_back(archInfo);
      }

      return !component.architectures.empty();
   }
#else
   return false;
#endif
}

bool MetadataExtractor::extractMachOFrameworks(ComponentInfo& component)
{
   // This would need to be implemented using the Mach-O extractor
   // For now, return false as a placeholder
   return false;
}

bool MetadataExtractor::extractMacOSAppBundleMetadata(ComponentInfo& component)
{
#ifdef __APPLE__
   std::string infoPlistPath;

   // Detect if we have an executable path within an app bundle
   if (component.filePath.find(".app/Contents/MacOS/") != std::string::npos)
   {
      // Extract the .app bundle path
      size_t appPos = component.filePath.find(".app/Contents/MacOS/");
      if (appPos != std::string::npos)
      {
         std::string appBundleRoot = component.filePath.substr(0, appPos + 4);  // Include ".app"
         infoPlistPath             = appBundleRoot + "/Contents/Info.plist";
      }
   }
   else if (component.filePath.find(".app") != std::string::npos)
   {
      // Assume it's already the app bundle path
      infoPlistPath = component.filePath + "/Contents/Info.plist";
   }
   else
   {
      return false;
   }

   std::ifstream file(infoPlistPath);
   if (!file.is_open())
   {
      return false;
   }

   std::string line;
   bool        foundVersionKey    = false;
   bool        foundNameKey       = false;
   bool        foundBundleNameKey = false;

   while (std::getline(file, line))
   {
      // Look for CFBundleShortVersionString key
      if (line.find("<key>CFBundleShortVersionString</key>") != std::string::npos)
      {
         foundVersionKey = true;
         continue;
      }

      // Look for CFBundleName key
      if (line.find("<key>CFBundleName</key>") != std::string::npos)
      {
         foundNameKey = true;
         continue;
      }

      // Look for CFBundleDisplayName key
      if (line.find("<key>CFBundleDisplayName</key>") != std::string::npos)
      {
         foundBundleNameKey = true;
         continue;
      }

      // Extract version value after finding the key
      if (foundVersionKey && line.find("<string>") != std::string::npos)
      {
         size_t start = line.find("<string>") + 8;
         size_t end   = line.find("</string>");
         if (end != std::string::npos && end > start)
         {
            component.version = line.substr(start, end - start);
            foundVersionKey   = false;
         }
      }

      // Extract bundle name value after finding the key
      if ((foundNameKey || foundBundleNameKey) && line.find("<string>") != std::string::npos)
      {
         size_t start = line.find("<string>") + 8;
         size_t end   = line.find("</string>");
         if (end != std::string::npos && end > start)
         {
            std::string bundleName = line.substr(start, end - start);
            if (!bundleName.empty() && component.name != bundleName)
            {
               component.name = bundleName;
            }
            foundNameKey       = false;
            foundBundleNameKey = false;
         }
      }

      // Stop if we found both
      if (!component.version.empty() && !component.name.empty())
      {
         break;
      }
   }

   return !component.version.empty() || !component.name.empty();
#else
   return false;
#endif
}

// Helper function for architecture string conversion
std::string getArchitectureString(uint32_t cputype, uint32_t cpusubtype)
{
#ifdef __APPLE__
   switch (cputype)
   {
      case CPU_TYPE_X86:
         return "i386";
      case CPU_TYPE_X86_64:
         return "x86_64";
      case CPU_TYPE_ARM:
         return "arm";
      case CPU_TYPE_ARM64:
         return "arm64";
      case CPU_TYPE_POWERPC:
         return "ppc";
      case CPU_TYPE_POWERPC64:
         return "ppc64";
      default:
         return "Unknown";
   }
#else
   return "Unknown";
#endif
}

// Helper function for component name extraction
std::string extractComponentName(const std::string& filePath)
{
   std::string fileName = heimdall::FileUtils::getFileName(filePath);

   // Special handling for macOS app bundles
   if (filePath.find(".app/Contents/MacOS/") != std::string::npos)
   {
      // Extract app name from bundle path
      size_t appPos = filePath.find(".app/");
      if (appPos != std::string::npos)
      {
         size_t lastSlash = filePath.rfind('/', appPos);
         if (lastSlash != std::string::npos)
         {
            std::string appName = filePath.substr(lastSlash + 1, appPos - lastSlash - 1);
            if (!appName.empty())
            {
               return appName;
            }
         }
      }
   }

   // Remove common prefixes and extensions
   if (fileName.substr(0, 3) == "lib")
   {
      fileName = fileName.substr(3);
   }

   std::string extension = heimdall::FileUtils::getFileExtension(fileName);
   if (!extension.empty())
   {
      fileName = fileName.substr(0, fileName.length() - extension.length());
   }

   // Remove version numbers and suffixes (e.g., -1.2.3, _debug)
   size_t dashPos = fileName.find('-');
   if (dashPos != std::string::npos)
   {
      // Check if what follows is a version number (contains digits and dots)
      std::string suffix    = fileName.substr(dashPos + 1);
      bool        isVersion = false;
      bool        hasDigit  = false;
      for (char c : suffix)
      {
         if (std::isdigit(c))
         {
            hasDigit = true;
         }
         else if (c != '.' && c != '-')
         {
            isVersion = false;
            break;
         }
      }
      if (hasDigit && suffix.find('.') != std::string::npos)
      {
         fileName = fileName.substr(0, dashPos);
      }
   }

   // Remove _debug, _release, etc. suffixes
   size_t underscorePos = fileName.find('_');
   if (underscorePos != std::string::npos)
   {
      std::string suffix = fileName.substr(underscorePos + 1);
      if (suffix == "debug" || suffix == "release" || suffix == "static" || suffix == "shared" ||
          suffix == "dll" || suffix == "so")
      {
         fileName = fileName.substr(0, underscorePos);
      }
   }

   return fileName;
}

}  // namespace heimdall