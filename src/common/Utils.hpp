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
 * @file Utils.hpp
 * @brief Utility functions for file operations, string manipulation, and system interactions
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <algorithm>
#include <cctype>
#include "compat/compatibility.hpp"
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

namespace heimdall::Utils {

/**
 * @brief Extract the filename from a file path
 * @param filePath The full file path
 * @return The filename without the directory path
 */
std::string getFileName(const std::string& filePath);

/**
 * @brief Extract the file extension from a file path
 * @param filePath The full file path
 * @return The file extension (including the dot)
 */
std::string getFileExtension(const std::string& filePath);

/**
 * @brief Extract the directory path from a file path
 * @param filePath The full file path
 * @return The directory path without the filename
 */
std::string getDirectory(const std::string& filePath);

/**
 * @brief Normalize a file path (resolve relative paths, remove redundant separators)
 * @param path The path to normalize
 * @return The normalized path
 */
std::string normalizePath(const std::string& path);

/**
 * @brief Check if a file exists
 * @param filePath The path to check
 * @return true if the file exists, false otherwise
 */
bool fileExists(const std::string& filePath);

/**
 * @brief Get the size of a file in bytes
 * @param filePath The path to the file
 * @return The file size in bytes, or 0 if the file doesn't exist
 */
uint64_t getFileSize(const std::string& filePath);

/**
 * @brief Calculate SHA256 checksum of a file
 * @param filePath The path to the file
 * @return The SHA256 hash as a hexadecimal string
 */
std::string getFileChecksum(const std::string& filePath);

/**
 * @brief Calculate SHA1 checksum of a file
 * @param filePath The path to the file
 * @return The SHA1 hash as a hexadecimal string
 */
std::string getFileSHA1Checksum(const std::string& filePath);

/**
 * @brief Calculate SHA1 checksum of a string
 * @param input The input string to hash
 * @return The SHA1 hash as a hexadecimal string
 */
std::string getStringSHA1Checksum(const std::string& input);

/**
 * @brief Check if a file is an object file (.o, .obj)
 * @param filePath The path to the file
 * @return true if the file is an object file
 */
bool isObjectFile(const std::string& filePath);

/**
 * @brief Check if a file is a static library (.a, .lib)
 * @param filePath The path to the file
 * @return true if the file is a static library
 */
bool isStaticLibrary(const std::string& filePath);

/**
 * @brief Check if a file is a shared library (.so, .dylib, .dll)
 * @param filePath The path to the file
 * @return true if the file is a shared library
 */
bool isSharedLibrary(const std::string& filePath);

/**
 * @brief Check if a file is an executable
 * @param filePath The path to the file
 * @return true if the file is an executable
 */
bool isExecutable(const std::string& filePath);

/**
 * @brief Calculate SHA256 hash of a file (alias for getFileChecksum)
 * @param filePath The path to the file
 * @return The SHA256 hash as a hexadecimal string
 */
std::string calculateSHA256(const std::string& filePath);

/**
 * @brief Convert a string to lowercase
 * @param str The string to convert
 * @return The lowercase version of the string
 */
std::string toLower(const std::string& str);

/**
 * @brief Convert a string to uppercase
 * @param str The string to convert
 * @return The uppercase version of the string
 */
std::string toUpper(const std::string& str);

/**
 * @brief Remove leading and trailing whitespace from a string
 * @param str The string to trim
 * @return The trimmed string
 */
std::string trim(const std::string& str);

/**
 * @brief Split a string by a delimiter
 * @param str The string to split
 * @param delimiter The delimiter character
 * @return Vector of substrings
 */
std::vector<std::string> split(const std::string& str, char delimiter);

/**
 * @brief Join a vector of strings with a separator
 * @param parts The vector of strings to join
 * @param separator The separator string
 * @return The joined string
 */
std::string join(const std::vector<std::string>& parts, const std::string& separator);

/**
 * @brief Check if a string starts with a prefix
 * @param str The string to check
 * @param prefix The prefix to look for
 * @return true if the string starts with the prefix
 */
bool startsWith(const std::string& str, const std::string& prefix);

/**
 * @brief Check if a string ends with a suffix
 * @param str The string to check
 * @param suffix The suffix to look for
 * @return true if the string ends with the suffix
 */
bool endsWith(const std::string& str, const std::string& suffix);

/**
 * @brief Replace all occurrences of a substring in a string
 * @param str The original string
 * @param from The substring to replace
 * @param to The replacement substring
 * @return The string with replacements made
 */
std::string replace(const std::string& str, const std::string& from, const std::string& to);

/**
 * @brief Get the current working directory
 * @return The current working directory path
 */
std::string getCurrentWorkingDirectory();

/**
 * @brief Get the value of an environment variable
 * @param name The name of the environment variable
 * @return The value of the environment variable, or empty string if not found
 */
std::string getEnvironmentVariable(const std::string& name);

/**
 * @brief Get the list of library search paths
 * @return Vector of library search paths
 */
std::vector<std::string> getLibrarySearchPaths();

/**
 * @brief Find a library in the system search paths
 * @param libraryName The name of the library to find
 * @return The full path to the library, or empty string if not found
 */
std::string findLibrary(const std::string& libraryName);

/**
 * @brief Check if a library is a system library
 * @param libraryPath The path to the library
 * @return true if the library is in a system directory
 */
bool isSystemLibrary(const std::string& libraryPath);

/**
 * @brief Detect the package manager based on file path
 * @param filePath The path to the file
 * @return The detected package manager name (e.g., "rpm", "deb", "conan", etc.)
 */
std::string detectPackageManager(const std::string& filePath);

/**
 * @brief Extract version information from a file path
 * @param filePath The path to extract version from
 * @return The extracted version string, or empty if not found
 */
std::string extractVersionFromPath(const std::string& filePath);

/**
 * @brief Extract package name from a file path
 * @param filePath The path to extract package name from
 * @return The extracted package name, or empty if not found
 */
std::string extractPackageName(const std::string& filePath);

/**
 * @brief Split a path into its components
 * @param path The path to split
 * @return Vector of path components
 */
std::vector<std::string> splitPath(const std::string& path);

/**
 * @brief Print a debug message (only if HEIMDALL_DEBUG_ENABLED is defined)
 * @param message The message to print
 */
void debugPrint(const std::string& message);

/**
 * @brief Print an error message
 * @param message The error message to print
 */
void errorPrint(const std::string& message);

/**
 * @brief Print a warning message
 * @param message The warning message to print
 */
void warningPrint(const std::string& message);

/**
 * @brief Escape special characters in a string for JSON output
 * @param str The string to escape
 * @return The escaped string
 */
std::string escapeJsonString(const std::string& str);

/**
 * @brief Format a value for JSON output
 * @param value The value to format
 * @return The formatted JSON value
 */
std::string formatJsonValue(const std::string& value);

/**
 * @brief Format an array of strings for JSON output
 * @param array The array to format
 * @return The formatted JSON array string
 */
std::string formatJsonArray(const std::vector<std::string>& array);

/**
 * @brief Detect license based on component name
 * @param componentName The name of the component
 * @return The detected license, or empty if not detected
 */
std::string detectLicenseFromName(const std::string& componentName);

/**
 * @brief Resolve a library name to its full path
 * @param libraryName The library name (e.g., "libssl.so.3")
 * @return The full path to the library, or empty string if not found
 */
std::string resolveLibraryPath(const std::string& libraryName);

/**
 * @brief Detect license based on file path
 * @param filePath The path to the file
 * @return The detected license, or empty if not detected
 */
std::string detectLicenseFromPath(const std::string& filePath);

/**
 * @brief Generate a UUID v4 string
 * @return A UUID v4 string in the format "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
 */
std::string generateUUID();

}  // namespace heimdall::Utils
