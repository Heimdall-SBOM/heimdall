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
 * @file MetadataExtractor.hpp
 * @brief Main class for extracting metadata from binary files and libraries
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ComponentInfo.hpp"
#include "LazySymbolExtractor.hpp"
#include "../compat/compatibility.hpp"

namespace heimdall {

/**
 * @brief Main class for extracting metadata from binary files
 *
 * This class provides functionality to extract various types of metadata
 * from binary files including symbols, sections, dependencies, and
 * package manager information.
 */
class MetadataExtractor {
public:
    /**
     * @brief Default constructor
     */
    MetadataExtractor();
    /**
     * @brief Destructor
     */
    ~MetadataExtractor();

    /**
     * @brief Extract all metadata from a component
     * @param component The component to extract metadata from
     * @return true if extraction was successful
     */
    bool extractMetadata(ComponentInfo& component);

    /**
     * @brief Extract version information from a component
     * @param component The component to extract version from
     * @return true if version was extracted successfully
     */
    bool extractVersionInfo(ComponentInfo& component);

    /**
     * @brief Extract license information from a component
     * @param component The component to extract license from
     * @return true if license was extracted successfully
     */
    bool extractLicenseInfo(ComponentInfo& component);

    /**
     * @brief Extract symbol information from a component
     * @param component The component to extract symbols from
     * @return true if symbols were extracted successfully
     */
    bool extractSymbolInfo(ComponentInfo& component);

    /**
     * @brief Extract section information from a component
     * @param component The component to extract sections from
     * @return true if sections were extracted successfully
     */
    bool extractSectionInfo(ComponentInfo& component);

    /**
     * @brief Extract debug information from a component
     * @param component The component to extract debug info from
     * @return true if debug info was extracted successfully
     */
    bool extractDebugInfo(ComponentInfo& component);

    /**
     * @brief Extract dependency information from a component
     * @param component The component to extract dependencies from
     * @return true if dependencies were extracted successfully
     */
    bool extractDependencyInfo(ComponentInfo& component);

    /**
     * @brief Check if a file is in ELF format
     * @param filePath The path to the file
     * @return true if the file is in ELF format
     */
    bool isELF(const std::string& filePath);

    /**
     * @brief Check if a file is in Mach-O format
     * @param filePath The path to the file
     * @return true if the file is in Mach-O format
     */
    bool isMachO(const std::string& filePath);

    /**
     * @brief Check if a file is in PE format
     * @param filePath The path to the file
     * @return true if the file is in PE format
     */
    bool isPE(const std::string& filePath);

    /**
     * @brief Check if a file is an archive
     * @param filePath The path to the file
     * @return true if the file is an archive
     */
    bool isArchive(const std::string& filePath);

    /**
     * @brief Extract Conan package manager metadata
     * @param component The component to extract metadata from
     * @return true if metadata was extracted successfully
     */
    bool extractConanMetadata(ComponentInfo& component);

    /**
     * @brief Extract vcpkg package manager metadata
     * @param component The component to extract metadata from
     * @return true if metadata was extracted successfully
     */
    bool extractVcpkgMetadata(ComponentInfo& component);

    /**
     * @brief Extract system package manager metadata
     * @param component The component to extract metadata from
     * @return true if metadata was extracted successfully
     */
    bool extractSystemMetadata(ComponentInfo& component);

    /**
     * @brief Detect RPM package manager metadata
     * @param component The component to detect metadata for
     * @return true if RPM metadata was detected
     */
    bool detectRpmMetadata(ComponentInfo& component);

    /**
     * @brief Detect Debian package manager metadata
     * @param component The component to detect metadata for
     * @return true if Debian metadata was detected
     */
    bool detectDebMetadata(ComponentInfo& component);

    /**
     * @brief Detect Pacman package manager metadata
     * @param component The component to detect metadata for
     * @return true if Pacman metadata was detected
     */
    bool detectPacmanMetadata(ComponentInfo& component);

    /**
     * @brief Set verbose output mode
     * @param verbose true to enable verbose output
     */
    void setVerbose(bool verbose);

    /**
     * @brief Set whether to extract debug information
     * @param extract true to extract debug information
     */
    void setExtractDebugInfo(bool extract);

    /**
     * @brief Set whether to suppress warnings (for test mode)
     * @param suppress true to suppress warnings
     */
    void setSuppressWarnings(bool suppress);

    /**
     * @brief Extract metadata from multiple files with optimal batching
     * @param filePaths Vector of file paths to process
     * @param components Output vector for extracted components
     * @param batch_size Size of processing batches (0 for auto)
     * @return true if extraction was successful
     */
    bool extractMetadataBatched(const std::vector<std::string>& filePaths,
                               std::vector<ComponentInfo>& components,
                               size_t batch_size = 0);

