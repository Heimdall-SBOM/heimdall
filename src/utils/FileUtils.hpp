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
 * @file FileUtils.hpp
 * @brief Utility functions for file operations
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file defines utility functions for common file operations such as
 * path manipulation, file existence checks, and file type detection.
 */

#pragma once

#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief Utility class for file operations
 *
 * This class provides common file operations and utilities:
 * - Path manipulation and normalization
 * - File existence and type checks
 * - Directory operations
 * - File size and modification time queries
 * - File extension and name extraction
 */
class FileUtils
{
   public:
   /**
    * @brief Check if a file exists
    *
    * @param filePath Path to the file
    * @return true if file exists and is accessible
    * @return false otherwise
    */
   static bool fileExists(const std::string& filePath);

   /**
    * @brief Check if a path is a directory
    *
    * @param path Path to check
    * @return true if path is a directory
    * @return false otherwise
    */
   static bool isDirectory(const std::string& path);

   /**
    * @brief Check if a path is a regular file
    *
    * @param path Path to check
    * @return true if path is a regular file
    * @return false otherwise
    */
   static bool isRegularFile(const std::string& path);

   /**
    * @brief Check if a file is executable
    *
    * @param filePath Path to the file
    * @return true if file is executable
    * @return false otherwise
    */
   static bool isExecutable(const std::string& filePath);

   /**
    * @brief Get file size in bytes
    *
    * @param filePath Path to the file
    * @return File size in bytes, or -1 if error
    */
   static int64_t getFileSize(const std::string& filePath);

   /**
    * @brief Get file modification time
    *
    * @param filePath Path to the file
    * @return Modification time as Unix timestamp, or -1 if error
    */
   static int64_t getModificationTime(const std::string& filePath);

   /**
    * @brief Get file extension (including dot)
    *
    * @param filePath Path to the file
    * @return File extension (e.g., ".txt"), or empty string if no extension
    */
   static std::string getFileExtension(const std::string& filePath);

   /**
    * @brief Get file name without extension
    *
    * @param filePath Path to the file
    * @return File name without extension
    */
   static std::string getFileNameWithoutExtension(const std::string& filePath);

   /**
    * @brief Get file name with extension
    *
    * @param filePath Path to the file
    * @return File name with extension
    */
   static std::string getFileName(const std::string& filePath);

   /**
    * @brief Get directory path
    *
    * @param filePath Path to the file
    * @return Directory path
    */
   static std::string getDirectoryPath(const std::string& filePath);

   /**
    * @brief Get absolute path
    *
    * @param filePath Path to the file
    * @return Absolute path
    */
   static std::string getAbsolutePath(const std::string& filePath);

   /**
    * @brief Normalize path (resolve . and ..)
    *
    * @param filePath Path to normalize
    * @return Normalized path
    */
   static std::string normalizePath(const std::string& filePath);

   /**
    * @brief Join path components
    *
    * @param components Vector of path components
    * @return Joined path
    */
   static std::string joinPath(const std::vector<std::string>& components);

   /**
    * @brief Join two path components
    *
    * @param path1 First path component
    * @param path2 Second path component
    * @return Joined path
    */
   static std::string joinPath(const std::string& path1, const std::string& path2);

   /**
    * @brief Split path into components
    *
    * @param filePath Path to split
    * @return Vector of path components
    */
   static std::vector<std::string> splitPath(const std::string& filePath);

   /**
    * @brief Create directory (including parent directories)
    *
    * @param dirPath Path to the directory
    * @return true if directory was created successfully
    * @return false otherwise
    */
   static bool createDirectory(const std::string& dirPath);

   /**
    * @brief Create parent directories for a file
    *
    * @param filePath Path to the file
    * @return true if directories were created successfully
    * @return false otherwise
    */
   static bool createParentDirectories(const std::string& filePath);

   /**
    * @brief Remove file
    *
    * @param filePath Path to the file
    * @return true if file was removed successfully
    * @return false otherwise
    */
   static bool removeFile(const std::string& filePath);

   /**
    * @brief Remove directory (recursive)
    *
    * @param dirPath Path to the directory
    * @return true if directory was removed successfully
    * @return false otherwise
    */
   static bool removeDirectory(const std::string& dirPath);

   /**
    * @brief Copy file
    *
    * @param sourcePath Source file path
    * @param destPath Destination file path
    * @return true if file was copied successfully
    * @return false otherwise
    */
   static bool copyFile(const std::string& sourcePath, const std::string& destPath);

   /**
    * @brief Move file
    *
    * @param sourcePath Source file path
    * @param destPath Destination file path
    * @return true if file was moved successfully
    * @return false otherwise
    */
   static bool moveFile(const std::string& sourcePath, const std::string& destPath);

   /**
    * @brief Get list of files in directory
    *
    * @param dirPath Path to the directory
    * @param recursive Whether to search recursively
    * @return Vector of file paths
    */
   static std::vector<std::string> getFilesInDirectory(const std::string& dirPath,
                                                       bool               recursive = false);

   /**
    * @brief Get list of directories in directory
    *
    * @param dirPath Path to the directory
    * @param recursive Whether to search recursively
    * @return Vector of directory paths
    */
   static std::vector<std::string> getDirectoriesInDirectory(const std::string& dirPath,
                                                             bool               recursive = false);

   /**
    * @brief Check if file has specific extension
    *
    * @param filePath Path to the file
    * @param extension Extension to check (with or without dot)
    * @return true if file has the specified extension
    * @return false otherwise
    */
   static bool hasExtension(const std::string& filePath, const std::string& extension);

   /**
    * @brief Check if file has any of the specified extensions
    *
    * @param filePath Path to the file
    * @param extensions Vector of extensions to check
    * @return true if file has any of the specified extensions
    * @return false otherwise
    */
   static bool hasAnyExtension(const std::string&              filePath,
                               const std::vector<std::string>& extensions);

   /**
    * @brief Get current working directory
    *
    * @return Current working directory path
    */
   static std::string getCurrentWorkingDirectory();

   /**
    * @brief Change current working directory
    *
    * @param dirPath Path to the new working directory
    * @return true if directory was changed successfully
    * @return false otherwise
    */
   static bool changeWorkingDirectory(const std::string& dirPath);

   private:
   /**
    * @brief Private constructor to prevent instantiation
    */
   FileUtils() = delete;

   /**
    * @brief Private destructor
    */
   ~FileUtils() = delete;

   /**
    * @brief Private copy constructor
    */
   FileUtils(const FileUtils&) = delete;

   /**
    * @brief Private assignment operator
    */
   FileUtils& operator=(const FileUtils&) = delete;
};

}  // namespace heimdall