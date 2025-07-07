#pragma once

#include <string>
#include <memory>

namespace heimdall {
namespace StringUtils {

/**
 * @brief Check if a string contains a substring (C++23 compatible)
 * Falls back to .find() != std::string::npos for C++17
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
 * Falls back to .find() != std::string::npos for C++17
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
 * Falls back to manual check for C++17
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
 * Falls back to manual check for C++17
 */
inline bool ends_with(const std::string& str, const std::string& suffix) {
#if __cplusplus >= 202002L
    return str.ends_with(suffix);
#else
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
#endif
}

// make_unique for C++11
#if __cplusplus < 201402L
// C++11 only
    template <typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#else
    using std::make_unique;
#endif

// Filesystem compatibility
#if __cplusplus >= 201703L
    #include <filesystem>
    namespace fs = std::filesystem;
#else
    // Minimal fs namespace for C++11/14
    #include <sys/stat.h>
    #include <unistd.h>
    namespace fs {
        inline bool exists(const std::string& path) {
            struct stat buffer;
            return (stat(path.c_str(), &buffer) == 0);
        }
        inline uintmax_t file_size(const std::string& path) {
            struct stat buffer;
            if (stat(path.c_str(), &buffer) == 0) {
                return static_cast<uintmax_t>(buffer.st_size);
            }
            return 0;
        }
    }
#endif

} // namespace StringUtils
} // namespace heimdall 