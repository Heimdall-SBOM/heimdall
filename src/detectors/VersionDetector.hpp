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
 * @file VersionDetector.hpp
 * @brief Version detection and metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This detector identifies version information in projects and extracts
 * their metadata, including semantic versions, build numbers, and release information.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief Version information structure
 */
struct VersionInfo
{
   std::string major;                            ///< Major version number
   std::string minor;                            ///< Minor version number
   std::string patch;                            ///< Patch version number
   std::string prerelease;                       ///< Prerelease identifier (e.g., "alpha", "beta")
   std::string build;                            ///< Build metadata
   std::string fullVersion;                      ///< Full version string
   std::string source;                           ///< Source of version information
   std::string filePath;                         ///< Path to file containing version
   std::string commitHash;                       ///< Git commit hash
   std::string branch;                           ///< Git branch name
   std::string tag;                              ///< Git tag
   std::string buildDate;                        ///< Build date
   std::string buildTime;                        ///< Build time
   std::string compiler;                         ///< Compiler information
   std::string platform;                         ///< Target platform
   std::string architecture;                     ///< Target architecture
   bool        isRelease     = false;            ///< Whether this is a release version
   bool        isPrerelease  = false;            ///< Whether this is a prerelease
   bool        isDevelopment = false;            ///< Whether this is a development version
   double      confidence    = 0.0;              ///< Detection confidence (0.0-1.0)
   std::map<std::string, std::string> metadata;  ///< Additional metadata
};

/**
 * @brief Version detector interface
 *
 * This class provides detection and metadata extraction for version
 * information in software projects.
 */
class VersionDetector
{
   public:
   /**
    * @brief Default constructor
    */
   VersionDetector();

   /**
    * @brief Destructor
    */
   ~VersionDetector();

   /**
    * @brief Copy constructor
    * @param other The VersionDetector to copy from
    */
   VersionDetector(const VersionDetector& other);

   /**
    * @brief Move constructor
    * @param other The VersionDetector to move from
    */
   VersionDetector(VersionDetector&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The VersionDetector to copy from
    * @return Reference to this VersionDetector
    */
   VersionDetector& operator=(const VersionDetector& other);

   /**
    * @brief Move assignment operator
    * @param other The VersionDetector to move from
    * @return Reference to this VersionDetector
    */
   VersionDetector& operator=(VersionDetector&& other) noexcept;

   /**
    * @brief Detect versions in a directory
    * @param directoryPath Path to the directory to scan
    * @param versions Vector to populate with detected versions
    * @return true if detection was successful, false otherwise
    */
   bool detectVersions(const std::string& directoryPath, std::vector<VersionInfo>& versions);

   /**
    * @brief Detect version from a specific file
    * @param filePath Path to the file to analyze
    * @param version VersionInfo to populate with detected data
    * @return true if detection was successful, false otherwise
    */
   bool detectVersionFromFile(const std::string& filePath, VersionInfo& version);

   /**
    * @brief Detect version from text content
    * @param text Text content to analyze
    * @param version VersionInfo to populate with detected data
    * @return true if detection was successful, false otherwise
    */
   bool detectVersionFromText(const std::string& text, VersionInfo& version);

   /**
    * @brief Parse semantic version string
    * @param versionString Version string to parse
    * @param version VersionInfo to populate with parsed data
    * @return true if parsing was successful, false otherwise
    */
   bool parseSemanticVersion(const std::string& versionString, VersionInfo& version);

   /**
    * @brief Get list of supported version formats
    * @return Vector of supported version format names
    */
   std::vector<std::string> getSupportedVersionFormats() const;

   /**
    * @brief Check if a directory contains version information
    * @param directoryPath Path to the directory to check
    * @return true if version information is present, false otherwise
    */
   bool hasVersionInformation(const std::string& directoryPath) const;

   /**
    * @brief Get version priority (higher values indicate higher priority)
    * @param versionFormat Name of the version format
    * @return Priority value
    */
   int getVersionPriority(const std::string& versionFormat) const;

   /**
    * @brief Set verbose output mode
    * @param verbose Whether to enable verbose output
    */
   void setVerbose(bool verbose);

   /**
    * @brief Get last error message
    * @return Last error message
    */
   std::string getLastError() const;

   /**
    * @brief Set minimum confidence threshold
    * @param threshold Minimum confidence value (0.0-1.0)
    */
   void setConfidenceThreshold(double threshold);

   /**
    * @brief Get minimum confidence threshold
    * @return Minimum confidence threshold
    */
   double getConfidenceThreshold() const;

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Detect version from CMakeLists.txt
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromCMake(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from package.json
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromPackageJson(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from Cargo.toml
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromCargoToml(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from pom.xml
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromPomXml(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from build.gradle
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromBuildGradle(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from composer.json
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromComposerJson(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from go.mod
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromGoMod(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from .csproj
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromCsproj(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from header files
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromHeader(const std::string& text, VersionInfo& version);

   /**
    * @brief Detect version from git information
    * @param directoryPath Path to the directory
    * @param version VersionInfo to populate
    * @return true if version was detected, false otherwise
    */
   bool detectVersionFromGit(const std::string& directoryPath, VersionInfo& version);

   /**
    * @brief Extract build information
    * @param text Text content to analyze
    * @param version VersionInfo to populate
    * @return true if build info was extracted, false otherwise
    */
   bool extractBuildInfo(const std::string& text, VersionInfo& version);

   /**
    * @brief Calculate confidence score for version detection
    * @param text Text content
    * @param versionFormat Format of version being detected
    * @return Confidence score (0.0-1.0)
    */
   double calculateConfidence(const std::string& text, const std::string& versionFormat);

   /**
    * @brief Normalize version string
    * @param versionString Version string to normalize
    * @return Normalized version string
    */
   std::string normalizeVersionString(const std::string& versionString);

   /**
    * @brief Find version files in directory
    * @param directoryPath Path to directory
    * @param versionFiles Vector to populate with found version file paths
    * @return true if version files were found, false otherwise
    */
   bool findVersionFiles(const std::string& directoryPath, std::vector<std::string>& versionFiles);
};

}  // namespace heimdall