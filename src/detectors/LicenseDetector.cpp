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
 * @file LicenseDetector.cpp
 * @brief License detection and metadata extraction implementation
 * @author Trevor Bakker
 * @date 2025
 */

#include "LicenseDetector.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include "../utils/FileUtils.hpp"

namespace heimdall
{

class LicenseDetector::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   bool        verbose = false;
   std::string lastError;
   double      confidenceThreshold = 0.7;

   // License detection methods
   bool detectMITLicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectApacheLicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectGPLLicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectBSDLicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectISCLicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectUnlicenseImpl(const std::string& text, LicenseInfo& license);
   bool detectCreativeCommonsLicenseImpl(const std::string& text, LicenseInfo& license);

   // Helper methods
   bool        extractCopyrightImpl(const std::string& text, std::string& copyright);
   bool        extractAuthorImpl(const std::string& text, std::string& author);
   double      calculateConfidenceImpl(const std::string& text, const std::string& licenseType);
   std::string normalizeTextImpl(const std::string& text);
   bool        findLicenseFilesImpl(const std::string&        directoryPath,
                                    std::vector<std::string>& licenseFiles);
   std::vector<std::string> getSupportedLicensesImpl() const;
   int                      getLicensePriorityImpl(const std::string& licenseName) const;
   void                     setLastError(const std::string& error);
};

// LicenseDetector implementation
LicenseDetector::LicenseDetector() : pImpl(std::make_unique<Impl>()) {}

LicenseDetector::~LicenseDetector() = default;

LicenseDetector::LicenseDetector(const LicenseDetector& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

LicenseDetector::LicenseDetector(LicenseDetector&& other) noexcept : pImpl(std::move(other.pImpl))
{
}

LicenseDetector& LicenseDetector::operator=(const LicenseDetector& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

LicenseDetector& LicenseDetector::operator=(LicenseDetector&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

bool LicenseDetector::detectLicenses(const std::string&        directoryPath,
                                     std::vector<LicenseInfo>& licenses)
{
   licenses.clear();

   if (!hasLicenseFiles(directoryPath))
   {
      pImpl->setLastError("No license files found in directory");
      return false;
   }

   std::vector<std::string> licenseFiles;
   if (!pImpl->findLicenseFilesImpl(directoryPath, licenseFiles))
   {
      pImpl->setLastError("Failed to find license files");
      return false;
   }

   for (const auto& filePath : licenseFiles)
   {
      LicenseInfo license;
      if (detectLicenseFromFile(filePath, license))
      {
         if (license.confidence >= pImpl->confidenceThreshold)
         {
            licenses.push_back(license);
         }
      }
   }

   // Sort by confidence (higher confidence first)
   std::sort(licenses.begin(), licenses.end(), [](const LicenseInfo& a, const LicenseInfo& b)
             { return a.confidence > b.confidence; });

   return !licenses.empty();
}

bool LicenseDetector::detectLicenseFromFile(const std::string& filePath, LicenseInfo& license)
{
   if (!heimdall::FileUtils::fileExists(filePath))
   {
      pImpl->setLastError("License file does not exist: " + filePath);
      return false;
   }

   std::ifstream file(filePath);
   if (!file.is_open())
   {
      pImpl->setLastError("Failed to open license file: " + filePath);
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string text = buffer.str();

   license.filePath = filePath;
   return detectLicenseFromText(text, license);
}

bool LicenseDetector::detectLicenseFromText(const std::string& text, LicenseInfo& license)
{
   std::string normalizedText = pImpl->normalizeTextImpl(text);

   // Try to detect each supported license type
   if (pImpl->detectMITLicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectApacheLicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectGPLLicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectBSDLicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectISCLicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectUnlicenseImpl(normalizedText, license))
   {
      return true;
   }
   else if (pImpl->detectCreativeCommonsLicenseImpl(normalizedText, license))
   {
      return true;
   }

   // Extract copyright and author information even if license type is unknown
   pImpl->extractCopyrightImpl(text, license.copyright);
   pImpl->extractAuthorImpl(text, license.author);

   license.name       = "Unknown License";
   license.spdxId     = "Unknown";
   license.text       = text;
   license.confidence = 0.0;

   pImpl->setLastError("No known license type detected");
   return false;
}

std::vector<std::string> LicenseDetector::getSupportedLicenses() const
{
   return pImpl->getSupportedLicensesImpl();
}

bool LicenseDetector::hasLicenseFiles(const std::string& directoryPath) const
{
   std::vector<std::string> licenseFiles;
   return pImpl->findLicenseFilesImpl(directoryPath, licenseFiles);
}

int LicenseDetector::getLicensePriority(const std::string& licenseName) const
{
   return pImpl->getLicensePriorityImpl(licenseName);
}

void LicenseDetector::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
}

std::string LicenseDetector::getLastError() const
{
   return pImpl->lastError;
}

void LicenseDetector::setConfidenceThreshold(double threshold)
{
   if (threshold >= 0.0 && threshold <= 1.0)
   {
      pImpl->confidenceThreshold = threshold;
   }
}

double LicenseDetector::getConfidenceThreshold() const
{
   return pImpl->confidenceThreshold;
}

// Private helper methods
bool LicenseDetector::detectMITLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectMITLicenseImpl(text, license);
}

bool LicenseDetector::detectApacheLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectApacheLicenseImpl(text, license);
}

bool LicenseDetector::detectGPLLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectGPLLicenseImpl(text, license);
}

bool LicenseDetector::detectBSDLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectBSDLicenseImpl(text, license);
}

