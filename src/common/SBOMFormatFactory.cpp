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
 * @file SBOMFormatFactory.cpp
 * @brief Factory implementation for SBOM format handlers
 * @author Trevor Bakker
 * @date 2025
 */

#include <algorithm>
#include <stdexcept>
#include "CycloneDXHandler.hpp"
#include "SBOMFormats.hpp"
#include "SPDXHandler.hpp"

namespace heimdall
{

std::unique_ptr<ISPDXHandler> SBOMFormatFactory::createSPDXHandler(const std::string& version)
{
   if (version == "2.3")
   {
      return std::make_unique<SPDX2_3Handler>();
   }
   else if (version == "3.0.0" || version == "3.0")
   {
      return std::make_unique<SPDX3_0_0Handler>();
   }
   else if (version == "3.0.1")
   {
      return std::make_unique<SPDX3_0_1Handler>();
   }
   else
   {
      throw std::invalid_argument("Unsupported SPDX version: " + version);
   }
}

std::unique_ptr<ICycloneDXHandler> SBOMFormatFactory::createCycloneDXHandler(
   const std::string& version)
{
   if (version == "1.4")
   {
      return std::make_unique<CycloneDX1_4Handler>();
   }
   else if (version == "1.5")
   {
      return std::make_unique<CycloneDX1_5Handler>();
   }
   else if (version == "1.6")
   {
      return std::make_unique<CycloneDX1_6Handler>();
   }
   else
   {
      throw std::invalid_argument("Unsupported CycloneDX version: " + version);
   }
}

std::unique_ptr<ISBOMFormatHandler> SBOMFormatFactory::createHandler(const std::string& format,
                                                                     const std::string& version)
{
   std::string lowerFormat = format;
   std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);

   if (lowerFormat == "spdx")
   {
      std::string spdxVersion = version.empty() ? "2.3" : version;
      try
      {
         return createSPDXHandler(spdxVersion);
      }
      catch (const std::invalid_argument&)
      {
         return nullptr;
      }
   }
   else if (lowerFormat == "cyclonedx" || lowerFormat == "cyclone")
   {
      std::string cyclonedxVersion = version.empty() ? "1.6" : version;
      try
      {
         return createCycloneDXHandler(cyclonedxVersion);
      }
      catch (const std::invalid_argument&)
      {
         return nullptr;
      }
   }
   else
   {
      return nullptr;
   }
}

std::vector<std::string> SBOMFormatFactory::getSupportedFormats()
{
   return {"spdx", "cyclonedx"};
}

std::vector<std::string> SBOMFormatFactory::getSupportedVersions(const std::string& format)
{
   std::string lowerFormat = format;
   std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);

   if (lowerFormat == "spdx")
   {
      return {"2.3", "3.0.0", "3.0.1"};
   }
   else if (lowerFormat == "cyclonedx" || lowerFormat == "cyclone")
   {
      return {"1.4", "1.5", "1.6"};
   }
   else
   {
      return {};
   }
}

}  // namespace heimdall