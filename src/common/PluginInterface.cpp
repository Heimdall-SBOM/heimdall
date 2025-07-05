#include "PluginInterface.hpp"
#include "Utils.hpp"
#include <iostream>
#include <regex>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace heimdall {

PluginInterface::PluginInterface()
    : sbomGenerator(std::make_unique<SBOMGenerator>())
    , verbose(false)
    , extractDebugInfo(true)
    , includeSystemLibraries(false)
{
}

PluginInterface::~PluginInterface() = default;

void PluginInterface::addComponent(const ComponentInfo& component)
{
    // Check if we should process this component
    if (!shouldProcessFile(component.filePath))
    {
        if (verbose)
        {
            PluginUtils::logDebug("Skipping component: " + component.name);
        }
        return;
    }
    
    // Add to processed components list
    processedComponents.push_back(component);
    
    // Add to SBOM generator
    sbomGenerator->processComponent(component);
    
    if (verbose)
    {
        PluginUtils::logInfo("Added component: " + component.name + " (" + component.getFileTypeString() + ")");
    }
}

void PluginInterface::updateComponent(const std::string& name, const std::string& filePath, 
                                     const std::vector<SymbolInfo>& symbols)
{
    // Find existing component
    for (auto& component : processedComponents)
    {
        if (component.name == name && component.filePath == filePath)
        {
            // Add new symbols
            for (const auto& symbol : symbols)
            {
                component.addSymbol(symbol);
            }
            
            // Update SBOM generator
            sbomGenerator->processComponent(component);
            
            if (verbose)
            {
                PluginUtils::logDebug("Updated component: " + name + " with " + std::to_string(symbols.size()) + " symbols");
            }
            return;
        }
    }
    
    // Component not found, create new one
    ComponentInfo newComponent(name, filePath);
    for (const auto& symbol : symbols)
    {
        newComponent.addSymbol(symbol);
    }
    addComponent(newComponent);
}

bool PluginInterface::shouldProcessFile(const std::string& filePath) const
{
    // Skip system libraries if not requested
    if (!includeSystemLibraries && Utils::isSystemLibrary(filePath))
    {
        return false;
    }
    
    // Check if file exists
    if (!Utils::fileExists(filePath))
    {
        return false;
    }
    
    // Check file type
    std::string extension = Utils::getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Only process known file types
    std::vector<std::string> validExtensions = {".o", ".obj", ".a", ".lib", ".so", ".dylib", ".dll", ".exe"};
    return std::find(validExtensions.begin(), validExtensions.end(), extension) != validExtensions.end();
}

std::string PluginInterface::extractComponentName(const std::string& filePath) const
{
    std::string fileName = Utils::getFileName(filePath);
    
    // Remove common prefixes
    std::vector<std::string> prefixes = {"lib"};
    for (const auto& prefix : prefixes)
    {
        if (Utils::startsWith(fileName, prefix))
        {
            fileName = fileName.substr(prefix.length());
            break;
        }
    }
    
    // Remove extensions
    std::vector<std::string> extensions = {".o", ".obj", ".a", ".lib", ".so", ".dylib", ".dll", ".exe"};
    for (const auto& ext : extensions)
    {
        if (Utils::endsWith(fileName, ext))
        {
            fileName = fileName.substr(0, fileName.length() - ext.length());
            break;
        }
    }
    
    return fileName;
}

