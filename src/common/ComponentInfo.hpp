#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace heimdall {

enum class FileType {
    Unknown,
    Object,
    StaticLibrary,
    SharedLibrary,
    Executable
};

enum class LinkerType {
    LLD,
    Gold,
    BFD,
    Unknown
};

struct SymbolInfo {
    std::string name;
    uint64_t address = 0;
    uint64_t size = 0;
    bool isDefined = false;
    bool isWeak = false;
    bool isGlobal = false;
    std::string section;
};

struct SectionInfo {
    std::string name;
    uint64_t address = 0;
    uint64_t size = 0;
    uint32_t flags = 0;
    std::string type;
};

struct ComponentInfo {
    std::string name;
    std::string filePath;
    std::string version;
    std::string supplier;
    std::string downloadLocation;
    std::string homepage;
    std::string license;
    std::string checksum;
    std::string packageManager;
    FileType fileType = FileType::Unknown;
    uint64_t fileSize = 0;
    
    std::vector<SymbolInfo> symbols;
    std::vector<SectionInfo> sections;
    std::vector<std::string> dependencies;
    std::vector<std::string> sourceFiles;
    
    bool wasProcessed = false;
    std::string processingError;
    LinkerType detectedBy = LinkerType::Unknown;
    
    bool isSystemLibrary = false;
    bool containsDebugInfo = false;
    bool isStripped = false;
    
    // Constructors
    ComponentInfo() = default;
    ComponentInfo(const std::string& componentName, const std::string& path);
    
    // Methods
    void addSymbol(const SymbolInfo& symbol) {
        symbols.push_back(symbol);
    }
    
    void addSection(const SectionInfo& section) {
        sections.push_back(section);
    }
    
    void addDependency(const std::string& dependency);
    void addSourceFile(const std::string& sourceFile);
    void setVersion(const std::string& ver);
    void setSupplier(const std::string& sup);
    void setDownloadLocation(const std::string& location);
    void setHomepage(const std::string& page);
    void setLicense(const std::string& lic);
    void setPackageManager(const std::string& pkgMgr);
    void markAsProcessed();
    void setProcessingError(const std::string& error);
    void setDetectedBy(LinkerType linker);
    void markAsSystemLibrary();
    void setContainsDebugInfo(bool hasDebug);
    void setStripped(bool stripped);
    
    bool hasSymbol(const std::string& symbolName) const;
    bool hasSection(const std::string& sectionName) const;
    std::string getFileTypeString() const;
    std::string getLinkerTypeString() const;
    
    size_t getSymbolCount() const { return symbols.size(); }
    size_t getSectionCount() const { return sections.size(); }
};

struct BuildInfo {
    std::string targetName;
    std::string targetType;
    std::string buildId;
    std::string buildTimestamp;
    std::string compiler;
    std::string compilerVersion;
    std::string architecture;
    std::string operatingSystem;
    LinkerType linkerUsed = LinkerType::Unknown;
    std::string linkerVersion;
    std::vector<std::string> linkFlags;
    std::vector<std::string> libraryPaths;
};

} // namespace heimdall
