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
 * @file SBOMGenerator.hpp
 * @brief Software Bill of Materials (SBOM) generator
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <memory>
#include <unordered_map>
#include "ComponentInfo.hpp"

namespace heimdall
{

/**
 * @brief Software Bill of Materials (SBOM) generator
 *
 * This class is responsible for collecting component information
 * and generating SBOMs in various formats (SPDX, CycloneDX, etc.).
 */
class SBOMGenerator
{
   public:
   /**
    * @brief Default constructor
    */
   SBOMGenerator();
   /**
    * @brief Destructor
    */
   ~SBOMGenerator();

   /**
    * @brief Process a component and add it to the SBOM
    * @param component The component to process
    */
   void processComponent(const ComponentInfo& component);
   /**
    * @brief Generate the SBOM in the specified format
    */
   void generateSBOM();
   /**
    * @brief Set the output path for the SBOM file
    * @param path The output file path
    */
   void setOutputPath(const std::string& path);
   /**
    * @brief Set the output format for the SBOM
    * @param format The format (e.g., "spdx", "cyclonedx")
    */
   void setFormat(const std::string& format);

   /**
    * @brief Set the SPDX version for the SBOM
    * @param version The SPDX version (e.g., "2.3", "3.0.0", "3.0.1")
    */
   void setSPDXVersion(const std::string& version);

   /**
    * @brief Set the CycloneDX version for the SBOM
    * @param version The CycloneDX version (e.g., "1.4", "1.6")
    */
   void setCycloneDXVersion(const std::string& version);

   /**
    * @brief Set whether to suppress warnings (for test mode)
    * @param suppress true to suppress warnings
    */
   void setSuppressWarnings(bool suppress);

   /**
    * @brief Set whether to recursively include transitive dependencies
    * @param transitive true to include transitive dependencies, false for direct only
    */
   void setTransitiveDependencies(bool transitive);

   /**
    * @brief Get the number of components in the SBOM
    * @return Number of components
    */
   [[nodiscard]] size_t getComponentCount() const;
   /**
    * @brief Check if a component exists in the SBOM
    * @param name The component name to check
    * @return true if the component exists
    */
   [[nodiscard]] bool hasComponent(const std::string& name) const;
   /**
    * @brief Print statistics about the SBOM
    */
   void printStatistics() const;

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

}  // namespace heimdall
