#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <tuple>

namespace heimdall {
namespace compat {

// Feature detection macros
#if __cplusplus >= 201703L
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
    
#endif

} // namespace compat
} // namespace heimdall