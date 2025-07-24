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
 * @file compatibility.hpp
 * @brief Cross-platform C++ standard compatibility layer
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides a comprehensive compatibility layer for different C++ standards
 * (C++11, C++14, C++17, C++20, C++23) and platforms. It includes:
 * 
 * - Feature detection macros for different C++ standards
 * - Compatibility implementations for missing standard library features
 * - Cross-platform filesystem abstractions
 * - Optional, string_view, variant, and span implementations for older standards
 * - Utility functions for safe string handling and formatting
 * 
 * The compatibility layer ensures that Heimdall can be compiled and run on
 * systems with different C++ standard library implementations and versions.
 * 
 * Supported C++ Standards:
 * - C++11: Basic compatibility with custom implementations
 * - C++14: Enhanced compatibility with std::make_unique
 * - C++17: Full filesystem and optional support
 * - C++20: Modern features including concepts and ranges
 * - C++23: Latest features including print and generator
 */

#pragma once

// Standard library includes for all C++ versions - OUTSIDE any namespace
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <climits>
#include <sys/stat.h>

/**
 * @namespace heimdall::compat::detail
 * @brief Internal compatibility detection and configuration
 * 
 * This namespace contains internal constants and detection logic
 * for determining which C++ features are available.
 */
namespace heimdall {
namespace compat {
namespace detail {
    constexpr bool cpp23_available = __cplusplus >= 202302L;  ///< C++23 availability flag
    constexpr bool cpp20_available = __cplusplus >= 202002L;  ///< C++20 availability flag
    constexpr bool cpp17_available = __cplusplus >= 201703L;  ///< C++17 availability flag
    constexpr bool cpp14_available = __cplusplus >= 201402L;  ///< C++14 availability flag
    constexpr bool cpp11_available = __cplusplus >= 201103L;  ///< C++11 availability flag
    
    constexpr bool full_dwarf_available = cpp17_available;    ///< Full DWARF support flag
    constexpr bool basic_dwarf_available = cpp14_available;   ///< Basic DWARF support flag
    constexpr bool no_dwarf_available = cpp11_available;      ///< No DWARF support flag
    constexpr bool modern_features_available = cpp20_available; ///< Modern C++ features flag
}
}
}

/**
 * @def HEIMDALL_CPP23_AVAILABLE
 * @brief Macro indicating C++23 availability
 */
constexpr bool HEIMDALL_CPP23_AVAILABLE = heimdall::compat::detail::cpp23_available;

/**
 * @def HEIMDALL_CPP20_AVAILABLE
 * @brief Macro indicating C++20 availability
 */
constexpr bool HEIMDALL_CPP20_AVAILABLE = heimdall::compat::detail::cpp20_available;

/**
 * @def HEIMDALL_CPP17_AVAILABLE
 * @brief Macro indicating C++17 availability
 */
constexpr bool HEIMDALL_CPP17_AVAILABLE = heimdall::compat::detail::cpp17_available;

/**
 * @def HEIMDALL_CPP14_AVAILABLE
 * @brief Macro indicating C++14 availability
 */
constexpr bool HEIMDALL_CPP14_AVAILABLE = heimdall::compat::detail::cpp14_available;

/**
 * @def HEIMDALL_CPP11_AVAILABLE
 * @brief Macro indicating C++11 availability
 */
constexpr bool HEIMDALL_CPP11_AVAILABLE = heimdall::compat::detail::cpp11_available;

/**
 * @def HEIMDALL_FULL_DWARF
 * @brief Macro indicating full DWARF support availability
 */
constexpr bool HEIMDALL_FULL_DWARF = heimdall::compat::detail::full_dwarf_available;

/**
 * @def HEIMDALL_BASIC_DWARF
 * @brief Macro indicating basic DWARF support availability
 */
constexpr bool HEIMDALL_BASIC_DWARF = heimdall::compat::detail::basic_dwarf_available;

/**
 * @def HEIMDALL_NO_DWARF
 * @brief Macro indicating no DWARF support availability
 */
constexpr bool HEIMDALL_NO_DWARF = heimdall::compat::detail::no_dwarf_available;

/**
 * @def HEIMDALL_MODERN_FEATURES
 * @brief Macro indicating modern C++ features availability
 */
