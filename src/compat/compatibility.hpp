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

// Feature detection constants
namespace heimdall {
namespace compat {
namespace detail {
    constexpr bool cpp23_available = __cplusplus >= 202302L;
    constexpr bool cpp20_available = __cplusplus >= 202002L;
    constexpr bool cpp17_available = __cplusplus >= 201703L;
    constexpr bool cpp14_available = __cplusplus >= 201402L;
    constexpr bool cpp11_available = __cplusplus >= 201103L;
    
    constexpr bool full_dwarf_available = cpp17_available;
    constexpr bool basic_dwarf_available = cpp14_available;
    constexpr bool no_dwarf_available = cpp11_available;
    constexpr bool modern_features_available = cpp20_available;
}
}
}

// Feature detection constants for backward compatibility
constexpr bool HEIMDALL_CPP23_AVAILABLE = heimdall::compat::detail::cpp23_available;
constexpr bool HEIMDALL_CPP20_AVAILABLE = heimdall::compat::detail::cpp20_available;
constexpr bool HEIMDALL_CPP17_AVAILABLE = heimdall::compat::detail::cpp17_available;
constexpr bool HEIMDALL_CPP14_AVAILABLE = heimdall::compat::detail::cpp14_available;
constexpr bool HEIMDALL_CPP11_AVAILABLE = heimdall::compat::detail::cpp11_available;
constexpr bool HEIMDALL_FULL_DWARF = heimdall::compat::detail::full_dwarf_available;
constexpr bool HEIMDALL_BASIC_DWARF = heimdall::compat::detail::basic_dwarf_available;
constexpr bool HEIMDALL_NO_DWARF = heimdall::compat::detail::no_dwarf_available;
constexpr bool HEIMDALL_MODERN_FEATURES = heimdall::compat::detail::modern_features_available;

#if !HEIMDALL_CPP11_AVAILABLE
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

// C++11/14/17/20/23 compatibility: make_unique implementation
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
    
    // Custom optional implementation for C++11/14
    template<typename T>
    class optional {
    private:
        bool has_value_;
        union {
            T value_;
            char dummy_;
        };
    public:
        optional() : has_value_(false), dummy_() {}
        optional(const T& value) : has_value_(true), value_(value) {}
        optional(T&& value) : has_value_(true), value_(std::move(value)) {}
        optional(const optional& other) : has_value_(other.has_value_) {
            if (has_value_) {
                new (&value_) T(other.value_);
            }
        }
        optional(optional&& other) noexcept : has_value_(other.has_value_) {
            if (has_value_) {
                new (&value_) T(std::move(other.value_));
                other.has_value_ = false;
            }
        }
        ~optional() {
            if (has_value_) {
                value_.~T();
            }
        }
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
        bool has_value() const { return has_value_; }
        explicit operator bool() const { return has_value_; }
        T& value() {
            if (!has_value_) {
                throw std::runtime_error("optional has no value");
            }
            return value_;
        }
        const T& value() const {
            if (!has_value_) {
                throw std::runtime_error("optional has no value");
            }
            return value_;
        }
        T& operator*() { return value(); }
        const T& operator*() const { return value(); }
        T* operator->() { return &value(); }
        const T* operator->() const { return &value(); }
        template<typename U>
        T value_or(U&& default_value) const {
            return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
        }
    };
    
    // Custom string_view implementation for C++11/14
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
        size_t length() const { return size_; }
        bool empty() const { return size_ == 0; }
        const char& operator[](size_t pos) const { return data_[pos]; }
        const char* begin() const { return data_; }
        const char* end() const { return data_ + size_; }
        std::string to_string() const { return std::string(data_, size_); }
        operator std::string() const { return to_string(); }
        size_t find(char ch, size_t pos = 0) const {
            for (size_t i = pos; i < size_; ++i) {
                if (data_[i] == ch) return i;
            }
            return std::string::npos;
        }
        size_t find(const string_view& str, size_t pos = 0) const {
            if (str.size() > size_ - pos) return std::string::npos;
            for (size_t i = pos; i <= size_ - str.size(); ++i) {
                if (std::equal(str.begin(), str.end(), data_ + i)) {
                    return i;
                }
            }
            return std::string::npos;
        }
        string_view substr(size_t pos = 0, size_t count = std::string::npos) const {
            if (pos > size_) throw std::out_of_range("string_view::substr");
            size_t actual_count = std::min(count, size_ - pos);
            return string_view(data_ + pos, actual_count);
        }
    };
    
    // Simple variant implementation for C++11/14 (limited functionality)
    template<typename... Types>
    class variant {
        static_assert(sizeof...(Types) > 0, "variant must have at least one type");
    private:
        using first_type = typename std::tuple_element<0, std::tuple<Types...>>::type;
        first_type value_;
        int index_;
    public:
        variant() : value_(), index_(0) {}
        variant(const first_type& value) : value_(value), index_(0) {}
        first_type& get() { return value_; }
        const first_type& get() const { return value_; }
        int index() const { return index_; }
    };
    
    struct monostate {};
    
    // Custom span implementation for C++11/14
    template<typename T>
    class span {
    private:
        T* data_;
        size_t size_;
    public:
        span() : data_(nullptr), size_(0) {}
        span(T* data, size_t size) : data_(data), size_(size) {}
        template<typename Container>
        span(Container& c) : data_(c.data()), size_(c.size()) {}
        T* data() const { return data_; }
        size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }
        T& operator[](size_t index) const { return data_[index]; }
        T* begin() const { return data_; }
        T* end() const { return data_ + size_; }
        T& front() const { return data_[0]; }
        T& back() const { return data_[size_ - 1]; }
    };
#endif

// Utility functions that work across all C++ standards
namespace utils {
    template<typename T>
    string_view to_string_view(const T& value) {
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || defined(HEIMDALL_CPP23_AVAILABLE)
        if constexpr (std::is_convertible_v<T, string_view>) {
            return string_view(value);
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            return string_view(std::string(value));
        } else {
            static thread_local std::string temp;
            temp = std::to_string(value);
            return string_view(temp);
        }
#else
        // C++11/14: no if constexpr, no is_convertible_v
        // Only handle string_view, std::string, and fallback to std::to_string
        return string_view(std::to_string(value));
#endif
    }
    
    template<typename... Args>
    std::string format_string(const std::string& fmt, Args&&... args) {
#if HEIMDALL_CPP17_AVAILABLE
        // Use std::format if available (C++20+), else fallback
        return fmt; // No formatting for C++11/14/17
#else
        std::string result = fmt;
        return result;
#endif
    }
    
    template<typename T>
    T get_optional_value(const optional<T>& opt, const T& default_value = T{}) {
        return opt.has_value() ? opt.value() : default_value;
    }
    
    template<typename Enum>
    std::string enum_to_string(Enum e) {
#if defined(HEIMDALL_CPP17_AVAILABLE) || defined(HEIMDALL_CPP20_AVAILABLE) || defined(HEIMDALL_CPP23_AVAILABLE)
        return std::to_string(static_cast<std::underlying_type_t<Enum>>(e));
#else
        return std::to_string(static_cast<typename std::underlying_type<Enum>::type>(e));
#endif
    }
} // namespace utils

} // namespace compat
} // namespace heimdall