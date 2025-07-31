
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
 * and platforms, ensuring consistent behavior across C++11, C++14, C++17, C++20, and C++23.
 */

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// System headers for filesystem operations
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Include standard library headers based on C++ standard
#if __cplusplus >= 202302L
// C++23 includes
#include <barrier>
#include <bit>
#include <compare>
#include <concepts>
#include <coroutine>
#include <filesystem>
#include <latch>
#include <numbers>
#include <optional>
#include <ranges>
#include <semaphore>
#include <span>
#include <stop_token>
#include <string_view>
#include <syncstream>
#include <variant>
#include <version>
#if __has_include(<expected>)
#include <expected>
#endif
#if __has_include(<flat_map>)
#include <flat_map>
#include <flat_set>
#endif
#if __has_include(<generator>)
#include <generator>
#endif
#if __has_include(<print>)
#include <print>
#endif
#if __has_include(<source_location>)
#include <source_location>
#endif
#elif __cplusplus >= 202002L
// C++20 includes
#include <barrier>
#include <bit>
#include <compare>
#include <concepts>
#include <coroutine>
#include <filesystem>
#include <latch>
#include <numbers>
#include <optional>
#include <ranges>
#include <semaphore>
#include <span>
#include <stop_token>
#include <string_view>
#include <syncstream>
#include <variant>
#include <version>
#if __has_include(<expected>)
#include <expected>
#endif
#if __has_include(<flat_map>)
#include <flat_map>
#include <flat_set>
#endif
#if __has_include(<source_location>)
#include <source_location>
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
namespace heimdall
{
namespace compat
{
namespace fs
{
class path
{
   private:
   std::string path_str;

   public:
   path() = default;
   path(const std::string& str) : path_str(str) {}
   path(const char* str) : path_str(str ? str : "") {}

   // Basic operations
   std::string string() const
   {
      return path_str;
   }

   path filename() const
   {
      size_t pos = path_str.find_last_of("/\\");
      return (pos == std::string::npos) ? path(path_str) : path(path_str.substr(pos + 1));
   }

   path parent_path() const
   {
      size_t pos = path_str.find_last_of("/\\");
      return (pos == std::string::npos) ? path() : path(path_str.substr(0, pos));
   }

   path& operator/=(const path& other)
   {
      if (other.path_str.empty())
         return *this;
      if (path_str.empty())
      {
         path_str = other.path_str;
         return *this;
      }
      if (path_str.back() == '/' || path_str.back() == '\\')
      {
         path_str += other.path_str;
      }
      else
      {
         path_str += "/" + other.path_str;
      }
      return *this;
   }

   path operator/(const path& other) const
   {
      path result = *this;
      result /= other;
      return result;
   }