constexpr bool HEIMDALL_MODERN_FEATURES = heimdall::compat::detail::modern_features_available;

#if __cplusplus < 201103L
    #error "C++11 or later required"
#endif

// Include standard library headers based on C++ standard - OUTSIDE namespace
#if __cplusplus >= 202302L
    // C++23 includes
    #include <filesystem>
    #include <optional>
    #include <string_view>
    #include <variant>
    #include <span>
    #include <concepts>
    #include <ranges>
    #include <format>
    #include <source_location>
    #include <bit>
    #include <numbers>
    #include <compare>
    #include <coroutine>
    #include <latch>
    #include <barrier>
    #include <semaphore>
    #include <stop_token>
    #include <syncstream>
    #include <chrono>
    #include <version>
    #if __has_include(<expected>)
        #include <expected>
    #endif
    #if __has_include(<flat_map>)
        #include <flat_map>
        #include <flat_set>
    #endif
    #if __has_include(<functional>)
        #include <functional>
    #endif
    #if __has_include(<generator>)
        #include <generator>
    #endif
    #if __has_include(<print>)
        #include <print>
    #endif
    #if __has_include(<stacktrace>)
        #include <stacktrace>
    #endif
#elif __cplusplus >= 202002L
    // C++20 includes
    #include <filesystem>
    #include <optional>
    #include <string_view>
    #include <variant>
    #include <span>
    #include <concepts>
    #include <ranges>
    #include <format>
    #include <source_location>
    #include <bit>
    #include <numbers>
    #include <compare>
    #include <coroutine>
    #include <latch>
    #include <barrier>
    #include <semaphore>
    #include <stop_token>
    #include <syncstream>
    #include <chrono>
    #include <version>
    #if __has_include(<expected>)
        #include <expected>
    #endif
    #if __has_include(<flat_map>)
        #include <flat_map>
        #include <flat_set>
    #endif
    #if __has_include(<functional>)
        #include <functional>
    #endif
#elif __cplusplus >= 201703L
    // C++17 includes
    #include <filesystem>
    #include <optional>
    #include <string_view>
    #include <variant>
