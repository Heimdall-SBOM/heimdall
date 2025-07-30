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
 * @file VersionDetector.cpp
 * @brief Version detection and metadata extraction implementation
 * @author Trevor Bakker
 * @date 2025
 */

#include "VersionDetector.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include "../utils/FileUtils.hpp"

namespace heimdall
{

class VersionDetector::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   bool        verbose = false;
   std::string lastError;
   double      confidenceThreshold = 0.7;

   // Version detection methods
   bool detectVersionFromCMakeImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromPackageJsonImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromCargoTomlImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromPomXmlImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromBuildGradleImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromComposerJsonImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromGoModImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromCsprojImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromHeaderImpl(const std::string& text, VersionInfo& version);
   bool detectVersionFromGitImpl(const std::string& directoryPath, VersionInfo& version);

   // Helper methods
   bool        extractBuildInfoImpl(const std::string& text, VersionInfo& version);
   double      calculateConfidenceImpl(const std::string& text, const std::string& versionFormat);
   std::string normalizeVersionStringImpl(const std::string& versionString);
   bool        findVersionFilesImpl(const std::string&        directoryPath,
                                    std::vector<std::string>& versionFiles);
   std::vector<std::string> getSupportedVersionFormatsImpl() const;
   int                      getVersionPriorityImpl(const std::string& versionFormat) const;
   void                     setLastError(const std::string& error);
};

// VersionDetector implementation
VersionDetector::VersionDetector() : pImpl(std::make_unique<Impl>()) {}

VersionDetector::~VersionDetector() = default;

