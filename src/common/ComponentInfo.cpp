/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file ComponentInfo.cpp
 * @brief Implementation of ComponentInfo class and related data structures
 * @author Trevor Bakker
 * @date 2025
 */

#include "ComponentInfo.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "Utils.hpp"

namespace heimdall {

/**
 * @brief Calculate SHA256 checksum of a file using modern OpenSSL EVP API
 * @param filePath The path to the file
 * @return The SHA256 hash as a hexadecimal string
 */
std::string calculateSHA256(const std::string& filePath) {
    return Utils::getFileChecksum(filePath);
}

/**
 * @brief Helper function to get file size
 * @param filePath The path to the file
 * @return The file size in bytes
 */
uint64_t getFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    return file.tellg();
}

/**
 * @brief Helper function to determine file type from extension
 * @param filePath The path to the file
 * @return The determined file type
 */
FileType determineFileType(const std::string& filePath) {
    std::string lowerPath = filePath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    // Helper function to check if string ends with suffix
    auto endsWith = [](const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length())
            return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    };

    if (endsWith(lowerPath, ".o") || endsWith(lowerPath, ".obj")) {
        return FileType::Object;
    } else if (endsWith(lowerPath, ".a") || endsWith(lowerPath, ".lib")) {
        return FileType::StaticLibrary;
    } else if (endsWith(lowerPath, ".so") || endsWith(lowerPath, ".dylib") ||
               endsWith(lowerPath, ".dll")) {
        return FileType::SharedLibrary;
    } else if (endsWith(lowerPath, ".exe") || lowerPath.find("bin/") != std::string::npos) {
        return FileType::Executable;
    }

    return FileType::Unknown;
}

/**
 * @brief Constructor with component name and file path
 * @param componentName The name of the component
 * @param path The file path
 */
ComponentInfo::ComponentInfo(std::string componentName, const std::string& path)
    : name(std::move(componentName)),
      filePath(path),
      fileType(determineFileType(path)),
      fileSize(getFileSize(path)),
      wasProcessed(false),
      detectedBy(LinkerType::Unknown),
      isSystemLibrary(false),
      containsDebugInfo(false),
      isStripped(false) {
    checksum = calculateSHA256(path);
}

/**
 * @brief Add a dependency to the component
 * @param dependency The dependency to add
 */
void ComponentInfo::addDependency(const std::string& dependency) {
    if (std::find(dependencies.begin(), dependencies.end(), dependency) == dependencies.end()) {
        dependencies.push_back(dependency);
    }
}

/**
 * @brief Add a source file to the component
 * @param sourceFile The source file to add
 */
void ComponentInfo::addSourceFile(const std::string& sourceFile) {
    if (std::find(sourceFiles.begin(), sourceFiles.end(), sourceFile) == sourceFiles.end()) {
        sourceFiles.push_back(sourceFile);
    }
}

/**
 * @brief Set the component version
 * @param ver The version string
 */
void ComponentInfo::setVersion(const std::string& ver) {
    version = ver;
}

/**
 * @brief Set the component supplier
 * @param sup The supplier name
 */
void ComponentInfo::setSupplier(const std::string& sup) {
    supplier = sup;
}

/**
 * @brief Set the download location
 * @param location The download URL
 */
void ComponentInfo::setDownloadLocation(const std::string& location) {
    downloadLocation = location;
}

/**
 * @brief Set the homepage URL
 * @param page The homepage URL
 */
void ComponentInfo::setHomepage(const std::string& page) {
    homepage = page;
}

/**
 * @brief Set the license information
 * @param lic The license string
 */
void ComponentInfo::setLicense(const std::string& lic) {
    license = lic;
}

/**
 * @brief Set the package manager
 * @param pkgMgr The package manager name
 */
void ComponentInfo::setPackageManager(const std::string& pkgMgr) {
    packageManager = pkgMgr;
}

/**
 * @brief Mark the component as processed
 */
void ComponentInfo::markAsProcessed() {
    wasProcessed = true;
}

/**
 * @brief Set a processing error message
 * @param error The error message
 */
void ComponentInfo::setProcessingError(const std::string& error) {
    processingError = error;
    wasProcessed = false;
}

/**
 * @brief Set the linker that detected this component
 * @param linker The linker type
 */
void ComponentInfo::setDetectedBy(LinkerType linker) {
    detectedBy = linker;
}

/**
 * @brief Add a property to the component
 * @param key The property key
 * @param value The property value
 */
void ComponentInfo::addProperty(const std::string& key, const std::string& value) {
    properties[key] = value;
}

/**
 * @brief Get a property value
 * @param key The property key
 * @return The property value, or empty string if not found
 */
std::string ComponentInfo::getProperty(const std::string& key) const {
    auto it = properties.find(key);
    return (it != properties.end()) ? it->second : "";
}

/**
 * @brief Mark the component as a system library
 */
void ComponentInfo::markAsSystemLibrary() {
    isSystemLibrary = true;
}

/**
 * @brief Set whether the component contains debug information
 * @param hasDebug true if debug info is present
 */
void ComponentInfo::setContainsDebugInfo(bool hasDebug) {
    containsDebugInfo = hasDebug;
}

/**
 * @brief Set whether the component has been stripped
 * @param stripped true if the file has been stripped
 */
void ComponentInfo::setStripped(bool stripped) {
    isStripped = stripped;
}

/**
 * @brief Check if the component has a specific symbol
 * @param symbolName The symbol name to look for
 * @return true if the symbol is found
 */
bool ComponentInfo::hasSymbol(const std::string& symbolName) const {
    return std::any_of(symbols.begin(), symbols.end(), [&symbolName](const SymbolInfo& symbol) {
        return symbol.name == symbolName;
    });
}

/**
 * @brief Check if the component has a specific section
 * @param sectionName The section name to look for
 * @return true if the section is found
 */
bool ComponentInfo::hasSection(const std::string& sectionName) const {
    return std::any_of(
        sections.begin(), sections.end(),
        [&sectionName](const SectionInfo& section) { return section.name == sectionName; });
}

/**
 * @brief Get the file type as a string
 * @return String representation of the file type
 */
std::string ComponentInfo::getFileTypeString(const std::string& spdxVersion) const {
    // For SPDX 2.3, SharedLibrary must be mapped to BINARY
    if (spdxVersion == "2.3") {
        switch (fileType) {
            case FileType::Object:
                return "SOURCE"; // or "OBJECT" if you want to be more specific, but SPDX 2.3 uses SOURCE
            case FileType::StaticLibrary:
                return "ARCHIVE";
            case FileType::SharedLibrary:
                return "BINARY";
            case FileType::Executable:
                return "APPLICATION";
            case FileType::Source:
                return "SOURCE";
            default:
                return "OTHER";
        }
    } else {
        // Old behavior for other versions
        switch (fileType) {
            case FileType::Object:
                return "Object";
            case FileType::StaticLibrary:
                return "StaticLibrary";
            case FileType::SharedLibrary:
                return "SharedLibrary";
            case FileType::Executable:
                return "Executable";
            case FileType::Source:
                return "Source";
            default:
                return "Unknown";
        }
    }
}

/**
 * @brief Get the linker type as a string
 * @return String representation of the linker type
 */
std::string ComponentInfo::getLinkerTypeString() const {
    switch (detectedBy) {
        case LinkerType::LLD:
            return "LLD";
        case LinkerType::Gold:
            return "Gold";
        case LinkerType::BFD:
            return "BFD";
        default:
            return "Unknown";
    }
}

}  // namespace heimdall