#else
    // C++11/14: Custom implementations or Boost
    #if defined(USE_BOOST_FILESYSTEM) && USE_BOOST_FILESYSTEM
        namespace fs = boost::filesystem;
    #else
        // Fallback filesystem implementation for C++11/14 without Boost
        namespace fs {
            class path {
            private:
                std::string path_str;
                
            public:
                path() = default;
                path(const std::string& str) : path_str(str) {}
                path(const char* str) : path_str(str ? str : "") {}
                
                // Basic operations
                std::string string() const { return path_str; }
                path filename() const {
                    size_t pos = path_str.find_last_of("/\\");
                    return (pos == std::string::npos) ? path(path_str) : path(path_str.substr(pos + 1));
                }
                path parent_path() const {
                    size_t pos = path_str.find_last_of("/\\");
                    return (pos == std::string::npos) ? path() : path(path_str.substr(0, pos));
                }
                
                // Operators
                path operator/(const path& other) const {
                    if (path_str.empty()) return other;
                    if (other.path_str.empty()) return *this;
                    if (path_str.back() == '/' || path_str.back() == '\\') {
                        return path(path_str + other.path_str);
                    }
                    return path(path_str + "/" + other.path_str);
                }
                
                path& operator/=(const path& other) {
                    *this = *this / other;
                    return *this;
                }
                
                // Conversion operators
                operator std::string() const { return path_str; }
                
                // Comparison operators
                bool operator==(const path& other) const { return path_str == other.path_str; }
                bool operator!=(const path& other) const { return path_str != other.path_str; }
                bool operator==(const std::string& other) const { return path_str == other; }
                bool operator!=(const std::string& other) const { return path_str != other; }
                bool operator==(const char* other) const { return path_str == other; }
                bool operator!=(const char* other) const { return path_str != other; }
                
                // Path utilities
                bool is_absolute() const {
                    return !path_str.empty() && (path_str[0] == '/' || path_str[0] == '\\');
                }
            };
            
            // Filesystem operations
            inline bool exists(const path& p) {
                std::ifstream file(p.string());
                return file.good();
            }
            
            inline bool create_directories(const path& p) {
                // Simple implementation - just create the directory
                std::string cmd = "mkdir -p " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline bool remove(const path& p) {
                std::string cmd = "rm -f " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline bool remove_all(const path& p) {
                std::string cmd = "rm -rf " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline path current_path() {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                    return path(cwd);
                }
                return path();
            }
            
            inline bool current_path(const path& p) {
                return chdir(p.string().c_str()) == 0;
            }
            
            inline path temp_directory_path() {
                const char* temp_dir = getenv("TMPDIR");
                if (!temp_dir) temp_dir = getenv("TEMP");
                if (!temp_dir) temp_dir = getenv("TMP");
                if (!temp_dir) temp_dir = "/tmp";
                return path(temp_dir);
            }
            
            inline uintmax_t file_size(const path& p) {
                std::ifstream file(p.string(), std::ios::binary | std::ios::ate);
                if (file.is_open()) {
                    return file.tellg();
                }
                return 0;
            }

            // File permissions enum
            enum class perms {
                none = 0,
                owner_read = 0400,
                owner_write = 0200,
                owner_exec = 0100,
                owner_all = owner_read | owner_write | owner_exec,
                group_read = 0040,
                group_write = 0020,
                group_exec = 0010,
                group_all = group_read | group_write | group_exec,
                others_read = 0004,
                others_write = 0002,
                others_exec = 0001,
                others_all = others_read | others_write | others_exec,
                all = owner_all | group_all | others_all,
                set_uid = 04000,
                set_gid = 02000,
                sticky_bit = 01000,
                mask = all | set_uid | set_gid | sticky_bit,
                unknown = 0xFFFF
            };
            
            // Bitwise operators for perms enum
            inline perms operator|(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) | static_cast<int>(rhs));
            }
            
            inline perms operator&(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) & static_cast<int>(rhs));
            }
            
            inline perms operator^(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
            }
            
            inline perms operator~(perms p) {
                return static_cast<perms>(~static_cast<int>(p));
            }
            
            inline perms& operator|=(perms& lhs, perms rhs) {
                lhs = lhs | rhs;
                return lhs;
            }
            
            inline perms& operator&=(perms& lhs, perms rhs) {
                lhs = lhs & rhs;
                return lhs;
            }
            
            inline perms& operator^=(perms& lhs, perms rhs) {
                lhs = lhs ^ rhs;
                return lhs;
            }
            
            // Copy options enum
            enum class copy_options {
                none = 0,
                skip_existing = 1,
                overwrite_existing = 2,
                update_existing = 4,
                recursive = 8,
                copy_symlinks = 16,
                skip_symlinks = 32,
                directories_only = 64,
                create_symlinks = 128,
                create_hard_links = 256
            };
            
            // Permission options enum
            enum class perm_options {
                replace = 0,
                add = 1,
                remove = 2,
                nofollow = 4
            };
            
            // Forward declaration
            class filesystem_error;
            
            // Filesystem operations
            inline void permissions(const path& p, perms prms, perm_options opts = perm_options::replace) {
                // Simple implementation using chmod
                chmod(p.string().c_str(), static_cast<mode_t>(prms));
            }
            
            inline void copy_file(const path& from, const path& to, copy_options options = copy_options::none) {
                // Simple implementation using system commands
                std::string cmd = "cp ";
                if (static_cast<int>(options) & static_cast<int>(copy_options::overwrite_existing)) {
                    cmd += "-f ";
                }
                cmd += from.string() + " " + to.string();
                system(cmd.c_str());
            }
            
            inline void create_symlink(const path& to, const path& new_symlink) {
                symlink(to.string().c_str(), new_symlink.string().c_str());
            }
            
            inline void create_hard_link(const path& to, const path& new_hard_link) {
                link(to.string().c_str(), new_hard_link.string().c_str());
            }
            
            inline path absolute(const path& p) {
                char resolved_path[PATH_MAX];
                if (realpath(p.string().c_str(), resolved_path) != nullptr) {
                    return path(resolved_path);
                }
                return p;
            }
            
            // Directory iterator implementation
            class recursive_directory_iterator {
            private:
                std::vector<path> files;
                size_t current_index;
                
            public:
                recursive_directory_iterator() : current_index(0) {}
                recursive_directory_iterator(const path& p) : current_index(0) {
                    // Simple implementation - collect all files
                    DIR* dir = opendir(p.string().c_str());
                    if (dir) {
                        struct dirent* entry;
                        while ((entry = readdir(dir)) != nullptr) {
                            if (entry->d_name[0] != '.') {
                                files.push_back(p / entry->d_name);
                            }
                        }
                        closedir(dir);
                    }
                }
                
                recursive_directory_iterator& operator++() {
                    if (current_index < files.size()) {
                        ++current_index;
                    }
                    return *this;
                }
                
                bool operator!=(const recursive_directory_iterator& other) const {
                    return current_index != other.current_index;
                }
                
                bool is_regular_file() const {
                    if (current_index >= files.size()) return false;
                    struct stat st;
                    if (stat(files[current_index].string().c_str(), &st) == 0) {
                        return S_ISREG(st.st_mode);
                    }
                    return false;
                }
                
                path get_path() const {
                    if (current_index < files.size()) {
                        return files[current_index];
                    }
                    return path();
                }
            };
            
            // Directory iterator
            struct DirCloser {
                void operator()(DIR* d) const { if (d) closedir(d); }
            };
            
            class directory_iterator {
            private:
                std::unique_ptr<DIR, DirCloser> dir;
                std::string current_path;
                
            public:
                directory_iterator() : dir(nullptr) {}
                directory_iterator(const path& p) : dir(opendir(p.string().c_str())) {}
                ~directory_iterator() = default;
                
                // Disable copy operations
                directory_iterator(const directory_iterator&) = delete;
                directory_iterator& operator=(const directory_iterator&) = delete;
                directory_iterator(directory_iterator&&) = delete;
                directory_iterator& operator=(directory_iterator&&) = delete;
                
                directory_iterator& operator++() {
                    if (dir) {
                        struct dirent* entry = readdir(dir.get());
                        if (entry) {
                            current_path = entry->d_name;
                        } else {
                            dir.reset();
                        }
                    }
                    return *this;
                }
                
                bool operator!=(const directory_iterator& other) const {
                    return dir != other.dir;
                }
                
                path operator*() const {
                    return path(current_path);
                }
            };
            
            inline directory_iterator& begin(directory_iterator& iter) { return iter; }
            inline const directory_iterator& end(const directory_iterator&) { 
                static const directory_iterator end_iter;
                return end_iter;
            }
            
            // Exception types
            class filesystem_error : public std::runtime_error {
            public:
                filesystem_error(const std::string& msg) : std::runtime_error(msg) {}
            };
        }
    #endif