VersionDetector::VersionDetector(const VersionDetector& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

VersionDetector::VersionDetector(VersionDetector&& other) noexcept : pImpl(std::move(other.pImpl))
{
}

VersionDetector& VersionDetector::operator=(const VersionDetector& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

VersionDetector& VersionDetector::operator=(VersionDetector&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

bool VersionDetector::detectVersions(const std::string&        directoryPath,
                                     std::vector<VersionInfo>& versions)
{
   versions.clear();

   if (!hasVersionInformation(directoryPath))
   {
      pImpl->setLastError("No version information found in directory");
      return false;
   }

   std::vector<std::string> versionFiles;
   if (!pImpl->findVersionFilesImpl(directoryPath, versionFiles))
   {
      pImpl->setLastError("Failed to find version files");
      return false;
   }

   for (const auto& filePath : versionFiles)
   {
      VersionInfo version;
      if (detectVersionFromFile(filePath, version))
      {
         if (version.confidence >= pImpl->confidenceThreshold)
         {
            versions.push_back(version);
         }
      }
   }

   // Also try to detect version from git information
   VersionInfo gitVersion;
   if (pImpl->detectVersionFromGitImpl(directoryPath, gitVersion))
   {
      if (gitVersion.confidence >= pImpl->confidenceThreshold)
      {
         versions.push_back(gitVersion);
      }
   }

   // Sort by confidence (higher confidence first)
   std::sort(versions.begin(), versions.end(), [](const VersionInfo& a, const VersionInfo& b)
             { return a.confidence > b.confidence; });

   return !versions.empty();
}

bool VersionDetector::detectVersionFromFile(const std::string& filePath, VersionInfo& version)
{
   if (!heimdall::FileUtils::fileExists(filePath))
   {
      pImpl->setLastError("Version file does not exist: " + filePath);
      return false;
   }

   std::ifstream file(filePath);
   if (!file.is_open())
   {
      pImpl->setLastError("Failed to open version file: " + filePath);
      return false;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string text = buffer.str();

   version.filePath = filePath;
   return detectVersionFromText(text, version);
}

bool VersionDetector::detectVersionFromText(const std::string& text, VersionInfo& version)
{
   // Try to detect version from different file types
   if (pImpl->detectVersionFromCMakeImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromPackageJsonImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromCargoTomlImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromPomXmlImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromBuildGradleImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromComposerJsonImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromGoModImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromCsprojImpl(text, version))
   {
      return true;
   }
   else if (pImpl->detectVersionFromHeaderImpl(text, version))
   {
      return true;
   }

   // Extract build information even if version type is unknown
   pImpl->extractBuildInfoImpl(text, version);

   version.source      = "Unknown";
   version.fullVersion = "Unknown";
   version.confidence  = 0.0;

   pImpl->setLastError("No known version format detected");
   return false;
}

bool VersionDetector::parseSemanticVersion(const std::string& versionString, VersionInfo& version)
{
   std::string normalized = pImpl->normalizeVersionStringImpl(versionString);

   // Basic semantic version regex: major.minor.patch[-prerelease][+build]
   std::regex  semver_regex(R"((\d+)\.(\d+)\.(\d+)(?:-([\w.-]+))?(?:\+([\w.-]+))?)");
   std::smatch matches;

   if (std::regex_match(normalized, matches, semver_regex))
   {
      version.major       = matches[1].str();
      version.minor       = matches[2].str();
      version.patch       = matches[3].str();
      version.prerelease  = matches[4].str();
      version.build       = matches[5].str();
      version.fullVersion = versionString;
      version.source      = "semantic";
      version.confidence  = 1.0;

      // Determine version type
      version.isPrerelease  = !version.prerelease.empty();
      version.isDevelopment = version.prerelease.find("dev") != std::string::npos ||
                              version.prerelease.find("alpha") != std::string::npos ||
                              version.prerelease.find("beta") != std::string::npos;
      version.isRelease = !version.isPrerelease && !version.isDevelopment;

      return true;
   }

   return false;
}

std::vector<std::string> VersionDetector::getSupportedVersionFormats() const
{
   return pImpl->getSupportedVersionFormatsImpl();
}

bool VersionDetector::hasVersionInformation(const std::string& directoryPath) const
{
   std::vector<std::string> versionFiles;
   return pImpl->findVersionFilesImpl(directoryPath, versionFiles);
}

int VersionDetector::getVersionPriority(const std::string& versionFormat) const
{
   return pImpl->getVersionPriorityImpl(versionFormat);
}

void VersionDetector::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
}

std::string VersionDetector::getLastError() const
{
   return pImpl->lastError;
}

void VersionDetector::setConfidenceThreshold(double threshold)
{
   if (threshold >= 0.0 && threshold <= 1.0)
   {
      pImpl->confidenceThreshold = threshold;
   }
}

double VersionDetector::getConfidenceThreshold() const
{
   return pImpl->confidenceThreshold;
}

// Private helper methods
bool VersionDetector::detectVersionFromCMake(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromCMakeImpl(text, version);
}

bool VersionDetector::detectVersionFromPackageJson(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromPackageJsonImpl(text, version);
}

bool VersionDetector::detectVersionFromCargoToml(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromCargoTomlImpl(text, version);
}

bool VersionDetector::detectVersionFromPomXml(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromPomXmlImpl(text, version);
}

bool VersionDetector::detectVersionFromBuildGradle(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromBuildGradleImpl(text, version);
}

bool VersionDetector::detectVersionFromComposerJson(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromComposerJsonImpl(text, version);
}

bool VersionDetector::detectVersionFromGoMod(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromGoModImpl(text, version);
}

bool VersionDetector::detectVersionFromCsproj(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromCsprojImpl(text, version);
}

bool VersionDetector::detectVersionFromHeader(const std::string& text, VersionInfo& version)
{
   return pImpl->detectVersionFromHeaderImpl(text, version);
}

bool VersionDetector::detectVersionFromGit(const std::string& directoryPath, VersionInfo& version)
{
   return pImpl->detectVersionFromGitImpl(directoryPath, version);
}

bool VersionDetector::extractBuildInfo(const std::string& text, VersionInfo& version)
{
   return pImpl->extractBuildInfoImpl(text, version);
}

double VersionDetector::calculateConfidence(const std::string& text,
                                            const std::string& versionFormat)
{
   return pImpl->calculateConfidenceImpl(text, versionFormat);
}

std::string VersionDetector::normalizeVersionString(const std::string& versionString)
{
   return pImpl->normalizeVersionStringImpl(versionString);
}

bool VersionDetector::findVersionFiles(const std::string&        directoryPath,
                                       std::vector<std::string>& versionFiles)
{
   return pImpl->findVersionFilesImpl(directoryPath, versionFiles);
}

// Impl implementation
bool VersionDetector::Impl::detectVersionFromCMakeImpl(const std::string& text,
                                                       VersionInfo&       version)
{
   // TODO: Implement CMake version detection
   if (verbose)
   {
      std::cout << "Detecting version from CMake..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromPackageJsonImpl(const std::string& text,
                                                             VersionInfo&       version)
{
   // TODO: Implement package.json version detection
   if (verbose)
   {
      std::cout << "Detecting version from package.json..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromCargoTomlImpl(const std::string& text,
                                                           VersionInfo&       version)
{
   // TODO: Implement Cargo.toml version detection
   if (verbose)
   {
      std::cout << "Detecting version from Cargo.toml..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromPomXmlImpl(const std::string& text,
                                                        VersionInfo&       version)
{
   // TODO: Implement pom.xml version detection
   if (verbose)
   {
      std::cout << "Detecting version from pom.xml..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromBuildGradleImpl(const std::string& text,
                                                             VersionInfo&       version)
{
   // TODO: Implement build.gradle version detection
   if (verbose)
   {
      std::cout << "Detecting version from build.gradle..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromComposerJsonImpl(const std::string& text,
                                                              VersionInfo&       version)
{
   // TODO: Implement composer.json version detection
   if (verbose)
   {
      std::cout << "Detecting version from composer.json..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromGoModImpl(const std::string& text,
                                                       VersionInfo&       version)
{
   // TODO: Implement go.mod version detection
   if (verbose)
   {
      std::cout << "Detecting version from go.mod..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromCsprojImpl(const std::string& text,
                                                        VersionInfo&       version)
{
   // TODO: Implement .csproj version detection
   if (verbose)
   {
      std::cout << "Detecting version from .csproj..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromHeaderImpl(const std::string& text,
                                                        VersionInfo&       version)
{
   // TODO: Implement header file version detection
   if (verbose)
   {
      std::cout << "Detecting version from header file..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::detectVersionFromGitImpl(const std::string& directoryPath,
                                                     VersionInfo&       version)
{
   // TODO: Implement git version detection
   if (verbose)
   {
      std::cout << "Detecting version from git..." << std::endl;
   }
   return false;
}

bool VersionDetector::Impl::extractBuildInfoImpl(const std::string& text, VersionInfo& version)
{
   // TODO: Implement build info extraction
   if (verbose)
   {
      std::cout << "Extracting build information..." << std::endl;
   }
   return false;
}

double VersionDetector::Impl::calculateConfidenceImpl(const std::string& text,
                                                      const std::string& versionFormat)
{
   // TODO: Implement confidence calculation
   if (verbose)
   {
      std::cout << "Calculating confidence for " << versionFormat << "..." << std::endl;
   }
   return 0.5;  // Default confidence
}

std::string VersionDetector::Impl::normalizeVersionStringImpl(const std::string& versionString)
{
   std::string normalized = versionString;

   // Remove leading/trailing whitespace
   normalized.erase(0, normalized.find_first_not_of(" \t\r\n"));
   normalized.erase(normalized.find_last_not_of(" \t\r\n") + 1);

   // Remove common prefixes
   std::vector<std::string> prefixes = {"v", "V", "version", "Version", "VERSION"};
   for (const auto& prefix : prefixes)
   {
      if (normalized.substr(0, prefix.length()) == prefix)
      {
         normalized = normalized.substr(prefix.length());
         break;
      }
   }

   return normalized;
}

bool VersionDetector::Impl::findVersionFilesImpl(const std::string&        directoryPath,
                                                 std::vector<std::string>& versionFiles)
{
   versionFiles.clear();

   // Common version file names
   std::vector<std::string> versionFileNames = {
      "CMakeLists.txt", "package.json",     "Cargo.toml",    "pom.xml",
      "build.gradle",   "build.gradle.kts", "composer.json", "go.mod",
      "*.csproj",       "version.h",        "version.hpp",   "Version.h",
      "Version.hpp",    "VERSION",          "version.txt",   "version.md"};

   for (const auto& fileName : versionFileNames)
   {
      std::string filePath = directoryPath + "/" + fileName;
      if (heimdall::FileUtils::fileExists(filePath))
      {
         versionFiles.push_back(filePath);
      }
   }

   return !versionFiles.empty();
}

std::vector<std::string> VersionDetector::Impl::getSupportedVersionFormatsImpl() const
{
   return {"semantic",      "cmake",  "package.json", "cargo.toml", "pom.xml", "build.gradle",
           "composer.json", "go.mod", "csproj",       "header",     "git"};
}

int VersionDetector::Impl::getVersionPriorityImpl(const std::string& versionFormat) const
{
   // Priority order: higher numbers = higher priority
   if (versionFormat == "semantic")
      return 100;
   if (versionFormat == "cmake")
      return 90;
   if (versionFormat == "package.json")
      return 80;
   if (versionFormat == "cargo.toml")
      return 70;
   if (versionFormat == "pom.xml")
      return 60;
   if (versionFormat == "build.gradle")
      return 50;
   if (versionFormat == "composer.json")
      return 40;
   if (versionFormat == "go.mod")
      return 30;
   if (versionFormat == "csproj")
      return 20;
   if (versionFormat == "header")
      return 10;
   if (versionFormat == "git")
      return 5;
   return 0;
}

void VersionDetector::Impl::setLastError(const std::string& error)
{
   lastError = error;
   if (verbose)
   {
      std::cerr << "VersionDetector error: " << error << std::endl;
   }
}

}  // namespace heimdall