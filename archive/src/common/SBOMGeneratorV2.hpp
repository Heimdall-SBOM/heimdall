/**
 * @file SBOMGeneratorV2.hpp
 * @brief Refactored SBOM generator with clean format separation
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a clean, modular SBOM generator that separates concerns
 * between different SBOM formats and provides a unified interface for SBOM generation.
 */

#pragma once

#include "SBOMFormats.hpp"
#include "ComponentInfo.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace heimdall
{

/**
 * @brief Refactored SBOM generator with clean format separation
 *
 * This class provides a clean interface for generating SBOMs in various formats.
 * It uses the modular format handlers to separate concerns and make the code
 * more maintainable and extensible.
 */
class SBOMGeneratorV2
{
public:
    /**
     * @brief Default constructor
     */
    SBOMGeneratorV2();

    /**
     * @brief Destructor
     */
    ~SBOMGeneratorV2();

    /**
     * @brief Copy constructor
     */
    SBOMGeneratorV2(const SBOMGeneratorV2& other);

    /**
     * @brief Move constructor
     */
    SBOMGeneratorV2(SBOMGeneratorV2&& other) noexcept;

    /**
     * @brief Copy assignment operator
     */
    SBOMGeneratorV2& operator=(const SBOMGeneratorV2& other);

    /**
     * @brief Move assignment operator
     */
    SBOMGeneratorV2& operator=(SBOMGeneratorV2&& other) noexcept;

    /**
     * @brief Process a component and add it to the SBOM
     * @param component The component to process
     */
    void processComponent(const ComponentInfo& component);

    /**
     * @brief Generate the SBOM in the specified format
     * @return true if generation was successful
     */
    bool generateSBOM();

    /**
     * @brief Generate SBOM content without writing to file
     * @return Generated SBOM content as string
     */
    std::string generateSBOMContent();

    /**
     * @brief Set the output path for the SBOM file
     * @param path The output file path
     */
    void setOutputPath(const std::string& path);

    /**
     * @brief Set the output format for the SBOM
     * @param format The format (e.g., "spdx", "cyclonedx")
     */
    void setFormat(const std::string& format);

    /**
     * @brief Set the SPDX version for the SBOM
     * @param version The SPDX version (e.g., "2.3", "3.0.0", "3.0.1")
     */
    void setSPDXVersion(const std::string& version);

    /**
     * @brief Set the CycloneDX version for the SBOM
     * @param version The CycloneDX version (e.g., "1.4", "1.5", "1.6")
     */
    void setCycloneDXVersion(const std::string& version);

    /**
     * @brief Set whether to suppress warnings (for test mode)
     * @param suppress true to suppress warnings
     */
    void setSuppressWarnings(bool suppress);

    /**
     * @brief Set whether to recursively include transitive dependencies
     * @param transitive true to include transitive dependencies, false for direct only
     */
    void setTransitiveDependencies(bool transitive);

    /**
     * @brief Add metadata to the SBOM
     * @param key Metadata key
     * @param value Metadata value
     */
    void addMetadata(const std::string& key, const std::string& value);

    /**
     * @brief Get the number of components in the SBOM
     * @return Number of components
     */
    [[nodiscard]] size_t getComponentCount() const;

    /**
     * @brief Check if a component exists in the SBOM
     * @param name The component name to check
     * @return true if the component exists
     */
    [[nodiscard]] bool hasComponent(const std::string& name) const;

    /**
     * @brief Print statistics about the SBOM
     */
    void printStatistics() const;

    /**
     * @brief Validate the generated SBOM
     * @return Validation result
     */
    ValidationResult validateSBOM() const;

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

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace heimdall 