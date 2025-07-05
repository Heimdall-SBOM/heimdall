#include "MetadataExtractor.hpp"
#include "Utils.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <regex>
#include <algorithm>

#ifdef __linux__
#include <elf.h>
#include <link.h>
#endif

#ifdef __APPLE__
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/fat.h>
#endif

namespace heimdall {

class MetadataExtractor::Impl {
public:
    bool verbose = false;
    bool extractDebugInfo = true;
    
    // File format detection
    bool detectFileFormat(const std::string& filePath);
    std::string fileFormat;
};

MetadataExtractor::MetadataExtractor() : pImpl(std::make_unique<Impl>())
{
}

MetadataExtractor::~MetadataExtractor() = default;

bool MetadataExtractor::extractMetadata(ComponentInfo& component)
{
    if (!Utils::fileExists(component.filePath))
    {
        Utils::errorPrint("File does not exist: " + component.filePath);
        return false;
    }
    
    // Detect file format
    if (!pImpl->detectFileFormat(component.filePath))
    {
        Utils::warningPrint("Could not detect file format for: " + component.filePath);
    }
    
    bool success = true;
    
    // Extract basic metadata
    success &= extractVersionInfo(component);
    success &= extractLicenseInfo(component);
    success &= extractSymbolInfo(component);
    success &= extractSectionInfo(component);
    
    if (pImpl->extractDebugInfo)
    {
        success &= extractDebugInfo(component);
    }
    
    success &= extractDependencyInfo(component);
    
    // Package manager specific extraction
    std::string packageManager = Utils::detectPackageManager(component.filePath);
    if (packageManager == "conan")
    {
        extractConanMetadata(component);
    }
    else if (packageManager == "vcpkg")
    {
        extractVcpkgMetadata(component);
    }
    else if (packageManager == "system")
    {
        extractSystemMetadata(component);
    }
    
    component.markAsProcessed();
    return success;
}

bool MetadataExtractor::extractVersionInfo(ComponentInfo& component)
{
    // Try to extract version from file content
    std::string version = MetadataHelpers::detectVersionFromFile(component.filePath);
    if (!version.empty())
    {
        component.setVersion(version);
        return true;
    }
    
    // Try to extract version from path
    version = MetadataHelpers::detectVersionFromPath(component.filePath);
    if (!version.empty())
    {
        component.setVersion(version);
        return true;
    }
    
    // Try to extract version from symbols
    version = MetadataHelpers::detectVersionFromSymbols(component.symbols);
    if (!version.empty())
    {
        component.setVersion(version);
        return true;
    }
    
    return false;
}

bool MetadataExtractor::extractLicenseInfo(ComponentInfo& component)
{
    // Try to detect license from file content
    std::string license = MetadataHelpers::detectLicenseFromFile(component.filePath);
    if (!license.empty())
    {
        component.setLicense(license);
        return true;
    }
    
    // Try to detect license from path
    license = MetadataHelpers::detectLicenseFromPath(component.filePath);
    if (!license.empty())
    {
        component.setLicense(license);
        return true;
    }
    
    // Try to detect license from symbols
    license = MetadataHelpers::detectLicenseFromSymbols(component.symbols);
    if (!license.empty())
    {
        component.setLicense(license);
        return true;
    }
    
    return false;
}

bool MetadataExtractor::extractSymbolInfo(ComponentInfo& component)
{
    if (isELF(component.filePath))
    {
        return MetadataHelpers::extractELFSymbols(component.filePath, component.symbols);
    }
    else if (isMachO(component.filePath))
    {
        return MetadataHelpers::extractMachOSymbols(component.filePath, component.symbols);
    }
    else if (isPE(component.filePath))
    {
        return MetadataHelpers::extractPESymbols(component.filePath, component.symbols);
    }
    else if (isArchive(component.filePath))
    {
        return MetadataHelpers::extractArchiveSymbols(component.filePath, component.symbols);
    }
    
    return false;
}

bool MetadataExtractor::extractSectionInfo(ComponentInfo& component)
{
    if (isELF(component.filePath))
    {
        return MetadataHelpers::extractELFSections(component.filePath, component.sections);
    }
    else if (isMachO(component.filePath))
    {
        return MetadataHelpers::extractMachOSections(component.filePath, component.sections);
    }
    else if (isPE(component.filePath))
    {
        return MetadataHelpers::extractPESections(component.filePath, component.sections);
    }
    
    return false;
}

bool MetadataExtractor::extractDebugInfo(ComponentInfo& component)
{
    return MetadataHelpers::extractDebugInfo(component.filePath, component);
}

bool MetadataExtractor::extractDependencyInfo(ComponentInfo& component)
{
    std::vector<std::string> deps = MetadataHelpers::detectDependencies(component.filePath);
    for (const auto& dep : deps)
    {
        component.addDependency(dep);
    }
    return !deps.empty();
}

bool MetadataExtractor::isELF(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    char magic[4];
    file.read(magic, 4);
    
    return (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

bool MetadataExtractor::isMachO(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    
    return (magic == MH_MAGIC || magic == MH_MAGIC_64 || 
            magic == MH_CIGAM || magic == MH_CIGAM_64 ||
            magic == FAT_MAGIC || magic == FAT_CIGAM);
}

bool MetadataExtractor::isPE(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    char magic[2];
    file.read(magic, 2);
    
    return (magic[0] == 'M' && magic[1] == 'Z');
}

bool MetadataExtractor::isArchive(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    char magic[8];
    file.read(magic, 8);
    
    return (strncmp(magic, "!<arch>", 7) == 0);
}

bool MetadataExtractor::extractConanMetadata(ComponentInfo& component)
{
    component.setPackageManager("conan");
    
    // Extract version from path (conan typically includes version in path)
    std::string version = Utils::extractVersionFromPath(component.filePath);
    if (!version.empty())
    {
        component.setVersion(version);
    }
    
    // Extract package name
    std::string packageName = Utils::extractPackageName(component.filePath);
    if (!packageName.empty())
    {
        component.setSupplier("conan-center");
        component.setDownloadLocation("https://conan.io/center/" + packageName);
    }
    
    return true;
}

bool MetadataExtractor::extractVcpkgMetadata(ComponentInfo& component)
{
    component.setPackageManager("vcpkg");
    
    // Extract version from path
    std::string version = Utils::extractVersionFromPath(component.filePath);
    if (!version.empty())
    {
        component.setVersion(version);
    }
    
    // Extract package name
    std::string packageName = Utils::extractPackageName(component.filePath);
    if (!packageName.empty())
    {
        component.setSupplier("vcpkg");
        component.setDownloadLocation("https://github.com/microsoft/vcpkg");
    }
    
    return true;
}

bool MetadataExtractor::extractSystemMetadata(ComponentInfo& component)
{
    component.setPackageManager("system");
    component.markAsSystemLibrary();
    
    // Try to extract version from package manager
    std::string packageName = Utils::extractPackageName(component.filePath);
    if (!packageName.empty())
    {
        component.setSupplier("system-package-manager");
    }
    
    return true;
}

void MetadataExtractor::setVerbose(bool verbose)
{
    pImpl->verbose = verbose;
}

void MetadataExtractor::setExtractDebugInfo(bool extract)
{
    pImpl->extractDebugInfo = extract;
}

bool MetadataExtractor::Impl::detectFileFormat(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    char magic[16];
    file.read(magic, sizeof(magic));
    
    if (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
    {
        fileFormat = "ELF";
        return true;
    }
    else if (magic[0] == 'M' && magic[1] == 'Z')
    {
        fileFormat = "PE";
        return true;
    }
    else if (strncmp(magic, "!<arch>", 7) == 0)
    {
        fileFormat = "Archive";
        return true;
    }
    
    // Check for Mach-O (multiple possible magic values)
    uint32_t* magic32 = reinterpret_cast<uint32_t*>(magic);
    if (*magic32 == MH_MAGIC || *magic32 == MH_MAGIC_64 || 
        *magic32 == MH_CIGAM || *magic32 == MH_CIGAM_64 ||
        *magic32 == FAT_MAGIC || *magic32 == FAT_CIGAM)
    {
        fileFormat = "Mach-O";
        return true;
    }
    
    fileFormat = "Unknown";
    return false;
}

// MetadataHelpers implementation
namespace MetadataHelpers {

bool isMachO(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    
    return (magic == MH_MAGIC || magic == MH_MAGIC_64 || 
            magic == MH_CIGAM || magic == MH_CIGAM_64 ||
            magic == FAT_MAGIC || magic == FAT_CIGAM);
}

bool extractELFSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
#ifdef __linux__
    // This is a simplified implementation
    // In a real implementation, you would use libelf or similar
    Utils::debugPrint("ELF symbol extraction not fully implemented");
    return false;
#else
    Utils::debugPrint("ELF symbol extraction not supported on this platform");
    return false;
#endif
}

bool extractELFSections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
#ifdef __linux__
    // This is a simplified implementation
    Utils::debugPrint("ELF section extraction not fully implemented");
    return false;
#else
    Utils::debugPrint("ELF section extraction not supported on this platform");
    return false;
#endif
}

bool extractELFVersion(const std::string& filePath, std::string& version)
{
    // Try to extract version from file content
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    
    // Read file content and look for version patterns
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    std::regex versionRegex(R"((\d+\.\d+\.\d+))");
    std::smatch match;
    
    if (std::regex_search(content, match, versionRegex))
    {
        version = match[1].str();
        return true;
    }
    
    return false;
}

bool extractELFBuildId(const std::string& filePath, std::string& buildId)
{
    // Implementation would use libelf to extract build ID
    Utils::debugPrint("ELF build ID extraction not implemented");
    return false;
}

bool extractMachOSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
#ifdef __APPLE__
    // Open file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Utils::debugPrint("Failed to open Mach-O file for symbol extraction: " + filePath);
        return false;
    }

    // Read magic to check for fat or thin
    uint32_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.seekg(0);

    if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
        // Fat binary: read first architecture only for now
        struct fat_header fatHeader;
        file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
        uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
        struct fat_arch arch;
        file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
        uint32_t offset = OSSwapBigToHostInt32(arch.offset);
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.seekg(offset);
    }

    bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    // Read Mach-O header
    if (is64) {
        struct mach_header_64 mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        // Iterate load commands
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            if (lc.cmd == LC_SYMTAB) {
                struct symtab_command symtab;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));
                // Read string table
                std::vector<char> strtab(symtab.strsize);
                file.seekg(symtab.stroff);
                file.read(strtab.data(), symtab.strsize);
                // Read symbol table
                file.seekg(symtab.symoff);
                for (uint32_t j = 0; j < symtab.nsyms; ++j) {
                    struct nlist_64 nlsym;
                    file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));
                    SymbolInfo sym;
                    if (nlsym.n_un.n_strx < symtab.strsize)
                        sym.name = &strtab[nlsym.n_un.n_strx];
                    else
                        sym.name = "<badstrx>";
                    sym.address = nlsym.n_value;
                    sym.size = 0; // Mach-O doesn't store symbol size
                    sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
                    sym.isGlobal = (nlsym.n_type & N_EXT);
                    sym.section = std::to_string(nlsym.n_sect);
                    symbols.push_back(sym);
                }
                break;
            }
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    } else {
        struct mach_header mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            if (lc.cmd == LC_SYMTAB) {
                struct symtab_command symtab;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&symtab), sizeof(symtab));
                std::vector<char> strtab(symtab.strsize);
                file.seekg(symtab.stroff);
                file.read(strtab.data(), symtab.strsize);
                file.seekg(symtab.symoff);
                for (uint32_t j = 0; j < symtab.nsyms; ++j) {
                    struct nlist nlsym;
                    file.read(reinterpret_cast<char*>(&nlsym), sizeof(nlsym));
                    SymbolInfo sym;
                    if (nlsym.n_un.n_strx < symtab.strsize)
                        sym.name = &strtab[nlsym.n_un.n_strx];
                    else
                        sym.name = "<badstrx>";
                    sym.address = nlsym.n_value;
                    sym.size = 0;
                    sym.isDefined = !(nlsym.n_type & N_STAB) && (nlsym.n_type & N_TYPE) != N_UNDF;
                    sym.isGlobal = (nlsym.n_type & N_EXT);
                    sym.section = std::to_string(nlsym.n_sect);
                    symbols.push_back(sym);
                }
                break;
            }
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    }
    return !symbols.empty();