// PluginUtils implementation
namespace PluginUtils {

bool isObjectFile(const std::string& filePath)
{
    std::string extension = Utils::getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".o" || extension == ".obj";
}

bool isStaticLibrary(const std::string& filePath)
{
    std::string extension = Utils::getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".a" || extension == ".lib";
}

bool isSharedLibrary(const std::string& filePath)
{
    std::string extension = Utils::getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".so" || extension == ".dylib" || extension == ".dll";
}

bool isExecutable(const std::string& filePath)
{
    std::string extension = Utils::getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".exe" || extension.empty();
}

std::string normalizeLibraryPath(const std::string& libraryPath)
{
    return Utils::normalizePath(libraryPath);
}

std::string resolveLibraryPath(const std::string& libraryName)
{
    return Utils::findLibrary(libraryName);
}

std::vector<std::string> getLibrarySearchPaths()
{
    return Utils::getLibrarySearchPaths();
}

bool isSystemSymbol(const std::string& symbolName)
{
    // Common system symbol prefixes
    std::vector<std::string> systemPrefixes = {
        "_", "__", "___", "GLOBAL_OFFSET_TABLE_", "_DYNAMIC", "_GLOBAL_OFFSET_TABLE_",
        "start", "main", "_start", "_main", "__libc_", "__gmon_start__"
    };
    
    for (const auto& prefix : systemPrefixes)
    {
        if (Utils::startsWith(symbolName, prefix))
        {
            return true;
        }
    }
    
    return false;
}

bool isWeakSymbol(const std::string& symbolName)
{
    // Weak symbols often have specific patterns
    return symbolName.find("weak") != std::string::npos ||
           symbolName.find("WEAK") != std::string::npos;
}

std::string extractSymbolVersion(const std::string& symbolName)
{
    // Look for version information in symbol name
    std::regex versionRegex(R"((\d+\.\d+\.\d+))");
    std::smatch match;
    
    if (std::regex_search(symbolName, match, versionRegex))
    {
        return match[1].str();
    }
    
    return "";
}

bool loadConfigFromFile(const std::string& configPath, PluginConfig& config)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = Utils::trim(line.substr(0, pos));
            std::string value = Utils::trim(line.substr(pos + 1));
            
            if (key == "output_path")
            {
                config.outputPath = value;
            }
            else if (key == "format")
            {
                config.format = value;
            }
            else if (key == "verbose")
            {
                config.verbose = (value == "true" || value == "1");
            }
            else if (key == "extract_debug_info")
            {
                config.extractDebugInfo = (value == "true" || value == "1");
            }
            else if (key == "include_system_libraries")
            {
                config.includeSystemLibraries = (value == "true" || value == "1");
            }
        }
    }
    
    return true;
}

bool saveConfigToFile(const std::string& configPath, const PluginConfig& config)
{
    std::ofstream file(configPath);
    if (!file.is_open())
    {
        return false;
    }
    
    file << "# Heimdall Plugin Configuration\n";
    file << "output_path=" << config.outputPath << "\n";
    file << "format=" << config.format << "\n";
    file << "verbose=" << (config.verbose ? "true" : "false") << "\n";
    file << "extract_debug_info=" << (config.extractDebugInfo ? "true" : "false") << "\n";
    file << "include_system_libraries=" << (config.includeSystemLibraries ? "true" : "false") << "\n";
    
    return true;
}

bool parseCommandLineOptions(int argc, char* argv[], PluginConfig& config)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        
        if (arg == "--sbom-output" && i + 1 < argc)
        {
            config.outputPath = argv[++i];
        }
        else if (arg == "--format" && i + 1 < argc)
        {
            config.format = argv[++i];
        }
        else if (arg == "--verbose")
        {
            config.verbose = true;
        }
        else if (arg == "--no-debug-info")
        {
            config.extractDebugInfo = false;
        }
        else if (arg == "--include-system-libs")
        {
            config.includeSystemLibraries = true;
        }
        else if (arg == "--exclude" && i + 1 < argc)
        {
            config.excludePatterns.push_back(argv[++i]);
        }
        else if (arg == "--include" && i + 1 < argc)
        {
            config.includePatterns.push_back(argv[++i]);
        }
    }
    
    return true;
}

void logInfo(const std::string& message)
{
    std::cout << "[INFO] " << message << std::endl;
}

void logWarning(const std::string& message)
{
    std::cerr << "[WARNING] " << message << std::endl;
}

void logError(const std::string& message)
{
    std::cerr << "[ERROR] " << message << std::endl;
}

void logDebug(const std::string& message)
{
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cerr << "[DEBUG] " << message << std::endl;
#endif
}

} // namespace PluginUtils

} // namespace heimdall
