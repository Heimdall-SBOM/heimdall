#include "GoldAdapter.hpp"
#include "../common/PluginInterface.hpp"
#include "../common/Utils.hpp"
#include <iostream>
#include <vector>
#include <string>

#ifdef __linux__
#include <plugin-api.h>
#include <bfd.h>
#include <elf-bfd.h>
#endif

namespace heimdall {

class GoldAdapterImpl : public PluginInterface {
public:
    GoldAdapterImpl();
    ~GoldAdapterImpl() override;
    
    // PluginInterface implementation
    bool initialize() override;
    void cleanup() override;
    void processInputFile(const std::string& filePath) override;
    void processLibrary(const std::string& libraryPath) override;
    void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) override;
    void setOutputPath(const std::string& path) override;
    void setFormat(const std::string& format) override;
    void generateSBOM() override;
    void setVerbose(bool verbose) override;
    void setExtractDebugInfo(bool extract) override;
    void setIncludeSystemLibraries(bool include) override;
    size_t getComponentCount() const override;
    void printStatistics() const override;
    
    // Gold-specific methods
    void handleGoldPlugin(const char* pluginName, const char* pluginOpt);
    void processGoldInputFile(const char* filePath);
    void processGoldLibrary(const char* libraryPath);
    
private:
    std::vector<std::string> processedFiles;
    std::vector<std::string> processedLibraries;
    std::vector<SymbolInfo> currentSymbols;
    std::string currentFile;
    
    // Helper methods
    void extractSymbolsFromFile(const std::string& filePath);
    void processArchiveFile(const std::string& filePath);
    void processObjectFile(const std::string& filePath);
    void processSharedLibrary(const std::string& filePath);
};

GoldAdapter::GoldAdapter() : pImpl(std::make_unique<GoldAdapterImpl>())
{
}

GoldAdapter::~GoldAdapter() = default;

void GoldAdapter::initialize()
{
    pImpl->initialize();
}

void GoldAdapter::processInputFile(const std::string& filePath)
{
    pImpl->processInputFile(filePath);
}

void GoldAdapter::finalize()
{
    pImpl->generateSBOM();
    pImpl->cleanup();
}

// GoldAdapterImpl implementation
GoldAdapterImpl::GoldAdapterImpl()
    : PluginInterface()
{
}

GoldAdapterImpl::~GoldAdapterImpl() = default;

bool GoldAdapterImpl::initialize()
{
    PluginUtils::logInfo("Heimdall Gold Plugin initialized");
    
    // Set default configuration
    setVerbose(false);
    setExtractDebugInfo(true);
    setIncludeSystemLibraries(false);
    setOutputPath("heimdall-sbom.json");
    setFormat("spdx");
    
    return true;
}

void GoldAdapterImpl::cleanup()
{
    PluginUtils::logInfo("Heimdall Gold Plugin cleanup");
    processedFiles.clear();
    processedLibraries.clear();
    currentSymbols.clear();
}

// Helper to set minimum SBOM fields for a component
static void setMinimumSBOMFields(ComponentInfo& component) {
    if (component.supplier.empty()) component.supplier = "Organization: UNKNOWN";
    if (component.downloadLocation.empty()) component.downloadLocation = "NOASSERTION";
    if (component.homepage.empty()) component.homepage = "N/A";
    if (component.version.empty()) component.version = "UNKNOWN";
    if (component.license.empty()) {
        std::string detectedLicense = Utils::detectLicenseFromName(component.name);
        if (detectedLicense == "NOASSERTION") {
            detectedLicense = Utils::detectLicenseFromPath(component.filePath);
        }
        component.license = detectedLicense;
    }
}