#else
    Utils::debugPrint("Mach-O symbol extraction not supported on this platform");
    return false;
#endif
}

bool extractMachOSections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
#ifdef __APPLE__
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Utils::debugPrint("Failed to open Mach-O file for section extraction: " + filePath);
        return false;
    }
    uint32_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.seekg(0);
    if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
        struct fat_header fatHeader;
        file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
        uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
        struct fat_arch arch;
        file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
        uint32_t offset = OSSwapBigToHostInt32(arch.offset);
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.seekg(offset);
    }
    bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    if (is64) {
        struct mach_header_64 mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            if (lc.cmd == LC_SEGMENT_64) {
                struct segment_command_64 seg;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&seg), sizeof(seg));
                for (uint32_t j = 0; j < seg.nsects; ++j) {
                    struct section_64 sect;
                    file.read(reinterpret_cast<char*>(&sect), sizeof(sect));
                    SectionInfo s;
                    s.name = sect.sectname;
                    s.address = sect.addr;
                    s.size = sect.size;
                    s.flags = sect.flags;
                    s.type = seg.segname;
                    sections.push_back(s);
                }
            }
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    } else {
        struct mach_header mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            if (lc.cmd == LC_SEGMENT) {
                struct segment_command seg;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&seg), sizeof(seg));
                for (uint32_t j = 0; j < seg.nsects; ++j) {
                    struct section sect;
                    file.read(reinterpret_cast<char*>(&sect), sizeof(sect));
                    SectionInfo s;
                    s.name = sect.sectname;
                    s.address = sect.addr;
                    s.size = sect.size;
                    s.flags = sect.flags;
                    s.type = seg.segname;
                    sections.push_back(s);
                }
            }
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    }
    return !sections.empty();
