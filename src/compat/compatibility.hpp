
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

// Provide compatibility namespace for C++17+ as well
namespace heimdall
{
namespace compat
{
namespace fs     = std::filesystem;
}
}
#else
// C++11/14: Custom implementations or Boost
#include <chrono>

// C++14 optional implementation will be defined in heimdall::compat namespace

#if defined(USE_BOOST_FILESYSTEM) && USE_BOOST_FILESYSTEM
namespace fs = boost::filesystem;
#else
// Fallback filesystem implementation for C++11/14 without Boost
namespace heimdall
{
namespace compat
{

// bad_optional_access exception (needs to be declared before optional)
class bad_optional_access : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "bad optional access";
    }
};

// C++14 optional implementation
template<typename T>
class optional
{
private:
    bool has_value_;
    union {
        T value_;
        char dummy_;
    };

public:
    // Constructors
    optional() : has_value_(false), dummy_() {}
    
    optional(const T& value) : has_value_(true), value_(value) {}
    
    optional(T&& value) : has_value_(true), value_(std::move(value)) {}
    
    optional(const optional& other) : has_value_(other.has_value_)
    {
        if (has_value_) {
            new (&value_) T(other.value_);
        }
    }
    
    optional(optional&& other) : has_value_(other.has_value_)
    {
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        }
    }
    
    // Destructor
    ~optional()
    {
        if (has_value_) {
            value_.~T();
        }
    }
    
    // Assignment operators
    optional& operator=(const optional& other)
    {
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
    
    optional& operator=(optional&& other)
    {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value_) T(std::move(other.value_));
            }
        }
        return *this;
    }
    
    // Value access
    T& operator*() { return value_; }
    const T& operator*() const { return value_; }
    
    T* operator->() { return &value_; }
    const T* operator->() const { return &value_; }
    
    // Boolean conversion
    explicit operator bool() const { return has_value_; }
    
    // Value access with checks
    T& value() { 
        if (!has_value_) throw bad_optional_access();
        return value_; 
    }
    
    const T& value() const { 
        if (!has_value_) throw bad_optional_access();
        return value_; 
    }
    
    // Has value check
    bool has_value() const { return has_value_; }
    
    // Reset
    void reset() 
    { 
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
    }
};

// nullopt implementation
struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};
constexpr nullopt_t nullopt{0};
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

   path extension() const
   {
      std::string filename_str = filename().string();
      size_t pos = filename_str.find_last_of('.');
      return (pos == std::string::npos) ? path() : path(filename_str.substr(pos));
   }

   path stem() const
   {
      std::string filename_str = filename().string();
      size_t pos = filename_str.find_last_of('.');
      return (pos == std::string::npos) ? path(filename_str) : path(filename_str.substr(0, pos));
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

   // Comparison operators
   bool operator==(const path& other) const
   {
      return path_str == other.path_str;
   }

   bool operator==(const std::string& other) const
   {
      return path_str == other;
   }

   bool operator==(const char* other) const
   {
      return path_str == other;
   }

   bool operator!=(const path& other) const
   {
      return path_str != other.path_str;
   }

   bool operator!=(const std::string& other) const
   {
      return path_str != other;
   }

   bool operator!=(const char* other) const
   {
      return path_str != other;
   }

   path lexically_normal() const
   {
      // Simplified implementation - just return the path as-is
      return *this;
   }

   // Iterator support for path components
   class iterator
   {
   private:
      std::string path_str;
      size_t pos;
      std::string current;

   public:
      iterator() : pos(std::string::npos) {}
      iterator(const std::string& str, size_t p) : path_str(str), pos(p)
      {
         if (pos != std::string::npos)
         {
            size_t next_pos = path_str.find('/', pos);
            if (next_pos == std::string::npos)
            {
               current = path_str.substr(pos);
               pos = std::string::npos;
            }
            else
            {
               current = path_str.substr(pos, next_pos - pos);
               pos = next_pos + 1;
            }
         }
      }

      iterator& operator++()
      {
         if (pos != std::string::npos)
         {
            size_t next_pos = path_str.find('/', pos);
            if (next_pos == std::string::npos)
            {
               current = path_str.substr(pos);
               pos = std::string::npos;
            }
            else
            {
               current = path_str.substr(pos, next_pos - pos);
               pos = next_pos + 1;
            }
         }
         return *this;
      }

      const std::string& operator*() const { return current; }
      bool operator!=(const iterator& other) const { return pos != other.pos; }
   };

   iterator begin() const
   {
      return iterator(path_str, 0);
   }

   iterator end() const
   {
      return iterator();
   }
};

