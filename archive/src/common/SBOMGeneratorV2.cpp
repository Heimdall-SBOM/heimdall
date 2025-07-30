/**
 * @file SBOMGeneratorV2.cpp
 * @brief Implementation of refactored SBOM generator with clean format separation
 * @author Trevor Bakker
 * @date 2025
 */

#include "SBOMGeneratorV2.hpp"
#include "SBOMFormats.hpp"
#include "Utils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace heimdall
{

class SBOMGeneratorV2::Impl
{
public:
    std::unordered_map<std::string, ComponentInfo> components;
    std::string outputPath;
    std::string format = "spdx";
    std::string spdxVersion = "2.3";
    std::string cyclonedxVersion = "1.6";
    bool suppressWarnings = false;
    bool transitiveDependencies = true;
    std::map<std::string, std::string> metadata;
    std::unique_ptr<ISBOMFormatHandler> formatHandler;

    Impl() = default;
    ~Impl() = default;

    void createFormatHandler();
    void processDependenciesRecursively(const ComponentInfo& component, std::set<std::string>& processedKeys);
};

// SBOMGeneratorV2 implementation
SBOMGeneratorV2::SBOMGeneratorV2() : pImpl(std::make_unique<Impl>()) {}

SBOMGeneratorV2::~SBOMGeneratorV2() = default;

SBOMGeneratorV2::SBOMGeneratorV2(const SBOMGeneratorV2& other) : pImpl(std::make_unique<Impl>(*other.pImpl)) {}

SBOMGeneratorV2::SBOMGeneratorV2(SBOMGeneratorV2&& other) noexcept : pImpl(std::move(other.pImpl)) {}

SBOMGeneratorV2& SBOMGeneratorV2::operator=(const SBOMGeneratorV2& other)
{
    if (this != &other)
    {
        pImpl = std::make_unique<Impl>(*other.pImpl);
    }
    return *this;
}

SBOMGeneratorV2& SBOMGeneratorV2::operator=(SBOMGeneratorV2&& other) noexcept
{
    if (this != &other)
    {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

void SBOMGeneratorV2::processComponent(const ComponentInfo& component)
{
    // Use canonical file path as unique key
    std::string canonicalPath = Utils::resolveLibraryPath(component.filePath);
    std::string key = canonicalPath;

    // Skip if component already processed
    if (pImpl->components.find(key) != pImpl->components.end())
    {
        return;
    }

    // Add component to map
    pImpl->components[key] = component;

    // Process dependencies if enabled
    if (pImpl->transitiveDependencies)
    {
        std::set<std::string> processedKeys;
        pImpl->processDependenciesRecursively(component, processedKeys);
    }
}

bool SBOMGeneratorV2::generateSBOM()
{
    if (pImpl->components.empty())
    {
        Utils::errorPrint("No components to generate SBOM from");
        return false;
    }

    if (pImpl->outputPath.empty())
    {
        Utils::errorPrint("No output path specified");
        return false;
    }

    // Create format handler if not already created
    pImpl->createFormatHandler();
    if (!pImpl->formatHandler)
    {
        Utils::errorPrint("Failed to create format handler for: " + pImpl->format);
        return false;
    }

    // Generate SBOM content
    std::string content = pImpl->formatHandler->generateSBOM(pImpl->components, pImpl->metadata);

    // Write to file
    std::ofstream file(pImpl->outputPath);
    if (!file.is_open())
    {
        Utils::errorPrint("Could not open output file: " + pImpl->outputPath);
        return false;
    }

    file << content;
    file.close();

    Utils::debugPrint("SBOM generated successfully: " + pImpl->outputPath);
    return true;
}

std::string SBOMGeneratorV2::generateSBOMContent()
{
    if (pImpl->components.empty())
    {
        Utils::errorPrint("No components to generate SBOM from");
        return "";
    }

    // Create format handler if not already created
    pImpl->createFormatHandler();
    if (!pImpl->formatHandler)
    {
        Utils::errorPrint("Failed to create format handler for: " + pImpl->format);
        return "";
    }

    return pImpl->formatHandler->generateSBOM(pImpl->components, pImpl->metadata);
}

void SBOMGeneratorV2::setOutputPath(const std::string& path)
{
    pImpl->outputPath = path;
}

void SBOMGeneratorV2::setFormat(const std::string& format)
{
    pImpl->format = Utils::toLower(format);
    pImpl->formatHandler.reset(); // Reset handler to recreate with new format
}

void SBOMGeneratorV2::setSPDXVersion(const std::string& version)
{
    pImpl->spdxVersion = version;
    if (pImpl->format == "spdx")
    {
        pImpl->formatHandler.reset(); // Reset handler to recreate with new version
    }
}

void SBOMGeneratorV2::setCycloneDXVersion(const std::string& version)
{
    pImpl->cyclonedxVersion = version;
    if (pImpl->format == "cyclonedx")
    {
        pImpl->formatHandler.reset(); // Reset handler to recreate with new version
    }
}

void SBOMGeneratorV2::setSuppressWarnings(bool suppress)
{
    pImpl->suppressWarnings = suppress;
}

void SBOMGeneratorV2::setTransitiveDependencies(bool transitive)
{
    pImpl->transitiveDependencies = transitive;
}

void SBOMGeneratorV2::addMetadata(const std::string& key, const std::string& value)
{
    pImpl->metadata[key] = value;
}

size_t SBOMGeneratorV2::getComponentCount() const
{
    return pImpl->components.size();
}

bool SBOMGeneratorV2::hasComponent(const std::string& name) const
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

void SBOMGeneratorV2::printStatistics() const
{
    std::cout << "SBOM Statistics:\n";
    std::cout << "  Format: " << pImpl->format << "\n";
    std::cout << "  Components: " << pImpl->components.size() << "\n";
    std::cout << "  Transitive Dependencies: " << (pImpl->transitiveDependencies ? "enabled" : "disabled") << "\n";
    
    if (pImpl->format == "spdx")
    {
        std::cout << "  SPDX Version: " << pImpl->spdxVersion << "\n";
    }
    else if (pImpl->format == "cyclonedx")
    {
        std::cout << "  CycloneDX Version: " << pImpl->cyclonedxVersion << "\n";
    }

    std::cout << "  Metadata entries: " << pImpl->metadata.size() << "\n";
}

ValidationResult SBOMGeneratorV2::validateSBOM() const
{
    if (!pImpl->formatHandler)
    {
        ValidationResult result;
        result.addError("No format handler available for validation");
        return result;
    }

    std::string content = generateSBOMContent();
    if (content.empty())
    {
        ValidationResult result;
        result.addError("Failed to generate SBOM content for validation");
        return result;
    }

    return pImpl->formatHandler->validateContent(content);
}

std::vector<std::string> SBOMGeneratorV2::getSupportedFormats()
{
    return SBOMFormatFactory::getSupportedFormats();
}

std::vector<std::string> SBOMGeneratorV2::getSupportedVersions(const std::string& format)
{
    return SBOMFormatFactory::getSupportedVersions(format);
}

// Impl methods
void SBOMGeneratorV2::Impl::createFormatHandler()
{
    if (formatHandler)
    {
        return; // Already created
    }

    if (format == "spdx")
    {
        auto spdxHandler = SBOMFormatFactory::createSPDXHandler(spdxVersion);
        formatHandler = std::move(spdxHandler);
    }
    else if (format == "cyclonedx")
    {
        auto cyclonedxHandler = SBOMFormatFactory::createCycloneDXHandler(cyclonedxVersion);
        formatHandler = std::move(cyclonedxHandler);
    }
    else
    {
        formatHandler = SBOMFormatFactory::createHandler(format);
    }
}

void SBOMGeneratorV2::Impl::processDependenciesRecursively(const ComponentInfo& component, std::set<std::string>& processedKeys)
{
    for (const auto& depPath : component.dependencies)
    {
        std::string resolvedPath = depPath;

        // Handle @rpath dependencies (resolve relative to app bundle)
        if (depPath.find("@rpath/") == 0)
        {
            std::string appDir = component.filePath;
            size_t lastSlash = appDir.rfind('/');
            if (lastSlash != std::string::npos)
            {
                appDir = appDir.substr(0, lastSlash);
                resolvedPath = appDir + "/" + depPath.substr(7); // Remove "@rpath/"
            }
        }

        // Resolve library paths to canonical absolute paths
        std::string canonicalPath = Utils::resolveLibraryPath(resolvedPath);
        std::string depKey = canonicalPath;

        // Skip if already processed
        if (processedKeys.find(depKey) != processedKeys.end() || components.find(depKey) != components.end())
        {
            continue;
        }

        processedKeys.insert(depKey);

        // Create component for dependency
        ComponentInfo depComponent(Utils::getFileName(resolvedPath), resolvedPath);
        components[depKey] = depComponent;

        // Recursively process dependencies of this dependency
        processDependenciesRecursively(depComponent, processedKeys);
    }
}

} // namespace heimdall 