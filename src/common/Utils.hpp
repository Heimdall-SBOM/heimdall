#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

namespace heimdall {

namespace Utils {

// File and path utilities
std::string getFileName(const std::string& filePath);
std::string getFileExtension(const std::string& filePath);
std::string getDirectory(const std::string& filePath);
std::string normalizePath(const std::string& path);
bool fileExists(const std::string& filePath);
uint64_t getFileSize(const std::string& filePath);
std::string getFileChecksum(const std::string& filePath);

// File type detection
bool isObjectFile(const std::string& filePath);
bool isStaticLibrary(const std::string& filePath);
bool isSharedLibrary(const std::string& filePath);
bool isExecutable(const std::string& filePath);
std::string calculateSHA256(const std::string& filePath);

// String utilities
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string join(const std::vector<std::string>& parts, const std::string& separator);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);
std::string replace(const std::string& str, const std::string& from, const std::string& to);

// System utilities
std::string getCurrentWorkingDirectory();
std::string getEnvironmentVariable(const std::string& name);
std::vector<std::string> getLibrarySearchPaths();
std::string findLibrary(const std::string& libraryName);
bool isSystemLibrary(const std::string& libraryPath);

// Package manager detection
std::string detectPackageManager(const std::string& filePath);
std::string extractVersionFromPath(const std::string& filePath);
std::string extractPackageName(const std::string& filePath);

// Debug and logging
void debugPrint(const std::string& message);
void errorPrint(const std::string& message);
void warningPrint(const std::string& message);

// JSON utilities (for SBOM generation)
std::string escapeJsonString(const std::string& str);
std::string formatJsonValue(const std::string& value);
std::string formatJsonArray(const std::vector<std::string>& array);

// License detection
std::string detectLicenseFromName(const std::string& componentName);
std::string detectLicenseFromPath(const std::string& filePath);

} // namespace Utils

} // namespace heimdall