    /**
     * @brief Extract Ada metadata from ALI files
     * @param component The component to extract metadata from
     * @param aliFiles Vector of ALI file paths to parse
     * @return true if extraction was successful
     */
    bool extractAdaMetadata(ComponentInfo& component, 
                          const std::vector<std::string>& aliFiles);

    /**
     * @brief Check if a file is an Ada ALI file
     * @param filePath Path to the file
     * @return true if the file is an ALI file
     */
    bool isAdaAliFile(const std::string& filePath);

    /**
     * @brief Find ALI files in a directory
     * @param directory Directory to search
     * @param aliFiles Output vector of ALI file paths
     * @return true if search was successful
     */
    bool findAdaAliFiles(const std::string& directory, 
                        std::vector<std::string>& aliFiles);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Namespace containing helper functions for metadata extraction
 */
namespace MetadataHelpers {

/**
 * @brief Check if a file is in ELF format
 * @param filePath The path to the file
 * @return true if the file is in ELF format
 */
bool isELF(const std::string& filePath);

/**
 * @brief Check if a file is in PE format
 * @param filePath The path to the file
 * @return true if the file is in PE format
 */
bool isPE(const std::string& filePath);

/**
 * @brief Check if a file is an archive
 * @param filePath The path to the file
 * @return true if the file is an archive
 */
bool isArchive(const std::string& filePath);

/**
 * @brief Extract symbols from an ELF file
 * @param filePath The path to the ELF file
 * @param symbols Vector to store extracted symbols
 * @return true if symbols were extracted successfully
 */
bool extractELFSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);

/**
 * @brief Extract sections from an ELF file
 * @param filePath The path to the ELF file
 * @param sections Vector to store extracted sections
 * @return true if sections were extracted successfully
 */
bool extractELFSections(const std::string& filePath, std::vector<SectionInfo>& sections);

/**
 * @brief Extract version information from an ELF file
 * @param filePath The path to the ELF file
 * @param version String to store the extracted version
 * @return true if version was extracted successfully
 */
bool extractELFVersion(const std::string& filePath, std::string& version);

/**
 * @brief Extract build ID from an ELF file
 * @param filePath The path to the ELF file
 * @param buildId String to store the extracted build ID
 * @return true if build ID was extracted successfully
 */
bool extractELFBuildId(const std::string& filePath, std::string& buildId);

/**
 * @brief Extract dependencies from an ELF file
 * @param filePath The path to the ELF file
 * @return Vector of dependency names
 */
std::vector<std::string> extractELFDependencies(const std::string& filePath);

/**
 * @brief Check if a file is in Mach-O format
 * @param filePath The path to the file
 * @return true if the file is in Mach-O format
 */
bool isMachO(const std::string& filePath);

/**
 * @brief Extract symbols from a Mach-O file
 * @param filePath The path to the Mach-O file
 * @param symbols Vector to store extracted symbols
 * @return true if symbols were extracted successfully
 */
bool extractMachOSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);

/**
 * @brief Extract sections from a Mach-O file
 * @param filePath The path to the Mach-O file
 * @param sections Vector to store extracted sections
 * @return true if sections were extracted successfully
 */
bool extractMachOSections(const std::string& filePath, std::vector<SectionInfo>& sections);

/**
 * @brief Extract version information from a Mach-O file
 * @param filePath The path to the Mach-O file
 * @param version String to store the extracted version
 * @return true if version was extracted successfully
 */
bool extractMachOVersion(const std::string& filePath, std::string& version);

/**
 * @brief Extract UUID from a Mach-O file
 * @param filePath The path to the Mach-O file
 * @param uuid String to store the extracted UUID
 * @return true if UUID was extracted successfully
 */
bool extractMachOUUID(const std::string& filePath, std::string& uuid);

/**
 * @brief Extract linked libraries from a Mach-O file
 * @param filePath The path to the Mach-O file
 * @return Vector of linked library names
 */
std::vector<std::string> extractMachOLinkedLibraries(const std::string& filePath);

/**
 * @brief Extract symbols from a PE file
 * @param filePath The path to the PE file
 * @param symbols Vector to store extracted symbols
 * @return true if symbols were extracted successfully
 */
bool extractPESymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);

/**
 * @brief Extract sections from a PE file
 * @param filePath The path to the PE file
 * @param sections Vector to store extracted sections
 * @return true if sections were extracted successfully
 */
bool extractPESections(const std::string& filePath, std::vector<SectionInfo>& sections);

/**
 * @brief Extract version information from a PE file
 * @param filePath The path to the PE file
 * @param version String to store the extracted version
 * @return true if version was extracted successfully
 */
bool extractPEVersion(const std::string& filePath, std::string& version);

/**
 * @brief Extract company name from a PE file
 * @param filePath The path to the PE file
 * @param company String to store the extracted company name
 * @return true if company name was extracted successfully
 */
bool extractPECompanyName(const std::string& filePath, std::string& company);

/**
 * @brief Extract member names from an archive file
 * @param filePath The path to the archive file
 * @param members Vector to store extracted member names
 * @return true if members were extracted successfully
 */
