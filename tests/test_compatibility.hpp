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

#pragma once

#include <string>
#include <cstring>
#include <cerrno>
#include <cstdint>

// For C++11/14 compatibility, provide minimal filesystem support
// This is included when ENABLE_CPP11_14 is defined (C++11/14 builds)
#ifdef ENABLE_CPP11_14
#include <cstdio>      // for std::remove
#include <unistd.h>    // for getcwd
#include <limits.h>    // for PATH_MAX
#include <linux/limits.h> // for PATH_MAX on Linux
#include <sys/stat.h>  // for mkdir, stat, chmod
#include <sys/types.h> // for stat struct
#include <vector>
#include <dirent.h>

// Fallback for PATH_MAX if not defined
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

namespace test_compat {
    class path {
    public:
        path() : str_("") {}
        path(const std::string& s) : str_(s) {}
        path(const char* s) : str_(s) {}
        
        const std::string& string() const { return str_; }
        std::string& string() { return str_; }
        
        path filename() const {
            size_t pos = str_.find_last_of('/');
            if (pos == std::string::npos) {
                return path(str_);
            }
            return path(str_.substr(pos + 1));
        }
        
        path operator/(const std::string& other) const {
            if (str_.empty()) return path(other);
            if (str_.back() == '/') return path(str_ + other);
            return path(str_ + "/" + other);
        }
        
        path operator/(const path& other) const {
            return *this / other.string();
        }
        
        path operator/(const char* other) const {
            return *this / std::string(other);
        }
        
        bool operator==(const std::string& other) const {
            return str_ == other;
        }
        
        bool operator==(const char* other) const {
            return str_ == std::string(other);
        }
        
    private:
        std::string str_;
    };
    
    namespace filesystem {
        using path = ::test_compat::path;
        
        inline bool exists(const path& p) {
            return access(p.string().c_str(), F_OK) == 0;
        }
        
        inline bool remove(const path& p) {
            return std::remove(p.string().c_str()) == 0;
        }
        
        inline path absolute(const path& p) {
            char buf[PATH_MAX];
            if (realpath(p.string().c_str(), buf)) {
                return path(buf);
            } else {
                return p;
            }
        }
        
        inline path current_path() {
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf))) {
                return path(buf);
            } else {
                return path(".");
            }
        }
        
        inline path temp_directory_path() {
            const char* tmpdir = getenv("TMPDIR");
            if (!tmpdir) tmpdir = getenv("TMP");
            if (!tmpdir) tmpdir = getenv("TEMP");
            if (!tmpdir) tmpdir = "/tmp";
            return path(tmpdir);
        }
        
        inline bool create_directories(const path& p) {
            std::string dir = p.string();
            if (dir.empty()) return true;
            
            // Remove trailing slash
            if (dir.back() == '/') dir.pop_back();
            
            // Create parent directories first
            size_t pos = 0;
            while ((pos = dir.find('/', pos + 1)) != std::string::npos) {
                std::string parent = dir.substr(0, pos);
                if (mkdir(parent.c_str(), 0755) != 0 && errno != EEXIST) {
                    return false;
                }
            }
            
            // Create the final directory
            return mkdir(dir.c_str(), 0755) == 0 || errno == EEXIST;
        }
        
        inline bool remove_all(const path& p) {
            // Simple implementation - just remove the file/directory
            return remove(p);
        }
        
        inline uint64_t file_size(const path& p) {
            struct stat st;
            if (stat(p.string().c_str(), &st) == 0) {
                return static_cast<uint64_t>(st.st_size);
            }
            return 0;
        }
        
        inline bool copy_file(const path& from, const path& to, int options = 0) {
            FILE* src = fopen(from.string().c_str(), "rb");
            if (!src) return false;
            
            FILE* dst = fopen(to.string().c_str(), "wb");
            if (!dst) {
                fclose(src);
                return false;
            }
            
            char buffer[4096];
            size_t bytes;
            bool success = true;
            
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                if (fwrite(buffer, 1, bytes, dst) != bytes) {
                    success = false;
                    break;
                }
            }
            
            fclose(src);
            fclose(dst);
            return success;
        }
        
        namespace copy_options {
            const int overwrite_existing = 1;
        }
        
        namespace perms {
            const int none = 0;
            const int owner_read = 0400;
            const int owner_write = 0200;
        }
        
        inline void permissions(const path& p, int perms) {
            chmod(p.string().c_str(), perms);
        }
        
        // Simple recursive directory iterator stub
        class recursive_directory_iterator {
        public:
            recursive_directory_iterator() : dir_(nullptr) {}
            recursive_directory_iterator(const path& p) : dir_(nullptr) {
                dir_ = opendir(p.string().c_str());
            }
            
            ~recursive_directory_iterator() {
                if (dir_) closedir(dir_);
            }
            
            bool operator!=(const recursive_directory_iterator& other) const {
                return dir_ != other.dir_;
            }
            
            recursive_directory_iterator& operator++() {
                // Simple implementation - just close the directory
                if (dir_) {
                    closedir(dir_);
                    dir_ = nullptr;
                }
                return *this;
            }
            
            struct entry {
                ::test_compat::path path() const { return path_value_; }
                bool is_regular_file() const { return is_file_; }
                ::test_compat::path filename() const { return filename_value_; }
                
                ::test_compat::path path_value_;
                bool is_file_;
                ::test_compat::path filename_value_;
            };
            
            entry operator*() const {
                return entry{::test_compat::path(""), false, ::test_compat::path("")};
            }
            
            // Iterator support
            recursive_directory_iterator begin() const { return *this; }
            recursive_directory_iterator end() const { return recursive_directory_iterator(); }
            
        private:
            DIR* dir_;
        };
    }
}
#endif 