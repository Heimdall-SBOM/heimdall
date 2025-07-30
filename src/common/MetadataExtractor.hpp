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
 * @file MetadataExtractor.hpp
 * @brief Main class for extracting metadata using modular components
 * @author Trevor Bakker
 * @date 2025
 *
 * This is the main MetadataExtractor class that uses the modular
 * architecture with separate extractors and detectors.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ComponentInfo.hpp"

// Forward declarations
namespace heimdall
{
class IBinaryExtractor;
class BinaryFormatFactory;
class PackageManagerDetector;
class LicenseDetector;
class VersionDetector;
// LazySymbolExtractor moved to extractors directory
// AdaExtractor is now in extractors directory
class DWARFExtractor;
class SBOMGenerator;
class SBOMSigner;
class SBOMValidator;
class PerformanceMonitor;
class MetadataCache;
}  // namespace heimdall

namespace heimdall
{

/**
 * @brief Main class for extracting metadata from binary files
 *
 * This class provides functionality to extract various types of metadata
 * from binary files using the modular architecture with separate
 * extractors and detectors.
 */
class MetadataExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   MetadataExtractor();

   /**
    * @brief Destructor
    */
   ~MetadataExtractor();

   /**
    * @brief Copy constructor
    * @param other The MetadataExtractor to copy from
    */
   MetadataExtractor(const MetadataExtractor& other);

