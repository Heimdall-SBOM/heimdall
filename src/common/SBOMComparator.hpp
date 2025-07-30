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
 * @file SBOMComparator.hpp
 * @brief SBOM comparison, merging, and diff generation functionality using format handlers
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides comprehensive functionality for comparing, merging, and
 * analyzing Software Bill of Materials (SBOM) documents using the new modular
 * format handlers. It includes:
 *
 * - SBOM component representation and comparison
 * - Unified comparator that uses format handlers for parsing
 * - SBOM comparison and difference detection
 * - SBOM merging capabilities
 * - Diff report generation in multiple formats
 * - Factory pattern for creating comparators
 *
 * Supported SBOM Formats:
 * - SPDX 2.3, 3.0.0, and 3.0.1
 * - CycloneDX 1.4, 1.5, and 1.6
 *
 * Output Formats:
 * - Text reports
 * - JSON reports
 * - CSV reports
 */

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "../compat/compatibility.hpp"
#include "SBOMFormats.hpp"

namespace heimdall
{

/**
 * @brief Represents a component in an SBOM
 */
struct SBOMComponent
{
   std::string                        id;
   std::string                        bomRef;  // BOM reference for CycloneDX compatibility
   std::string                        name;
   std::string                        version;
   std::string                        type;
   std::string                        purl;
   std::string                        license;
   std::string                        description;   // Component description
   std::string                        scope;         // Component scope (required/optional/excluded)
   std::string                        group;         // Component group/organization
   std::string                        mimeType;      // MIME type of the component
   std::string                        copyright;     // Copyright information
   std::string                        cpe;           // Common Platform Enumeration identifier
   std::string                        supplier;      // Organization that supplied the component
   std::string                        manufacturer;  // Organization that created the component
   std::string                        publisher;     // Organization that published the component
   std::map<std::string, std::string> properties;
   std::vector<std::string>           dependencies;        // BOM references to dependent components
   std::vector<std::string>           externalReferences;  // External reference URLs

   // Default constructor
   SBOMComponent() = default;

   // Move constructor
   SBOMComponent(SBOMComponent&& other) noexcept = default;

   // Move assignment operator
   SBOMComponent& operator=(SBOMComponent&& other) noexcept = default;

   // Copy constructor
   SBOMComponent(const SBOMComponent& other) = default;

   // Copy assignment operator
   SBOMComponent& operator=(const SBOMComponent& other) = default;

   /**
    * @brief Construct a new SBOMComponent with all fields
    * @param id The component ID
    * @param bomRef The BOM reference
    * @param name The component name
    * @param version The component version
    * @param type The component type
    * @param purl The package URL
    * @param license The license
    * @param description The component description
    * @param scope The component scope
    * @param group The component group
    * @param mimeType The MIME type
    * @param copyright The copyright information
    * @param cpe The CPE identifier
    * @param supplier The supplier organization
    * @param manufacturer The manufacturer organization
    * @param publisher The publisher organization
    * @param properties Additional properties
    * @param dependencies List of BOM references to dependencies
    * @param externalReferences List of external reference URLs
    */
   SBOMComponent(std::string id, std::string bomRef, std::string name, std::string version,
                 std::string type, std::string purl, std::string license,
                 std::string description = "", std::string scope = "required",
                 std::string group = "", std::string mimeType = "", std::string copyright = "",
                 std::string cpe = "", std::string supplier = "", std::string manufacturer = "",
                 std::string                               publisher          = "",
                 const std::map<std::string, std::string>& properties         = {},
                 const std::vector<std::string>&           dependencies       = {},
                 const std::vector<std::string>&           externalReferences = {})
      : id(std::move(id)),
        bomRef(std::move(bomRef)),
        name(std::move(name)),
        version(std::move(version)),
        type(std::move(type)),
        purl(std::move(purl)),
        license(std::move(license)),
        description(std::move(description)),
        scope(std::move(scope)),
        group(std::move(group)),
        mimeType(std::move(mimeType)),
        copyright(std::move(copyright)),
        cpe(std::move(cpe)),
        supplier(std::move(supplier)),
        manufacturer(std::move(manufacturer)),
        publisher(std::move(publisher)),
        properties(properties),
        dependencies(dependencies),
        externalReferences(externalReferences)
   {
   }

