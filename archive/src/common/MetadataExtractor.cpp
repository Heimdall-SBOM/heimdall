/**
 * @file MetadataExtractor.cpp
 * @brief Compatibility layer for MetadataExtractor delegating to MetadataExtractor
 * @author Trevor Bakker
 * @date 2025
 *
 * @deprecated This file provides a compatibility layer for gradual migration to MetadataExtractor.
 * All functionality is delegated to the new MetadataExtractor implementation.
 */

#include "MetadataExtractor.hpp"
#include "MetadataExtractor.hpp"
#include "../factories/BinaryFormatFactory.hpp"
#include "../extractors/AdaExtractor.hpp"
#include "Utils.hpp"

namespace heimdall
{

class MetadataExtractor::Impl
{
public:
    std::unique_ptr<MetadataExtractor> v2Extractor;
    
    Impl() : v2Extractor(std::make_unique<MetadataExtractor>()) {}
    ~Impl() = default;
};

// Constructor and destructor
MetadataExtractor::MetadataExtractor() : pImpl(heimdall::compat::make_unique<Impl>()) {}

MetadataExtractor::~MetadataExtractor() = default;

// Main extraction method
bool MetadataExtractor::extractMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractMetadata(component);
}

// Individual extraction methods - delegate to V2
bool MetadataExtractor::extractVersionInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractVersionMetadata(component);
}

bool MetadataExtractor::extractLicenseInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractLicenseMetadata(component);
}

bool MetadataExtractor::extractSymbolInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractSectionInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractDebugInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractDependencyInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractEnhancedMachOMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOCodeSignInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOBuildConfig(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOPlatformInfo(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOEntitlements(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOArchitectures(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

bool MetadataExtractor::extractMachOFrameworks(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractBinaryMetadata(component);
}

// File format detection methods
bool MetadataExtractor::isELF(const std::string& filePath)
{
    auto extractor = BinaryFormatFactory::createExtractor(filePath);
    return extractor && extractor->getFormatName() == "ELF";
}

bool MetadataExtractor::isMachO(const std::string& filePath)
{
    auto extractor = BinaryFormatFactory::createExtractor(filePath);
    return extractor && extractor->getFormatName() == "Mach-O";
}

bool MetadataExtractor::isPE(const std::string& filePath)
{
    auto extractor = BinaryFormatFactory::createExtractor(filePath);
    return extractor && extractor->getFormatName() == "PE";
}

bool MetadataExtractor::isArchive(const std::string& filePath)
{
    auto extractor = BinaryFormatFactory::createExtractor(filePath);
    return extractor && extractor->getFormatName() == "Archive";
}

// Package manager methods
bool MetadataExtractor::extractConanMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::extractVcpkgMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::extractSystemMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::extractMacOSAppBundleMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::detectRpmMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::detectDebianMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::detectPacmanMetadata(ComponentInfo& component)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

// Ada metadata methods
bool MetadataExtractor::extractAdaMetadata(ComponentInfo& component, const std::vector<std::string>& aliFiles)
{
    return pImpl->v2Extractor->extractPackageManagerMetadata(component);
}

bool MetadataExtractor::isAdaAliFile(const std::string& filePath)
{
    // Use AdaExtractor to check if file is an Ada ALI file
    AdaExtractor extractor;
    return extractor.canHandle(filePath);
}

bool MetadataExtractor::findAdaAliFiles(const std::string& directory, std::vector<std::string>& aliFiles)
{
    // Use AdaExtractor to find ALI files
    AdaExtractor extractor;
    // This is a simplified implementation - in practice, we would scan the directory
    // For now, return false to indicate no ALI files found
    return false;
}

// Batch extraction
bool MetadataExtractor::extractMetadataBatched(const std::vector<std::string>& filePaths,
                                               std::vector<ComponentInfo>& components)
{
    return pImpl->v2Extractor->extractMetadataBatched(filePaths, components);
}

// Configuration methods
void MetadataExtractor::setVerbose(bool verbose)
{
    pImpl->v2Extractor->setVerbose(verbose);
}

void MetadataExtractor::setExtractDebugInfo(bool extract)
{
    pImpl->v2Extractor->setExtractDebugInfo(extract);
}

void MetadataExtractor::setSuppressWarnings(bool suppress)
{
    pImpl->v2Extractor->setSuppressWarnings(suppress);
}

} // namespace heimdall

