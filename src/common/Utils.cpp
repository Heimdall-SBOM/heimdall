#include "Utils.hpp"
#include <iostream>
#include <cstring>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

namespace heimdall {

namespace Utils {

// File and path utilities
std::string getFileName(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.filename().string();
}

std::string getFileExtension(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.extension().string();
}

std::string getDirectory(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.parent_path().string();
}

std::string normalizePath(const std::string& path)
{
    std::filesystem::path fsPath(path);
    return fsPath.lexically_normal().string();
}

bool fileExists(const std::string& filePath)
{
    return std::filesystem::exists(filePath);
}

uint64_t getFileSize(const std::string& filePath)
{
    if (!fileExists(filePath))
    {
        return 0;
    }
    
    std::filesystem::path path(filePath);
    return std::filesystem::file_size(path);
}

std::string getFileChecksum(const std::string& filePath)
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

// String utilities
std::string toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter))
    {
        if (!item.empty())
        {
            result.push_back(trim(item));
        }
    }
    
    return result;
}

std::string join(const std::vector<std::string>& parts, const std::string& separator)
{
    if (parts.empty())
    {
        return "";
    }
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
    {
        result += separator + parts[i];
    }
    
    return result;
}

bool startsWith(const std::string& str, const std::string& prefix)
{
    if (str.length() < prefix.length())
    {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix)
{
    if (str.length() < suffix.length())
    {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string replace(const std::string& str, const std::string& from, const std::string& to)
{
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos)
    {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// System utilities
std::string getCurrentWorkingDirectory()
{
    char buffer[4096];
    if (getcwd(buffer, sizeof(buffer)) != nullptr)
    {
        return std::string(buffer);
    }
    return "";
}

std::string getEnvironmentVariable(const std::string& name)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

std::vector<std::string> getLibrarySearchPaths()
{
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
    
    if (!libPath.empty())
    {
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

std::string findLibrary(const std::string& libraryName)
{
    std::vector<std::string> searchPaths = getLibrarySearchPaths();
    
    for (const auto& path : searchPaths)
    {
        std::string fullPath = path + "/" + libraryName;
        if (fileExists(fullPath))
        {
            return fullPath;
        }
    }
    
    return "";
}

bool isSystemLibrary(const std::string& libraryPath)
{
    std::string normalizedPath = normalizePath(libraryPath);
    
    // Check if it's in a system directory
    std::vector<std::string> systemPaths = {
        "/usr/lib", "/usr/lib64", "/lib", "/lib64",
        "/System/Library", "/usr/local/lib"
    };
    
    for (const auto& sysPath : systemPaths)
    {
        if (startsWith(normalizedPath, sysPath))
        {
            return true;
        }
    }
    
    return false;
}

// Package manager detection
std::string detectPackageManager(const std::string& filePath)
{
    std::string normalizedPath = normalizePath(filePath);
    
    if (normalizedPath.find("/usr/lib") != std::string::npos ||
        normalizedPath.find("/lib") != std::string::npos)
    {
        return "system";
    }
    else if (normalizedPath.find("/usr/local/lib") != std::string::npos)
    {
        return "local";
    }
    else if (normalizedPath.find("conan") != std::string::npos)
    {
        return "conan";
    }
    else if (normalizedPath.find("vcpkg") != std::string::npos)
    {
        return "vcpkg";
    }
    else if (normalizedPath.find("brew") != std::string::npos ||
             normalizedPath.find("/opt/homebrew") != std::string::npos)
    {
        return "homebrew";
    }
    
    return "unknown";
}

std::string extractVersionFromPath(const std::string& filePath)
{
    std::regex versionRegex(R"((\d+\.\d+\.\d+))");
    std::smatch match;
    
    if (std::regex_search(filePath, match, versionRegex))
    {
        return match[1].str();
    }
    
    return "";
}

std::string extractPackageName(const std::string& filePath)
{
    std::string fileName = getFileName(filePath);
    
    // Remove common prefixes and suffixes
    std::vector<std::string> prefixes = {"lib", "lib"};
    std::vector<std::string> suffixes = {".so", ".dylib", ".dll", ".a", ".lib"};
    
    for (const auto& prefix : prefixes)
    {
        if (startsWith(fileName, prefix))
        {
            fileName = fileName.substr(prefix.length());
            break;
        }
    }
    
    for (const auto& suffix : suffixes)
    {
        if (endsWith(fileName, suffix))
        {
            fileName = fileName.substr(0, fileName.length() - suffix.length());
            break;
        }
    }
    
    return fileName;
}

// Debug and logging
void debugPrint(const std::string& message)
{
#ifdef HEIMDALL_DEBUG_ENABLED
    std::cerr << "[DEBUG] " << message << std::endl;
#endif
}

void errorPrint(const std::string& message)
{
    std::cerr << "[ERROR] " << message << std::endl;
}

void warningPrint(const std::string& message)
{
    std::cerr << "[WARNING] " << message << std::endl;
}

// JSON utilities
std::string escapeJsonString(const std::string& str)
{
    std::string result;
    result.reserve(str.length());
    
    for (char c : str)
    {
        switch (c)
        {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 32)
                {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", (unsigned char)c);
                    result += hex;
                }
                else
                {
                    result += c;
                }
                break;
        }
    }
    
    return result;
}

std::string formatJsonValue(const std::string& value)
{
    if (value.empty())
    {
        return "null";
    }
    return "\"" + escapeJsonString(value) + "\"";
}

std::string formatJsonArray(const std::vector<std::string>& array)
{
    if (array.empty())
    {
        return "[]";
    }
    
    std::string result = "[";
    for (size_t i = 0; i < array.size(); ++i)
    {
        if (i > 0) result += ", ";
        result += formatJsonValue(array[i]);
    }
    result += "]";
    
    return result;
}

// File type detection functions
bool isObjectFile(const std::string& filePath)
{
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".o" || ext == ".obj";
}

bool isStaticLibrary(const std::string& filePath)
{
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".a" || ext == ".lib";
}

bool isSharedLibrary(const std::string& filePath)
{
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".so" || ext == ".dylib" || ext == ".dll";
}

bool isExecutable(const std::string& filePath)
{
    std::string ext = toLower(getFileExtension(filePath));
    return ext == ".exe" || ext == "" || filePath.find("bin/") != std::string::npos;
}

std::string calculateSHA256(const std::string& filePath)
{
    return getFileChecksum(filePath);
}

std::string detectLicenseFromName(const std::string& componentName)
{
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

std::string detectLicenseFromPath(const std::string& filePath)
{
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

} // namespace Utils

} // namespace heimdall