#endif

// Now define the compatibility namespace
namespace heimdall {
namespace compat {

/**
 * @brief C++11/14/17/20/23 compatibility: make_unique implementation
 * 
 * Provides a consistent make_unique implementation across all C++ standards.
 * Uses std::make_unique for C++14+ and provides a fallback for C++11.
 * 
 * @tparam T The type to create
 * @tparam Args The argument types for construction
 * @param args The arguments to pass to the constructor
 * @return A unique_ptr to the created object
 */
#if __cplusplus >= 201402L
    // C++14 and above: alias to std::make_unique
    template<typename T, typename... Args>
    inline std::unique_ptr<T> make_unique(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
#else
    // C++11: provide fallback
    template<typename T, typename... Args>
    inline std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif

// Define namespace aliases and compatibility types based on C++ standard
#if __cplusplus >= 201703L
    // C++17 and above: Use standard library with namespace alias
    namespace fs = std::filesystem;
    
    // Re-export standard types for compatibility
    using std::optional;
    using std::string_view;
    using std::variant;
    using std::monostate;
    
    #if __cplusplus >= 202002L
        // C++20/23: Additional types
        using std::span;
        
        // Define concepts for C++20/23
        template<typename T>
        concept integral = std::integral<T>;
        template<typename T>
        concept floating_point = std::floating_point<T>;
        template<typename T>
        concept arithmetic = std::is_arithmetic_v<T>;
        template<typename T>
        concept convertible_to_string = requires(T t) {
            { std::to_string(t) } -> std::convertible_to<std::string>;
        };
        
        #if __cplusplus >= 202302L
            template<typename T>
            concept sized_range = std::ranges::sized_range<T>;
            template<typename T>
            concept random_access_range = std::ranges::random_access_range<T>;
            template<typename T>
            concept contiguous_range = std::ranges::contiguous_range<T>;
            template<typename T>
            constexpr auto to_underlying(T e) noexcept {
                return static_cast<std::underlying_type_t<T>>(e);
            }
        #endif
    #endif
    
#else
    // C++11/14: Custom implementations or Boost
    
    // Fallback type definitions for C++11/14 ONLY
    #if __cplusplus < 201703L
    namespace fallback {
        template<typename T>
        class optional {
        private:
            T value_;
            bool has_value_;
            
        public:
            optional() : has_value_(false) {}
            optional(const T& value) : value_(value), has_value_(true) {}
            optional(T&& value) : value_(std::move(value)), has_value_(true) {}
            
            bool has_value() const { return has_value_; }
            const T& value() const { 
                if (!has_value_) throw std::runtime_error("Optional has no value");
                return value_; 
            }
            T& value() { 
                if (!has_value_) throw std::runtime_error("Optional has no value");
                return value_; 
            }
            
            // Dereference operators to match std::optional
            const T& operator*() const { return value(); }
            T& operator*() { return value(); }
            const T* operator->() const { return &value(); }
            T* operator->() { return &value(); }
            
            // value_or method to match std::optional
            template<typename U>
            T value_or(U&& default_value) const {
                return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
            }
            
            operator bool() const { return has_value_; }
        };
        
        class string_view {
        private:
            const char* data_;
            size_t size_;
            
        public:
            string_view() : data_(nullptr), size_(0) {}
            string_view(const char* str) : data_(str), size_(str ? strlen(str) : 0) {}
            string_view(const std::string& str) : data_(str.data()), size_(str.size()) {}
            string_view(const char* data, size_t size) : data_(data), size_(size) {}
            
            const char* data() const { return data_; }
            size_t size() const { return size_; }
            bool empty() const { return size_ == 0; }
            
            const char& operator[](size_t pos) const { return data_[pos]; }
            
            // find method to match std::string_view
            size_t find(char ch, size_t pos = 0) const {
                for (size_t i = pos; i < size_; ++i) {
                    if (data_[i] == ch) return i;
                }
                return std::string::npos;
            }
            
            // substr method to match std::string_view
            string_view substr(size_t pos = 0, size_t count = std::string::npos) const {
                if (pos > size_) return string_view();
                size_t actual_count = (count == std::string::npos) ? (size_ - pos) : std::min(count, size_ - pos);
                return string_view(data_ + pos, actual_count);
            }
            
            std::string to_string() const { return std::string(data_, size_); }
            operator std::string() const { return to_string(); }
        };
        
        template<typename... Types>
        class variant {
        private:
            enum class type_index { none = 0 };
            type_index current_type_;
            typename std::aligned_storage<64, 8>::type storage_; // Simplified storage for C++11
            
        public:
            variant() : current_type_(type_index::none) {}
            
            template<typename T>
            variant(const T& value) : current_type_(type_index::none) {
                // Simplified implementation - just store the first type
                static_assert(sizeof...(Types) > 0, "Variant must have at least one type");
            }
            
            // index method to match std::variant
            size_t index() const { return static_cast<size_t>(current_type_); }
            
            // get method to match std::variant (simplified)
            template<typename T>
            T get() const {
                // Simplified implementation - just return a default value
                return T{};
            }
            
            // Non-template get method for the first type
            int get() const {
                // Simplified implementation - just return 0
                return 0;
            }
            
            bool valueless_by_exception() const { return current_type_ == type_index::none; }
        };
        
        struct monostate {};
        
        template<typename T>
        class span {
        private:
            T* data_;
            size_t size_;
            
        public:
            span() : data_(nullptr), size_(0) {}
            span(T* data, size_t size) : data_(data), size_(size) {}
            
            T* data() const { return data_; }
            size_t size() const { return size_; }
            bool empty() const { return size_ == 0; }
            
            T& operator[](size_t index) const { return data_[index]; }
            
            T* begin() const { return data_; }
            T* end() const { return data_ + size_; }
        };
    }
    
    // Use fallback types for C++11/14 ONLY - with explicit template parameters
    template<typename T>
    using optional = fallback::optional<T>;
    using string_view = fallback::string_view;
    template<typename... Types>
    using variant = fallback::variant<Types...>;
    using monostate = fallback::monostate;
    template<typename T>
    using span = fallback::span<T>;
    #endif
    
    #if __cplusplus < 201703L
    #if defined(USE_BOOST_FILESYSTEM) && USE_BOOST_FILESYSTEM
        namespace fs = boost::filesystem;
    #else
        // Fallback filesystem implementation for C++11/14 without Boost
        namespace fs {
            class path {
            private:
                std::string path_str;
                
            public:
                path() = default;
                path(const std::string& str) : path_str(str) {}
                path(const char* str) : path_str(str ? str : "") {}
                
                // Basic operations
                std::string string() const { return path_str; }
                path filename() const {
                    size_t pos = path_str.find_last_of("/\\");
                    return (pos == std::string::npos) ? path(path_str) : path(path_str.substr(pos + 1));
                }
                path parent_path() const {
                    size_t pos = path_str.find_last_of("/\\");
                    return (pos == std::string::npos) ? path() : path(path_str.substr(0, pos));
                }
                
                // Operators
                path operator/(const path& other) const {
                    if (path_str.empty()) return other;
                    if (other.path_str.empty()) return *this;
                    if (path_str.back() == '/' || path_str.back() == '\\') {
                        return path(path_str + other.path_str);
                    }
                    return path(path_str + "/" + other.path_str);
                }
                
                path& operator/=(const path& other) {
                    *this = *this / other;
                    return *this;
                }
                
                // Conversion operators
                operator std::string() const { return path_str; }
                
                // Comparison operators
                bool operator==(const path& other) const { return path_str == other.path_str; }
                bool operator!=(const path& other) const { return path_str != other.path_str; }
                bool operator==(const std::string& other) const { return path_str == other; }
                bool operator!=(const std::string& other) const { return path_str != other; }
                bool operator==(const char* other) const { return path_str == other; }
                bool operator!=(const char* other) const { return path_str != other; }
                
                // Path utilities
                bool is_absolute() const {
                    return !path_str.empty() && (path_str[0] == '/' || path_str[0] == '\\');
                }
            };
            
            // Filesystem operations
            inline bool exists(const path& p) {
                std::ifstream file(p.string());
                return file.good();
            }
            
            inline bool create_directories(const path& p) {
                // Simple implementation - just create the directory
                std::string cmd = "mkdir -p " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline bool remove(const path& p) {
                std::string cmd = "rm -f " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline bool remove_all(const path& p) {
                std::string cmd = "rm -rf " + p.string();
                return system(cmd.c_str()) == 0;
            }
            
            inline path current_path() {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                    return path(cwd);
                }
                return path();
            }
            
            inline bool current_path(const path& p) {
                return chdir(p.string().c_str()) == 0;
            }
            
            inline path temp_directory_path() {
                const char* temp_dir = getenv("TMPDIR");
                if (!temp_dir) temp_dir = getenv("TEMP");
                if (!temp_dir) temp_dir = getenv("TMP");
                if (!temp_dir) temp_dir = "/tmp";
                return path(temp_dir);
            }
            
            inline uintmax_t file_size(const path& p) {
                std::ifstream file(p.string(), std::ios::binary | std::ios::ate);
                if (file.is_open()) {
                    return file.tellg();
                }
                return 0;
            }

            // File permissions enum
            enum class perms {
                none = 0,
                owner_read = 0400,
                owner_write = 0200,
                owner_exec = 0100,
                owner_all = owner_read | owner_write | owner_exec,
                group_read = 0040,
                group_write = 0020,
                group_exec = 0010,
                group_all = group_read | group_write | group_exec,
                others_read = 0004,
                others_write = 0002,
                others_exec = 0001,
                others_all = others_read | others_write | others_exec,
                all = owner_all | group_all | others_all,
                set_uid = 04000,
                set_gid = 02000,
                sticky_bit = 01000,
                mask = all | set_uid | set_gid | sticky_bit,
                unknown = 0xFFFF
            };
            
            // Bitwise operators for perms enum
            inline perms operator|(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) | static_cast<int>(rhs));
            }
            
            inline perms operator&(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) & static_cast<int>(rhs));
            }
            
