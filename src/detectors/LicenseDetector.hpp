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
 * @file LicenseDetector.hpp
 * @brief License detection and metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This detector identifies licenses used in projects and extracts
 * their metadata, including license text, SPDX identifiers, and compliance information.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief License information structure
 */
struct LicenseInfo
{
   std::string                        name;    ///< License name (e.g., "MIT License", "Apache 2.0")
   std::string                        spdxId;  ///< SPDX identifier (e.g., "MIT", "Apache-2.0")
   std::string                        text;    ///< Full license text
   std::string                        filePath;              ///< Path to license file
   std::string                        copyright;             ///< Copyright notice
   std::string                        author;                ///< Author information
   std::string                        year;                  ///< Copyright year
   std::vector<std::string>           keywords;              ///< License keywords for matching
   bool                               isOpenSource = true;   ///< Whether license is open source
   bool                               isCopyleft   = false;  ///< Whether license is copyleft
   bool                               isPermissive = false;  ///< Whether license is permissive
   double                             confidence   = 0.0;    ///< Detection confidence (0.0-1.0)
   std::map<std::string, std::string> metadata;              ///< Additional metadata
};

/**
 * @brief License detector interface
 *
 * This class provides detection and metadata extraction for various
 * software licenses used in projects.
 */
class LicenseDetector
{
   public:
   /**
    * @brief Default constructor
    */
   LicenseDetector();

   /**
    * @brief Destructor
    */
   ~LicenseDetector();

   /**
    * @brief Copy constructor
    * @param other The LicenseDetector to copy from
    */
   LicenseDetector(const LicenseDetector& other);

   /**
    * @brief Move constructor
    * @param other The LicenseDetector to move from
    */
   LicenseDetector(LicenseDetector&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The LicenseDetector to copy from
    * @return Reference to this LicenseDetector
    */
   LicenseDetector& operator=(const LicenseDetector& other);

   /**
    * @brief Move assignment operator
    * @param other The LicenseDetector to move from
    * @return Reference to this LicenseDetector
    */
   LicenseDetector& operator=(LicenseDetector&& other) noexcept;

   /**
    * @brief Detect licenses in a directory
    * @param directoryPath Path to the directory to scan
    * @param licenses Vector to populate with detected licenses
    * @return true if detection was successful, false otherwise
    */
   bool detectLicenses(const std::string& directoryPath, std::vector<LicenseInfo>& licenses);

   /**
    * @brief Detect license from a specific file
    * @param filePath Path to the file to analyze
    * @param license LicenseInfo to populate with detected data
    * @return true if detection was successful, false otherwise
    */
   bool detectLicenseFromFile(const std::string& filePath, LicenseInfo& license);

   /**
    * @brief Detect license from text content
    * @param text Text content to analyze
    * @param license LicenseInfo to populate with detected data
    * @return true if detection was successful, false otherwise
    */
   bool detectLicenseFromText(const std::string& text, LicenseInfo& license);

   /**
    * @brief Get list of supported license types
    * @return Vector of supported license names
    */
   std::vector<std::string> getSupportedLicenses() const;

   /**
    * @brief Check if a directory contains license files
    * @param directoryPath Path to the directory to check
    * @return true if license files are present, false otherwise
    */
   bool hasLicenseFiles(const std::string& directoryPath) const;

   /**
    * @brief Get license priority (higher values indicate higher priority)
    * @param licenseName Name of the license
    * @return Priority value
    */
   int getLicensePriority(const std::string& licenseName) const;

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
    * @brief Detect MIT license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if MIT license was detected, false otherwise
    */
   bool detectMITLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect Apache license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if Apache license was detected, false otherwise
    */
   bool detectApacheLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect GPL license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if GPL license was detected, false otherwise
    */
   bool detectGPLLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect BSD license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if BSD license was detected, false otherwise
    */
   bool detectBSDLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect ISC license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if ISC license was detected, false otherwise
    */
   bool detectISCLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect Unlicense
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if Unlicense was detected, false otherwise
    */
   bool detectUnlicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Detect Creative Commons license
    * @param text Text content to analyze
    * @param license LicenseInfo to populate
    * @return true if Creative Commons license was detected, false otherwise
    */
   bool detectCreativeCommonsLicense(const std::string& text, LicenseInfo& license);

   /**
    * @brief Extract copyright information from text
    * @param text Text content to analyze
    * @param copyright Copyright string to populate
    * @return true if copyright was found, false otherwise
    */
   bool extractCopyright(const std::string& text, std::string& copyright);

   /**
    * @brief Extract author information from text
    * @param text Text content to analyze
    * @param author Author string to populate
    * @return true if author was found, false otherwise
    */
   bool extractAuthor(const std::string& text, std::string& author);

   /**
    * @brief Calculate confidence score for license detection
    * @param text Text content
    * @param licenseType Type of license being detected
    * @return Confidence score (0.0-1.0)
    */
   double calculateConfidence(const std::string& text, const std::string& licenseType);

   /**
    * @brief Normalize text for comparison
    * @param text Text to normalize
    * @return Normalized text
    */
   std::string normalizeText(const std::string& text);

   /**
    * @brief Find license files in directory
    * @param directoryPath Path to directory
    * @param licenseFiles Vector to populate with found license file paths
    * @return true if license files were found, false otherwise
    */
   bool findLicenseFiles(const std::string& directoryPath, std::vector<std::string>& licenseFiles);
};

}  // namespace heimdall