   // Hash for comparison
   std::string getHash() const;

   // Compare with another component
   bool operator==(const SBOMComponent& other) const;
   bool operator!=(const SBOMComponent& other) const;
};

/**
 * @brief Represents a difference between two SBOMs
 */
struct SBOMDifference
{
   enum class Type
   {
      ADDED,     // Component added in new SBOM
      REMOVED,   // Component removed from old SBOM
      MODIFIED,  // Component modified between SBOMs
      UNCHANGED  // Component unchanged
   };

   Type                                      type;
   SBOMComponent                             component;
   std::optional<SBOMComponent>              oldComponent;

   // Default constructor
   SBOMDifference() = default;

   // Move constructor
   SBOMDifference(SBOMDifference&& other) noexcept = default;

   // Move assignment operator
   SBOMDifference& operator=(SBOMDifference&& other) noexcept = default;

   // Copy constructor
   SBOMDifference(const SBOMDifference& other) = default;

   // Copy assignment operator
   SBOMDifference& operator=(const SBOMDifference& other) = default;

   // Parameterized constructors
   SBOMDifference(Type t, SBOMComponent comp) : type(t), component(std::move(comp)) {}
   SBOMDifference(Type t, SBOMComponent comp, const SBOMComponent& oldComp)
      : type(t), component(std::move(comp)), oldComponent(oldComp)
   {
   }
};

/**
 * @brief Unified SBOM comparator that uses format handlers
 *
 * This comparator uses the SBOMFormatFactory to create appropriate format handlers
 * and delegates parsing to them. It automatically detects the format and version
 * from the content.
 */
class UnifiedSBOMComparator
{
   public:
   /**
    * @brief Default constructor
    */
   UnifiedSBOMComparator() = default;

   /**
    * @brief Destructor
    */
   ~UnifiedSBOMComparator() = default;

   /**
    * @brief Compare two SBOM files
    * @param oldSBOM Path to the old SBOM file
    * @param newSBOM Path to the new SBOM file
    * @return Vector of differences between the SBOMs
    */
   std::vector<SBOMDifference> compare(const std::string& oldSBOM, const std::string& newSBOM);

   /**
    * @brief Compare two SBOM contents
    * @param oldContent Old SBOM content
    * @param newContent New SBOM content
    * @return Vector of differences between the SBOMs
    */
   std::vector<SBOMDifference> compareContent(const std::string& oldContent,
                                              const std::string& newContent);

   /**
    * @brief Merge multiple SBOM files
    * @param sbomFiles Vector of SBOM file paths
    * @param outputFormat Output format (spdx, cyclonedx)
    * @param outputVersion Output version
    * @return Merged SBOM content
    */
   std::string merge(const std::vector<std::string>& sbomFiles, const std::string& outputFormat,
                     const std::string& outputVersion = "");

   /**
    * @brief Generate a diff report
    * @param differences Vector of differences
    * @param format Report format (text, json, csv)
    * @return Generated report
    */
   std::string generateDiffReport(const std::vector<SBOMDifference>& differences,
                                  const std::string&                 format = "text");

   /**
    * @brief Get statistics about differences
    * @param differences Vector of differences
    * @return Map of difference type to count
    */
   std::map<std::string, int> getDiffStatistics(const std::vector<SBOMDifference>& differences);

   private:
   /**
    * @brief Detect format from file
    * @param filePath Path to the SBOM file
    * @return Detected format name
    */
   std::string detectFormatFromFile(const std::string& filePath) const;