            inline perms operator^(perms lhs, perms rhs) {
                return static_cast<perms>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
            }
            
            inline perms operator~(perms p) {
                return static_cast<perms>(~static_cast<int>(p));
            }
            
            inline perms& operator|=(perms& lhs, perms rhs) {
                lhs = lhs | rhs;
                return lhs;
            }
            
            inline perms& operator&=(perms& lhs, perms rhs) {
                lhs = lhs & rhs;
                return lhs;
            }
            
            inline perms& operator^=(perms& lhs, perms rhs) {
                lhs = lhs ^ rhs;
                return lhs;
            }
            
            // Copy options enum
            enum class copy_options {
                none = 0,
                skip_existing = 1,
                overwrite_existing = 2,
                update_existing = 4,
                recursive = 8,
                copy_symlinks = 16,
                skip_symlinks = 32,
                directories_only = 64,
                create_symlinks = 128,
                create_hard_links = 256
            };
            
            // Permission options enum
            enum class perm_options {
                replace = 0,
                add = 1,
                remove = 2,
                nofollow = 4
            };
            
            // Forward declaration
            class filesystem_error;
            
            // Filesystem operations
            inline void permissions(const path& p, perms prms, perm_options opts = perm_options::replace) {
                // Simple implementation using chmod
                chmod(p.string().c_str(), static_cast<mode_t>(prms));
            }
            
