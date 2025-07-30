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
 * @file PackageManagerDetector.cpp
 * @brief Package manager detection and metadata extraction implementation
 * @author Trevor Bakker
 * @date 2025
 */

#include "PackageManagerDetector.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../utils/FileUtils.hpp"

namespace heimdall
{

class PackageManagerDetector::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   bool        verbose = false;
   std::string lastError;

   // Package manager detection methods
   bool detectNpmImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectPipImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectCargoImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectMavenImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectGradleImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectComposerImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectGoModulesImpl(const std::string& directoryPath, PackageManagerInfo& info);
   bool detectNugetImpl(const std::string& directoryPath, PackageManagerInfo& info);

   // File parsing methods
   bool parsePackageJsonImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseRequirementsTxtImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseCargoTomlImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parsePomXmlImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseBuildGradleImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseComposerJsonImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseGoModImpl(const std::string& filePath, PackageManagerInfo& info);
   bool parseCsprojImpl(const std::string& filePath, PackageManagerInfo& info);

   // Helper methods
   bool                     fileExists(const std::string& filePath) const;
   std::vector<std::string> getSupportedPackageManagersImpl() const;
   int  getPackageManagerPriorityImpl(const std::string& packageManagerName) const;
   void setLastError(const std::string& error);
};

// PackageManagerDetector implementation
PackageManagerDetector::PackageManagerDetector() : pImpl(std::make_unique<Impl>()) {}

PackageManagerDetector::~PackageManagerDetector() = default;