bool extractArchiveMembers(const std::string& filePath, std::vector<std::string>& members);

/**
 * @brief Extract symbols from an archive file
 * @param filePath The path to the archive file
 * @param symbols Vector to store extracted symbols
 * @return true if symbols were extracted successfully
 */
bool extractArchiveSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols);

/**
 * @brief Extract debug information from a file
 * @param filePath The path to the file
 * @param component The component to store debug info in
 * @return true if debug info was extracted successfully
 */
bool extractDebugInfo(const std::string& filePath, ComponentInfo& component);

/**
 * @brief Extract source file names from debug information
 * @param filePath The path to the file
 * @param sourceFiles Vector to store extracted source file names
 * @return true if source files were extracted successfully
 */
bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles);

/**
 * @brief Extract compile unit names from debug information
 * @param filePath The path to the file
 * @param units Vector to store extracted compile unit names
 * @return true if compile units were extracted successfully
 */
bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& units);

/**
 * @brief Detect license from file content
 * @param filePath The path to the file
 * @return The detected license string
 */
std::string detectLicenseFromFile(const std::string& filePath);

/**
 * @brief Detect license from file path
 * @param filePath The path to the file
 * @return The detected license string
 */
std::string detectLicenseFromPath(const std::string& filePath);

/**
 * @brief Detect license from symbol names
 * @param symbols Vector of symbols to analyze
 * @return The detected license string
 */
std::string detectLicenseFromSymbols(const std::vector<SymbolInfo>& symbols);

/**
 * @brief Detect version from file content
 * @param filePath The path to the file
 * @return The detected version string
 */
std::string detectVersionFromFile(const std::string& filePath);

/**
 * @brief Detect version from file path
 * @param filePath The path to the file
 * @return The detected version string
 */
std::string detectVersionFromPath(const std::string& filePath);

/**
 * @brief Detect version from symbol names
 * @param symbols Vector of symbols to analyze
 * @return The detected version string
 */
std::string detectVersionFromSymbols(const std::vector<SymbolInfo>& symbols);

/**
 * @brief Detect dependencies from a file
 * @param filePath The path to the file
 * @return Vector of dependency names
 */
std::vector<std::string> detectDependencies(const std::string& filePath);

/**
 * @brief Extract dynamic dependencies from a file
 * @param filePath The path to the file
 * @return Vector of dynamic dependency names
 */
std::vector<std::string> extractDynamicDependencies(const std::string& filePath);

/**
 * @brief Extract static dependencies from a file
 * @param filePath The path to the file
 * @return Vector of static dependency names
 */
std::vector<std::string> extractStaticDependencies(const std::string& filePath);

/**
 * @brief Detect RPM package manager metadata
 * @param component The component to detect metadata for
 * @return true if RPM metadata was detected
 */
bool detectRpmMetadata(ComponentInfo& component);

/**
 * @brief Detect Debian package manager metadata
 * @param component The component to detect metadata for
 * @return true if Debian metadata was detected
 */
bool detectDebMetadata(ComponentInfo& component);

/**
 * @brief Detect Pacman package manager metadata
 * @param component The component to detect metadata for
 * @return true if Pacman metadata was detected
 */
bool detectPacmanMetadata(ComponentInfo& component);

/**
 * @brief Detect Conan package manager metadata
 * @param component The component to detect metadata for
 * @return true if Conan metadata was detected
 */
bool detectConanMetadata(ComponentInfo& component);

/**
 * @brief Detect vcpkg package manager metadata
 * @param component The component to detect metadata for
 * @return true if vcpkg metadata was detected
 */
bool detectVcpkgMetadata(ComponentInfo& component);

/**
 * @brief Detect Spack package manager metadata
 * @param component The component to detect metadata for
 * @return true if Spack metadata was detected
 */
bool detectSpackMetadata(ComponentInfo& component);

/**
 * @brief Check if a file is an Ada ALI file
 * @param filePath Path to the file
 * @return true if the file is an ALI file
 */
bool isAdaAliFile(const std::string& filePath);

/**
 * @brief Find ALI files in a directory
 * @param directory Directory to search
 * @param aliFiles Output vector of ALI file paths
 * @return true if search was successful
 */
bool findAdaAliFiles(const std::string& directory, 
                    std::vector<std::string>& aliFiles);

/**
 * @brief Thread-safe test mode control
 * @param enabled Whether to enable test mode
 */
void setTestMode(bool enabled);

/**
 * @brief Check if test mode is enabled
 * @return true if test mode is enabled
 */
bool isTestMode();

/**
 * @brief Extract Ada metadata from ALI files
 * @param aliFiles Vector of ALI file paths
 * @param component The component to store metadata in
 * @return true if extraction was successful
 */
bool extractAdaMetadata(const std::vector<std::string>& aliFiles, 
                       ComponentInfo& component);

}  // namespace MetadataHelpers

}  // namespace heimdall