bool LicenseDetector::detectISCLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectISCLicenseImpl(text, license);
}

bool LicenseDetector::detectUnlicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectUnlicenseImpl(text, license);
}

bool LicenseDetector::detectCreativeCommonsLicense(const std::string& text, LicenseInfo& license)
{
   return pImpl->detectCreativeCommonsLicenseImpl(text, license);
}

bool LicenseDetector::extractCopyright(const std::string& text, std::string& copyright)
{
   return pImpl->extractCopyrightImpl(text, copyright);
}

bool LicenseDetector::extractAuthor(const std::string& text, std::string& author)
{
   return pImpl->extractAuthorImpl(text, author);
}

double LicenseDetector::calculateConfidence(const std::string& text, const std::string& licenseType)
{
   return pImpl->calculateConfidenceImpl(text, licenseType);
}

std::string LicenseDetector::normalizeText(const std::string& text)
{
   return pImpl->normalizeTextImpl(text);
}

bool LicenseDetector::findLicenseFiles(const std::string&        directoryPath,
                                       std::vector<std::string>& licenseFiles)
{
   return pImpl->findLicenseFilesImpl(directoryPath, licenseFiles);
}

// Impl implementation
bool LicenseDetector::Impl::detectMITLicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement MIT license detection
   if (verbose)
   {
      std::cout << "Detecting MIT license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectApacheLicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement Apache license detection
   if (verbose)
   {
      std::cout << "Detecting Apache license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectGPLLicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement GPL license detection
   if (verbose)
   {
      std::cout << "Detecting GPL license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectBSDLicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement BSD license detection
   if (verbose)
   {
      std::cout << "Detecting BSD license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectISCLicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement ISC license detection
   if (verbose)
   {
      std::cout << "Detecting ISC license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectUnlicenseImpl(const std::string& text, LicenseInfo& license)
{
   // TODO: Implement Unlicense detection
   if (verbose)
   {
      std::cout << "Detecting Unlicense..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::detectCreativeCommonsLicenseImpl(const std::string& text,
                                                             LicenseInfo&       license)
{
   // TODO: Implement Creative Commons license detection
   if (verbose)
   {
      std::cout << "Detecting Creative Commons license..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::extractCopyrightImpl(const std::string& text, std::string& copyright)
{
   // TODO: Implement copyright extraction
   if (verbose)
   {
      std::cout << "Extracting copyright information..." << std::endl;
   }
   return false;
}

bool LicenseDetector::Impl::extractAuthorImpl(const std::string& text, std::string& author)
{
   // TODO: Implement author extraction
   if (verbose)
   {
      std::cout << "Extracting author information..." << std::endl;
   }
   return false;
}

double LicenseDetector::Impl::calculateConfidenceImpl(const std::string& text,
                                                      const std::string& licenseType)
{
   // TODO: Implement confidence calculation
   if (verbose)
   {
      std::cout << "Calculating confidence for " << licenseType << "..." << std::endl;
   }
   return 0.5;  // Default confidence
}

std::string LicenseDetector::Impl::normalizeTextImpl(const std::string& text)
{
   std::string normalized = text;

   // Convert to lowercase
   std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

   // Remove extra whitespace
   std::regex whitespace_regex("\\s+");
   normalized = std::regex_replace(normalized, whitespace_regex, " ");

   // Trim leading and trailing whitespace
   normalized.erase(0, normalized.find_first_not_of(" \t\r\n"));
   normalized.erase(normalized.find_last_not_of(" \t\r\n") + 1);

   return normalized;
}

bool LicenseDetector::Impl::findLicenseFilesImpl(const std::string&        directoryPath,
                                                 std::vector<std::string>& licenseFiles)
{
   licenseFiles.clear();

   // Common license file names
   std::vector<std::string> licenseFileNames = {
      "LICENSE",     "LICENSE.txt", "LICENSE.md",  "LICENSE.rst", "COPYING",
      "COPYING.txt", "COPYING.md",  "license",     "license.txt", "license.md",
      "license.rst", "copying",     "copying.txt", "copying.md"};

   for (const auto& fileName : licenseFileNames)
   {
      std::string filePath = directoryPath + "/" + fileName;
      if (heimdall::FileUtils::fileExists(filePath))
      {
         licenseFiles.push_back(filePath);
      }
   }

   return !licenseFiles.empty();
}

std::vector<std::string> LicenseDetector::Impl::getSupportedLicensesImpl() const
{
   return {"MIT",          "Apache-2.0", "GPL-3.0",   "GPL-2.0",  "BSD-3-Clause",
           "BSD-2-Clause", "ISC",        "Unlicense", "CC-BY-4.0"};
}

int LicenseDetector::Impl::getLicensePriorityImpl(const std::string& licenseName) const
{
   // Priority order: higher numbers = higher priority
   if (licenseName == "MIT")
      return 100;
   if (licenseName == "Apache-2.0")
      return 90;
   if (licenseName == "GPL-3.0")
      return 80;
   if (licenseName == "GPL-2.0")
      return 70;
   if (licenseName == "BSD-3-Clause")
      return 60;
   if (licenseName == "BSD-2-Clause")
      return 50;
   if (licenseName == "ISC")
      return 40;
   if (licenseName == "Unlicense")
      return 30;
   if (licenseName == "CC-BY-4.0")
      return 20;
   return 0;
}

void LicenseDetector::Impl::setLastError(const std::string& error)
{
   lastError = error;
   if (verbose)
   {
      std::cerr << "LicenseDetector error: " << error << std::endl;
   }
}

}  // namespace heimdall