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
 * @file CompilerMetadata.hpp
 * @brief Compiler plugin metadata structures and collection interface
 * @author Trevor Bakker
 * @date 2025
 *
 * This file defines the data structures and interfaces for collecting
 * compile-time metadata including file hashes, license information,
 * and build environment details.
 */

#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "../../detectors/LicenseDetector.hpp"
#include "../../common/Utils.hpp"
#include "nlohmann/json.hpp"

namespace heimdall
{
namespace compiler
{

/**
 * @brief Component hash information for integrity verification
 */
struct ComponentHashes
{
    std::string sha256;      ///< SHA-256 hash
    std::string sha1;        ///< SHA-1 hash  
    std::string md5;         ///< MD5 hash
    uint64_t    file_size;   ///< File size in bytes

    /**
     * @brief Default constructor
     */
    ComponentHashes() : file_size(0) {}

    /**
     * @brief Check if hashes are valid (non-empty)
     * @return true if at least SHA-256 hash is present
     */
    bool isValid() const
    {
        return !sha256.empty();
    }

    /**
     * @brief Convert to JSON representation
     * @return JSON object with hash information
     */
    nlohmann::json to_json() const;

    /**
     * @brief Load from JSON representation
     * @param j JSON object to load from
     */
    void from_json(const nlohmann::json& j);
};

/**
 * @brief File component metadata with complete provenance information
 */
struct FileComponent
{
    std::string                file_path;          ///< Full file path
    std::string                relative_path;      ///< Relative path from project root
    std::string                file_type;          ///< File type (source, header, system_header)
    ComponentHashes            hashes;             ///< File hashes for integrity
    LicenseInfo                license;            ///< Detected license information
    std::string                copyright_notice;   ///< Copyright notice from file
    std::vector<std::string>   authors;            ///< Author information
    std::string                modification_time;  ///< Last modification timestamp (ISO 8601)
    bool                       is_system_file;     ///< Whether file is from system directories
    bool                       is_generated;       ///< Whether file is generated (not original source)

    /**
     * @brief Default constructor
     */
    FileComponent() : is_system_file(false), is_generated(false) {}

    /**
     * @brief Constructor with file path
     * @param path File path to initialize with
     */
    FileComponent(const std::string& path) 
        : file_path(path), is_system_file(false), is_generated(false) {}

    /**
     * @brief Get component name (filename without path)
     * @return Component name
     */
    std::string getName() const
    {
        return Utils::getFileName(file_path);
    }

    /**
     * @brief Check if component has valid hash information
     * @return true if hashes are present
     */
    bool hasValidHashes() const
    {
        return hashes.isValid();
    }

    /**
     * @brief Convert to JSON representation
     * @return JSON object with component information
     */
    nlohmann::json to_json() const;

    /**
     * @brief Load from JSON representation
     * @param j JSON object to load from
     */
    void from_json(const nlohmann::json& j);
};

/**
 * @brief Complete compiler metadata for a compilation unit
 */
struct CompilerMetadata
{
    std::string                         compiler_type;          ///< Compiler type (gcc, clang)
    std::string                         compiler_version;       ///< Compiler version string
    std::string                         main_source_file;       ///< Main source file being compiled
    std::string                         object_file;            ///< Output object file path
    std::vector<FileComponent>          source_files;           ///< Source files with metadata
    std::vector<FileComponent>          include_files;          ///< Include files with metadata
    std::vector<std::string>            functions;              ///< Function names defined
    std::vector<std::string>            global_variables;       ///< Global variables defined
    std::vector<std::string>            macro_definitions;      ///< Macro definitions used
    std::map<std::string, std::string>  compiler_flags;         ///< Compiler flags and settings
    std::string                         target_architecture;    ///< Target architecture
    std::string                         compilation_timestamp;  ///< Compilation timestamp (ISO 8601)
    std::string                         project_root;           ///< Project root directory

    /**
     * @brief Default constructor
     */
    CompilerMetadata()
    {
        compilation_timestamp = getCurrentTimestamp();
    }

    /**
     * @brief Get total number of files processed
     * @return Total file count
     */
    size_t getTotalFileCount() const
    {
        return source_files.size() + include_files.size();
    }

    /**
     * @brief Get unique license list from all files
     * @return Vector of unique licenses found
     */
    std::vector<LicenseInfo> getUniqueLicenses() const;

