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
 * @file Utils.cpp
 * @brief Implementation of utility functions for file operations, string manipulation, and system
 * interactions
 * @author Trevor Bakker
 * @date 2025
 */

#include "Utils.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "../compat/compatibility.hpp"
#include "compat/compatibility.hpp"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#else
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace heimdall::Utils {

std::string getFileName(const std::string& filePath) {
    if (filePath.empty()) {
        return "";
    }
    
    // Manual filename extraction
    size_t last_slash = filePath.find_last_of('/');
    if (last_slash == std::string::npos) {
        return filePath;
    }
    
    if (last_slash == filePath.size() - 1) {
        // Path ends with slash, return empty
        return "";
    }
    
    std::string filename = filePath.substr(last_slash + 1);
    
    // Handle edge case where filename() returns "." for empty paths
    if (filename == "." && filePath.empty()) {
        return "";
    }
    
    return filename;
}
std::string getFileExtension(const std::string& filePath) {
    if (filePath.empty()) {
        return "";
    }
    
    size_t last_dot = filePath.find_last_of('.');
    if (last_dot == std::string::npos || last_dot == 0) {
        return "";
    }
    
    // Check if the dot is not the last character
    if (last_dot == filePath.length() - 1) {
        return "";
    }
    
    return filePath.substr(last_dot);
}
std::string getDirectory(const std::string& filePath) {
    if (filePath.empty()) {
        return "";
    }
    
    // Manual directory extraction
    size_t last_slash = filePath.find_last_of('/');
    if (last_slash == std::string::npos) {
        // No slash found, no directory
        return "";
    }
    
    if (last_slash == 0) {
        // Root directory
        return "/";
    }
    
    std::string directory = filePath.substr(0, last_slash);
    
    // Handle edge case where parent_path() returns "." for relative paths
    if (directory == "." && !filePath.empty() && filePath.find('/') == std::string::npos) {
        return "";
    }
    
    return directory;
}
std::string normalizePath(const std::string& path) {
    if (path.empty()) return "";

    std::vector<std::string> stack;
    bool is_absolute = !path.empty() && path[0] == '/';
    size_t i = 0, n = path.size();

    while (i < n) {
        // Skip consecutive slashes
        while (i < n && path[i] == '/') ++i;
        size_t start = i;
        while (i < n && path[i] != '/') ++i;
        std::string part = path.substr(start, i - start);
        if (part == "" || part == ".") continue;
        if (part == "..") {
            if (!stack.empty() && stack.back() != "..") stack.pop_back();
            else if (!is_absolute) stack.push_back("..");
        } else {
            stack.push_back(part);
        }
    }

    std::string result = is_absolute ? "/" : "";
    for (size_t j = 0; j < stack.size(); ++j) {
        if (j > 0) result += "/";
        result += stack[j];
    }
    if (result.empty()) result = is_absolute ? "/" : "";
    // Preserve trailing slash if present in input (except for root)
    if (path.size() > 1 && path.back() == '/' && result.size() > 1 && result.back() != '/')
        result += '/';
    return result;
}
std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> result;
    
    if (path.empty()) {
        return result;
    }
    
    // Handle root directory specially
    if (path == "/" || path == "\\") {
        result.push_back("/");
        return result;
    }
    
    // Manual path splitting
    size_t start = 0;
    if (path[0] == '/') {
        result.push_back("/");
        start = 1;
    }
    
    while (start < path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string::npos) {
            end = path.size();
        }
        
        if (end > start) {
            std::string part = path.substr(start, end - start);
            result.push_back(part);
        }
        
        start = end + 1;
    }
    
    // For absolute paths, ensure we have at least root and one component
    if (path[0] == '/' && result.size() == 1 && result[0] == "/") {
        // This is just "/", which is correct
    } else if (path[0] == '/' && result.size() == 1) {
        // This is "/something", which is correct
    }
    
    return result;
}
bool fileExists(const std::string& filePath) {
    if (filePath.empty()) {
        return false;
    }
    
    std::ifstream file(filePath);
    return file.good();
}
uint64_t getFileSize(const std::string& filePath) {
    if (!fileExists(filePath)) {
        return 0;
    }
    
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    
    std::streampos end = file.tellg();
    file.close();
    
    return static_cast<uint64_t>(end);
}