PackageManagerDetector::PackageManagerDetector(const PackageManagerDetector& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

PackageManagerDetector::PackageManagerDetector(PackageManagerDetector&& other) noexcept
   : pImpl(std::move(other.pImpl))
{
}

PackageManagerDetector& PackageManagerDetector::operator=(const PackageManagerDetector& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

PackageManagerDetector& PackageManagerDetector::operator=(PackageManagerDetector&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

bool PackageManagerDetector::detectPackageManagers(const std::string&               directoryPath,
                                                   std::vector<PackageManagerInfo>& packageManagers)
{
   packageManagers.clear();

   // Try to detect each supported package manager
   std::vector<std::string> supported = getSupportedPackageManagers();
   for (const auto& pm : supported)
   {
      if (hasPackageManager(directoryPath, pm))
      {
         PackageManagerInfo info;
         if (extractPackageManagerMetadata(directoryPath, pm, info))
         {
            packageManagers.push_back(info);
         }
      }
   }

   // Sort by priority (higher priority first)
   std::sort(packageManagers.begin(), packageManagers.end(),
             [this](const PackageManagerInfo& a, const PackageManagerInfo& b)
             { return getPackageManagerPriority(a.name) > getPackageManagerPriority(b.name); });

   return !packageManagers.empty();
}

bool PackageManagerDetector::extractPackageManagerMetadata(const std::string&  directoryPath,
                                                           const std::string&  packageManagerName,
                                                           PackageManagerInfo& info)
{
   if (packageManagerName == "npm")
   {
      return pImpl->detectNpmImpl(directoryPath, info);
   }
   else if (packageManagerName == "pip")
   {
      return pImpl->detectPipImpl(directoryPath, info);
   }
   else if (packageManagerName == "cargo")
   {
      return pImpl->detectCargoImpl(directoryPath, info);
   }
   else if (packageManagerName == "maven")
   {
      return pImpl->detectMavenImpl(directoryPath, info);
   }
   else if (packageManagerName == "gradle")
   {
      return pImpl->detectGradleImpl(directoryPath, info);
   }
   else if (packageManagerName == "composer")
   {
      return pImpl->detectComposerImpl(directoryPath, info);
   }
   else if (packageManagerName == "go")
   {
      return pImpl->detectGoModulesImpl(directoryPath, info);
   }
   else if (packageManagerName == "nuget")
   {
      return pImpl->detectNugetImpl(directoryPath, info);
   }

   pImpl->setLastError("Unsupported package manager: " + packageManagerName);
   return false;
}

std::vector<std::string> PackageManagerDetector::getSupportedPackageManagers() const
{
   return pImpl->getSupportedPackageManagersImpl();
}

bool PackageManagerDetector::hasPackageManager(const std::string& directoryPath,
                                               const std::string& packageManagerName) const
{
   if (packageManagerName == "npm")
   {
      return pImpl->fileExists(directoryPath + "/package.json");
   }
   else if (packageManagerName == "pip")
   {
      return pImpl->fileExists(directoryPath + "/requirements.txt") ||
             pImpl->fileExists(directoryPath + "/setup.py") ||
             pImpl->fileExists(directoryPath + "/pyproject.toml");
   }
   else if (packageManagerName == "cargo")
   {
      return pImpl->fileExists(directoryPath + "/Cargo.toml");
   }
   else if (packageManagerName == "maven")
   {
      return pImpl->fileExists(directoryPath + "/pom.xml");
   }
   else if (packageManagerName == "gradle")
   {
      return pImpl->fileExists(directoryPath + "/build.gradle") ||
             pImpl->fileExists(directoryPath + "/build.gradle.kts");
   }
   else if (packageManagerName == "composer")
   {
      return pImpl->fileExists(directoryPath + "/composer.json");
   }
   else if (packageManagerName == "go")
   {
      return pImpl->fileExists(directoryPath + "/go.mod");
   }
   else if (packageManagerName == "nuget")
   {
      return pImpl->fileExists(directoryPath + "/*.csproj");
   }

   return false;
}

int PackageManagerDetector::getPackageManagerPriority(const std::string& packageManagerName) const
{
   return pImpl->getPackageManagerPriorityImpl(packageManagerName);
}

void PackageManagerDetector::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
}

std::string PackageManagerDetector::getLastError() const
{
   return pImpl->lastError;
}

// Private helper methods
bool PackageManagerDetector::detectNpm(const std::string& directoryPath, PackageManagerInfo& info)
{
   return pImpl->detectNpmImpl(directoryPath, info);
}

bool PackageManagerDetector::detectPip(const std::string& directoryPath, PackageManagerInfo& info)
{
   return pImpl->detectPipImpl(directoryPath, info);
}

bool PackageManagerDetector::detectCargo(const std::string& directoryPath, PackageManagerInfo& info)
{
   return pImpl->detectCargoImpl(directoryPath, info);
}

bool PackageManagerDetector::detectMaven(const std::string& directoryPath, PackageManagerInfo& info)
{
   return pImpl->detectMavenImpl(directoryPath, info);
}

bool PackageManagerDetector::detectGradle(const std::string&  directoryPath,
                                          PackageManagerInfo& info)
{
   return pImpl->detectGradleImpl(directoryPath, info);
}

bool PackageManagerDetector::detectComposer(const std::string&  directoryPath,
                                            PackageManagerInfo& info)
{
   return pImpl->detectComposerImpl(directoryPath, info);
}

bool PackageManagerDetector::detectGoModules(const std::string&  directoryPath,
                                             PackageManagerInfo& info)
{
   return pImpl->detectGoModulesImpl(directoryPath, info);
}

bool PackageManagerDetector::detectNuget(const std::string& directoryPath, PackageManagerInfo& info)
{
   return pImpl->detectNugetImpl(directoryPath, info);
}

bool PackageManagerDetector::parsePackageJson(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parsePackageJsonImpl(filePath, info);
}

bool PackageManagerDetector::parseRequirementsTxt(const std::string&  filePath,
                                                  PackageManagerInfo& info)
{
   return pImpl->parseRequirementsTxtImpl(filePath, info);
}

bool PackageManagerDetector::parseCargoToml(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parseCargoTomlImpl(filePath, info);
}

bool PackageManagerDetector::parsePomXml(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parsePomXmlImpl(filePath, info);
}

bool PackageManagerDetector::parseBuildGradle(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parseBuildGradleImpl(filePath, info);
}

bool PackageManagerDetector::parseComposerJson(const std::string&  filePath,
                                               PackageManagerInfo& info)
{
   return pImpl->parseComposerJsonImpl(filePath, info);
}

bool PackageManagerDetector::parseGoMod(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parseGoModImpl(filePath, info);
}

bool PackageManagerDetector::parseCsproj(const std::string& filePath, PackageManagerInfo& info)
{
   return pImpl->parseCsprojImpl(filePath, info);
}

// Impl implementation
bool PackageManagerDetector::Impl::detectNpmImpl(const std::string&  directoryPath,
                                                 PackageManagerInfo& info)
{
   std::string packageJsonPath = directoryPath + "/package.json";
   if (!fileExists(packageJsonPath))
   {
      setLastError("package.json not found");
      return false;
   }

   info.name           = "npm";
   info.manifestFile   = packageJsonPath;
   info.lockFile       = directoryPath + "/package-lock.json";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "npm install";
   info.updateCommand  = "npm update";

   return parsePackageJsonImpl(packageJsonPath, info);
}

bool PackageManagerDetector::Impl::detectPipImpl(const std::string&  directoryPath,
                                                 PackageManagerInfo& info)
{
   std::string requirementsPath = directoryPath + "/requirements.txt";
   if (!fileExists(requirementsPath))
   {
      setLastError("requirements.txt not found");
      return false;
   }

   info.name           = "pip";
   info.manifestFile   = requirementsPath;
   info.lockFile       = directoryPath + "/requirements.lock";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "pip install -r requirements.txt";
   info.updateCommand  = "pip install --upgrade -r requirements.txt";

   return parseRequirementsTxtImpl(requirementsPath, info);
}

bool PackageManagerDetector::Impl::detectCargoImpl(const std::string&  directoryPath,
                                                   PackageManagerInfo& info)
{
   std::string cargoTomlPath = directoryPath + "/Cargo.toml";
   if (!fileExists(cargoTomlPath))
   {
      setLastError("Cargo.toml not found");
      return false;
   }

   info.name           = "cargo";
   info.manifestFile   = cargoTomlPath;
   info.lockFile       = directoryPath + "/Cargo.lock";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "cargo build";
   info.updateCommand  = "cargo update";

   return parseCargoTomlImpl(cargoTomlPath, info);
}

bool PackageManagerDetector::Impl::detectMavenImpl(const std::string&  directoryPath,
                                                   PackageManagerInfo& info)
{
   std::string pomXmlPath = directoryPath + "/pom.xml";
   if (!fileExists(pomXmlPath))
   {
      setLastError("pom.xml not found");
      return false;
   }

   info.name           = "maven";
   info.manifestFile   = pomXmlPath;
   info.lockFile       = "";  // Maven doesn't use lock files
   info.hasLockFile    = false;
   info.installCommand = "mvn install";
   info.updateCommand  = "mvn clean install";

   return parsePomXmlImpl(pomXmlPath, info);
}

bool PackageManagerDetector::Impl::detectGradleImpl(const std::string&  directoryPath,
                                                    PackageManagerInfo& info)
{
   std::string buildGradlePath = directoryPath + "/build.gradle";
   if (!fileExists(buildGradlePath))
   {
      setLastError("build.gradle not found");
      return false;
   }

   info.name           = "gradle";
   info.manifestFile   = buildGradlePath;
   info.lockFile       = directoryPath + "/gradle.lockfile";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "gradle build";
   info.updateCommand  = "gradle build --refresh-dependencies";

   return parseBuildGradleImpl(buildGradlePath, info);
}

bool PackageManagerDetector::Impl::detectComposerImpl(const std::string&  directoryPath,
                                                      PackageManagerInfo& info)
{
   std::string composerJsonPath = directoryPath + "/composer.json";
   if (!fileExists(composerJsonPath))
   {
      setLastError("composer.json not found");
      return false;
   }

   info.name           = "composer";
   info.manifestFile   = composerJsonPath;
   info.lockFile       = directoryPath + "/composer.lock";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "composer install";
   info.updateCommand  = "composer update";

   return parseComposerJsonImpl(composerJsonPath, info);
}

bool PackageManagerDetector::Impl::detectGoModulesImpl(const std::string&  directoryPath,
                                                       PackageManagerInfo& info)
{
   std::string goModPath = directoryPath + "/go.mod";
   if (!fileExists(goModPath))
   {
      setLastError("go.mod not found");
      return false;
   }

   info.name           = "go";
   info.manifestFile   = goModPath;
   info.lockFile       = directoryPath + "/go.sum";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "go mod download";
   info.updateCommand  = "go get -u";

   return parseGoModImpl(goModPath, info);
}

bool PackageManagerDetector::Impl::detectNugetImpl(const std::string&  directoryPath,
                                                   PackageManagerInfo& info)
{
   // Look for .csproj files
   std::string csprojPath = directoryPath + "/*.csproj";
   if (!fileExists(csprojPath))
   {
      setLastError("No .csproj files found");
      return false;
   }

   info.name           = "nuget";
   info.manifestFile   = csprojPath;
   info.lockFile       = directoryPath + "/packages.lock.json";
   info.hasLockFile    = fileExists(info.lockFile);
   info.installCommand = "dotnet restore";
   info.updateCommand  = "dotnet restore --force";

   return parseCsprojImpl(csprojPath, info);
}

// File parsing implementations (placeholder)
bool PackageManagerDetector::Impl::parsePackageJsonImpl(const std::string&  filePath,
                                                        PackageManagerInfo& info)
{
   // TODO: Implement JSON parsing for package.json
   if (verbose)
   {
      std::cout << "Parsing package.json: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseRequirementsTxtImpl(const std::string&  filePath,
                                                            PackageManagerInfo& info)
{
   // TODO: Implement parsing for requirements.txt
   if (verbose)
   {
      std::cout << "Parsing requirements.txt: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseCargoTomlImpl(const std::string&  filePath,
                                                      PackageManagerInfo& info)
{
   // TODO: Implement TOML parsing for Cargo.toml
   if (verbose)
   {
      std::cout << "Parsing Cargo.toml: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parsePomXmlImpl(const std::string&  filePath,
                                                   PackageManagerInfo& info)
{
   // TODO: Implement XML parsing for pom.xml
   if (verbose)
   {
      std::cout << "Parsing pom.xml: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseBuildGradleImpl(const std::string&  filePath,
                                                        PackageManagerInfo& info)
{
   // TODO: Implement parsing for build.gradle
   if (verbose)
   {
      std::cout << "Parsing build.gradle: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseComposerJsonImpl(const std::string&  filePath,
                                                         PackageManagerInfo& info)
{
   // TODO: Implement JSON parsing for composer.json
   if (verbose)
   {
      std::cout << "Parsing composer.json: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseGoModImpl(const std::string&  filePath,
                                                  PackageManagerInfo& info)
{
   // TODO: Implement parsing for go.mod
   if (verbose)
   {
      std::cout << "Parsing go.mod: " << filePath << std::endl;
   }
   return true;
}

bool PackageManagerDetector::Impl::parseCsprojImpl(const std::string&  filePath,
                                                   PackageManagerInfo& info)
{
   // TODO: Implement XML parsing for .csproj
   if (verbose)
   {
      std::cout << "Parsing .csproj: " << filePath << std::endl;
   }
   return true;
}

// Helper methods
bool PackageManagerDetector::Impl::fileExists(const std::string& filePath) const
{
   return heimdall::FileUtils::fileExists(filePath);
}

std::vector<std::string> PackageManagerDetector::Impl::getSupportedPackageManagersImpl() const
{
   return {"npm", "pip", "cargo", "maven", "gradle", "composer", "go", "nuget"};
}

int PackageManagerDetector::Impl::getPackageManagerPriorityImpl(
   const std::string& packageManagerName) const
{
   // Priority order: higher numbers = higher priority
   if (packageManagerName == "npm")
      return 100;
   if (packageManagerName == "pip")
      return 90;
   if (packageManagerName == "cargo")
      return 80;
   if (packageManagerName == "maven")
      return 70;
   if (packageManagerName == "gradle")
      return 60;
   if (packageManagerName == "composer")
      return 50;
   if (packageManagerName == "go")
      return 40;
   if (packageManagerName == "nuget")
      return 30;
   return 0;
}

void PackageManagerDetector::Impl::setLastError(const std::string& error)
{
   lastError = error;
   if (verbose)
   {
      std::cerr << "PackageManagerDetector error: " << error << std::endl;
   }
}

}  // namespace heimdall