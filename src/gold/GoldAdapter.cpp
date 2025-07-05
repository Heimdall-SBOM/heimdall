#include "GoldAdapter.hpp"
#include "../common/PluginInterface.hpp"
#include "../common/Utils.hpp"
#include "../common/MetadataExtractor.hpp"
#include <iostream>
#include <vector>
#include <string>

#ifdef __linux__
#include <plugin-api.h>
#include <bfd.h>
#include <elf-bfd.h>
#endif

namespace heimdall {

GoldAdapter::GoldAdapter() : pImpl(std::make_unique<Impl>())
{
}

GoldAdapter::~GoldAdapter() = default;

bool GoldAdapter::initialize()
{
    return pImpl->initialize();
}

void GoldAdapter::cleanup()
{
    pImpl->cleanup();
}

void GoldAdapter::processInputFile(const std::string& filePath)
{
    pImpl->processInputFile(filePath);
}

void GoldAdapter::processLibrary(const std::string& libraryPath)
{
    pImpl->processLibrary(libraryPath);
}

void GoldAdapter::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size)
{
    pImpl->processSymbol(symbolName, address, size);
}

void GoldAdapter::setOutputPath(const std::string& path)
{
    pImpl->setOutputPath(path);
}

void GoldAdapter::setFormat(const std::string& format)
{
    pImpl->setFormat(format);
}

void GoldAdapter::generateSBOM()
{
    pImpl->generateSBOM();
}

void GoldAdapter::setVerbose(bool verbose)
{
    pImpl->setVerbose(verbose);
}

void GoldAdapter::setExtractDebugInfo(bool extract)
{
    pImpl->setExtractDebugInfo(extract);
}

void GoldAdapter::setIncludeSystemLibraries(bool include)
{
    pImpl->setIncludeSystemLibraries(include);
}

size_t GoldAdapter::getComponentCount() const
{
    return pImpl->getComponentCount();
}

void GoldAdapter::printStatistics() const
{
    pImpl->printStatistics();
}

GoldAdapter::Impl::Impl()
{
}

GoldAdapter::Impl::~Impl() = default;

bool GoldAdapter::Impl::initialize()
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

void GoldAdapter::Impl::cleanup()
{
    PluginUtils::logInfo("Heimdall Gold Plugin cleanup");
    processedFiles.clear();
    processedLibraries.clear();
    currentSymbols.clear();
}

// Helper to set minimum SBOM fields for a component
static void setMinimumSBOMFields(ComponentInfo& component)
{
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

void GoldAdapter::Impl::processInputFile(const std::string& filePath)
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

void GoldAdapter::Impl::processLibrary(const std::string& libraryPath)
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

void GoldAdapter::Impl::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size)
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

void GoldAdapter::Impl::setOutputPath(const std::string& path)
{
    sbomGenerator->setOutputPath(path);
}

void GoldAdapter::Impl::setFormat(const std::string& format)
{
    sbomGenerator->setFormat(format);
}

void GoldAdapter::Impl::generateSBOM()
{
    PluginUtils::logInfo("Generating SBOM with " + std::to_string(getComponentCount()) + " components");
    sbomGenerator->generateSBOM();
}

void GoldAdapter::Impl::setVerbose(bool verbose)
{
    this->verbose = verbose;
    if (sbomGenerator)
    {
        // Note: SBOMGenerator doesn't have a setVerbose method in the current interface
        // This could be added if needed
    }
}

void GoldAdapter::Impl::setExtractDebugInfo(bool extract)
{
    extractDebugInfo = extract;
}

void GoldAdapter::Impl::setIncludeSystemLibraries(bool include)
{
    includeSystemLibraries = include;
}

size_t GoldAdapter::Impl::getComponentCount() const
{
    return sbomGenerator->getComponentCount();
}

void GoldAdapter::Impl::printStatistics() const
{
    PluginUtils::logInfo("Gold Plugin Statistics:");
    PluginUtils::logInfo("  Processed files: " + std::to_string(processedFiles.size()));
    PluginUtils::logInfo("  Processed libraries: " + std::to_string(processedLibraries.size()));
    PluginUtils::logInfo("  Total components: " + std::to_string(getComponentCount()));
    
    sbomGenerator->printStatistics();
}

