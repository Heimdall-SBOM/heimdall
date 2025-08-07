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
 * @file CompilerMetadata.cpp
 * @brief Implementation of compiler metadata structures and collection
 * @author Trevor Bakker
 * @date 2025
 */

#include "CompilerMetadata.hpp"
#include "../../compat/compatibility.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <openssl/md5.h>

namespace heimdall
{
namespace compiler
{

// ComponentHashes implementation
nlohmann::json ComponentHashes::to_json() const
{
    nlohmann::json j;
    j["sha256"] = sha256;
    j["sha1"] = sha1;
    j["md5"] = md5;
    j["file_size"] = file_size;
    return j;
}

void ComponentHashes::from_json(const nlohmann::json& j)
{
    if (j.contains("sha256")) sha256 = j["sha256"];
    if (j.contains("sha1")) sha1 = j["sha1"];
    if (j.contains("md5")) md5 = j["md5"];
    if (j.contains("file_size")) file_size = j["file_size"];
}

// FileComponent implementation
nlohmann::json FileComponent::to_json() const
{
    nlohmann::json j;
    j["file_path"] = file_path;
    j["relative_path"] = relative_path;
    j["file_type"] = file_type;
    j["hashes"] = hashes.to_json();
    
    // License information
    if (!license.name.empty()) {
        nlohmann::json license_json;
        license_json["name"] = license.name;
        license_json["spdxId"] = license.spdxId;
        license_json["confidence"] = license.confidence;
        if (!license.copyright.empty()) license_json["copyright"] = license.copyright;
        if (!license.author.empty()) license_json["author"] = license.author;
        j["license"] = license_json;
    }
    
    j["copyright_notice"] = copyright_notice;
    j["authors"] = authors;
    j["modification_time"] = modification_time;
    j["is_system_file"] = is_system_file;
    j["is_generated"] = is_generated;
    
    return j;
}

void FileComponent::from_json(const nlohmann::json& j)
{
    if (j.contains("file_path")) file_path = j["file_path"];
    if (j.contains("relative_path")) relative_path = j["relative_path"];
    if (j.contains("file_type")) file_type = j["file_type"];
    
    if (j.contains("hashes")) {
        hashes.from_json(j["hashes"]);
    }
    
    if (j.contains("license")) {
        const auto& license_json = j["license"];
        if (license_json.contains("name")) license.name = license_json["name"];
        if (license_json.contains("spdxId")) license.spdxId = license_json["spdxId"];
        if (license_json.contains("confidence")) license.confidence = license_json["confidence"];
        if (license_json.contains("copyright")) license.copyright = license_json["copyright"];
        if (license_json.contains("author")) license.author = license_json["author"];
    }
    
    if (j.contains("copyright_notice")) copyright_notice = j["copyright_notice"];
    if (j.contains("authors")) authors = j["authors"];
    if (j.contains("modification_time")) modification_time = j["modification_time"];
    if (j.contains("is_system_file")) is_system_file = j["is_system_file"];
    if (j.contains("is_generated")) is_generated = j["is_generated"];
}

// CompilerMetadata implementation
std::vector<LicenseInfo> CompilerMetadata::getUniqueLicenses() const
{
    std::vector<LicenseInfo> unique_licenses;
    std::set<std::string> seen_licenses;
    
    auto addLicense = [&](const LicenseInfo& license) {
        if (!license.name.empty() && seen_licenses.find(license.spdxId) == seen_licenses.end()) {
            unique_licenses.push_back(license);
            seen_licenses.insert(license.spdxId);
        }
    };
    
    for (const auto& file : source_files) {
        addLicense(file.license);
    }
    
    for (const auto& file : include_files) {
        addLicense(file.license);
    }
    
    return unique_licenses;
}

std::map<std::string, size_t> CompilerMetadata::getFileStatistics() const
{
    std::map<std::string, size_t> stats;
    
    for (const auto& file : source_files) {
        stats[file.file_type]++;
    }
    
    for (const auto& file : include_files) {
        stats[file.file_type]++;
    }
    
    return stats;
}

nlohmann::json CompilerMetadata::to_json() const
{
    nlohmann::json j;
    j["compiler_type"] = compiler_type;
    j["compiler_version"] = compiler_version;
    j["main_source_file"] = main_source_file;
    j["object_file"] = object_file;
    j["project_root"] = project_root;
    
    // Convert source files
    nlohmann::json source_array = nlohmann::json::array();
    for (const auto& file : source_files) {
        source_array.push_back(file.to_json());
    }
    j["source_files"] = source_array;
    
    // Convert include files
    nlohmann::json include_array = nlohmann::json::array();
    for (const auto& file : include_files) {
        include_array.push_back(file.to_json());
    }
    j["include_files"] = include_array;
    
    j["functions"] = functions;
    j["global_variables"] = global_variables;
    j["macro_definitions"] = macro_definitions;
    j["compiler_flags"] = compiler_flags;
    j["target_architecture"] = target_architecture;
    j["compilation_timestamp"] = compilation_timestamp;
    
    return j;
}

void CompilerMetadata::from_json(const nlohmann::json& j)
{
    if (j.contains("compiler_type")) compiler_type = j["compiler_type"];
    if (j.contains("compiler_version")) compiler_version = j["compiler_version"];
    if (j.contains("main_source_file")) main_source_file = j["main_source_file"];
    if (j.contains("object_file")) object_file = j["object_file"];
    if (j.contains("project_root")) project_root = j["project_root"];
    
    if (j.contains("source_files")) {
        source_files.clear();
        for (const auto& file_json : j["source_files"]) {
            FileComponent file;
            file.from_json(file_json);
            source_files.push_back(file);
        }
    }
    
    if (j.contains("include_files")) {
        include_files.clear();
        for (const auto& file_json : j["include_files"]) {
            FileComponent file;
            file.from_json(file_json);
            include_files.push_back(file);
        }
    }
    
    if (j.contains("functions")) functions = j["functions"];
    if (j.contains("global_variables")) global_variables = j["global_variables"];
    if (j.contains("macro_definitions")) macro_definitions = j["macro_definitions"];
    if (j.contains("compiler_flags")) compiler_flags = j["compiler_flags"];
    if (j.contains("target_architecture")) target_architecture = j["target_architecture"];
    if (j.contains("compilation_timestamp")) compilation_timestamp = j["compilation_timestamp"];
}

// CompilerMetadataCollector implementation
CompilerMetadataCollector::CompilerMetadataCollector()
    : verbose_(false)
{
    license_detector_ = std::make_unique<LicenseDetector>();
    output_directory_ = "/tmp/heimdall-metadata-" + std::to_string(getpid());
}

CompilerMetadataCollector::~CompilerMetadataCollector() = default;

void CompilerMetadataCollector::setCompilerType(const std::string& type)
{
    metadata_.compiler_type = type;
}

void CompilerMetadataCollector::setCompilerVersion(const std::string& version)
{
    metadata_.compiler_version = version;
}

void CompilerMetadataCollector::setMainSourceFile(const std::string& file)
{
    metadata_.main_source_file = file;
}

void CompilerMetadataCollector::setObjectFile(const std::string& file)
{
    metadata_.object_file = file;
}

void CompilerMetadataCollector::addSourceFile(const std::string& file)
{
    processFileComponent(file, "source");
}

void CompilerMetadataCollector::addIncludeFile(const std::string& file)
{
    processFileComponent(file, "header");
}

void CompilerMetadataCollector::addFunction(const std::string& function)
{
    metadata_.functions.push_back(function);
}

void CompilerMetadataCollector::addGlobalVariable(const std::string& variable)
{
    metadata_.global_variables.push_back(variable);
}

void CompilerMetadataCollector::addMacroDefinition(const std::string& macro)
{
    metadata_.macro_definitions.push_back(macro);
}

void CompilerMetadataCollector::addCompilerFlag(const std::string& key, const std::string& value)
{
    metadata_.compiler_flags[key] = value;
}

void CompilerMetadataCollector::setTargetArchitecture(const std::string& arch)
{
    metadata_.target_architecture = arch;
}

void CompilerMetadataCollector::setProjectRoot(const std::string& root)
{
    metadata_.project_root = root;
}

void CompilerMetadataCollector::processFileComponent(const std::string& file_path, 
                                                   const std::string& file_type)
{
    FileComponent component(file_path);
    component.file_type = file_type;
    
    // Calculate relative path from project root
    if (!metadata_.project_root.empty()) {
        component.relative_path = getRelativePath(file_path, metadata_.project_root);
    } else {
        component.relative_path = file_path;
    }
    
    // Calculate file hashes
    component.hashes = calculateFileHashes(file_path);
    
    // Detect license information
    component.license = detectFileLicense(file_path);
    
    // Extract copyright and author information
    component.copyright_notice = extractCopyrightNotice(file_path);
    component.authors = extractAuthorInfo(file_path);
    
    // Set file attributes
    component.is_system_file = isSystemFile(file_path);
    component.is_generated = isGeneratedFile(file_path);
    
    // Get file modification time
    component.modification_time = getFileModificationTime(file_path);
    
    // Add to appropriate collection
    if (file_type == "source") {
        metadata_.source_files.push_back(component);
    } else {
        // Determine if it's a system header
        if (component.is_system_file) {
            component.file_type = "system_header";
        }
        metadata_.include_files.push_back(component);
    }
    
    if (verbose_) {
        Utils::infoPrint("Processed file component: " + file_path + " [" + file_type + "]");
    }
}

ComponentHashes CompilerMetadataCollector::calculateFileHashes(const std::string& file_path)
{
    ComponentHashes hashes;
    
    // Check cache first to avoid recalculating
    auto cache_it = hash_cache_.find(file_path);
    if (cache_it != hash_cache_.end()) {
        return cache_it->second;
    }
    
    if (Utils::fileExists(file_path)) {
        hashes.sha256 = Utils::calculateSHA256(file_path);
        hashes.sha1 = Utils::getFileSHA1Checksum(file_path);
        hashes.md5 = calculateMD5(file_path);
        hashes.file_size = Utils::getFileSize(file_path);
        
        // Cache the results
        hash_cache_[file_path] = hashes;
        
        if (verbose_) {
            Utils::debugPrint("Calculated hashes for: " + file_path + 
                            " (SHA256: " + hashes.sha256.substr(0, 16) + "...)");
        }
    }
    
    return hashes;
}

LicenseInfo CompilerMetadataCollector::detectFileLicense(const std::string& file_path)
{
    LicenseInfo license;
    
    // Use Heimdall's existing license detector
    if (!license_detector_->detectLicenseFromFile(file_path, license)) {
        // Fallback to path-based detection
        std::string detected_license = Utils::detectLicenseFromPath(file_path);
        if (!detected_license.empty()) {
            license.name = detected_license;
            license.spdxId = convertToSPDXId(detected_license);
            license.confidence = 0.7; // Lower confidence for path-based detection
        }
    }
    
    if (verbose_ && !license.name.empty()) {
        Utils::debugPrint("Detected license for " + file_path + ": " + 
                         license.name + " (confidence: " + std::to_string(license.confidence) + ")");
    }
    
    return license;
}

std::string CompilerMetadataCollector::extractCopyrightNotice(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return "";
    }
    