#else
    Utils::debugPrint("Mach-O section extraction not supported on this platform");
    return false;
#endif
}

bool extractMachOVersion(const std::string& filePath, std::string& version)
{
    // Implementation would use Mach-O APIs
    Utils::debugPrint("Mach-O version extraction not implemented");
    return false;
}

bool extractMachOUUID(const std::string& filePath, std::string& uuid)
{
#ifdef __APPLE__
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Utils::debugPrint("Failed to open Mach-O file for UUID extraction: " + filePath);
        return false;
    }

    // Handle fat binaries
    uint32_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.seekg(0);

    if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
        struct fat_header fatHeader;
        file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
        uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
        struct fat_arch arch;
        file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
        uint32_t offset = OSSwapBigToHostInt32(arch.offset);
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.seekg(offset);
    }

    bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    
    // Read Mach-O header
    if (is64) {
        struct mach_header_64 mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        
        // Iterate load commands
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            
            if (lc.cmd == LC_UUID) {
                struct uuid_command uuid_cmd;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));
                
                // Format UUID as string
                char uuid_str[37];
                snprintf(uuid_str, sizeof(uuid_str), 
                        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                        uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                        uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                        uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                        uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
                uuid = uuid_str;
                return true;
            }
            
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    } else {
        struct mach_header mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            
            if (lc.cmd == LC_UUID) {
                struct uuid_command uuid_cmd;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&uuid_cmd), sizeof(uuid_cmd));
                
                char uuid_str[37];
                snprintf(uuid_str, sizeof(uuid_str), 
                        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                        uuid_cmd.uuid[0], uuid_cmd.uuid[1], uuid_cmd.uuid[2], uuid_cmd.uuid[3],
                        uuid_cmd.uuid[4], uuid_cmd.uuid[5], uuid_cmd.uuid[6], uuid_cmd.uuid[7],
                        uuid_cmd.uuid[8], uuid_cmd.uuid[9], uuid_cmd.uuid[10], uuid_cmd.uuid[11],
                        uuid_cmd.uuid[12], uuid_cmd.uuid[13], uuid_cmd.uuid[14], uuid_cmd.uuid[15]);
                uuid = uuid_str;
                return true;
            }
            
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    }
    
    return false;
#else
    Utils::debugPrint("Mach-O UUID extraction not supported on this platform");
    return false;
#endif
}

bool extractPESymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
    // Implementation would use PE parsing libraries
    Utils::debugPrint("PE symbol extraction not implemented");
    return false;
}

bool extractPESections(const std::string& filePath, std::vector<SectionInfo>& sections)
{
    // Implementation would use PE parsing libraries
    Utils::debugPrint("PE section extraction not implemented");
    return false;
}

bool extractPEVersion(const std::string& filePath, std::string& version)
{
    // Implementation would use PE parsing libraries
    Utils::debugPrint("PE version extraction not implemented");
    return false;
}

bool extractPECompanyName(const std::string& filePath, std::string& company)
{
    // Implementation would use PE parsing libraries
    Utils::debugPrint("PE company name extraction not implemented");
    return false;
}

bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members)
{
    // Implementation would parse archive format
    Utils::debugPrint("Archive member extraction not implemented");
    return false;
}

bool extractArchiveSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols)
{
    // Implementation would parse archive symbol table
    Utils::debugPrint("Archive symbol extraction not implemented");
    return false;
}

bool extractDebugInfo(const std::string& filePath, ComponentInfo& component)
{
    // Try to extract source files from debug info
    std::vector<std::string> sourceFiles;
    if (extractSourceFiles(filePath, sourceFiles))
    {
        for (const auto& sourceFile : sourceFiles)
        {
            component.addSourceFile(sourceFile);
        }
        component.setContainsDebugInfo(true);
        return true;
    }
    
    return false;
}

bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles)
{
    // This would use DWARF parsing libraries or similar
    Utils::debugPrint("Source file extraction not implemented");
    return false;
}

bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& units)
{
    // This would use DWARF parsing libraries
    Utils::debugPrint("Compile unit extraction not implemented");
    return false;
}

std::string detectLicenseFromFile(const std::string& filePath)
{
    // Try to find license information in the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }
    
    // Read a portion of the file to look for license strings
    std::string content;
    content.resize(4096);
    file.read(&content[0], content.size());
    content.resize(file.gcount());
    
    // Convert to string for regex search
    std::string textContent(content.begin(), content.end());
    
    // Look for common license patterns
    std::vector<std::pair<std::regex, std::string>> licensePatterns = {
        {std::regex(R"(GPL|GNU General Public License)", std::regex::icase), "GPL"},
        {std::regex(R"(LGPL|GNU Lesser General Public License)", std::regex::icase), "LGPL"},
        {std::regex(R"(MIT License|MIT)", std::regex::icase), "MIT"},
        {std::regex(R"(Apache License|Apache)", std::regex::icase), "Apache"},
        {std::regex(R"(BSD License|BSD)", std::regex::icase), "BSD"},
        {std::regex(R"(MPL|Mozilla Public License)", std::regex::icase), "MPL"}
    };
    
    for (const auto& pattern : licensePatterns)
    {
        if (std::regex_search(textContent, pattern.first))
        {
            return pattern.second;
        }
    }
    
    return "";
}

std::string detectLicenseFromPath(const std::string& filePath)
{
    // Try to detect license from directory structure
    std::string normalizedPath = Utils::normalizePath(filePath);
    
    if (normalizedPath.find("gpl") != std::string::npos)
    {
        return "GPL";
    }
    else if (normalizedPath.find("lgpl") != std::string::npos)
    {
        return "LGPL";
    }
    else if (normalizedPath.find("mit") != std::string::npos)
    {
        return "MIT";
    }
    else if (normalizedPath.find("apache") != std::string::npos)
    {
        return "Apache";
    }
    else if (normalizedPath.find("bsd") != std::string::npos)
    {
        return "BSD";
    }
    
    return "";
}

std::string detectLicenseFromSymbols(const std::vector<SymbolInfo>& symbols)
{
    // Look for license-related symbols
    for (const auto& symbol : symbols)
    {
        std::string lowerName = Utils::toLower(symbol.name);
        if (lowerName.find("gpl") != std::string::npos)
        {
            return "GPL";
        }
        else if (lowerName.find("lgpl") != std::string::npos)
        {
            return "LGPL";
        }
        else if (lowerName.find("mit") != std::string::npos)
        {
            return "MIT";
        }
        else if (lowerName.find("apache") != std::string::npos)
        {
            return "Apache";
        }
        else if (lowerName.find("bsd") != std::string::npos)
        {
            return "BSD";
        }
    }
    
    return "";
}

std::string detectVersionFromFile(const std::string& filePath)
{
    // Try to extract version from file content
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }
    
    // Read file content and look for version patterns
    std::string content;
    content.resize(4096);
    file.read(&content[0], content.size());
    content.resize(file.gcount());
    
    // Convert to string for regex search
    std::string textContent(content.begin(), content.end());
    
    // Look for version patterns
    std::regex versionRegex(R"((\d+\.\d+\.\d+))");
    std::smatch match;
    
    if (std::regex_search(textContent, match, versionRegex))
    {
        return match[1].str();
    }
    
    return "";
}

std::string detectVersionFromPath(const std::string& filePath)
{
    return Utils::extractVersionFromPath(filePath);
}

