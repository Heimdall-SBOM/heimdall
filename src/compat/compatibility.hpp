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
#if defined(HEIMDALL_CPP17_AVAILABLE) && !defined(HEIMDALL_CPP20_AVAILABLE)
    // C++17 includes
    #include <filesystem>
    #include <optional>
    #include <string_view>
    #include <variant>
#elif defined(HEIMDALL_CPP20_AVAILABLE) || defined(HEIMDALL_CPP23_AVAILABLE)
    // C++20/23 includes
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
    #if defined(HEIMDALL_CPP23_AVAILABLE)
        #if __has_include(<generator>)
            #include <generator>
        #endif
        #if __has_include(<print>)
            #include <print>
        #endif
        #if __has_include(<stacktrace>)
            #include <stacktrace>
        #endif
    #endif
#else
    // C++11/14 - Boost.Filesystem
    #if defined(USE_BOOST_FILESYSTEM) && USE_BOOST_FILESYSTEM
        #include <boost/filesystem.hpp>
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
#if defined(HEIMDALL_CPP17_AVAILABLE) && !defined(HEIMDALL_CPP20_AVAILABLE)
    // C++17: Use standard library with namespace alias
    namespace fs = std::filesystem;
    
    // Re-export standard types for compatibility
    using std::optional;
    using std::string_view;
    using std::variant;
    using std::monostate;
    
#elif defined(HEIMDALL_CPP20_AVAILABLE) || defined(HEIMDALL_CPP23_AVAILABLE)
    // C++20/23: Use standard library with namespace alias
    namespace fs = std::filesystem;
    
    // Re-export standard types for compatibility
    using std::optional;
    using std::string_view;
    using std::variant;
    using std::monostate;
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
    
    #if defined(HEIMDALL_CPP23_AVAILABLE)
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
    
