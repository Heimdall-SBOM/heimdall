#pragma once

#include "ComponentInfo.hpp"
#include "SBOMGenerator.hpp"
#include <string>
#include <vector>
#include <memory>

namespace heimdall {

// Common plugin interface for both LLD and Gold
class PluginInterface {
public:
    PluginInterface();
    virtual ~PluginInterface();
    
    // Plugin lifecycle
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    
    // Component processing
    virtual void processInputFile(const std::string& filePath) = 0;
    virtual void processLibrary(const std::string& libraryPath) = 0;
    virtual void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) = 0;
    
    // SBOM generation
    virtual void setOutputPath(const std::string& path) = 0;
    virtual void setFormat(const std::string& format) = 0;
    virtual void generateSBOM() = 0;
    
    // Configuration
    virtual void setVerbose(bool verbose) = 0;
    virtual void setExtractDebugInfo(bool extract) = 0;
    virtual void setIncludeSystemLibraries(bool include) = 0;
    
    // Statistics and reporting
    virtual size_t getComponentCount() const = 0;
    virtual void printStatistics() const = 0;
    
protected:
    std::unique_ptr<SBOMGenerator> sbomGenerator;
    std::vector<ComponentInfo> processedComponents;
    bool verbose = false;
    bool extractDebugInfo = true;
    bool includeSystemLibraries = false;
    
    // Helper methods
    void addComponent(const ComponentInfo& component);
    void updateComponent(const std::string& name, const std::string& filePath, 
                        const std::vector<SymbolInfo>& symbols);
    bool shouldProcessFile(const std::string& filePath) const;
    std::string extractComponentName(const std::string& filePath) const;
};

// Plugin configuration structure
struct PluginConfig {
    std::string outputPath = "heimdall-sbom.json";
    std::string format = "spdx";
    bool verbose = false;
    bool extractDebugInfo = true;
    bool includeSystemLibraries = false;
    bool generateChecksums = true;
    bool extractMetadata = true;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> includePatterns;
};

// Plugin statistics structure
struct PluginStatistics {
    size_t totalFiles = 0;
    size_t objectFiles = 0;
    size_t staticLibraries = 0;
    size_t sharedLibraries = 0;
    size_t executables = 0;
    size_t systemLibraries = 0;
    size_t totalSymbols = 0;
    size_t processedComponents = 0;
    size_t skippedFiles = 0;
    std::chrono::milliseconds processingTime{0};
};

// Common plugin utilities
namespace PluginUtils {
    
    // File type detection
    bool isObjectFile(const std::string& filePath);
    bool isStaticLibrary(const std::string& filePath);
    bool isSharedLibrary(const std::string& filePath);
    bool isExecutable(const std::string& filePath);
    
    // Path utilities
    std::string normalizeLibraryPath(const std::string& libraryPath);
    std::string resolveLibraryPath(const std::string& libraryName);
    std::vector<std::string> getLibrarySearchPaths();
    
    // Symbol utilities
    bool isSystemSymbol(const std::string& symbolName);
    bool isWeakSymbol(const std::string& symbolName);
    std::string extractSymbolVersion(const std::string& symbolName);
    
    // Configuration utilities
    bool loadConfigFromFile(const std::string& configPath, PluginConfig& config);
    bool saveConfigToFile(const std::string& configPath, const PluginConfig& config);
    bool parseCommandLineOptions(int argc, char* argv[], PluginConfig& config);
    
    // Logging utilities
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    void logDebug(const std::string& message);
    
} // namespace PluginUtils

} // namespace heimdall