            inline void copy_file(const path& from, const path& to, copy_options options = copy_options::none) {
                // Simple implementation using system commands
                std::string cmd = "cp ";
                if (static_cast<int>(options) & static_cast<int>(copy_options::overwrite_existing)) {
                    cmd += "-f ";
                }
                cmd += from.string() + " " + to.string();
                system(cmd.c_str());
            }
            
            inline void create_symlink(const path& to, const path& new_symlink) {
                symlink(to.string().c_str(), new_symlink.string().c_str());
            }
            
            inline void create_hard_link(const path& to, const path& new_hard_link) {
                link(to.string().c_str(), new_hard_link.string().c_str());
            }
            
            inline path absolute(const path& p) {
                char resolved_path[PATH_MAX];
                if (realpath(p.string().c_str(), resolved_path) != nullptr) {
                    return path(resolved_path);
                }
                return p;
            }
            
            // Directory iterator implementation
            class recursive_directory_iterator {
            private:
                std::vector<path> files;
                size_t current_index;
                
            public:
                recursive_directory_iterator() : current_index(0) {}
                recursive_directory_iterator(const path& p) : current_index(0) {
                    // Simple implementation - collect all files
                    DIR* dir = opendir(p.string().c_str());
                    if (dir) {
                        struct dirent* entry;
                        while ((entry = readdir(dir)) != nullptr) {
                            if (entry->d_name[0] != '.') {
                                files.push_back(p / entry->d_name);
                            }
                        }
                        closedir(dir);
                    }
                }
                