void GoldAdapterImpl::processInputFile(const std::string& filePath)
{
    if (std::find(processedFiles.begin(), processedFiles.end(), filePath) != processedFiles.end())
    {
        // Already processed this file
        return;
    }
    
    processedFiles.push_back(filePath);
    currentFile = filePath;
    
    if (verbose)
    {
        PluginUtils::logInfo("Processing input file: " + filePath);
    }
    
    // Extract component name
    std::string componentName = extractComponentName(filePath);
    
    // Create component info
    ComponentInfo component(componentName, filePath);
    component.setDetectedBy(LinkerType::Gold);
    
    // Extract symbols and other metadata
    extractSymbolsFromFile(filePath);
    for (const auto& symbol : currentSymbols)
    {
        component.addSymbol(symbol);
    }
    
    // Set minimum SBOM fields
    setMinimumSBOMFields(component);
    
    // Add component to SBOM
    addComponent(component);
    
    // Process linked libraries as components
    std::vector<std::string> deps = MetadataHelpers::detectDependencies(filePath);
    for (const auto& dep : deps) {
        std::string depPath;
        if (!dep.empty() && dep[0] == '/') {
            depPath = dep;
        } else {
            std::vector<std::string> libPaths = {"/usr/lib", "/usr/local/lib", "/opt/local/lib", "/opt/homebrew/lib"};
            for (const auto& libDir : libPaths) {
                std::string candidate = libDir + "/" + dep;
                if (Utils::fileExists(candidate)) {
                    depPath = candidate;
                    break;
                }
            }
        }
        if (!depPath.empty() && Utils::fileExists(depPath)) {
            ComponentInfo libComponent(Utils::getFileName(depPath), depPath);
            libComponent.setDetectedBy(LinkerType::Gold);
            libComponent.fileType = FileType::SharedLibrary;
            libComponent.checksum = Utils::calculateSHA256(depPath);
            libComponent.fileSize = Utils::getFileSize(depPath);
            setMinimumSBOMFields(libComponent);
            addComponent(libComponent);
        }
    }
    
    // Clear current symbols for next file
    currentSymbols.clear();
}

void GoldAdapterImpl::processLibrary(const std::string& libraryPath)
{
    if (std::find(processedLibraries.begin(), processedLibraries.end(), libraryPath) != processedLibraries.end())
    {
        // Already processed this library
        return;
    }
    
    processedLibraries.push_back(libraryPath);
    
    if (verbose)
    {
        PluginUtils::logInfo("Processing library: " + libraryPath);
    }
    
    // Extract component name
    std::string componentName = extractComponentName(libraryPath);
    
    // Create component info
    ComponentInfo component(componentName, libraryPath);
    component.setDetectedBy(LinkerType::Gold);
    
    // Determine file type and process accordingly
    if (PluginUtils::isStaticLibrary(libraryPath))
    {
        processArchiveFile(libraryPath);
    }
    else if (PluginUtils::isSharedLibrary(libraryPath))
    {
        processSharedLibrary(libraryPath);
    }
    
    // Add component to SBOM
    addComponent(component);
}

void GoldAdapterImpl::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size)
{
    SymbolInfo symbol;
    symbol.name = symbolName;
    symbol.address = address;
    symbol.size = size;
    symbol.isDefined = true;
    symbol.isGlobal = true;
    
    // Determine if it's a weak symbol
    symbol.isWeak = PluginUtils::isWeakSymbol(symbolName);
    
    // Add to current symbols
    currentSymbols.push_back(symbol);
    
    if (verbose)
    {
        PluginUtils::logDebug("Processed symbol: " + symbolName + " at " + std::to_string(address));
    }
}

void GoldAdapterImpl::setOutputPath(const std::string& path)
{
    sbomGenerator->setOutputPath(path);
}

void GoldAdapterImpl::setFormat(const std::string& format)
{
    sbomGenerator->setFormat(format);
}

void GoldAdapterImpl::generateSBOM()
{
    PluginUtils::logInfo("Generating SBOM with " + std::to_string(getComponentCount()) + " components");
    sbomGenerator->generateSBOM();
}

void GoldAdapterImpl::setVerbose(bool verbose)
{
    this->verbose = verbose;
    if (sbomGenerator)
    {
        // Note: SBOMGenerator doesn't have a setVerbose method in the current interface
        // This could be added if needed
    }
}

void GoldAdapterImpl::setExtractDebugInfo(bool extract)
{
    extractDebugInfo = extract;
}