inline bool exists(const path& p)
{
   struct stat st;
   return stat(p.string().c_str(), &st) == 0;
}

inline bool is_directory(const path& p)
{
   struct stat st;
   return stat(p.string().c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool is_regular_file(const path& p)
{
   struct stat st;
   return stat(p.string().c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

inline uintmax_t file_size(const path& p)
{
   struct stat st;
   if (stat(p.string().c_str(), &st) == 0)
   {
      return static_cast<uintmax_t>(st.st_size);
   }
   return static_cast<uintmax_t>(-1);
}

inline path canonical(const path& p)
{
   char resolved_path[PATH_MAX];
   if (realpath(p.string().c_str(), resolved_path) != nullptr)
   {
      return path(resolved_path);
   }
   return p;
}





// Simplified file_time_type
class file_time_type
{
private:
    std::chrono::system_clock::time_point time_;

public:
    file_time_type() : time_(std::chrono::system_clock::now()) {}
    file_time_type(std::chrono::system_clock::time_point t) : time_(t) {}
    
    std::chrono::system_clock::time_point time_since_epoch() const { return time_; }
    
    // Add duration_cast support
    template<typename Rep, typename Period>
    auto duration_cast() const {
        return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(time_.time_since_epoch());
    }
};

inline file_time_type last_write_time(const path& p)
{
   struct stat st;
   if (stat(p.string().c_str(), &st) == 0)
   {
      auto time_point = std::chrono::system_clock::from_time_t(st.st_mtime);
      return file_time_type(time_point);
   }
   return file_time_type();
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

// Simplified file_status class
class file_status
{
private:
    perms permissions_;

public:
       file_status() : permissions_(perms::none) {}
   file_status(perms p) : permissions_(p) {}
   
   perms permissions() const { return permissions_; }
   void permissions(perms p) { permissions_ = p; }
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

inline file_status status(const path& p)
{
   struct stat st;
   if (stat(p.string().c_str(), &st) == 0)
   {
      perms p = perms::none;
      if (st.st_mode & S_IRUSR) p |= perms::owner_read;
      if (st.st_mode & S_IWUSR) p |= perms::owner_write;
      if (st.st_mode & S_IXUSR) p |= perms::owner_exec;
      if (st.st_mode & S_IRGRP) p |= perms::group_read;
      if (st.st_mode & S_IWGRP) p |= perms::group_write;
      if (st.st_mode & S_IXGRP) p |= perms::group_exec;
      if (st.st_mode & S_IROTH) p |= perms::others_read;
      if (st.st_mode & S_IWOTH) p |= perms::others_write;
      if (st.st_mode & S_IXOTH) p |= perms::others_exec;
      return file_status(p);
   }
   return file_status();
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
   std::vector<std::string> all_files;
   size_t current_index;
   std::string root_path;

   void collect_files_recursively(const std::string& root_path)
   {
      std::vector<std::string> dirs_to_process;
      dirs_to_process.push_back(root_path);
      
      while (!dirs_to_process.empty())
      {
         std::string current_dir = dirs_to_process.back();
         dirs_to_process.pop_back();
         
         DIR* dir = opendir(current_dir.c_str());
         if (!dir) 
         {
            continue;
         }
         
         struct dirent* entry;
         while ((entry = readdir(dir)) != nullptr)
         {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
               continue;
               
            std::string full_path = current_dir + "/" + entry->d_name;
            
            struct stat st;
            if (stat(full_path.c_str(), &st) == 0)
            {
               if (S_ISDIR(st.st_mode))
               {
                  // Add subdirectory to processing list
                  dirs_to_process.push_back(full_path);
               }
               else if (S_ISREG(st.st_mode))
               {
                  // Add regular file to results
                  all_files.push_back(full_path);
               }
            }
         }
         closedir(dir);
      }
   }

   public:
   recursive_directory_iterator() : current_index(0) {}
   
   recursive_directory_iterator(const path& p) : current_index(0), root_path(p.string())
   {
      if (fs::exists(p) && fs::is_directory(p))
      {
         collect_files_recursively(p.string());
      }
   }

   recursive_directory_iterator& operator++()
   {
      if (current_index < all_files.size())
      {
         ++current_index;
      }
      return *this;
   }

   bool operator!=(const recursive_directory_iterator& other) const
   {
      // If both iterators are at the end, they're equal
      if (current_index >= all_files.size() && other.current_index >= other.all_files.size())
         return false;
      // If one is at the end and the other isn't, they're different
      if (current_index >= all_files.size() || other.current_index >= other.all_files.size())
         return true;
      // If they have different root paths, they're different
      if (root_path != other.root_path)
         return true;
      // If they have the same root path but different indices, they're different
      return current_index != other.current_index;
   }

   bool operator==(const recursive_directory_iterator& other) const
   {
      return !(*this != other);
   }

   const path operator*() const
   {
      if (current_index < all_files.size())
      {
         return path(all_files[current_index]);
      }
      return path();
   }

   const path* operator->() const
   {
      static path current_path;
      if (current_index < all_files.size())
      {
         current_path = path(all_files[current_index]);
      }
      return &current_path;
   }

   bool is_regular_file() const
   {
      if (current_index >= all_files.size()) return false;
      struct stat st;
      return stat(all_files[current_index].c_str(), &st) == 0 && S_ISREG(st.st_mode);
   }

   path get_path() const
   {
      if (current_index < all_files.size())
      {
         return path(all_files[current_index]);
      }
      return path();
   }

   path current_path() const
   {
      return get_path();
   }

   // Additional methods needed by FileUtils
   bool empty() const
   {
      return current_index >= all_files.size();
   }

   std::string string() const
   {
      if (current_index < all_files.size())
      {
         return all_files[current_index];
      }
      return "";
   }

   path get_current_path() const
   {
      return get_path();
   }
};

// Free functions for range-based for loop support
inline recursive_directory_iterator begin(recursive_directory_iterator& iter)
{
   return iter;
}

inline recursive_directory_iterator end(recursive_directory_iterator& iter)
{
   return recursive_directory_iterator();
}

inline recursive_directory_iterator begin(const recursive_directory_iterator& iter)
{
   return iter;
}

inline recursive_directory_iterator end(const recursive_directory_iterator& iter)
{
   return recursive_directory_iterator();
}



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
   path                            current_path_obj;

   public:
   directory_iterator() : dir(nullptr) {}
   directory_iterator(const path& p) : dir(opendir(p.string().c_str())), current_path_obj(p) {}
   ~directory_iterator() = default;

   // Enable copy operations for range-based for loops
   directory_iterator(const directory_iterator& other) : dir(nullptr), current_path(other.current_path), current_path_obj(other.current_path_obj) 
   {
      if (other.dir) {
         // Reopen the directory
         dir.reset(opendir(current_path_obj.string().c_str()));
      }
   }
   
   directory_iterator& operator=(const directory_iterator& other)
   {
      if (this != &other) {
         dir.reset();
         current_path = other.current_path;
         current_path_obj = other.current_path_obj;
         if (other.dir) {
            dir.reset(opendir(current_path_obj.string().c_str()));
         }
      }
      return *this;
   }

   directory_iterator(directory_iterator&& other) = default;
   directory_iterator& operator=(directory_iterator&& other) = default;

   directory_iterator& operator++()
   {
      if (dir)
      {
         struct dirent* entry = readdir(dir.get());
         if (entry)
         {
            current_path = entry->d_name;
            current_path_obj = current_path_obj / current_path;
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

   bool operator==(const directory_iterator& other) const
   {
      return dir == other.dir;
   }

   bool is_regular_file() const
   {
      if (current_path.empty())
         return false;
      struct stat st;
      return stat(current_path_obj.string().c_str(), &st) == 0 && S_ISREG(st.st_mode);
   }

   path get_path() const
   {
      return current_path_obj;
   }

   const path& operator*() const
   {
      return current_path_obj;
   }

   const path* operator->() const
   {
      return &current_path_obj;
   }
};

// Free functions for range-based for loop support
inline directory_iterator begin(directory_iterator& iter)
{
   return iter;
}

inline directory_iterator end(directory_iterator& iter)
{
   return directory_iterator();
}

inline directory_iterator begin(const directory_iterator& iter)
{
   return iter;
}

inline directory_iterator end(const directory_iterator& iter)
{
   return directory_iterator();
}



class filesystem_error : public std::runtime_error
{
   public:
   filesystem_error(const std::string& msg) : std::runtime_error(msg) {}
};

inline void rename(const path& from, const path& to)
{
   if (::rename(from.string().c_str(), to.string().c_str()) != 0)
   {
      throw filesystem_error("Failed to rename file");
   }
}
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

// std::optional alias for C++14 (always provide when C++17 is not available)
#if __cplusplus < 201703L
namespace std {
    template<typename T>
    using optional = heimdall::compat::optional<T>;
    using nullopt_t = heimdall::compat::nullopt_t;
    constexpr nullopt_t nullopt{0};
    using bad_optional_access = heimdall::compat::bad_optional_access;
}
#endif

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