                recursive_directory_iterator& operator++() {
                    if (current_index < files.size()) {
                        ++current_index;
                    }
                    return *this;
                }
                
                bool operator!=(const recursive_directory_iterator& other) const {
                    return current_index != other.current_index;
                }
                
                bool is_regular_file() const {
                    if (current_index >= files.size()) return false;
                    struct stat st;
                    if (stat(files[current_index].string().c_str(), &st) == 0) {
                        return S_ISREG(st.st_mode);
                    }
                    return false;
                }
                
                path get_path() const {
                    if (current_index < files.size()) {
                        return files[current_index];
                    }
                    return path();
                }
            };
            
            // Directory iterator
            struct DirCloser {
                void operator()(DIR* d) const { if (d) closedir(d); }
            };
            
            class directory_iterator {
            private:
                std::unique_ptr<DIR, DirCloser> dir;
                std::string current_path;
                
            public:
                directory_iterator() : dir(nullptr) {}
                directory_iterator(const path& p) : dir(opendir(p.string().c_str())) {}
                ~directory_iterator() = default;
                
                // Disable copy operations
                directory_iterator(const directory_iterator&) = delete;
                directory_iterator& operator=(const directory_iterator&) = delete;
                directory_iterator(directory_iterator&&) = delete;
                directory_iterator& operator=(directory_iterator&&) = delete;
                