void GoldAdapterImpl::setIncludeSystemLibraries(bool include)
{
    includeSystemLibraries = include;
}

size_t GoldAdapterImpl::getComponentCount() const
{
    return sbomGenerator->getComponentCount();
}

void GoldAdapterImpl::printStatistics() const
{
    PluginUtils::logInfo("Gold Plugin Statistics:");
    PluginUtils::logInfo("  Processed files: " + std::to_string(processedFiles.size()));
    PluginUtils::logInfo("  Processed libraries: " + std::to_string(processedLibraries.size()));
    PluginUtils::logInfo("  Total components: " + std::to_string(getComponentCount()));
    
    sbomGenerator->printStatistics();
}

void GoldAdapterImpl::handleGoldPlugin(const char* pluginName, const char* pluginOpt)
{
    if (!pluginName || !pluginOpt)
    {
        return;
    }
    
    std::string name(pluginName);
    std::string opt(pluginOpt);
    
    if (name == "heimdall" || name == "sbom")
    {
        if (opt.substr(0, 12) == "sbom-output=")
        {
            setOutputPath(opt.substr(12));
        }
        else if (opt.substr(0, 7) == "format=")
        {
            setFormat(opt.substr(7));
        }
        else if (opt == "verbose")
        {
            setVerbose(true);
        }
        else if (opt == "no-debug-info")
        {
            setExtractDebugInfo(false);
        }
        else if (opt == "include-system-libs")
        {
            setIncludeSystemLibraries(true);
        }
    }
}

void GoldAdapterImpl::processGoldInputFile(const char* filePath)
{
    if (filePath)
    {
        processInputFile(std::string(filePath));
    }
}

void GoldAdapterImpl::processGoldLibrary(const char* libraryPath)
{
    if (libraryPath)
    {
        processLibrary(std::string(libraryPath));
    }
}

void GoldAdapterImpl::extractSymbolsFromFile(const std::string& filePath)
{
    // This is a simplified implementation
    // In a real implementation, you would use BFD or similar to extract symbols
    
    if (PluginUtils::isObjectFile(filePath))
    {
        processObjectFile(filePath);
    }
    else if (PluginUtils::isStaticLibrary(filePath))
    {
        processArchiveFile(filePath);
    }
    else if (PluginUtils::isSharedLibrary(filePath))
    {
        processSharedLibrary(filePath);
    }
}

void GoldAdapterImpl::processArchiveFile(const std::string& filePath)
{
    // Process static library (archive file)
    // This would use BFD to iterate through archive members
    PluginUtils::logDebug("Processing archive file: " + filePath);
    
    // For now, just create a placeholder symbol to indicate this is a library
    SymbolInfo symbol;
    symbol.name = "archive_" + extractComponentName(filePath);
    symbol.address = 0;
    symbol.size = 0;
    symbol.isDefined = true;
    symbol.isGlobal = true;
    symbol.section = "archive";
    
    currentSymbols.push_back(symbol);
}

void GoldAdapterImpl::processObjectFile(const std::string& filePath)
{
    // Process object file
    // This would use BFD to extract symbols
    PluginUtils::logDebug("Processing object file: " + filePath);
    
    // For now, just create a placeholder symbol
    SymbolInfo symbol;
    symbol.name = "object_" + extractComponentName(filePath);
    symbol.address = 0;
    symbol.size = 0;
    symbol.isDefined = true;
    symbol.isGlobal = true;
    symbol.section = "text";
    
    currentSymbols.push_back(symbol);
}

void GoldAdapterImpl::processSharedLibrary(const std::string& filePath)
{
    // Process shared library
    // This would use BFD to extract symbols
    PluginUtils::logDebug("Processing shared library: " + filePath);
    
    // For now, just create a placeholder symbol
    SymbolInfo symbol;
    symbol.name = "shared_" + extractComponentName(filePath);
    symbol.address = 0;
    symbol.size = 0;
    symbol.isDefined = true;
    symbol.isGlobal = true;
    symbol.section = "dynamic";
    
    currentSymbols.push_back(symbol);
}

} // namespace heimdall