   /**
    * @brief Detect format from content
    * @param content SBOM content
    * @return Detected format name
    */
   std::string detectFormatFromContent(const std::string& content) const;

   /**
    * @brief Create format handler
    * @param format Format name
    * @return Unique pointer to format handler
    */
   std::unique_ptr<ISBOMFormatHandler> createHandler(const std::string& format) const;

   /**
    * @brief Compare components from two SBOMs
    * @param oldComponents Components from old SBOM
    * @param newComponents Components from new SBOM
    * @return Vector of differences
    */
   std::vector<SBOMDifference> compareComponents(const std::vector<ComponentInfo>& oldComponents,
                                                 const std::vector<ComponentInfo>& newComponents);

   /**
    * @brief Merge multiple component lists
    * @param componentLists List of component lists to merge
    * @return Merged component list
    */
   std::vector<ComponentInfo> mergeComponents(
      const std::vector<std::vector<ComponentInfo>>& componentLists);

   /**
    * @brief Convert ComponentInfo to SBOMComponent
    * @param component ComponentInfo to convert
    * @return SBOMComponent
    */
   SBOMComponent convertToSBOMComponent(const ComponentInfo& component) const;

   /**
    * @brief Convert SBOMComponent to ComponentInfo
    * @param component SBOMComponent to convert
    * @return ComponentInfo
    */
   ComponentInfo convertToComponentInfo(const SBOMComponent& component) const;

   /**
    * @brief Generate SPDX output
    * @param components Components to include
    * @param format Output format
    * @param version Output version
    * @return Generated SPDX content
    */
   std::string generateSPDXOutput(const std::vector<ComponentInfo>& components,
                                  const std::string& format, const std::string& version);

   /**
    * @brief Generate CycloneDX output
    * @param components Components to include
    * @param format Output format
    * @param version Output version
    * @return Generated CycloneDX content
    */
   std::string generateCycloneDXOutput(const std::vector<ComponentInfo>& components,
                                       const std::string& format, const std::string& version);

   /**
    * @brief Generate JSON report
    * @param differences Vector of differences
    * @return JSON report
    */
   std::string generateJSONReport(const std::vector<SBOMDifference>& differences);

   /**
    * @brief Generate CSV report
    * @param differences Vector of differences
    * @return CSV report
    */
   std::string generateCSVReport(const std::vector<SBOMDifference>& differences);

   /**
    * @brief Generate text report
    * @param differences Vector of differences
    * @return Text report
    */
   std::string generateTextReport(const std::vector<SBOMDifference>& differences);

   /**
    * @brief Get string representation of difference type
    * @param type Difference type
    * @return String representation
    */
   std::string getDifferenceTypeString(SBOMDifference::Type type);

   /**
    * @brief Get lowercase string representation of difference type
    * @param type Difference type
    * @return Lowercase string representation
    */
   std::string getDifferenceTypeStringLowercase(SBOMDifference::Type type);

   /**
    * @brief Get current timestamp
    * @return Current timestamp string
    */
   std::string getCurrentTimestamp();
};

/**
 * @brief Factory for creating SBOM comparators
 *
 * This factory provides a unified interface for creating comparators.
 * Currently, it creates UnifiedSBOMComparator instances.
 */
class SBOMComparatorFactory
{
   public:
   /**
    * @brief Create a comparator for the specified format
    * @param format Format name (spdx, cyclonedx, or empty for auto-detection)
    * @return Unique pointer to the created comparator
    */
   static std::unique_ptr<UnifiedSBOMComparator> createComparator(const std::string& format = "");

   /**
    * @brief Get supported formats
    * @return Vector of supported format names
    */
   static std::vector<std::string> getSupportedFormats();

   /**
    * @brief Get supported versions for a format
    * @param format Format name
    * @return Vector of supported versions
    */
   static std::vector<std::string> getSupportedVersions(const std::string& format);
};

// Type alias for backward compatibility
using SBOMComparator = UnifiedSBOMComparator;

}  // namespace heimdall