   bool is_absolute() const
   {
      return !path_str.empty() && (path_str[0] == '/' || path_str[0] == '\\');
   }
};

inline bool exists(const path& p)
{
   struct stat st;
   return stat(p.string().c_str(), &st) == 0;
}

inline bool create_directories(const path& p)
{
   return mkdir(p.string().c_str(), 0755) == 0;
}

inline bool remove(const path& p)
{
   return unlink(p.string().c_str()) == 0;
}

inline bool remove_all(const path& p)
{
   return rmdir(p.string().c_str()) == 0;
}

inline path current_path()
{
   char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != nullptr)
   {
      return path(cwd);
   }
   return path();
}

inline bool current_path(const path& p)
{
   return chdir(p.string().c_str()) == 0;
}

inline path temp_directory_path()
{
   const char* temp = getenv("TMPDIR");
   if (!temp)
      temp = getenv("TMP");
   if (!temp)
      temp = getenv("TEMP");
   if (!temp)
      temp = "/tmp";
   return path(temp);
}

inline uintmax_t file_size(const path& p)
{
   struct stat st;
   if (stat(p.string().c_str(), &st) == 0)
   {
      return static_cast<uintmax_t>(st.st_size);
   }
   return 0;
}

enum class perms
{
   none         = 0,
   owner_read   = 0400,
   owner_write  = 0200,
   owner_exec   = 0100,
   owner_all    = owner_read | owner_write | owner_exec,
   group_read   = 0040,
   group_write  = 0020,
   group_exec   = 0010,
   group_all    = group_read | group_write | group_exec,
   others_read  = 0004,
   others_write = 0002,
   others_exec  = 0001,
   others_all   = others_read | others_write | others_exec,
   all          = owner_all | group_all | others_all,
   set_uid      = 04000,
   set_gid      = 02000,
   sticky_bit   = 01000,
   mask         = all | set_uid | set_gid | sticky_bit,
   unknown      = 0xFFFF
};

inline perms operator|(perms lhs, perms rhs)
{
   return static_cast<perms>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline perms operator&(perms lhs, perms rhs)
{
   return static_cast<perms>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline perms operator^(perms lhs, perms rhs)
{
   return static_cast<perms>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

inline perms operator~(perms p)
{
   return static_cast<perms>(~static_cast<int>(p));
}

inline perms& operator|=(perms& lhs, perms rhs)
{
   lhs = lhs | rhs;
   return lhs;
}

inline perms& operator&=(perms& lhs, perms rhs)
{
   lhs = lhs & rhs;
   return lhs;
}

inline perms& operator^=(perms& lhs, perms rhs)
{
   lhs = lhs ^ rhs;
   return lhs;
}

enum class copy_options
{
   none               = 0,
   skip_existing      = 1,
   overwrite_existing = 2,
   update_existing    = 4,
   recursive          = 8,
   copy_symlinks      = 16,
   skip_symlinks      = 32,
   directories_only   = 64,
   create_symlinks    = 128,
   create_hard_links  = 256
};

enum class perm_options
{
   replace  = 0,
   add      = 1,
   remove   = 2,
   nofollow = 4
};

class filesystem_error;

inline void permissions(const path& p, perms prms, perm_options opts = perm_options::replace)
{
   // Simplified implementation
}

inline void copy_file(const path& from, const path& to, copy_options options = copy_options::none)
{
   std::ifstream src(from.string(), std::ios::binary);
   std::ofstream dst(to.string(), std::ios::binary);
   dst << src.rdbuf();
}

inline void create_symlink(const path& to, const path& new_symlink)
{
   symlink(to.string().c_str(), new_symlink.string().c_str());
}

inline void create_hard_link(const path& to, const path& new_hard_link)
{
   link(to.string().c_str(), new_hard_link.string().c_str());
}

inline path absolute(const path& p)
{
   if (p.is_absolute())
      return p;
   return current_path() / p;
}

class recursive_directory_iterator
{
   private:
   std::vector<path> files;
   size_t            current_index;

   public:
   recursive_directory_iterator() : current_index(0) {}
   recursive_directory_iterator(const path& p) : current_index(0)
   {
      DIR* dir = opendir(p.string().c_str());
      if (dir)
      {
         struct dirent* entry;
         while ((entry = readdir(dir)) != nullptr)
         {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
               path full_path = p / entry->d_name;
               files.push_back(full_path);
            }
         }
         closedir(dir);
      }
   }

   recursive_directory_iterator& operator++()
   {
      if (current_index < files.size())
      {
         ++current_index;
      }
      return *this;
   }

   bool operator!=(const recursive_directory_iterator& other) const
   {
      return current_index != other.current_index;
   }

   bool is_regular_file() const
   {
      if (current_index >= files.size())
         return false;
      struct stat st;
      return stat(files[current_index].string().c_str(), &st) == 0 && S_ISREG(st.st_mode);
   }

   path get_path() const
   {
      if (current_index < files.size())
      {
         return files[current_index];
      }
      return path();
   }
};

struct DirCloser
{
   void operator()(DIR* d) const
   {
      if (d)
         closedir(d);
   }
};

class directory_iterator
{
   private:
   std::unique_ptr<DIR, DirCloser> dir;
   std::string                     current_path;

   public:
   directory_iterator() : dir(nullptr) {}
   directory_iterator(const path& p) : dir(opendir(p.string().c_str())) {}
   ~directory_iterator() = default;

   // Disable copy operations
   directory_iterator(const directory_iterator&)            = delete;
   directory_iterator& operator=(const directory_iterator&) = delete;
   directory_iterator(directory_iterator&&)                 = delete;
   directory_iterator& operator=(directory_iterator&&)      = delete;

   directory_iterator& operator++()
   {
      if (dir)
      {
         struct dirent* entry = readdir(dir.get());
         if (entry)
         {
            current_path = entry->d_name;
         }
         else
         {
            dir.reset();
         }
      }
      return *this;
   }

   bool operator!=(const directory_iterator& other) const
   {
      return dir != other.dir;
   }

   bool is_regular_file() const
   {
      if (current_path.empty())
         return false;
      struct stat st;
      return stat(current_path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
   }

   path get_path() const
   {
      return path(current_path);
   }
};

class filesystem_error : public std::runtime_error
{
   public:
   filesystem_error(const std::string& msg) : std::runtime_error(msg) {}
};
}  // namespace fs
}  // namespace compat
}  // namespace heimdall
#endif
#endif

// Now define the compatibility namespace
namespace heimdall
{
namespace compat
{

/**
 * @brief C++11/14/17/20/23 compatibility: make_unique implementation
 */
#if __cplusplus >= 201402L
// C++14 and above: alias to std::make_unique
template <typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args&&... args)
{
   return std::make_unique<T>(std::forward<Args>(args)...);
}
#else
// C++11: provide fallback
template <typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args&&... args)
{
   return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

// Define concepts for C++20/23
#if __cplusplus >= 202002L
template <typename T>
concept integral = std::integral<T>;
template <typename T>
concept floating_point = std::floating_point<T>;
template <typename T>
concept convertible_to_string = requires(T t) {
   { std::to_string(t) } -> std::convertible_to<std::string>;
};
template <typename T>
concept range = requires(T& t) {
   { std::begin(t) } -> std::input_iterator;
   { std::end(t) } -> std::input_iterator;
};
#else
// C++11/14/17: Use type traits instead
template <typename T>
using integral = typename std::enable_if<std::is_integral<T>::value>::type;
template <typename T>
using floating_point = typename std::enable_if<std::is_floating_point<T>::value>::type;
template <typename T>
using convertible_to_string = typename std::enable_if<
   std::is_convertible<decltype(std::to_string(std::declval<T>())), std::string>::value>::type;
#endif

// Utility functions
template <typename T>
constexpr auto to_underlying(T e) noexcept
{
   return static_cast<std::underlying_type_t<T>>(e);
}

// String formatting utilities
#if __cplusplus >= 202002L && __has_include(<format>)
template <typename... Args>
std::string format_string(const std::string& fmt, Args&&... args)
{
   return std::format(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void print_string(const std::string& fmt, Args&&... args)
{
   std::cout << format_string(fmt, std::forward<Args>(args)...);
}
#else
template <typename... Args>
std::string format_string(const std::string& fmt, Args&&... args)
{
   // Fallback implementation using stringstream
   std::ostringstream oss;
   // Simplified implementation - in practice you'd want a proper format library
   return fmt;
}

template <typename... Args>
void print_string(const std::string& fmt, Args&&... args)
{
   std::cout << format_string(fmt, std::forward<Args>(args)...);
}
#endif

// Range utilities for C++20/23
#if __cplusplus >= 202002L
template <range R, typename F>
auto filter(R&& r, F&& f)
{
   return std::views::filter(std::forward<R>(r), std::forward<F>(f));
}

template <range R, typename F>
auto transform(R&& r, F&& f)
{
   return std::views::transform(std::forward<R>(r), std::forward<F>(f));
}

template <range R>
auto take(R&& r, std::size_t n)
{
   return std::views::take(std::forward<R>(r), n);
}

template <range R>
auto drop(R&& r, std::size_t n)
{
   return std::views::drop(std::forward<R>(r), n);
}

template <range R>
auto reverse(R&& r)
{
   return std::views::reverse(std::forward<R>(r));
}

template <typename Container, range R>
Container to_container(R&& r)
{
   return Container(std::begin(r), std::end(r));
}

template <range R>
auto to_vector(R&& r)
{
   return std::vector<std::ranges::range_value_t<R>>(std::begin(r), std::end(r));
}

template <range R>
auto to_set(R&& r)
{
   return std::set<std::ranges::range_value_t<R>>(std::begin(r), std::end(r));
}
#else
// C++11/14/17 fallbacks
template <typename R, typename F>
auto filter(R&& r, F&& f)
{
   std::vector<typename std::decay_t<R>::value_type> result;
   for (const auto& item : r)
   {
      if (f(item))
      {
         result.push_back(item);
      }
   }
   return result;
}

template <typename R, typename F>
auto transform(R&& r, F&& f)
{
   std::vector<decltype(f(std::declval<typename std::decay_t<R>::value_type>()))> result;
   for (const auto& item : r)
   {
      result.push_back(f(item));
   }
   return result;
}

template <typename R>
auto take(R&& r, std::size_t n)
{
   std::vector<typename std::decay_t<R>::value_type> result;
   auto                                              it  = std::begin(r);
   auto                                              end = std::end(r);
   for (std::size_t i = 0; i < n && it != end; ++i, ++it)
   {
      result.push_back(*it);
   }
   return result;
}

template <typename R>
auto drop(R&& r, std::size_t n)
{
   std::vector<typename std::decay_t<R>::value_type> result;
   auto                                              it  = std::begin(r);
   auto                                              end = std::end(r);
   for (std::size_t i = 0; i < n && it != end; ++i, ++it)
   {
      // Skip first n elements
   }
   for (; it != end; ++it)
   {
      result.push_back(*it);
   }
   return result;
}

template <typename R>
auto reverse(R&& r)
{
   std::vector<typename std::decay_t<R>::value_type> result;
   for (auto it = std::rbegin(r); it != std::rend(r); ++it)
   {
      result.push_back(*it);
   }
   return result;
}

template <typename Container, typename R>
Container to_container(R&& r)
{
   return Container(std::begin(r), std::end(r));
}

template <typename R>
auto to_vector(R&& r)
{
   return std::vector<typename std::decay_t<R>::value_type>(std::begin(r), std::end(r));
}

template <typename R>
auto to_set(R&& r)
{
   return std::set<typename std::decay_t<R>::value_type>(std::begin(r), std::end(r));
}
#endif

}  // namespace compat
}  // namespace heimdall

// Namespace aliases for compatibility
#if __cplusplus >= 201703L
namespace fs = std::filesystem;
#else
// For C++11/14, use our fallback filesystem implementation
namespace fs = heimdall::compat::fs;
#endif

// Global compatibility macros
constexpr bool HEIMDALL_CPP23_AVAILABLE = __cplusplus >= 202302L;
constexpr bool HEIMDALL_CPP20_AVAILABLE = __cplusplus >= 202002L;
constexpr bool HEIMDALL_CPP17_AVAILABLE = __cplusplus >= 201703L;
constexpr bool HEIMDALL_CPP14_AVAILABLE = __cplusplus >= 201402L;
constexpr bool HEIMDALL_CPP11_AVAILABLE = __cplusplus >= 201103L;

constexpr bool HEIMDALL_FULL_DWARF      = HEIMDALL_CPP17_AVAILABLE;
constexpr bool HEIMDALL_BASIC_DWARF     = HEIMDALL_CPP14_AVAILABLE;
constexpr bool HEIMDALL_NO_DWARF        = HEIMDALL_CPP11_AVAILABLE;
constexpr bool HEIMDALL_MODERN_FEATURES = HEIMDALL_CPP20_AVAILABLE;