std::string detectVersionFromSymbols(const std::vector<SymbolInfo>& symbols)
{
    // Look for version-related symbols
    for (const auto& symbol : symbols)
    {
        std::string lowerName = Utils::toLower(symbol.name);
        if (lowerName.find("version") != std::string::npos)
        {
            // Try to extract version from symbol name
            std::regex versionRegex(R"((\d+\.\d+\.\d+))");
            std::smatch match;
            
            if (std::regex_search(symbol.name, match, versionRegex))
            {
                return match[1].str();
            }
        }
    }
    
    return "";
}

std::vector<std::string> detectDependencies(const std::string& filePath)
{
    std::vector<std::string> dependencies;
    
    // Extract dynamic dependencies
    auto dynamicDeps = extractDynamicDependencies(filePath);
    dependencies.insert(dependencies.end(), dynamicDeps.begin(), dynamicDeps.end());
    
    // Extract static dependencies
    auto staticDeps = extractStaticDependencies(filePath);
    dependencies.insert(dependencies.end(), staticDeps.begin(), staticDeps.end());
    
    return dependencies;
}

std::vector<std::string> extractDynamicDependencies(const std::string& filePath)
{
    std::vector<std::string> dependencies;
    
    // Use platform-specific extraction
#ifdef __APPLE__
    if (MetadataHelpers::isMachO(filePath)) {
        dependencies = extractMachOLinkedLibraries(filePath);
    }
#endif
    
    return dependencies;
}

std::vector<std::string> extractStaticDependencies(const std::string& filePath)
{
    std::vector<std::string> dependencies;
    
    // This would parse static library dependencies
    // For now, return empty vector
    Utils::debugPrint("Static dependency extraction not implemented");
    
    return dependencies;
}

std::vector<std::string> extractMachOLinkedLibraries(const std::string& filePath)
{
    std::vector<std::string> libraries;
    
#ifdef __APPLE__
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Utils::debugPrint("Failed to open Mach-O file for library extraction: " + filePath);
        return libraries;
    }

    // Handle fat binaries
    uint32_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.seekg(0);

    if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
        struct fat_header fatHeader;
        file.read(reinterpret_cast<char*>(&fatHeader), sizeof(fatHeader));
        uint32_t nfat_arch = OSSwapBigToHostInt32(fatHeader.nfat_arch);
        struct fat_arch arch;
        file.read(reinterpret_cast<char*>(&arch), sizeof(arch));
        uint32_t offset = OSSwapBigToHostInt32(arch.offset);
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.seekg(offset);
    }

    bool is64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    
    // Read Mach-O header
    if (is64) {
        struct mach_header_64 mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        
        // Iterate load commands
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            
            // Check for dynamic library load commands
            if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB || 
                lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB) {
                
                struct dylib_command dylib_cmd;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));
                
                // Read library name
                std::string libName;
                char ch;
                file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
                while (file.get(ch) && ch != '\0') {
                    libName += ch;
                }
                
                if (!libName.empty()) {
                    libraries.push_back(libName);
                }
            }
            
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    } else {
        struct mach_header mh;
        file.read(reinterpret_cast<char*>(&mh), sizeof(mh));
        uint32_t ncmds = mh.ncmds;
        
        for (uint32_t i = 0; i < ncmds; ++i) {
            std::streampos cmdStart = file.tellg();
            struct load_command lc;
            file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
            
            if (lc.cmd == LC_LOAD_DYLIB || lc.cmd == LC_LOAD_WEAK_DYLIB || 
                lc.cmd == LC_REEXPORT_DYLIB || lc.cmd == LC_LAZY_LOAD_DYLIB) {
                
                struct dylib_command dylib_cmd;
                file.seekg(cmdStart);
                file.read(reinterpret_cast<char*>(&dylib_cmd), sizeof(dylib_cmd));
                
                std::string libName;
                char ch;
                file.seekg(cmdStart + static_cast<std::streamoff>(dylib_cmd.dylib.name.offset));
                while (file.get(ch) && ch != '\0') {
                    libName += ch;
                }
                
                if (!libName.empty()) {
                    libraries.push_back(libName);
                }
            }
            
            file.seekg(cmdStart + static_cast<std::streamoff>(lc.cmdsize));
        }
    }
#else
    Utils::debugPrint("Mach-O library extraction not supported on this platform");
#endif

    return libraries;
}

} // namespace MetadataHelpers

} // namespace heimdall
