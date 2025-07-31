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

#include "SBOMValidator.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include "../compat/compatibility.hpp"
#include "Utils.hpp"

namespace heimdall
{

// UnifiedSBOMValidator Implementation

ValidationResult UnifiedSBOMValidator::validate(const std::string& filePath)
{
   ValidationResult result;

   if (!heimdall::Utils::fileExists(filePath))
   {
      result.addError("File does not exist: " + filePath);
      return result;
   }

   std::ifstream file(filePath);
   if (!file.is_open())
   {
      result.addError("Cannot open file: " + filePath);
      return result;
   }

   std::stringstream buffer;
   buffer << file.rdbuf();
   std::string content = buffer.str();

   return validateContent(content);
}

ValidationResult UnifiedSBOMValidator::validateContent(const std::string& content)
{
   ValidationResult result;

   // Immediate safety check for empty content
   if (content.empty())
   {
      result.addError("Content is empty");
      return result;
   }

   try
   {
      // If we have a specific format set, use it
      if (!format_.empty())
      {
         // Extract version from content
         std::string version = extractVersion(content, format_);
         if (version.empty())
         {
            result.addError("Unable to detect SBOM version from content");
            return result;
         }

         // Validate based on format
         if (format_ == "spdx")
         {
            return validateSPDX(content, version);
         }
         else if (format_ == "cyclonedx")
         {
            return validateCycloneDX(content, version);
         }
         else
         {
            result.addError("Unsupported SBOM format: " + format_);
            return result;
         }
      }
      else
      {
         // Auto-detect format from content
         std::string format = detectFormat(content);
         if (format.empty())
         {
            result.addError("Unable to detect SBOM format from content");
            return result;
         }

         // Extract version from content
         std::string version = extractVersion(content, format);
         if (version.empty())
         {
            result.addError("Unable to detect SBOM version from content");
            return result;
         }

         // Validate based on format
         if (format == "spdx")
         {
            return validateSPDX(content, version);
         }
         else if (format == "cyclonedx")
         {
            return validateCycloneDX(content, version);
         }
         else
         {
            result.addError("Unsupported SBOM format: " + format);
            return result;
         }
      }
   }
   catch (const std::exception& e)
   {
      result.addError("Validation failed with exception: " + std::string(e.what()));
      return result;
   }
}

ValidationResult UnifiedSBOMValidator::validateContent(const std::string& content,
                                                       const std::string& format,
                                                       const std::string& version)
{
   ValidationResult result;

   // Immediate safety check for empty content
   if (content.empty())
   {
      result.addError("Content is empty");
      return result;
   }

   try
   {
      // Validate based on format
      if (format == "spdx")
      {
         return validateSPDX(content, version);
      }
      else if (format == "cyclonedx")
      {
         return validateCycloneDX(content, version);
      }
      else
      {
         result.addError("Unsupported SBOM format: " + format);
         return result;
      }
   }
   catch (const std::exception& e)
   {
      result.addError("Validation failed with exception: " + std::string(e.what()));
      return result;
   }
}

std::string UnifiedSBOMValidator::detectFormat(const std::string& content) const
{
   // Convert to lowercase for case-insensitive comparison
   std::string lowerContent = content;
   std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

   // Check for CycloneDX format
   if (lowerContent.find("\"bomformat\":\"cyclonedx\"") != std::string::npos ||
       lowerContent.find("\"bomFormat\":\"CycloneDX\"") != std::string::npos)
   {
      return "cyclonedx";
   }

   // Check for SPDX format
   if (lowerContent.find("spdxversion:") != std::string::npos ||
       lowerContent.find("\"spdxversion\"") != std::string::npos ||
       lowerContent.find("\"spdxVersion\"") != std::string::npos ||
       lowerContent.find("@context") != std::string::npos)
   {
      return "spdx";
   }

   return "";
}

std::string UnifiedSBOMValidator::extractVersion(const std::string& content,
                                                 const std::string& format) const
{
   if (format == "cyclonedx")
   {
      // Extract CycloneDX version
      std::regex  versionRegex("\"specVersion\"\\s*:\\s*\"([^\"]+)\"");
      std::smatch match;
      if (std::regex_search(content, match, versionRegex))
      {
         return match[1].str();
      }
   }
   else if (format == "spdx")
   {
      // Extract SPDX version
      std::regex  versionRegex("SPDXVersion\\s*:\\s*SPDX-([0-9.]+)");
      std::smatch match;
      if (std::regex_search(content, match, versionRegex))
      {
         return match[1].str();
      }

      // Try JSON format with spdxVersion
      std::regex jsonVersionRegex("\"spdxVersion\"\\s*:\\s*\"SPDX-([0-9.]+)\"");
      if (std::regex_search(content, match, jsonVersionRegex))
      {
         return match[1].str();
      }

      // Try JSON format with specVersion (SPDX 3.0)
      std::regex specVersionRegex("\"specVersion\"\\s*:\\s*\"SPDX-([0-9.]+)\"");
      if (std::regex_search(content, match, specVersionRegex))
      {
         return match[1].str();
      }

      // Try JSON-LD format
      std::regex jsonldVersionRegex(
         "\"@context\"\\s*:\\s*\"[^\"]*spdx-context-([0-9.]+)\\.jsonld\"");
      if (std::regex_search(content, match, jsonldVersionRegex))
      {
         return match[1].str();
      }
   }

   return "";
}

ValidationResult UnifiedSBOMValidator::validateSPDX(const std::string& content,
                                                    const std::string& version)
{
   ValidationResult result;

   try
   {
      // Create SPDX handler using the factory
      auto spdxHandler = SBOMFormatFactory::createSPDXHandler(version);
      if (!spdxHandler)
      {
         result.addError("Failed to create SPDX handler for version: " + version);
         return result;
      }

      // Use the handler's validation method
      return spdxHandler->validateContent(content);
   }
   catch (const std::exception& e)
   {
      result.addError("SPDX validation failed: " + std::string(e.what()));
      return result;
   }
}

ValidationResult UnifiedSBOMValidator::validateCycloneDX(const std::string& content,
                                                         const std::string& version)
{
   ValidationResult result;

   try
   {
      // Create CycloneDX handler using the factory
      auto cyclonedxHandler = SBOMFormatFactory::createCycloneDXHandler(version);
      if (!cyclonedxHandler)
      {
         result.addError("Failed to create CycloneDX handler for version: " + version);
         return result;
      }

      // Use the handler's validation method
      return cyclonedxHandler->validateContent(content);
   }
   catch (const std::exception& e)
   {
      result.addError("CycloneDX validation failed: " + std::string(e.what()));
      return result;
   }
}

// SBOMValidatorFactory Implementation

std::unique_ptr<SBOMValidator> SBOMValidatorFactory::createValidator(const std::string& format)
{
   // Check if the format is supported
   if (format == "spdx" || format == "cyclonedx" || format.empty())
   {
      // Return a UnifiedSBOMValidator with the specified format
      return std::make_unique<UnifiedSBOMValidator>(format);
   }

   // Return nullptr for unsupported formats
   return nullptr;
}

std::vector<std::string> SBOMValidatorFactory::getSupportedFormats()
{
   return {"spdx", "cyclonedx"};
}

std::vector<std::string> SBOMValidatorFactory::getSupportedVersions(const std::string& format)
{
   if (format == "spdx")
   {
      return {"2.3", "3.0.0", "3.0.1"};
   }
   else if (format == "cyclonedx")
   {
      return {"1.4", "1.5", "1.6"};
   }
   else
   {
      return {};
   }
}

}  // namespace heimdall