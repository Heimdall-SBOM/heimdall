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
 * @brief SBOM comparison, merging, and diff generation functionality
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides comprehensive functionality for comparing, merging, and
 * analyzing Software Bill of Materials (SBOM) documents. It includes:
 *
 * - SBOM component representation and comparison
 * - Abstract parser interface for different SBOM formats
 * - Concrete implementations for SPDX and CycloneDX parsers
 * - SBOM comparison and difference detection
 * - SBOM merging capabilities
 * - Diff report generation in multiple formats
 * - Factory pattern for creating parsers
 *
 * Supported SBOM Formats:
 * - SPDX 2.3 and 3.0
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
#include <vector>
#include "../compat/compatibility.hpp"

namespace heimdall
{

/**
 * @brief Represents a component in an SBOM
 */
struct SBOMComponent
{
  std::string                        id;
  std::string                        name;
  std::string                        version;
  std::string                        type;
  std::string                        purl;
  std::string                        license;
  std::map<std::string, std::string> properties;
  std::vector<std::string>           dependencies;

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
   * @param name The component name
   * @param version The component version
   * @param type The component type
   * @param purl The package URL
   * @param license The license
   * @param properties Additional properties
   * @param dependencies List of dependencies
   */
  SBOMComponent(const std::string& id, const std::string& name, const std::string& version,
                const std::string& type, const std::string& purl, const std::string& license,
                const std::map<std::string, std::string>& properties   = {},
                const std::vector<std::string>&           dependencies = {})
    : id(id),
      name(name),
      version(version),
      type(type),
      purl(purl),
      license(license),
      properties(properties),
      dependencies(dependencies)
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
  heimdall::compat::optional<SBOMComponent> oldComponent;

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
  SBOMDifference(Type t, const SBOMComponent& comp) : type(t), component(comp) {}
  SBOMDifference(Type t, const SBOMComponent& comp, const SBOMComponent& oldComp)
    : type(t), component(comp), oldComponent(oldComp)
  {
  }
};

/**
 * @brief Abstract base class for SBOM parsers
 */
class SBOMParser
{
  public:
  virtual ~SBOMParser() = default;

  /**
   * @brief Parse SBOM file and extract components
   * @param filePath Path to SBOM file
   * @return Vector of components
   */
  virtual std::vector<SBOMComponent> parse(const std::string& filePath) = 0;

  /**
   * @brief Parse SBOM content from string
   * @param content SBOM content
   * @return Vector of components
   */
  virtual std::vector<SBOMComponent> parseContent(const std::string& content) = 0;

  /**
   * @brief Get parser name
   * @return Parser name
   */
  virtual std::string getName() const = 0;
};

/**
 * @brief SPDX parser implementation
 */
class SPDXParser : public SBOMParser
{
  public:
  std::vector<SBOMComponent> parse(const std::string& filePath) override;
  std::vector<SBOMComponent> parseContent(const std::string& content) override;
  std::string                getName() const override
  {
    return "SPDX Parser";
  }

  private:
  bool                       processSPDXLine(const std::string& line, SBOMComponent& component);
  std::vector<SBOMComponent> parseSPDX2_3(const std::string& content);
  std::vector<SBOMComponent> parseSPDX3_0(const std::string& content);
};

/**
 * @brief CycloneDX parser implementation
 */
class CycloneDXParser : public SBOMParser
{
  public:
  std::vector<SBOMComponent> parse(const std::string& filePath) override;
  std::vector<SBOMComponent> parseContent(const std::string& content) override;
  std::string                getName() const override
  {
    return "CycloneDX Parser";
  }

  private:
  std::string                extractVersion(const std::string& content) const;
  std::vector<SBOMComponent> parseCycloneDX1_4(const std::string& content) const;
  std::vector<SBOMComponent> parseCycloneDX1_5(const std::string& content) const;
  std::vector<SBOMComponent> parseCycloneDX1_6(const std::string& content) const;
};

/**
 * @brief SBOM comparison and merging functionality
 */
class SBOMComparator
{
  public:
  /**
   * @brief Compare two SBOM files
   * @param oldSBOM Path to old SBOM file
   * @param newSBOM Path to new SBOM file
   * @return Vector of differences
   */
  std::vector<SBOMDifference> compare(const std::string& oldSBOM, const std::string& newSBOM);

  /**
   * @brief Compare two SBOM contents
   * @param oldContent Old SBOM content
   * @param newContent New SBOM content
   * @param format SBOM format ("spdx" or "cyclonedx")
   * @return Vector of differences
   */
  std::vector<SBOMDifference> compareContent(const std::string& oldContent,
                                             const std::string& newContent,
                                             const std::string& format);

  /**
   * @brief Merge multiple SBOMs into one
   * @param sbomFiles Vector of SBOM file paths
   * @param outputFormat Output format ("spdx" or "cyclonedx")
   * @param outputVersion Output version
   * @return Merged SBOM content
   */
  std::string merge(const std::vector<std::string>& sbomFiles, const std::string& outputFormat,
                    const std::string& outputVersion);

  /**
   * @brief Generate diff report
   * @param differences Vector of differences
   * @param format Output format ("text", "json", "csv")
   * @return Diff report
   */
  std::string generateDiffReport(const std::vector<SBOMDifference>& differences,
                                 const std::string&                 format = "text");

  /**
   * @brief Get statistics about differences
   * @param differences Vector of differences
   * @return Map of statistics
   */
  std::map<std::string, int> getDiffStatistics(const std::vector<SBOMDifference>& differences);

  private:
  std::string                 detectFormatFromFile(const std::string& filePath);
  std::unique_ptr<SBOMParser> createParser(const std::string& format);
  std::vector<SBOMDifference> compareComponents(const std::vector<SBOMComponent>& oldComponents,
                                                const std::vector<SBOMComponent>& newComponents);
  std::string mergeComponents(const std::vector<std::vector<SBOMComponent>>& componentLists,
                              const std::string& outputFormat, const std::string& outputVersion);
  std::string generateSPDXOutput(const std::vector<SBOMComponent>& components,
                                 const std::string&                version);
  std::string generateCycloneDXOutput(const std::vector<SBOMComponent>& components,
                                      const std::string&                version);
  std::string generateJSONReport(const std::vector<SBOMDifference>& differences);
  std::string generateCSVReport(const std::vector<SBOMDifference>& differences);
  std::string generateTextReport(const std::vector<SBOMDifference>& differences);
  std::string getDifferenceTypeString(SBOMDifference::Type type);
  std::string getCurrentTimestamp();
};

/**
 * @brief Factory class for creating SBOM parsers
 */
class SBOMParserFactory
{
  public:
  /**
   * @brief Create a parser for the given format
   * @param format SBOM format ("spdx" or "cyclonedx")
   * @return Unique pointer to the created parser, or nullptr if format is not supported
   */
  static std::unique_ptr<SBOMParser> createParser(const std::string& format);

  /**
   * @brief Get list of supported SBOM formats
   * @return Vector of supported format names
   */
  static std::vector<std::string> getSupportedFormats();
};

}  // namespace heimdall