    std::string line;
    std::string copyright_notice;
    
    // Read first 50 lines looking for copyright notices
    int line_count = 0;
    while (std::getline(file, line) && line_count < 50) {
        std::string lower_line = Utils::toLower(line);
        if (lower_line.find("copyright") != std::string::npos ||
            lower_line.find("(c)") != std::string::npos ||
            lower_line.find("©") != std::string::npos) {
            copyright_notice = Utils::trim(line);
            // Remove comment characters
            copyright_notice = std::regex_replace(copyright_notice, std::regex("^[/*#\\s]*"), "");
            copyright_notice = std::regex_replace(copyright_notice, std::regex("[*/\\s]*$"), "");
            break;
        }
        line_count++;
    }
    
    return copyright_notice;
}

std::vector<std::string> CompilerMetadataCollector::extractAuthorInfo(const std::string& file_path)
{
    std::vector<std::string> authors;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return authors;
    }
    
    std::string line;
    
    // Read first 100 lines looking for author information
    int line_count = 0;
    while (std::getline(file, line) && line_count < 100) {
        std::string lower_line = Utils::toLower(line);
        if (lower_line.find("@author") != std::string::npos ||
            lower_line.find("author:") != std::string::npos ||
            lower_line.find("by:") != std::string::npos) {
            
            // Extract author name from the line
            std::string author = extractAuthorFromLine(line);
            if (!author.empty()) {
                authors.push_back(author);
            }
        }
        line_count++;
    }
    