void GoldAdapter::Impl::handleGoldPlugin(const char* pluginName, const char* pluginOpt)
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

void GoldAdapter::Impl::processGoldInputFile(const char* filePath)
{
    if (filePath)
    {
        processInputFile(std::string(filePath));
    }
}

void GoldAdapter::Impl::processGoldLibrary(const char* libraryPath)
{
    if (libraryPath)
    {
        processLibrary(std::string(libraryPath));
    }
}

void GoldAdapter::Impl::extractSymbolsFromFile(const std::string& filePath)
{
#ifdef __linux__
    bfd* abfd = bfd_openr(filePath.c_str(), nullptr);
    if (!abfd) {
        PluginUtils::logDebug("Failed to open file with BFD: " + filePath);
        return;
    }
    if (!bfd_check_format(abfd, bfd_object) && !bfd_check_format(abfd, bfd_archive)) {
        bfd_close(abfd);
        PluginUtils::logDebug("File is not a valid object or archive: " + filePath);
        return;
    }
    if (!bfd_check_format_matches(abfd, bfd_object, nullptr)) {
        bfd_close(abfd);
        PluginUtils::logDebug("BFD format check failed: " + filePath);
        return;
    }
    long symcount = bfd_get_symtab_upper_bound(abfd);
    if (symcount <= 0) {
        symcount = bfd_get_dynamic_symtab_upper_bound(abfd);
        if (symcount <= 0) {
            bfd_close(abfd);
            PluginUtils::logDebug("No symbol table found: " + filePath);
            return;
        }
    }
    asymbol** syms = static_cast<asymbol**>(malloc(symcount));
    if (!syms) {
        bfd_close(abfd);
        return;
    }
    long symsize = bfd_canonicalize_symtab(abfd, syms);
    if (symsize <= 0) {
        symsize = bfd_canonicalize_dynamic_symtab(abfd, syms);
    }
    if (symsize > 0) {
        for (long i = 0; i < symsize; ++i) {
            asymbol* sym = syms[i];
            if (!sym) continue;
            if (!verbose && (sym->flags & BSF_LOCAL)) continue;
            if (!extractDebugInfo && (sym->flags & BSF_DEBUGGING)) continue;
            SymbolInfo symbol;
            symbol.name = bfd_asymbol_name(sym);
            symbol.address = bfd_asymbol_value(sym);
            // BFD does not provide symbol size directly; set to 0 or infer if needed
            symbol.size = 0;
            symbol.isDefined = (sym->flags & BSF_GLOBAL) || (sym->flags & BSF_WEAK);
            symbol.isGlobal = (sym->flags & BSF_GLOBAL);
            symbol.isWeak = (sym->flags & BSF_WEAK);
            asection* sec = bfd_asymbol_section(sym);
            if (sec) {
                symbol.section = bfd_section_name(sec);
            }
            currentSymbols.push_back(symbol);
        }
    }
    free(syms);
    bfd_close(abfd);
    PluginUtils::logDebug("Extracted " + std::to_string(currentSymbols.size()) + " symbols from " + filePath);
#endif
}

void GoldAdapter::Impl::processArchiveFile(const std::string& filePath)
{
    // Process static library (archive file) using BFD
    // This would use BFD to iterate through archive members
    PluginUtils::logDebug("Processing archive file with BFD: " + filePath);
    
    // TODO: Implement full BFD-based archive processing
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

void GoldAdapter::Impl::processObjectFile(const std::string& filePath)
{
    // Process object file using BFD
    // This would use BFD to extract symbols
    PluginUtils::logDebug("Processing object file with BFD: " + filePath);
    
    // TODO: Implement full BFD-based object file processing
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

void GoldAdapter::Impl::processSharedLibrary(const std::string& filePath)
{
    // Process shared library using BFD
    // This would use BFD to extract symbols
    PluginUtils::logDebug("Processing shared library with BFD: " + filePath);
    
    // TODO: Implement full BFD-based shared library processing
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

void GoldAdapter::Impl::addComponent(const ComponentInfo& component)
{
    // Implementation of addComponent method
}

std::string GoldAdapter::Impl::extractComponentName(const std::string& filePath)
{
    // Implementation of extractComponentName method
    return ""; // Placeholder return, actual implementation needed
}

} // namespace heimdall
