/**
 * @file StringUtils.hpp
 * @brief String utility functions for substring checks and compatibility helpers
 * @author Trevor Bakker
 * @date 2025
 *
 * This header provides utility functions for string operations such as contains,
 * starts_with, ends_with, and compatibility helpers for C++17/20/23.
 */

#pragma once

#include <string>
#include <memory>
#include <filesystem>

/**
 * @namespace heimdall
 * @brief Main namespace for Heimdall project utilities and components.
 */
namespace heimdall {
/**
 * @namespace StringUtils
 * @brief Utility functions for string operations and compatibility.
 */
namespace StringUtils {

/**
 * @brief Check if a string contains a substring (C++23 compatible)
 * @param str The string to search in
 * @param substr The substring to search for
 * @return true if the substring is found, false otherwise
 */
inline bool contains(const std::string& str, const std::string& substr) {
#if __cplusplus >= 202302L
    return str.contains(substr);
#else
    return str.find(substr) != std::string::npos;
#endif
}

/**
 * @brief Check if a string contains a substring (C++23 compatible)
 * @param str The string to search in
 * @param substr The substring to search for (C-string)
 * @return true if the substring is found, false otherwise
 */
inline bool contains(const std::string& str, const char* substr) {
#if __cplusplus >= 202302L
    return str.contains(substr);
#else
    return str.find(substr) != std::string::npos;
#endif
}

/**
 * @brief Check if a string starts with a prefix (C++20 compatible)
 * @param str The string to check
 * @param prefix The prefix to look for
 * @return true if the string starts with the prefix, false otherwise
 */
inline bool starts_with(const std::string& str, const std::string& prefix) {
#if __cplusplus >= 202002L
    return str.starts_with(prefix);
#else
    if (str.length() < prefix.length()) return false;
    return str.compare(0, prefix.length(), prefix) == 0;
#endif
}

/**
 * @brief Check if a string ends with a suffix (C++20 compatible)
 * @param str The string to check
 * @param suffix The suffix to look for
 * @return true if the string ends with the suffix, false otherwise
 */
inline bool ends_with(const std::string& str, const std::string& suffix) {
#if __cplusplus >= 202002L
    return str.ends_with(suffix);
#else
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
#endif
}

// Filesystem compatibility (C++17+)
namespace fs = std::filesystem;

// make_unique for C++14+, but C++17+ always has it
using std::make_unique;

} // namespace StringUtils
} // namespace heimdall 