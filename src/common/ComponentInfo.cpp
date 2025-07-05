#include "ComponentInfo.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace heimdall {

// Helper function to calculate SHA256 checksum
std::string calculateSHA256(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)))
    {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    SHA256_Update(&sha256, buffer, file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// Helper function to get file size
uint64_t getFileSize(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return 0;
    }
    return file.tellg();
}

// Helper function to determine file type from extension
FileType determineFileType(const std::string& filePath)
{
    std::string lowerPath = filePath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    // Helper function to check if string ends with suffix
    auto endsWith = [](const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    };

    if (endsWith(lowerPath, ".o") || endsWith(lowerPath, ".obj"))
    {
        return FileType::Object;
    }
    else if (endsWith(lowerPath, ".a") || endsWith(lowerPath, ".lib"))
    {
        return FileType::StaticLibrary;
    }
    else if (endsWith(lowerPath, ".so") || endsWith(lowerPath, ".dylib") || endsWith(lowerPath, ".dll"))
    {
        return FileType::SharedLibrary;
    }
    else if (endsWith(lowerPath, ".exe") || lowerPath.find("bin/") != std::string::npos)
    {
        return FileType::Executable;
    }

    return FileType::Unknown;
}

// ComponentInfo methods
ComponentInfo::ComponentInfo(const std::string& componentName, const std::string& path)
    : name(componentName)
    , filePath(path)
    , fileType(determineFileType(path))
    , fileSize(getFileSize(path))
    , checksum(calculateSHA256(path))
    , wasProcessed(false)
    , detectedBy(LinkerType::Unknown)
    , isSystemLibrary(false)
    , containsDebugInfo(false)
    , isStripped(false)
{
}

void ComponentInfo::addDependency(const std::string& dependency)
{
    if (std::find(dependencies.begin(), dependencies.end(), dependency) == dependencies.end())
    {
        dependencies.push_back(dependency);
    }
}

void ComponentInfo::addSourceFile(const std::string& sourceFile)
{
    if (std::find(sourceFiles.begin(), sourceFiles.end(), sourceFile) == sourceFiles.end())
    {
        sourceFiles.push_back(sourceFile);
    }
}

void ComponentInfo::setVersion(const std::string& ver)
{
    version = ver;
}

void ComponentInfo::setSupplier(const std::string& sup)
{
    supplier = sup;
}

void ComponentInfo::setDownloadLocation(const std::string& location)
{
    downloadLocation = location;
}

void ComponentInfo::setHomepage(const std::string& page)
{
    homepage = page;
}

void ComponentInfo::setLicense(const std::string& lic)
{
    license = lic;
}

void ComponentInfo::setPackageManager(const std::string& pkgMgr)
{
    packageManager = pkgMgr;
}

void ComponentInfo::markAsProcessed()
{
    wasProcessed = true;
}

void ComponentInfo::setProcessingError(const std::string& error)
{
    processingError = error;
    wasProcessed = false;
}

void ComponentInfo::setDetectedBy(LinkerType linker)
{
    detectedBy = linker;
}

void ComponentInfo::markAsSystemLibrary()
{
    isSystemLibrary = true;
}

void ComponentInfo::setContainsDebugInfo(bool hasDebug)
{
    containsDebugInfo = hasDebug;
}

void ComponentInfo::setStripped(bool stripped)
{
    isStripped = stripped;
}

bool ComponentInfo::hasSymbol(const std::string& symbolName) const
{
    return std::any_of(symbols.begin(), symbols.end(),
        [&symbolName](const SymbolInfo& symbol) { return symbol.name == symbolName; });
}

bool ComponentInfo::hasSection(const std::string& sectionName) const
{
    return std::any_of(sections.begin(), sections.end(),
        [&sectionName](const SectionInfo& section) { return section.name == sectionName; });
}

std::string ComponentInfo::getFileTypeString() const
{
    switch (fileType)
    {
        case FileType::Object: return "Object";
        case FileType::StaticLibrary: return "StaticLibrary";
        case FileType::SharedLibrary: return "SharedLibrary";
        case FileType::Executable: return "Executable";
        default: return "Unknown";
    }
}

std::string ComponentInfo::getLinkerTypeString() const
{
    switch (detectedBy)
    {
        case LinkerType::LLD: return "LLD";
        case LinkerType::Gold: return "Gold";
        case LinkerType::BFD: return "BFD";
        default: return "Unknown";
    }
}

} // namespace heimdall
