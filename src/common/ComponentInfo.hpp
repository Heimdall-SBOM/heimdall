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
 * @file ComponentInfo.hpp
 * @brief Data structures for representing software components and their metadata
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace heimdall {

/**
 * @brief Enumeration of supported file types
 */
enum class FileType : std::uint8_t {
    Unknown,        ///< Unknown file type
    Object,         ///< Object file (.o, .obj)
    StaticLibrary,  ///< Static library (.a, .lib)
    SharedLibrary,  ///< Shared library (.so, .dylib, .dll)
    Executable,     ///< Executable file
    Source          ///< Source file (.c, .cpp, .h, etc.)
};

/**
 * @brief Enumeration of supported linker types
 */
enum class LinkerType : std::uint8_t {
    LLD,     ///< LLVM LLD linker
    Gold,    ///< GNU Gold linker
    BFD,     ///< GNU BFD linker
    Unknown  ///< Unknown linker
};

/**
 * @brief Structure representing a symbol in a binary file
 */
struct SymbolInfo {
    std::string name;        ///< Symbol name
    uint64_t address = 0;    ///< Symbol address
    uint64_t size = 0;       ///< Symbol size
    bool isDefined = false;  ///< Whether the symbol is defined
    bool isWeak = false;     ///< Whether the symbol is weak
    bool isGlobal = false;   ///< Whether the symbol is global
    std::string section;     ///< Section name containing the symbol
};

/**
 * @brief Structure representing a section in a binary file
 */
struct SectionInfo {
    std::string name;      ///< Section name
    uint64_t address = 0;  ///< Section address
    uint64_t size = 0;     ///< Section size
    uint32_t flags = 0;    ///< Section flags
    std::string type;      ///< Section type
};

/**
 * @brief Structure representing a software component with all its metadata
 */
struct ComponentInfo {
    std::string name;                       ///< Component name
    std::string filePath;                   ///< File path
    std::string version;                    ///< Component version
    std::string supplier;                   ///< Component supplier/vendor
    std::string downloadLocation;           ///< Download location URL
    std::string homepage;                   ///< Homepage URL
    std::string license;                    ///< License information
    std::string checksum;                   ///< File checksum
    std::string packageManager;             ///< Package manager name
    FileType fileType = FileType::Unknown;  ///< File type
    uint64_t fileSize = 0;                  ///< File size in bytes

    std::vector<SymbolInfo> symbols;        ///< List of symbols
    std::vector<SectionInfo> sections;      ///< List of sections
    std::vector<std::string> dependencies;  ///< List of dependencies
    std::vector<std::string> sourceFiles;   ///< List of source files
    std::vector<std::string> functions;     ///< List of function names from DWARF
    std::vector<std::string> compileUnits;  ///< List of compile units from DWARF
    std::map<std::string, std::string> properties; ///< Additional properties/metadata

    bool wasProcessed = false;                    ///< Whether the component has been processed
    std::string processingError;                  ///< Error message if processing failed
    LinkerType detectedBy = LinkerType::Unknown;  ///< Linker that detected this component

    bool isSystemLibrary = false;    ///< Whether this is a system library
    bool containsDebugInfo = false;  ///< Whether the file contains debug information
    bool isStripped = false;         ///< Whether the file has been stripped

    /**
     * @brief Default constructor
     */
    ComponentInfo() = default;

    /**
     * @brief Constructor with component name and file path
     * @param componentName The name of the component
     * @param path The file path
     */
    ComponentInfo(std::string componentName, const std::string& path);

    /**
     * @brief Add a symbol to the component
     * @param symbol The symbol to add
     */
    void addSymbol(const SymbolInfo& symbol) {
        symbols.push_back(symbol);
    }

    /**
     * @brief Add a section to the component
     * @param section The section to add
     */
    void addSection(const SectionInfo& section) {
        sections.push_back(section);
    }

    /**
     * @brief Add a dependency to the component
     * @param dependency The dependency to add
     */
    void addDependency(const std::string& dependency);

    /**
     * @brief Add a source file to the component
     * @param sourceFile The source file to add
     */
    void addSourceFile(const std::string& sourceFile);

    /**
     * @brief Set the component version
     * @param ver The version string
     */
    void setVersion(const std::string& ver);

    /**
     * @brief Set the component supplier
     * @param sup The supplier name
     */
    void setSupplier(const std::string& sup);

    /**
     * @brief Set the download location
     * @param location The download URL
     */
    void setDownloadLocation(const std::string& location);

    /**
     * @brief Set the homepage URL
     * @param page The homepage URL
     */
    void setHomepage(const std::string& page);

    /**
     * @brief Set the license information
     * @param lic The license string
     */
    void setLicense(const std::string& lic);

    /**
     * @brief Set the package manager
     * @param pkgMgr The package manager name
     */
    void setPackageManager(const std::string& pkgMgr);

    /**
     * @brief Mark the component as processed
     */
    void markAsProcessed();

    /**
     * @brief Set a processing error message
     * @param error The error message
     */
    void setProcessingError(const std::string& error);

    /**
     * @brief Set the linker that detected this component
     * @param linker The linker type
     */
    void setDetectedBy(LinkerType linker);

    /**
     * @brief Add a property to the component
     * @param key The property key
     * @param value The property value
     */
    void addProperty(const std::string& key, const std::string& value);

    /**
     * @brief Get a property value
     * @param key The property key
     * @return The property value, or empty string if not found
     */
    [[nodiscard]] std::string getProperty(const std::string& key) const;

    /**
     * @brief Mark the component as a system library
     */
    void markAsSystemLibrary();

    /**
     * @brief Set whether the component contains debug information
     * @param hasDebug true if debug info is present
     */
    void setContainsDebugInfo(bool hasDebug);

    /**
     * @brief Set whether the component has been stripped
     * @param stripped true if the file has been stripped
     */
    void setStripped(bool stripped);

    /**
     * @brief Check if the component has a specific symbol
     * @param symbolName The symbol name to look for
     * @return true if the symbol is found
     */
    [[nodiscard]] bool hasSymbol(const std::string& symbolName) const;

    /**
     * @brief Check if the component has a specific section
     * @param sectionName The section name to look for
     * @return true if the section is found
     */
    [[nodiscard]] bool hasSection(const std::string& sectionName) const;

    /**
     * @brief Get the file type as a string
     * @param spdxVersion The SPDX version (optional, default empty)
     * @return String representation of the file type
     */
    [[nodiscard]] std::string getFileTypeString(const std::string& spdxVersion = "") const;

    /**
     * @brief Get the linker type as a string
     * @return String representation of the linker type
     */
    [[nodiscard]] std::string getLinkerTypeString() const;

    /**
     * @brief Get the number of symbols
     * @return Number of symbols
     */
    [[nodiscard]] size_t getSymbolCount() const {
        return symbols.size();
    }

    /**
     * @brief Get the number of sections
     * @return Number of sections
     */
    [[nodiscard]] size_t getSectionCount() const {
        return sections.size();
    }
};

/**
 * @brief Structure representing build information
 */
struct BuildInfo {
    std::string targetName;                       ///< Target name
    std::string targetType;                       ///< Target type
    std::string buildId;                          ///< Build ID
    std::string buildTimestamp;                   ///< Build timestamp
    std::string compiler;                         ///< Compiler name
    std::string compilerVersion;                  ///< Compiler version
    std::string architecture;                     ///< Target architecture
    std::string operatingSystem;                  ///< Target operating system
    LinkerType linkerUsed = LinkerType::Unknown;  ///< Linker used
    std::string linkerVersion;                    ///< Linker version
    std::vector<std::string> linkFlags;           ///< Linker flags
    std::vector<std::string> libraryPaths;        ///< Library search paths
};

}  // namespace heimdall
