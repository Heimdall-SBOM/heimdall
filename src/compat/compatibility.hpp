#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>

namespace heimdall {
namespace compat {

// Feature detection macros
#if __cplusplus >= 202302L
    #define HEIMDALL_CPP23_AVAILABLE 1
    #define HEIMDALL_CPP20_AVAILABLE 1
    #define HEIMDALL_CPP17_AVAILABLE 1
    #define HEIMDALL_FULL_DWARF 1
    #define HEIMDALL_MODERN_FEATURES 1
#elif __cplusplus >= 202002L
    #define HEIMDALL_CPP20_AVAILABLE 1
    #define HEIMDALL_CPP17_AVAILABLE 1
    #define HEIMDALL_FULL_DWARF 1
    #define HEIMDALL_MODERN_FEATURES 1
#elif __cplusplus >= 201703L
    #define HEIMDALL_CPP17_AVAILABLE 1
    #define HEIMDALL_FULL_DWARF 1
#elif __cplusplus >= 201402L
    #define HEIMDALL_CPP14_AVAILABLE 1
    #define HEIMDALL_BASIC_DWARF 1
#elif __cplusplus >= 201103L
    #define HEIMDALL_CPP11_AVAILABLE 1
    #define HEIMDALL_NO_DWARF 1
#else
    #error "C++11 or later required"
#endif

#if HEIMDALL_CPP17_AVAILABLE
    // Use standard library features
    #include <filesystem>
    #include <optional>
    #include <string_view>
    #include <variant>
    
    namespace fs = std::filesystem;
    using std::optional;
    using std::string_view;
    using std::variant;
    using std::monostate;
    
    #if HEIMDALL_CPP20_AVAILABLE
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
        
        using std::span;
        using std::source_location;
        using std::strong_ordering;
        using std::weak_ordering;
        using std::partial_ordering;
        
        // C++20 concepts
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
        
        // C++20 ranges
        namespace ranges = std::ranges;
        
        // C++20 format
        namespace fmt = std::format;
        
        // C++20 expected (if available)
        #if __has_include(<expected>)
            #include <expected>
            using std::expected;
            using std::unexpected;
        #else
            // Custom expected implementation for C++20 without <expected>
            template<typename T, typename E>
            class expected {
            private:
                bool has_value_;
                union {
                    T value_;
                    E error_;
                };
                
            public:
                expected(const T& value) : has_value_(true), value_(value) {}
                expected(T&& value) : has_value_(true), value_(std::move(value)) {}
                expected(const unexpected<E>& err) : has_value_(false), error_(err.value()) {}
                
                ~expected() {
                    if (has_value_) {
                        value_.~T();
                    } else {
                        error_.~E();
                    }
                }
                
                bool has_value() const { return has_value_; }
                explicit operator bool() const { return has_value_; }
                
                T& value() {
                    if (!has_value_) {
                        throw std::runtime_error("expected has no value");
                    }
                    return value_;
                }
                
                const T& value() const {
                    if (!has_value_) {
                        throw std::runtime_error("expected has no value");
                    }
                    return value_;
                }
                
                E& error() {
                    if (has_value_) {
                        throw std::runtime_error("expected has value, not error");
                    }
                    return error_;
                }
                
                const E& error() const {
                    if (has_value_) {
                        throw std::runtime_error("expected has value, not error");
                    }
                    return error_;
                }
                
                T& operator*() { return value(); }
                const T& operator*() const { return value(); }
                
                T* operator->() { return &value(); }
                const T* operator->() const { return &value(); }
            };
            
            template<typename E>
            class unexpected {
            private:
                E value_;
                
            public:
                unexpected(const E& value) : value_(value) {}
                unexpected(E&& value) : value_(std::move(value)) {}
                
                const E& value() const { return value_; }
                E& value() { return value_; }
            };
        #endif
        
        // C++20 flat_map and flat_set (if available)
        #if __has_include(<flat_map>)
            #include <flat_map>
            #include <flat_set>
            using std::flat_map;
            using std::flat_set;
        #endif
        
        // C++20 move_only_function (if available)
        #if __has_include(<functional>)
            #include <functional>
            using std::move_only_function;
        #endif
        
    #endif // HEIMDALL_CPP20_AVAILABLE
    
    #if HEIMDALL_CPP23_AVAILABLE
        // C++23 features
        #include <generator>
        #include <expected>
        #include <flat_map>
        #include <flat_set>
        #include <move_only_function>
        #include <print>
        #include <stacktrace>
        #include <mdspan>
        #include <text_encoding>
        #include <string_view>
        #include <optional>
        
        using std::generator;
        using std::expected;
        using std::unexpected;
        using std::flat_map;
        using std::flat_set;
        using std::move_only_function;
        using std::print;
        using std::stacktrace;
        
        // C++23 concepts
        template<typename T>
        concept sized_range = std::ranges::sized_range<T>;
        
        template<typename T>
        concept random_access_range = std::ranges::random_access_range<T>;
        
        template<typename T>
        concept contiguous_range = std::ranges::contiguous_range<T>;
        
        // C++23 utilities
        template<typename T>
        constexpr auto to_underlying(T e) noexcept {
            return static_cast<std::underlying_type_t<T>>(e);
        }
        
    #endif // HEIMDALL_CPP23_AVAILABLE
    
#else
    // Use Boost.Filesystem and custom implementations
    #include <boost/filesystem.hpp>
    namespace fs = boost::filesystem;
    
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
        
        std::string to_string() const {
            return std::string(data_, size_);
        }
        
        operator std::string() const {
            return to_string();
        }
        
        // Basic find operations
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
        
        // Substring
        string_view substr(size_t pos = 0, size_t count = std::string::npos) const {
            if (pos > size_) throw std::out_of_range("string_view::substr");
            size_t actual_count = std::min(count, size_ - pos);
            return string_view(data_ + pos, actual_count);
        }
    };
    
    // Simple variant implementation for C++11/14 (limited functionality)
    template<typename... Types>
    class variant {
        // Simplified variant - only supports first type for now
        // Full implementation would be much more complex
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
    
    // Monostate for variant
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

// Safe string conversion
template<typename T>
string_view to_string_view(const T& value) {
    if constexpr (std::is_convertible_v<T, string_view>) {
        return string_view(value);
    } else if constexpr (std::is_convertible_v<T, std::string>) {
        return string_view(std::string(value));
    } else {
        // Fallback for other types
        static thread_local std::string temp;
        temp = std::to_string(value);
        return string_view(temp);
    }
}

// Format string (C++20+ uses std::format, older uses sprintf)
template<typename... Args>
std::string format_string(const std::string& fmt, Args&&... args) {
#if HEIMDALL_CPP20_AVAILABLE
    return std::format(fmt, std::forward<Args>(args)...);
#else
    // Simple implementation for older C++
    std::string result = fmt;
    // This is a simplified version - in practice you'd want a proper formatting library
    return result;
#endif
}

// Safe optional access
template<typename T>
T get_optional_value(const optional<T>& opt, const T& default_value = T{}) {
    return opt.has_value() ? opt.value() : default_value;
}

// Type-safe enum to string conversion
template<typename Enum>
std::string enum_to_string(Enum e) {
    if constexpr (HEIMDALL_CPP23_AVAILABLE) {
        return std::format("{}", to_underlying(e));
    } else {
        return std::to_string(static_cast<std::underlying_type_t<Enum>>(e));
    }
}

} // namespace utils

} // namespace compat
} // namespace heimdall