    return authors;
}

bool CompilerMetadataCollector::isSystemFile(const std::string& file_path)
{
    // Check if file is in system directories
    std::vector<std::string> system_paths = {
        "/usr/include",
        "/usr/local/include",
        "/opt/",
        "/System/Library",    // macOS
        "/Library/Developer", // macOS Xcode
        "/Applications/Xcode.app" // macOS Xcode
    };
    
    for (const auto& sys_path : system_paths) {
        if (file_path.find(sys_path) == 0) {
            return true;
        }
    }
    
    return false;
}

bool CompilerMetadataCollector::isGeneratedFile(const std::string& file_path)
{
    // Check for generated file patterns
    std::vector<std::string> generated_patterns = {
        "_generated",
        ".generated",
        "moc_",           // Qt MOC files
        "ui_",            // Qt UI files
        ".pb.h",          // Protocol buffers
        ".pb.cc",
        "_wrap.c",        // SWIG wrappers
        "_wrap.h",
        ".yacc.",         // Yacc/Bison
        ".lex.",          // Lex/Flex
        "lex.yy.c"        // Flex output
    };
    
    std::string filename = Utils::getFileName(file_path);
    for (const auto& pattern : generated_patterns) {
        if (filename.find(pattern) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

void CompilerMetadataCollector::setOutputDirectory(const std::string& dir)
{
    output_directory_ = dir;
}

void CompilerMetadataCollector::setVerbose(bool verbose)
{
    verbose_ = verbose;
    if (license_detector_) {
        license_detector_->setVerbose(verbose);
    }
}

void CompilerMetadataCollector::writeMetadata()
{
    ensureOutputDirectory();
    
    std::string metadata_file = getMetadataFilePath();
    std::ofstream file(metadata_file);
    
    if (!file.is_open()) {
        Utils::errorPrint("Failed to write metadata file: " + metadata_file);
        return;
    }
    
    nlohmann::json j = metadata_.to_json();
    file << j.dump(2);
    file.close();
    
    if (verbose_) {
        Utils::infoPrint("Wrote compiler metadata to: " + metadata_file);
        Utils::infoPrint("Processed " + std::to_string(metadata_.getTotalFileCount()) + " files");
    }
}

std::string CompilerMetadataCollector::getMetadataFilePath() const
{
    std::string filename = generateMetadataFileName(metadata_.main_source_file);
    return output_directory_ + "/" + filename;
}

// Static methods
std::vector<CompilerMetadata> CompilerMetadataCollector::loadMetadataFiles(const std::string& directory)
{
    std::vector<CompilerMetadata> metadata_list;
    
    if (!compat::fs::exists(directory)) {
        return metadata_list;
    }
    
    for (const auto& entry : compat::fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path().string());
            if (file.is_open()) {
                try {
                    nlohmann::json j;
                    file >> j;
                    
                    CompilerMetadata metadata;
                    metadata.from_json(j);
                    metadata_list.push_back(metadata);
                } catch (const std::exception& e) {
                    Utils::warningPrint("Failed to load metadata file: " + entry.path().string() + 
                                       " - " + e.what());
                }
            }
        }
    }
    
    return metadata_list;
}

void CompilerMetadataCollector::cleanupMetadataFiles(const std::string& directory)
{
    if (!compat::fs::exists(directory)) {
        return;
    }
    
    try {
        compat::fs::remove_all(directory);
    } catch (const std::exception& e) {
        Utils::warningPrint("Failed to cleanup metadata directory: " + directory + 
                           " - " + e.what());
    }
}

std::string CompilerMetadataCollector::generateMetadataFileName(const std::string& source_file)
{
    std::string basename = Utils::getFileName(source_file);
    // Replace dots and slashes with underscores
    std::replace(basename.begin(), basename.end(), '.', '_');
    std::replace(basename.begin(), basename.end(), '/', '_');
    return "heimdall_" + basename + "_" + std::to_string(std::time(nullptr)) + ".json";
}

// Private helper methods
std::string CompilerMetadataCollector::getRelativePath(const std::string& file_path, 
                                                      const std::string& base_path) const
{
    try {
        compat::fs::path file(file_path);
        compat::fs::path base(base_path);
        return compat::fs::relative(file, base).string();
    } catch (const std::exception&) {
        return file_path; // Return original path if relative calculation fails
    }
}

std::string CompilerMetadataCollector::getFileModificationTime(const std::string& file_path) const
{
    try {
        auto ftime = compat::fs::last_write_time(file_path);
        auto time_t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::time_point() + 
            std::chrono::duration_cast<std::chrono::system_clock::duration>(ftime.time_since_epoch())
        );
        
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    } catch (const std::exception&) {
        return "";
    }
}