#else
    // C++11/14: Custom implementations or Boost
    #if defined(USE_BOOST_FILESYSTEM) && USE_BOOST_FILESYSTEM
        namespace fs = boost::filesystem;
    #endif
    
    /**
     * @brief Custom optional implementation for C++11/14
     * 
     * Provides a basic optional implementation for C++11/14 that mimics
     * the std::optional interface from C++17.
     * 
     * @tparam T The type to store in the optional
     */
    template<typename T>
    class optional {
    private:
        bool has_value_;
        union {
            T value_;
            char dummy_;
        };
    public:
        /**
         * @brief Default constructor - creates an empty optional
         */
        optional() : has_value_(false), dummy_() {}
        
        /**
         * @brief Constructor with value
         * @param value The value to store
         */
        optional(const T& value) : has_value_(true), value_(value) {}
        
        /**
         * @brief Move constructor with value
         * @param value The value to move
         */
        optional(T&& value) : has_value_(true), value_(std::move(value)) {}
        
        /**
         * @brief Copy constructor
         * @param other The optional to copy from
         */
        optional(const optional& other) : has_value_(other.has_value_) {
            if (has_value_) {
                new (&value_) T(other.value_);
            }
        }
        
        /**
         * @brief Move constructor
         * @param other The optional to move from
         */
        optional(optional&& other) noexcept : has_value_(other.has_value_) {
            if (has_value_) {
                new (&value_) T(std::move(other.value_));
                other.has_value_ = false;
            }
        }
        
        /**
         * @brief Destructor
         */
        ~optional() {
            if (has_value_) {
                value_.~T();
            }
        }
        
        /**
         * @brief Copy assignment operator
         * @param other The optional to copy from
         * @return Reference to this optional
         */
        optional& operator=(const optional& other) {
            if (this != &other) {
                if (has_value_) {
                    value_.~T();
                }
                has_value_ = other.has_value_;
                if (has_value_) {
                    new (&value_) T(other.value_);
                }
            }
            return *this;
        }
        
        /**
         * @brief Move assignment operator
         * @param other The optional to move from
         * @return Reference to this optional
         */
        optional& operator=(optional&& other) noexcept {
            if (this != &other) {
                if (has_value_) {
                    value_.~T();
                }
                has_value_ = other.has_value_;
                if (has_value_) {
                    new (&value_) T(std::move(other.value_));
                    other.has_value_ = false;
                }
            }
            return *this;
        }
        
        /**
         * @brief Check if the optional has a value
         * @return true if the optional has a value, false otherwise
         */
        bool has_value() const { return has_value_; }
        
        /**
         * @brief Get the stored value
         * @return Reference to the stored value
         * @throws std::runtime_error if the optional is empty
         */
        T& value() {
            if (!has_value_) {
                throw std::runtime_error("Optional has no value");
            }
            return value_;
        }
        
        /**
         * @brief Get the stored value (const version)
         * @return Const reference to the stored value
         * @throws std::runtime_error if the optional is empty
         */
        const T& value() const {
            if (!has_value_) {
                throw std::runtime_error("Optional has no value");
            }
            return value_;
        }
        
        /**
         * @brief Dereference operator
         * @return Reference to the stored value
         */
        T& operator*() { return value(); }
        
        /**
         * @brief Dereference operator (const version)
         * @return Const reference to the stored value
         */
        const T& operator*() const { return value(); }

        /**
         * @brief Arrow operator
         * @return Pointer to the stored value
         */
        T* operator->() { return &value(); }

        /**
         * @brief Arrow operator (const version)
         * @return Const pointer to the stored value
         */
        const T* operator->() const { return &value(); }
        
        /**
         * @brief Get value or default
         * @param default_value The default value to return if optional is empty
         * @return The stored value or the default value
         */
        template<typename U>
        T value_or(U&& default_value) const {
            return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
        }
    };
    
    /**
     * @brief Custom string_view implementation for C++11/14
     * 
     * Provides a basic string_view implementation for C++11/14 that mimics
     * the std::string_view interface from C++17.
     */
    class string_view {
    private:
        const char* data_;
        size_t size_;
    public:
        /**
         * @brief Default constructor
         */
        string_view() : data_(nullptr), size_(0) {}
        
        /**
         * @brief Constructor from C-string
         * @param str The C-string to view
         */
        string_view(const char* str) : data_(str), size_(str ? std::strlen(str) : 0) {}
        
        /**
         * @brief Constructor from data and size
         * @param data The data pointer
         * @param size The size of the data
         */
        string_view(const char* data, size_t size) : data_(data), size_(size) {}
        
        /**
         * @brief Constructor from data with null termination check
         * @param data The data pointer
         * @param max_len The maximum length to check
         * @param find_null Whether to find null terminator
         */
        string_view(const char* data, size_t max_len, bool find_null) : data_(data), size_(0) {
            if (find_null && data) {
                size_ = std::strlen(data);
                if (size_ > max_len) {
                    size_ = max_len;
                }
            }
        }
        
        /**
         * @brief Constructor from std::string
         * @param str The string to view
         */
        string_view(const std::string& str) : data_(str.data()), size_(str.size()) {}
        
        /**
         * @brief Get the size of the string view
         * @return The size
         */
        size_t size() const { return size_; }
        
        /**
         * @brief Get the length of the string view
         * @return The length
         */
        size_t length() const { return size_; }
        
        /**
         * @brief Check if the string view is empty
         * @return true if empty, false otherwise
         */
        bool empty() const { return size_ == 0; }
        
        /**
         * @brief Access character at index
         * @param pos The position
         * @return Reference to the character
         */
        const char& operator[](size_t pos) const { return data_[pos]; }
        
        /**
         * @brief Convert to std::string
         * @return The string
         */
        std::string to_string() const { return std::string(data_, size_); }
        operator std::string() const { return to_string(); }
        
        /**
         * @brief Find character in string view
         * @param ch The character to find
         * @param pos The starting position
         * @return Position of the character, or std::string::npos if not found
         */
        size_t find(char ch, size_t pos = 0) const {
            for (size_t i = pos; i < size_; ++i) {
                if (data_[i] == ch) return i;
            }
            return std::string::npos;
        }
        
        /**
         * @brief Find substring in string view
         * @param str The substring to find
         * @param pos The starting position
         * @return Position of the substring, or std::string::npos if not found
         */
        size_t find(const string_view& str, size_t pos = 0) const {
            if (str.size() > size_ - pos) return std::string::npos;
            for (size_t i = pos; i <= size_ - str.size(); ++i) {
                if (std::memcmp(data_ + i, str.data_, str.size_) == 0) {
                    return i;
                }
            }
            return std::string::npos;
        }
        
        /**
         * @brief Get substring
         * @param pos The starting position
         * @param count The number of characters
         * @return The substring
         */
        string_view substr(size_t pos = 0, size_t count = std::string::npos) const {
            if (pos >= size_) return string_view();
            if (count == std::string::npos || pos + count > size_) {
                count = size_ - pos;
            }
            return string_view(data_ + pos, count);
        }
    };
    
    /**
     * @brief Simple variant implementation for C++11/14
     * 
     * Provides a basic variant implementation for C++11/14 that mimics
     * the std::variant interface from C++17.
     * 
     * @tparam Types The types that can be stored in the variant
     */
    template<typename... Types>
    class variant {
        using first_type = typename std::tuple_element<0, std::tuple<Types...>>::type;
    private:
        first_type value_;
        int index_;
    public:
        /**
         * @brief Default constructor
         */
        variant() : value_(), index_(0) {}
        
        /**
         * @brief Constructor with value
         * @param value The value to store
         */
        variant(const first_type& value) : value_(value), index_(0) {}
        
        /**
         * @brief Get the index of the stored type
         * @return The index
         */
        int index() const { return index_; }
        first_type& get() { return value_; }
        const first_type& get() const { return value_; }
    };
    
    /**
     * @brief Monostate for variant
     */
    struct monostate {};
    
    /**
     * @brief Simple span implementation for C++11/14
     * 
     * Provides a basic span implementation for C++11/14 that mimics
     * the std::span interface from C++20.
     * 
     * @tparam T The element type
     */
    template<typename T>
    class span {
    private:
        T* data_;
        size_t size_;
    public:
        /**
         * @brief Default constructor
         */
        span() : data_(nullptr), size_(0) {}
        
        /**
         * @brief Constructor from data and size
         * @param data The data pointer
         * @param size The size
         */
        span(T* data, size_t size) : data_(data), size_(size) {}
        
        /**
         * @brief Constructor from container
         * @param c The container
         */
        template<typename Container>
        span(Container& c) : data_(c.data()), size_(c.size()) {}
        
        /**
         * @brief Get the size of the span
         * @return The size
         */
        size_t size() const { return size_; }
        
        /**
         * @brief Check if the span is empty
         * @return true if empty, false otherwise
         */
        bool empty() const { return size_ == 0; }
        
        /**
         * @brief Access element at index
         * @param index The index
         * @return Reference to the element
         */
        T& operator[](size_t index) const { return data_[index]; }
    };
#endif

/**
 * @namespace heimdall::compat::utils
 * @brief Utility functions for compatibility layer
 * 
 * This namespace provides utility functions for safe string handling,
 * formatting, and other compatibility operations.
 */
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
    T get_optional_value(const optional<T>& opt, const T& default_value = T{}) {
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
} // namespace compat
} // namespace heimdall