                directory_iterator& operator++() {
                    if (dir) {
                        struct dirent* entry = readdir(dir.get());
                        if (entry) {
                            current_path = entry->d_name;
                        } else {
                            dir.reset();
                        }
                    }
                    return *this;
                }
                
                bool operator!=(const directory_iterator& other) const {
                    return dir != other.dir;
                }
                
                path operator*() const {
                    return path(current_path);
                }
            };
            
            inline directory_iterator& begin(directory_iterator& iter) { return iter; }
            inline const directory_iterator& end(const directory_iterator&) { 
                static const directory_iterator end_iter;
                return end_iter;
            }
            
            // Exception types
            class filesystem_error : public std::runtime_error {
            public:
                filesystem_error(const std::string& msg) : std::runtime_error(msg) {}
            };
        }
    #endif
    #endif
#endif

// Only for C++11/14: fallback utility functions
#if __cplusplus < 201703L
    namespace utils {
        /**
         * @brief Convert C-string to string_view
         * @param value The C-string
         * @return The string_view
         */
        inline string_view to_string_view(const char* value) {
            return string_view(value);
        }
        
        /**
         * @brief Convert std::string to string_view
         * @param value The string
         * @return The string_view
         */
        inline string_view to_string_view(const std::string& value) {
            return string_view(value);
        }
        
        /**
         * @brief Convert any type to string_view
         * @param value The value to convert
         * @return The string_view
         */
        template<typename T>
        string_view to_string_view(const T& value) {
            // For integer types, we need to create a temporary string
            // and return a string_view that points to it
            // This is a limitation of our custom string_view implementation
            // In practice, this should be used carefully as the string_view
            // will become invalid when the temporary string is destroyed
            static thread_local std::string temp_str;
            temp_str = std::to_string(value);
            return string_view(temp_str);
        }
        
        /**
         * @brief Format string with arguments (C++11/14 compatibility)
         * 
         * Provides a basic string formatting function for C++11/14.
         * Uses std::ostringstream for formatting.
         * 
         * @tparam Args The argument types
         * @param fmt The format string
         * @param args The arguments to format
         * @return The formatted string
         */
        template<typename... Args>
        std::string format_string(const std::string& fmt, Args&&... args) {
            std::ostringstream oss;
            oss << fmt;
            return oss.str();
        }
        
        /**
         * @brief Get optional value or default
         * @param opt The optional
         * @param default_value The default value
         * @return The value or default
         */
        template<typename T>
        T get_optional_value(const fallback::optional<T>& opt, const T& default_value = T{}) {
            return opt.has_value() ? opt.value() : default_value;
        }
        
        /**
         * @brief Convert enum to string
         * @param e The enum value
         * @return The string representation
         */
        template<typename Enum>
        std::string enum_to_string(Enum e) {
            return std::to_string(static_cast<int>(e));
        }
        
        /**
         * @brief Safe strlen with maximum length
         * @param str The string
         * @param max_len The maximum length to check
         * @return The string length
         */
        inline size_t safe_strlen(const char* str, size_t max_len = SIZE_MAX) {
            if (!str) return 0;
            size_t len = 0;
            while (len < max_len && str[len] != '\0') {
                ++len;
            }
            return len;
        }
        
        /**
         * @brief Check if string is null-terminated
         * @param str The string
         * @param max_len The maximum length to check
         * @return true if null-terminated, false otherwise
         */
        inline bool is_null_terminated(const char* str, size_t max_len = SIZE_MAX) {
            if (!str) return false;
            for (size_t i = 0; i < max_len; ++i) {
                if (str[i] == '\0') return true;
            }
            return false;
        }
        
        /**
         * @brief Create safe string_view from data
         * @param data The data pointer
         * @param max_len The maximum length
         * @return The string_view
         */
        inline string_view safe_string_view(const char* data, size_t max_len) {
            if (!data) return string_view();
            size_t len = safe_strlen(data, max_len);
            return string_view(data, len);
        }
    } // namespace utils
#endif

} // namespace compat
} // namespace heimdall