std::string CompilerMetadataCollector::extractAuthorFromLine(const std::string& line) const
{
    // Simple regex to extract author names
    std::regex author_regex(R"(@author\s+([^*\n]+)|author:\s*([^*\n]+)|by:\s*([^*\n]+))", 
                           std::regex_constants::icase);
    std::smatch match;
    
    if (std::regex_search(line, match, author_regex)) {
        for (size_t i = 1; i < match.size(); ++i) {
            if (match[i].matched) {
                std::string author = match[i].str();
                return Utils::trim(author);
            }
        }
    }
    
    return "";
}

std::string CompilerMetadataCollector::convertToSPDXId(const std::string& license_name) const
{
    // Simple mapping of common license names to SPDX IDs
    std::map<std::string, std::string> license_map = {
        {"MIT", "MIT"},
        {"MIT License", "MIT"},
        {"Apache", "Apache-2.0"},
        {"Apache License", "Apache-2.0"},
        {"Apache License 2.0", "Apache-2.0"},
        {"Apache-2.0", "Apache-2.0"},
        {"GPL", "GPL-3.0-or-later"},
        {"GPL v3", "GPL-3.0-or-later"},
        {"GPLv3", "GPL-3.0-or-later"},
        {"GPL-3.0", "GPL-3.0-or-later"},
        {"BSD", "BSD-3-Clause"},
        {"BSD License", "BSD-3-Clause"},
        {"ISC", "ISC"},
        {"Unlicense", "Unlicense"}
    };
    
    auto it = license_map.find(license_name);
    if (it != license_map.end()) {
        return it->second;
    }
    
    return license_name; // Return original if no mapping found
}