/**
 * @brief Calculate SHA256 checksum of a file
 * @param filePath The path to the file
 * @return The SHA256 hash as a hexadecimal string
 */
std::string getFileChecksum(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        return "";
    }
    const EVP_MD* md = EVP_sha256();
    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string getFileSHA1Checksum(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        return "";
    }
    const EVP_MD* md = EVP_sha1();
    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string getStringSHA1Checksum(const std::string& input) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        return "";
    }
    const EVP_MD* md = EVP_sha1();
    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    if (EVP_DigestUpdate(mdctx, input.c_str(), input.length()) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

/**
 * @brief Convert a string to lowercase
 * @param str The string to convert
 * @return The lowercase version of the string
 */
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

/**
 * @brief Convert a string to uppercase
 * @param str The string to convert
 * @return The uppercase version of the string
 */
std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

/**
 * @brief Remove leading and trailing whitespace from a string
 * @param str The string to trim
 * @return The trimmed string
 */
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

/**
 * @brief Split a string by a delimiter
 * @param str The string to split
 * @param delimiter The delimiter character
 * @return Vector of substrings
 */
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) {
            result.push_back(trim(item));
        }
    }

    return result;
}

/**
 * @brief Join a vector of strings with a separator
 * @param parts The vector of strings to join
 * @param separator The separator string
 * @return The joined string
 */
std::string join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) {
        return "";
    }

    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += separator + parts[i];
    }

    return result;
}

/**
 * @brief Check if a string starts with a prefix
 * @param str The string to check
 * @param prefix The prefix to look for
 * @return true if the string starts with the prefix
 */
bool startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

/**
 * @brief Check if a string ends with a suffix
 * @param str The string to check
 * @param suffix The suffix to look for
 * @return true if the string ends with the suffix
 */
bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

/**
 * @brief Replace all occurrences of a substring in a string
 * @param str The original string
 * @param from The substring to replace
 * @param to The replacement substring
 * @return The string with replacements made
 */
std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

/**
 * @brief Get the current working directory
 * @return The current working directory path
 */
std::string getCurrentWorkingDirectory() {
    char buffer[4096];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
    return "";
}

/**
 * @brief Get the value of an environment variable
 * @param name The name of the environment variable
 * @return The value of the environment variable, or empty string if not found
 */