    /**
     * @brief Get statistics about processed files
     * @return Map of file type to count
     */
    std::map<std::string, size_t> getFileStatistics() const;

    /**
     * @brief Convert to JSON representation
     * @return JSON object with complete metadata
     */
    nlohmann::json to_json() const;

    /**
     * @brief Load from JSON representation
     * @param j JSON object to load from
     */
    void from_json(const nlohmann::json& j);

private:
    /**
     * @brief Get current timestamp in ISO 8601 format
     * @return Current timestamp string
     */
    std::string getCurrentTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
        return ss.str();
    }
};

/**
 * @brief Compiler metadata collector and processor
 */
class CompilerMetadataCollector
{
private:
    CompilerMetadata                    metadata_;
    std::string                         output_directory_;
    std::unique_ptr<LicenseDetector>    license_detector_;
    std::map<std::string, ComponentHashes> hash_cache_;
    bool                                verbose_;

public:
    /**
     * @brief Default constructor
     */
    CompilerMetadataCollector();

    /**
     * @brief Destructor
     */
    ~CompilerMetadataCollector();

    /**
     * @brief Copy constructor (deleted)
     */
    CompilerMetadataCollector(const CompilerMetadataCollector&) = delete;

    /**
     * @brief Assignment operator (deleted)
     */
    CompilerMetadataCollector& operator=(const CompilerMetadataCollector&) = delete;

    /**
     * @brief Move constructor
     */
    CompilerMetadataCollector(CompilerMetadataCollector&&) noexcept = default;

    /**
     * @brief Move assignment operator
     */
    CompilerMetadataCollector& operator=(CompilerMetadataCollector&&) noexcept = default;

    // Basic metadata collection methods
    void setCompilerType(const std::string& type);
    void setCompilerVersion(const std::string& version);
    void setMainSourceFile(const std::string& file);
    void setObjectFile(const std::string& file);
    void addSourceFile(const std::string& file);
    void addIncludeFile(const std::string& file);
    void addFunction(const std::string& function);
    void addGlobalVariable(const std::string& variable);
    void addMacroDefinition(const std::string& macro);
    void addCompilerFlag(const std::string& key, const std::string& value);
    void setTargetArchitecture(const std::string& arch);
    void setProjectRoot(const std::string& root);

    // Enhanced component analysis methods
    void processFileComponent(const std::string& file_path, const std::string& file_type);
    ComponentHashes calculateFileHashes(const std::string& file_path);
    LicenseInfo detectFileLicense(const std::string& file_path);
    std::string extractCopyrightNotice(const std::string& file_path);
    std::vector<std::string> extractAuthorInfo(const std::string& file_path);
    bool isSystemFile(const std::string& file_path);
    bool isGeneratedFile(const std::string& file_path);

    // Configuration methods
    void setOutputDirectory(const std::string& dir);
    void setVerbose(bool verbose);

    // Output methods
    void writeMetadata();
    std::string getMetadataFilePath() const;

    // Accessors
    const CompilerMetadata& getMetadata() const { return metadata_; }
    size_t getProcessedFileCount() const { return metadata_.getTotalFileCount(); }

    // Static utility methods
    static std::vector<CompilerMetadata> loadMetadataFiles(const std::string& directory);
    static void cleanupMetadataFiles(const std::string& directory);
    static std::string generateMetadataFileName(const std::string& source_file);
    
    /**
     * @brief Clean up old metadata files based on age
     * @param metadata_dir Directory containing metadata files
     * @param max_age_hours Maximum age in hours before cleanup
     * @return Number of files cleaned up
     */
    static size_t cleanupOldMetadataFiles(const std::string& metadata_dir, int max_age_hours = 24);
    
    /**
     * @brief Get metadata file statistics
     * @param metadata_dir Directory containing metadata files
     * @return Pair of (file_count, total_size_bytes)
     */
    static std::pair<size_t, size_t> getMetadataStatistics(const std::string& metadata_dir);

private:
    // Helper methods
    std::string getRelativePath(const std::string& file_path, const std::string& base_path) const;
    std::string getFileModificationTime(const std::string& file_path) const;
    std::string extractAuthorFromLine(const std::string& line) const;
    std::string convertToSPDXId(const std::string& license_name) const;
    void ensureOutputDirectory() const;
    
    // Hash calculation helpers
    std::string calculateMD5(const std::string& file_path) const;
};

}  // namespace compiler
}  // namespace heimdall