void CompilerMetadataCollector::ensureOutputDirectory() const
{
    if (!compat::fs::exists(output_directory_)) {
        try {
            compat::fs::create_directories(output_directory_);
        } catch (const std::exception& e) {
            Utils::errorPrint("Failed to create output directory: " + output_directory_ + 
                             " - " + e.what());
        }
    }
}

std::string CompilerMetadataCollector::calculateMD5(const std::string& file_path) const
{
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    MD5_CTX ctx;
    MD5_Init(&ctx);
    
    char buffer[8192];
    while (file.read(buffer, sizeof(buffer))) {
        MD5_Update(&ctx, buffer, file.gcount());
    }
    if (file.gcount() > 0) {
        MD5_Update(&ctx, buffer, file.gcount());
    }
    
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &ctx);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    
    return ss.str();
}

// Static utility methods for cleanup
size_t CompilerMetadataCollector::cleanupOldMetadataFiles(const std::string& metadata_dir, int max_age_hours)
{
    if (!compat::fs::exists(metadata_dir)) {
        return 0;
    }
    
    size_t cleaned_count = 0;
    auto now = std::chrono::system_clock::now();
    auto max_age = std::chrono::hours(max_age_hours);
    
    try {
        for (const auto& entry : compat::fs::directory_iterator(metadata_dir)) {
            if (entry.path().extension() == ".json") {
                auto file_time_fs = compat::fs::last_write_time(entry.path());
                // Convert filesystem time to system time
                auto file_time = std::chrono::system_clock::time_point() + 
                    std::chrono::duration_cast<std::chrono::system_clock::duration>(file_time_fs.time_since_epoch());
                auto file_age = now - file_time;
                
                if (file_age > max_age) {
                    try {
                        compat::fs::remove(entry.path());
                        cleaned_count++;
                    } catch (const std::exception& e) {
                        Utils::warningPrint("Failed to remove old metadata file: " + 
                                           entry.path().string() + " - " + e.what());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to cleanup old metadata files: " + std::string(e.what()));
    }
    
    return cleaned_count;
}

std::pair<size_t, size_t> CompilerMetadataCollector::getMetadataStatistics(const std::string& metadata_dir)
{
    size_t file_count = 0;
    size_t total_size = 0;
    
    if (!compat::fs::exists(metadata_dir)) {
        return {0, 0};
    }
    
    try {
        for (const auto& entry : compat::fs::directory_iterator(metadata_dir)) {
            if (entry.path().extension() == ".json") {
                file_count++;
                try {
                    total_size += compat::fs::file_size(entry.path());
                } catch (const std::exception& e) {
                    Utils::warningPrint("Failed to get size of metadata file: " + 
                                       entry.path().string() + " - " + e.what());
                }
            }
        }
    } catch (const std::exception& e) {
        Utils::errorPrint("Failed to get metadata statistics: " + std::string(e.what()));
    }
    
    return {file_count, total_size};
}

}  // namespace compiler
}  // namespace heimdall