std::string getEnvironmentVariable(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

/**
 * @brief Get the list of library search paths
 * @return Vector of library search paths
 */
std::vector<std::string> getLibrarySearchPaths() {
    std::vector<std::string> paths;

    // Add standard library paths
#ifdef _WIN32
    paths.push_back("C:\\Windows\\System32");
    paths.push_back("C:\\Windows\\SysWOW64");
#else
    paths.push_back("/usr/lib");
    paths.push_back("/usr/lib64");
    paths.push_back("/usr/local/lib");
    paths.push_back("/usr/local/lib64");
    paths.push_back("/lib");
    paths.push_back("/lib64");
#endif

    // Add paths from LD_LIBRARY_PATH (Linux) or PATH (Windows)
    std::string libPath = getEnvironmentVariable(
#ifdef _WIN32
        "PATH"
#else
        "LD_LIBRARY_PATH"
#endif
    );

    if (!libPath.empty()) {
        auto pathParts = split(libPath,
#ifdef _WIN32
                               ';'
#else
                               ':'
#endif
        );
        paths.insert(paths.end(), pathParts.begin(), pathParts.end());
    }

    return paths;
}

/**
 * @brief Find a library in the system search paths
 * @param libraryName The name of the library to find
 * @return The full path to the library, or empty string if not found
 */
std::string findLibrary(const std::string& libraryName) {
    std::vector<std::string> searchPaths = getLibrarySearchPaths();

    for (const auto& path : searchPaths) {
        std::string fullPath = path + "/" + libraryName;
        if (fileExists(fullPath)) {
            return fullPath;
        }
    }

    return "";
}

/**
 * @brief Check if a library is a system library
 * @param libraryPath The path to the library
 * @return true if the library is in a system directory
 */
bool isSystemLibrary(const std::string& libraryPath) {
    std::string normalizedPath = normalizePath(libraryPath);

    // Check if it's in a system directory
    std::vector<std::string> systemPaths = {"/usr/lib", "/usr/lib64",      "/lib",
                                            "/lib64",   "/System/Library", "/usr/local/lib"};

    return std::any_of(systemPaths.begin(), systemPaths.end(), [&](const std::string& sysPath) {
        return startsWith(normalizedPath, sysPath);
    });
}

/**
 * @brief Detect the package manager based on file path
 * @param filePath The path to the file
 * @return The detected package manager name (e.g., "rpm", "deb", "conan", etc.)
 */
std::string detectPackageManager(const std::string& filePath) {
    std::string normalizedPath = normalizePath(filePath);

    if (normalizedPath.find("/usr/lib") != std::string::npos ||
        normalizedPath.find("/lib") != std::string::npos) {
        return "system";
    } else if (normalizedPath.find("/usr/local/lib") != std::string::npos) {
        return "local";
    } else if (normalizedPath.find("conan") != std::string::npos) {
        return "conan";
    } else if (normalizedPath.find("vcpkg") != std::string::npos) {
        return "vcpkg";
    } else if (normalizedPath.find("brew") != std::string::npos ||
               normalizedPath.find("/opt/homebrew") != std::string::npos) {
        return "homebrew";
    }

    return "unknown";
}

/**
 * @brief Extract version information from a file path
 * @param filePath The path to extract version from
 * @return The extracted version string, or empty if not found
 */
std::string extractVersionFromPath(const std::string& filePath) {
    std::regex versionRegex(R"((\d+\.\d+\.\d+))");
    std::smatch match;

    if (std::regex_search(filePath, match, versionRegex)) {
        return match[1].str();
    }

    return "";
}

/**
 * @brief Extract package name from a file path
 * @param filePath The path to extract package name from
 * @return The extracted package name, or empty if not found
 */
std::string extractPackageName(const std::string& filePath) {
    if (filePath.empty()) {
        return "";
    }
    std::string fileName = getFileName(filePath);

    // Remove common prefixes and suffixes
    std::vector<std::string> prefixes = {"lib", "lib"};
    std::vector<std::string> suffixes = {".so", ".dylib", ".dll", ".a", ".lib"};

    for (const auto& prefix : prefixes) {
        if (startsWith(fileName, prefix)) {
            fileName = fileName.substr(prefix.length());
            break;
        }
    }

    for (const auto& suffix : suffixes) {
        if (endsWith(fileName, suffix)) {
            fileName = fileName.substr(0, fileName.length() - suffix.length());
            break;
        }
    }

    return fileName;
}

/**
 * @brief Print a debug message (only if HEIMDALL_DEBUG_ENABLED is defined)
 * @param message The message to print
 */
void debugPrint(const std::string& message) {
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cerr << "[DEBUG] " << message << std::endl;
#endif
}

/**
 * @brief Print an error message
 * @param message The error message to print
 */
void errorPrint(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

/**
 * @brief Print a warning message
 * @param message The warning message to print
 */
void warningPrint(const std::string& message) {
    std::cerr << "[WARNING] " << message << std::endl;
}

/**
 * @brief Escape special characters in a string for JSON output
 * @param str The string to escape
 * @return The escaped string
 */
std::string escapeJsonString(const std::string& str) {
    std::string result;
    result.reserve(str.length());

    for (char c : str) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (c < 32) {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", (unsigned char)c);
                    result += hex;
                } else {
                    result += c;
                }
                break;
        }
    }

    return result;
}

/**
 * @brief Format a value for JSON output
 * @param value The value to format
 * @return The formatted JSON value
 */
std::string formatJsonValue(const std::string& value) {
    if (value.empty()) {
        return "null";
    }
    return "\"" + escapeJsonString(value) + "\"";
}

/**
 * @brief Check if a file is an object file (.o, .obj)
 * @param filePath The path to the file
 * @return true if the file is an object file
 */
bool isObjectFile(const std::string& filePath) {
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".o" || ext == ".obj";
}

/**
 * @brief Check if a file is a static library (.a, .lib)
 * @param filePath The path to the file
 * @return true if the file is a static library
 */
bool isStaticLibrary(const std::string& filePath) {
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".a" || ext == ".lib";
}

