// Heimdall Metadata Extractor

#pragma once

#include "ComponentInfo.hpp"
#include <string>
#include <vector>
#include <memory>

namespace heimdall {

class MetadataExtractor {
public:
    MetadataExtractor();
    ~MetadataExtractor();
    
    // Main extraction method
    bool extractMetadata(ComponentInfo& component);
    
    // Specific extraction methods
    bool extractVersionInfo(ComponentInfo& component);
    bool extractLicenseInfo(ComponentInfo& component);
    bool extractSymbolInfo(ComponentInfo& component);
    bool extractSectionInfo(ComponentInfo& component);
    bool extractDebugInfo(ComponentInfo& component);
    bool extractDependencyInfo(ComponentInfo& component);
    
    // File format detection
    bool isELF(const std::string& filePath);
    bool isMachO(const std::string& filePath);
    bool isPE(const std::string& filePath);
    bool isArchive(const std::string& filePath);
    
    // Package manager specific extraction
    bool extractConanMetadata(ComponentInfo& component);
    bool extractVcpkgMetadata(ComponentInfo& component);
    bool extractSystemMetadata(ComponentInfo& component);
    
    // Utility methods
    void setVerbose(bool verbose);
    void setExtractDebugInfo(bool extract);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// Helper functions for specific file formats
namespace MetadataHelpers {
    
    // ELF format helpers
    bool extractELFSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
    bool extractELFSections(const std::string& filePath, std::vector<SectionInfo>& sections);
    bool extractELFVersion(const std::string& filePath, std::string& version);
    bool extractELFBuildId(const std::string& filePath, std::string& buildId);
    
    // Mach-O format helpers
    bool isMachO(const std::string& filePath);
    bool extractMachOSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
    bool extractMachOSections(const std::string& filePath, std::vector<SectionInfo>& sections);
    bool extractMachOVersion(const std::string& filePath, std::string& version);
    bool extractMachOUUID(const std::string& filePath, std::string& uuid);
    std::vector<std::string> extractMachOLinkedLibraries(const std::string& filePath);
    
    // PE format helpers
    bool extractPESymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
    bool extractPESections(const std::string& filePath, std::vector<SectionInfo>& sections);
    bool extractPEVersion(const std::string& filePath, std::string& version);
    bool extractPECompanyName(const std::string& filePath, std::string& company);
    
    // Archive format helpers
    bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members);
    bool extractArchiveSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);
    
    // Debug info extraction
    bool extractDebugInfo(const std::string& filePath, ComponentInfo& component);
    bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles);
    bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& units);
    
    // License detection
    std::string detectLicenseFromFile(const std::string& filePath);
    std::string detectLicenseFromPath(const std::string& filePath);
    std::string detectLicenseFromSymbols(const std::vector<SymbolInfo>& symbols);
    
    // Version detection
    std::string detectVersionFromFile(const std::string& filePath);
    std::string detectVersionFromPath(const std::string& filePath);
    std::string detectVersionFromSymbols(const std::vector<SymbolInfo>& symbols);
    
    // Dependency detection
    std::vector<std::string> detectDependencies(const std::string& filePath);
    std::vector<std::string> extractDynamicDependencies(const std::string& filePath);
    std::vector<std::string> extractStaticDependencies(const std::string& filePath);
    
} // namespace MetadataHelpers

} // namespace heimdall
