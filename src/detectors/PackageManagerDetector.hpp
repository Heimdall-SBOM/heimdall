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
 * @file PackageManagerDetector.hpp
 * @brief Package manager detection and metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This detector identifies package managers used in projects and extracts
 * their metadata, including package lists, versions, and dependency information.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief Package manager information structure
 */
struct PackageManagerInfo
{
   std::string                        name;  ///< Package manager name (e.g., "npm", "pip", "cargo")
   std::string                        version;       ///< Package manager version
   std::string                        lockFile;      ///< Lock file path (e.g., "package-lock.json")
   std::string                        manifestFile;  ///< Manifest file path (e.g., "package.json")
   std::vector<std::string>           packages;      ///< List of package names
   std::map<std::string, std::string> packageVersions;      ///< Package name to version mapping
   std::vector<std::string>           dependencies;         ///< Direct dependencies
   std::vector<std::string>           devDependencies;      ///< Development dependencies
   std::string                        installCommand;       ///< Installation command
   std::string                        updateCommand;        ///< Update command
   bool                               hasLockFile = false;  ///< Whether lock file exists
   bool                               isLocked    = false;  ///< Whether dependencies are locked
};

/**
 * @brief Package manager detector interface
 *
 * This class provides detection and metadata extraction for various
 * package managers used in software projects.
 */
class PackageManagerDetector
{
   public:
   /**
    * @brief Default constructor
    */
   PackageManagerDetector();

   /**
    * @brief Destructor
    */
   ~PackageManagerDetector();

   /**
    * @brief Copy constructor
    * @param other The PackageManagerDetector to copy from
    */
   PackageManagerDetector(const PackageManagerDetector& other);

   /**
    * @brief Move constructor
    * @param other The PackageManagerDetector to move from
    */
   PackageManagerDetector(PackageManagerDetector&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The PackageManagerDetector to copy from
    * @return Reference to this PackageManagerDetector
    */
   PackageManagerDetector& operator=(const PackageManagerDetector& other);

   /**
    * @brief Move assignment operator
    * @param other The PackageManagerDetector to move from
    * @return Reference to this PackageManagerDetector
    */
   PackageManagerDetector& operator=(PackageManagerDetector&& other) noexcept;

   /**
    * @brief Detect package managers in a directory
    * @param directoryPath Path to the directory to scan
    * @param packageManagers Vector to populate with detected package managers
    * @return true if detection was successful, false otherwise
    */
   bool detectPackageManagers(const std::string&               directoryPath,
                              std::vector<PackageManagerInfo>& packageManagers);

   /**
    * @brief Extract metadata from a specific package manager
    * @param directoryPath Path to the directory containing the package manager files
    * @param packageManagerName Name of the package manager to extract from
    * @param info PackageManagerInfo to populate with extracted data
    * @return true if extraction was successful, false otherwise
    */
   bool extractPackageManagerMetadata(const std::string&  directoryPath,
                                      const std::string&  packageManagerName,
                                      PackageManagerInfo& info);

   /**
    * @brief Get list of supported package managers
    * @return Vector of supported package manager names
    */
   std::vector<std::string> getSupportedPackageManagers() const;

   /**
    * @brief Check if a directory contains a specific package manager
    * @param directoryPath Path to the directory to check
    * @param packageManagerName Name of the package manager to check for
    * @return true if the package manager is present, false otherwise
    */
   bool hasPackageManager(const std::string& directoryPath,
                          const std::string& packageManagerName) const;

   /**
    * @brief Get package manager priority (higher values indicate higher priority)
    * @param packageManagerName Name of the package manager
    * @return Priority value
    */
   int getPackageManagerPriority(const std::string& packageManagerName) const;

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

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Helper methods
   /**
    * @brief Detect npm/Node.js package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if npm was detected and info extracted, false otherwise
    */
   bool detectNpm(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect pip/Python package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if pip was detected and info extracted, false otherwise
    */
   bool detectPip(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect cargo/Rust package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if cargo was detected and info extracted, false otherwise
    */
   bool detectCargo(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect maven/Java package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if maven was detected and info extracted, false otherwise
    */
   bool detectMaven(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect gradle/Java package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if gradle was detected and info extracted, false otherwise
    */
   bool detectGradle(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect composer/PHP package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if composer was detected and info extracted, false otherwise
    */
   bool detectComposer(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect go modules
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if go modules were detected and info extracted, false otherwise
    */
   bool detectGoModules(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Detect nuget/.NET package manager
    * @param directoryPath Path to the directory
    * @param info PackageManagerInfo to populate
    * @return true if nuget was detected and info extracted, false otherwise
    */
   bool detectNuget(const std::string& directoryPath, PackageManagerInfo& info);

   /**
    * @brief Parse package.json file
    * @param filePath Path to package.json
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parsePackageJson(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse requirements.txt file
    * @param filePath Path to requirements.txt
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseRequirementsTxt(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse Cargo.toml file
    * @param filePath Path to Cargo.toml
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseCargoToml(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse pom.xml file
    * @param filePath Path to pom.xml
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parsePomXml(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse build.gradle file
    * @param filePath Path to build.gradle
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseBuildGradle(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse composer.json file
    * @param filePath Path to composer.json
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseComposerJson(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse go.mod file
    * @param filePath Path to go.mod
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseGoMod(const std::string& filePath, PackageManagerInfo& info);

   /**
    * @brief Parse .csproj file
    * @param filePath Path to .csproj
    * @param info PackageManagerInfo to populate
    * @return true if parsing was successful, false otherwise
    */
   bool parseCsproj(const std::string& filePath, PackageManagerInfo& info);
};

}  // namespace heimdall