/**
 * @brief Check if a file is a shared library (.so, .dylib, .dll)
 * @param filePath The path to the file
 * @return true if the file is a shared library
 */
bool isSharedLibrary(const std::string& filePath) {
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".so" || ext == ".dylib" || ext == ".dll";
}

/**
 * @brief Check if a file is an executable
 * @param filePath The path to the file
 * @return true if the file is an executable
 */
bool isExecutable(const std::string& filePath) {
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".exe" || ext.empty() || filePath.find("bin/") != std::string::npos;
}

/**
 * @brief Calculate SHA256 hash of a file (alias for getFileChecksum)
 * @param filePath The path to the file
 * @return The SHA256 hash as a hexadecimal string
 */
std::string calculateSHA256(const std::string& filePath) {
    return getFileChecksum(filePath);
}

/**
 * @brief Detect license based on component name
 * @param componentName The name of the component
 * @return The detected license, or empty if not detected
 */
std::string detectLicenseFromName(const std::string& componentName) {
    std::string lowerName = toLower(componentName);

    // OpenSSL and related libraries
    if (lowerName.find("openssl") != std::string::npos ||
        lowerName.find("ssl") != std::string::npos ||
        lowerName.find("crypto") != std::string::npos) {
        return "Apache-2.0";
    }

    // Pthread and threading libraries
    if (lowerName.find("pthread") != std::string::npos ||
        lowerName.find("thread") != std::string::npos) {
        return "MIT";
    }

    // System libraries (libc, libm, etc.)
    if (lowerName.find("libc") != std::string::npos) {
        return "LGPL-2.1";
    }
    if (lowerName.find("libm") != std::string::npos) {
        return "LGPL-2.1";
    }
    if (lowerName.find("libdl") != std::string::npos) {
        return "LGPL-2.1";
    }
    if (lowerName.find("libutil") != std::string::npos) {
        return "BSD-3-Clause";
    }

    // Apple system libraries
    if (lowerName.find("libsystem") != std::string::npos) {
        return "Apple-PSL";
    }
    if (lowerName.find("libobjc") != std::string::npos) {
        return "GPL-2.0";
    }

    // Common development libraries
    if (lowerName.find("libgcc") != std::string::npos) {
        return "GPL-3.0";
    }
    if (lowerName.find("libstdc++") != std::string::npos) {
        return "GPL-3.0";
    }

    return "NOASSERTION";
}

/**
 * @brief Detect license based on file path
 * @param filePath The path to the file
 * @return The detected license, or empty if not detected
 */
std::string detectLicenseFromPath(const std::string& filePath) {
    std::string lowerPath = toLower(filePath);

    // System library paths
    if (lowerPath.find("/usr/lib") != std::string::npos) {
        return "LGPL-2.1";
    }
    if (lowerPath.find("/usr/local/lib") != std::string::npos) {
        return "MIT";
    }
    if (lowerPath.find("/opt/local/lib") != std::string::npos) {
        return "MIT";
    }
    if (lowerPath.find("/opt/homebrew/lib") != std::string::npos) {
        return "MIT";
    }

    // Apple system paths
    if (lowerPath.find("/system/library") != std::string::npos) {
        return "Apple-PSL";
    }

    return "NOASSERTION";
}

/**
 * @brief Resolve a library name to its full path
 * @param libraryName The name of the library to resolve
 * @return The full path to the library, or the original name if not found
 */
std::string resolveLibraryPath(const std::string& libraryName) {
    // If it's already an absolute path, return as is
    if (!libraryName.empty() && libraryName[0] == '/') {
        return libraryName;
    }
    
    // Get library search paths
    std::vector<std::string> searchPaths = getLibrarySearchPaths();
    
    // Try to find the library in search paths
    for (const auto& path : searchPaths) {
        std::string fullPath = path + "/" + libraryName;
        if (fileExists(fullPath)) {
            return fullPath;
        }
        
        // Try with .so extension if not present
        if (libraryName.find(".so") == std::string::npos) {
            std::string soPath = path + "/" + libraryName + ".so";
            if (fileExists(soPath)) {
                return soPath;
            }
        }
    }
    
    // If not found, return the original name
    return libraryName;
}

}  // namespace heimdall::Utils
