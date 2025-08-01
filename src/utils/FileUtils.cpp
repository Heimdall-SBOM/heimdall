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
 * @file FileUtils.cpp
 * @brief Implementation of FileUtils utility class
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file implements the FileUtils class that provides common file operations
 * with platform-specific implementations.
 */

#include "FileUtils.hpp"

#include <algorithm>
#include <cstring>
#include <chrono>
#include <fstream>
#include <sstream>
#include <system_error>
#include "../compat/compatibility.hpp"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace heimdall
{

// namespace fs = heimdall::compat::fs;  // Removed to avoid conflicts

bool FileUtils::fileExists(const std::string& filePath)
{
   return heimdall::compat::fs::exists(filePath);
}

bool FileUtils::isDirectory(const std::string& path)
{
   return heimdall::compat::fs::is_directory(path);
}

bool FileUtils::isRegularFile(const std::string& path)
{
   return heimdall::compat::fs::is_regular_file(path);
}

bool FileUtils::isExecutable(const std::string& filePath)
{
   if (!fileExists(filePath))
   {
      return false;
   }

#ifdef _WIN32
   // On Windows, check if file has .exe, .bat, .cmd, .com extension
   std::string ext = getFileExtension(filePath);
   std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
   return (ext == ".exe" || ext == ".bat" || ext == ".cmd" || ext == ".com");
#else
   // On Unix-like systems, check execute permission
   return (heimdall::compat::fs::status(filePath).permissions() & heimdall::compat::fs::perms::owner_exec) != heimdall::compat::fs::perms::none;
#endif
}

int64_t FileUtils::getFileSize(const std::string& filePath)
{
   try
   {
      if (!fileExists(filePath))
      {
         return -1;
      }
      return static_cast<int64_t>(heimdall::compat::fs::file_size(filePath));
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return -1;
   }
}

int64_t FileUtils::getModificationTime(const std::string& filePath)
{
   try
   {
      if (!fileExists(filePath))
      {
         return -1;
      }
      auto time = heimdall::compat::fs::last_write_time(filePath);
      #if __cplusplus >= 201703L
       return std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
#else
       return time.duration_cast<int64_t, std::ratio<1>>().count();
#endif
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return -1;
   }
}

std::string FileUtils::getFileExtension(const std::string& filePath)
{
   heimdall::compat::fs::path    path(filePath);
   std::string ext = path.extension().string();
   return ext;
}

std::string FileUtils::getFileNameWithoutExtension(const std::string& filePath)
{
   heimdall::compat::fs::path path(filePath);
   return path.stem().string();
}

std::string FileUtils::getFileName(const std::string& filePath)
{
   heimdall::compat::fs::path path(filePath);
   return path.filename().string();
}

std::string FileUtils::getDirectoryPath(const std::string& filePath)
{
   heimdall::compat::fs::path path(filePath);
   return path.parent_path().string();
}

std::string FileUtils::getAbsolutePath(const std::string& filePath)
{
   try
   {
      heimdall::compat::fs::path path(filePath);
      return heimdall::compat::fs::absolute(path).string();
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return filePath;
   }
}

std::string FileUtils::normalizePath(const std::string& filePath)
{
   try
   {
      heimdall::compat::fs::path path(filePath);
      return heimdall::compat::fs::canonical(path).string();
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      // If canonical fails, try to normalize without resolving symlinks
      heimdall::compat::fs::path path(filePath);
      return path.lexically_normal().string();
   }
}

std::string FileUtils::joinPath(const std::vector<std::string>& components)
{
   if (components.empty())
   {
      return "";
   }

   heimdall::compat::fs::path result(components[0]);
   for (size_t i = 1; i < components.size(); ++i)
   {
      result /= components[i];
   }
   return result.string();
}

std::string FileUtils::joinPath(const std::string& path1, const std::string& path2)
{
   heimdall::compat::fs::path result(path1);
   result /= path2;
   return result.string();
}

std::vector<std::string> FileUtils::splitPath(const std::string& filePath)
{
   heimdall::compat::fs::path                 path_obj(filePath);
   std::vector<std::string> components;

   // Split the path string manually for C++14 compatibility
   std::string path_str = path_obj.string();
   std::string component;
   std::istringstream iss(path_str);
   
   while (std::getline(iss, component, '/'))
   {
      if (!component.empty())
      {
         components.push_back(component);
      }
   }

   return components;
}

bool FileUtils::createDirectory(const std::string& dirPath)
{
   try
   {
      return heimdall::compat::fs::create_directories(dirPath);
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

bool FileUtils::createParentDirectories(const std::string& filePath)
{
   heimdall::compat::fs::path path(filePath);
   return createDirectory(path.parent_path().string());
}

bool FileUtils::removeFile(const std::string& filePath)
{
   try
   {
      return heimdall::compat::fs::remove(filePath);
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

bool FileUtils::removeDirectory(const std::string& dirPath)
{
   try
   {
      return heimdall::compat::fs::remove_all(dirPath) > 0;
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

bool FileUtils::copyFile(const std::string& sourcePath, const std::string& destPath)
{
   try
   {
      heimdall::compat::fs::copy_file(sourcePath, destPath, heimdall::compat::fs::copy_options::overwrite_existing);
      return true;
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

bool FileUtils::moveFile(const std::string& sourcePath, const std::string& destPath)
{
   try
   {
      heimdall::compat::fs::rename(sourcePath, destPath);
      return true;
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

std::vector<std::string> FileUtils::getFilesInDirectory(const std::string& dirPath, bool recursive)
{
   std::vector<std::string> files;

   try
   {
      if (recursive)
      {
         heimdall::compat::fs::recursive_directory_iterator iter(dirPath);
         while (iter != heimdall::compat::fs::recursive_directory_iterator())
         {
#if __cplusplus >= 201703L
            if (heimdall::compat::fs::is_regular_file(iter->path()))
            {
               files.push_back(iter->path().string());
            }
#else
            if (heimdall::compat::fs::is_regular_file(iter.get_path()))
            {
               files.push_back(iter.get_path().string());
            }
#endif
            ++iter;
         }
      }
      else
      {
         heimdall::compat::fs::directory_iterator iter(dirPath);
         while (iter != heimdall::compat::fs::directory_iterator())
         {
#if __cplusplus >= 201703L
            if (heimdall::compat::fs::is_regular_file(iter->path()))
            {
               files.push_back(iter->path().string());
            }
#else
            if (heimdall::compat::fs::is_regular_file(iter.get_path()))
            {
               files.push_back(iter.get_path().string());
            }
#endif
            ++iter;
         }
      }
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      // Return empty vector on error
   }

   return files;
}

std::vector<std::string> FileUtils::getDirectoriesInDirectory(const std::string& dirPath,
                                                              bool               recursive)
{
   std::vector<std::string> directories;

   try
   {
      if (recursive)
      {
         heimdall::compat::fs::recursive_directory_iterator iter(dirPath);
         while (iter != heimdall::compat::fs::recursive_directory_iterator())
         {
#if __cplusplus >= 201703L
            if (heimdall::compat::fs::is_directory(iter->path()))
            {
               directories.push_back(iter->path().string());
            }
#else
            if (heimdall::compat::fs::is_directory(iter.get_path()))
            {
               directories.push_back(iter.get_path().string());
            }
#endif
            ++iter;
         }
      }
      else
      {
         heimdall::compat::fs::directory_iterator iter(dirPath);
         while (iter != heimdall::compat::fs::directory_iterator())
         {
#if __cplusplus >= 201703L
            if (heimdall::compat::fs::is_directory(iter->path()))
            {
               directories.push_back(iter->path().string());
            }
#else
            if (heimdall::compat::fs::is_directory(iter.get_path()))
            {
               directories.push_back(iter.get_path().string());
            }
#endif
            ++iter;
         }
      }
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      // Return empty vector on error
   }

   return directories;
}

bool FileUtils::hasExtension(const std::string& filePath, const std::string& extension)
{
   std::string fileExt  = getFileExtension(filePath);
   std::string checkExt = extension;

   // Ensure both extensions start with dot
   if (!fileExt.empty() && fileExt[0] != '.')
   {
      fileExt = "." + fileExt;
   }
   if (!checkExt.empty() && checkExt[0] != '.')
   {
      checkExt = "." + checkExt;
   }

   return fileExt == checkExt;
}

bool FileUtils::hasAnyExtension(const std::string&              filePath,
                                const std::vector<std::string>& extensions)
{
   return std::any_of(extensions.begin(), extensions.end(),
                      [&filePath](const std::string& ext) { return hasExtension(filePath, ext); });
}

std::string FileUtils::getCurrentWorkingDirectory()
{
   try
   {
      return heimdall::compat::fs::current_path().string();
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return "";
   }
}

bool FileUtils::changeWorkingDirectory(const std::string& dirPath)
{
   try
   {
      heimdall::compat::fs::current_path(dirPath);
      return true;
   }
   catch (const heimdall::compat::fs::filesystem_error&)
   {
      return false;
   }
}

}  // namespace heimdall