   /**
    * @brief Move constructor
    * @param other The MetadataExtractor to move from
    */
   MetadataExtractor(MetadataExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The MetadataExtractor to copy from
    * @return Reference to this MetadataExtractor
    */
   MetadataExtractor& operator=(const MetadataExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The MetadataExtractor to move from
    * @return Reference to this MetadataExtractor
    */
   MetadataExtractor& operator=(MetadataExtractor&& other) noexcept;

   /**
    * @brief Extract all metadata from a component
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractMetadata(ComponentInfo& component);

   /**
    * @brief Extract binary format metadata using appropriate extractor
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractBinaryMetadata(ComponentInfo& component);

   /**
    * @brief Extract package manager metadata using PackageManagerDetector
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractPackageManagerMetadata(ComponentInfo& component);

   /**
    * @brief Extract license information using LicenseDetector
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractLicenseMetadata(ComponentInfo& component);

   /**
    * @brief Extract version information using VersionDetector
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractVersionMetadata(ComponentInfo& component);

   /**
    * @brief Extract symbol information using LazySymbolExtractor
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractSymbolMetadata(ComponentInfo& component);

   /**
    * @brief Extract debug information using DWARFExtractor
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractDebugMetadata(ComponentInfo& component);

   /**
    * @brief Extract Ada-specific metadata using AdaExtractor
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractAdaMetadata(ComponentInfo& component);

   // Format detection methods
   /**
    * @brief Check if a file is an ELF binary
    * @param filePath Path to the file to check
    * @return true if the file is an ELF binary
    */
   bool isELF(const std::string& filePath) const;

   /**
    * @brief Check if a file is a Mach-O binary
    * @param filePath Path to the file to check
    * @return true if the file is a Mach-O binary
    */
   bool isMachO(const std::string& filePath) const;

   /**
    * @brief Check if a file is a PE binary
    * @param filePath Path to the file to check
    * @return true if the file is a PE binary
    */
   bool isPE(const std::string& filePath) const;

   /**
    * @brief Check if a file is an archive
    * @param filePath Path to the file to check
    * @return true if the file is an archive
    */
   bool isArchive(const std::string& filePath) const;

   // Individual extraction methods for backward compatibility
   /**
    * @brief Extract symbol information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractSymbolInfo(ComponentInfo& component);

   /**
    * @brief Extract section information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractSectionInfo(ComponentInfo& component);

   /**
    * @brief Extract dependency information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractDependencyInfo(ComponentInfo& component);

   /**
    * @brief Extract version information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractVersionInfo(ComponentInfo& component);

   /**
    * @brief Extract license information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractLicenseInfo(ComponentInfo& component);

   /**
    * @brief Extract debug information
    * @param component The component to extract metadata from
    * @return true if extraction was successful
    */
   bool extractDebugInfo(ComponentInfo& component);

   /**
    * @brief Generate SBOM using SBOMGenerator
    * @param component The component to generate SBOM for
    * @return true if generation was successful
    */
   bool generateSBOM(ComponentInfo& component);

   /**
    * @brief Compare SBOMs using SBOMComparator
    * @param component1 First component for comparison
    * @param component2 Second component for comparison
    * @return true if comparison was successful
    */
   bool compareSBOMs(const ComponentInfo& component1, const ComponentInfo& component2);

   /**
    * @brief Sign SBOM using SBOMSigner
    * @param component The component to sign
    * @return true if signing was successful
    */
   bool signSBOM(ComponentInfo& component);

   /**
    * @brief Validate SBOM using SBOMValidator
    * @param component The component to validate
    * @return true if validation was successful
    */
   bool validateSBOM(const ComponentInfo& component);

   /**
    * @brief Extract metadata from multiple files in batch
    * @param filePaths Vector of file paths to process
    * @param components Vector to populate with extracted component info
    * @return true if batch extraction was successful
    */
   bool extractMetadataBatched(const std::vector<std::string>& filePaths,
                               std::vector<ComponentInfo>&     components);

   /**
    * @brief Set verbose output mode
    * @param verbose Whether to enable verbose output
    */
   void setVerbose(bool verbose);

   /**
    * @brief Set whether to extract debug information
    * @param extract Whether to extract debug information
    */
   void setExtractDebugInfo(bool extract);

   /**
    * @brief Set whether to suppress warnings
    * @param suppress Whether to suppress warnings
    */
   void setSuppressWarnings(bool suppress);

   /**
    * @brief Set confidence threshold for detectors
    * @param threshold Confidence threshold (0.0-1.0)
    */
   void setConfidenceThreshold(double threshold);

   /**
    * @brief Get last error message
    * @return Last error message
    */
   std::string getLastError() const;

   /**
    * @brief Check if a file can be processed
    * @param filePath Path to the file to check
    * @return true if the file can be processed
    */
   bool canProcessFile(const std::string& filePath) const;

   /**
    * @brief Get supported file formats
    * @return Vector of supported file format names
    */
   std::vector<std::string> getSupportedFormats() const;

   /**
    * @brief Get component description
    * @param component The component to describe
    * @return Human-readable description of the component
    */
   std::string generateComponentDescription(const ComponentInfo& component) const;

   /**
    * @brief Determine component scope
    * @param component The component to analyze
    * @return Scope of the component (e.g., "application", "library", "framework")
    */
   std::string determineComponentScope(const ComponentInfo& component) const;

   /**
    * @brief Extract component group information
    * @param component The component to analyze
    * @return Group information for the component
    */
   std::string extractComponentGroup(const ComponentInfo& component) const;

   /**
    * @brief Determine MIME type of component
    * @param component The component to analyze
    * @return MIME type of the component
    */
   std::string determineMimeType(const ComponentInfo& component) const;

   /**
    * @brief Extract copyright information
    * @param component The component to analyze
    * @return Copyright information
    */
   std::string extractCopyrightInfo(const ComponentInfo& component) const;

   /**
    * @brief Add evidence to component
    * @param component The component to add evidence to
    */
   void addComponentEvidence(ComponentInfo& component) const;

   /**
    * @brief Extract enhanced Mach-O metadata (code signing, build config, etc.)
    * @param component The component to extract enhanced Mach-O metadata from
    * @return true if any enhanced metadata was extracted successfully
    */
   bool extractEnhancedMachOMetadata(ComponentInfo& component);

   /**
    * @brief Extract enhanced package information
    * @param component The component to extract enhanced package info from
    */
   void extractEnhancedPackageInfo(ComponentInfo& component);

   /**
    * @brief Extract Mach-O code signing information
    * @param component The component to extract code signing info from
    * @return true if code signing info was extracted successfully
    */
   bool extractMachOCodeSignInfo(ComponentInfo& component);

   /**
    * @brief Extract Mach-O build configuration information
    * @param component The component to extract build config from
    * @return true if build config was extracted successfully
    */
   bool extractMachOBuildConfig(ComponentInfo& component);

   /**
    * @brief Extract Mach-O platform information
    * @param component The component to extract platform info from
    * @return true if platform info was extracted successfully
    */
   bool extractMachOPlatformInfo(ComponentInfo& component);

   /**
    * @brief Extract Mach-O entitlements
    * @param component The component to extract entitlements from
    * @return true if entitlements were extracted successfully
    */
   bool extractMachOEntitlements(ComponentInfo& component);

   /**
    * @brief Extract Mach-O architectures
    * @param component The component to extract architectures from
    * @return true if architectures were extracted successfully
    */
   bool extractMachOArchitectures(ComponentInfo& component);

   /**
    * @brief Extract Mach-O frameworks
    * @param component The component to extract frameworks from
    * @return true if frameworks were extracted successfully
    */
   bool extractMachOFrameworks(ComponentInfo& component);

   /**
    * @brief Extract macOS app bundle metadata
    * @param component The component to extract app bundle metadata from
    * @return true if app bundle metadata was extracted successfully
    */
   bool extractMacOSAppBundleMetadata(ComponentInfo& component);

   /**
    * @brief Enable performance monitoring
    * @param enabled Whether to enable monitoring
    */
   void enablePerformanceMonitoring(bool enabled);

   /**
    * @brief Get performance monitor instance
    * @return Reference to performance monitor
    */
   PerformanceMonitor& getPerformanceMonitor();

   /**
    * @brief Enable caching
    * @param enabled Whether to enable caching
    */
   void enableCaching(bool enabled);

   /**
    * @brief Get cache instance
    * @return Reference to cache
    */
   MetadataCache& getCache();

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Initialize all components
    * @return true if initialization was successful
    */
   bool initializeComponents();

   /**
    * @brief Detect file format and create appropriate extractor
    * @param filePath Path to the file
    * @return Unique pointer to appropriate extractor
    */
   std::unique_ptr<IBinaryExtractor> createExtractor(const std::string& filePath);

   /**
    * @brief Extract metadata from directory (for package managers, licenses, etc.)
    * @param directoryPath Path to the directory
    * @param component Component to populate with metadata
    * @return true if extraction was successful
    */
   bool extractDirectoryMetadata(const std::string& directoryPath, ComponentInfo& component);

   /**
    * @brief Merge metadata from multiple sources
    * @param component Component to merge metadata into
    * @param additionalMetadata Additional metadata to merge
    */
   void mergeMetadata(ComponentInfo& component, const ComponentInfo& additionalMetadata);

   /**
    * @brief Validate extracted metadata
    * @param component Component to validate
    * @return true if metadata is valid
    */
   bool validateMetadata(const ComponentInfo& component);

   /**
    * @brief Post-process extracted metadata
    * @param component Component to post-process
    */
   void postProcessMetadata(ComponentInfo& component);
};

}  // namespace heimdall