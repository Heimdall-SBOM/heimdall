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
 * @file PluginInterface.hpp
 * @brief Common plugin interface for linker plugins (LLD and Gold)
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "ComponentInfo.hpp"
#include "SBOMGenerator.hpp"

namespace heimdall
{

/**
 * @brief Common plugin interface for both LLD and Gold linkers
 *
 * This abstract class defines the interface that all linker plugins
 * must implement to integrate with the Heimdall SBOM generation system.
 */
class PluginInterface
{
   public:
   /**
    * @brief Default constructor
    */
   PluginInterface();

   /**
    * @brief Virtual destructor
    */
   virtual ~PluginInterface();

   /**
    * @brief Initialize the plugin
    * @return true if initialization was successful
    */
   virtual bool initialize() = 0;

   /**
    * @brief Clean up plugin resources
    */
   virtual void cleanup() = 0;

   /**
    * @brief Process an input file
    * @param filePath The path to the input file
    */
   virtual void processInputFile(const std::string& filePath) = 0;

   /**
    * @brief Process a library file
    * @param libraryPath The path to the library file
    */
   virtual void processLibrary(const std::string& libraryPath) = 0;

   /**
    * @brief Process a symbol
    * @param symbolName The name of the symbol
    * @param address The symbol address
    * @param size The symbol size
    */
   virtual void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) = 0;

   /**
    * @brief Set the output path for the SBOM
    * @param path The output file path
    */
   virtual void setOutputPath(const std::string& path) = 0;

   /**
    * @brief Set the output format for the SBOM
    * @param format The format (e.g., "spdx", "cyclonedx")
    */
   virtual void setFormat(const std::string& format) = 0;

   /**
    * @brief Set the CycloneDX specification version
    * @param version The CycloneDX version (e.g., "1.4", "1.5", "1.6")
    * @note Only applies when format is "cyclonedx"
    */
   virtual void setCycloneDXVersion(const std::string& version);

   /**
    * @brief Set the SPDX specification version
    * @param version The SPDX version (e.g., "2.3", "3.0")
    */
   virtual void setSPDXVersion(const std::string& version);

   /**
    * @brief Generate the SBOM
    */
   virtual void generateSBOM() = 0;

   /**
    * @brief Set verbose output mode
    * @param verbose true to enable verbose output
    */
   virtual void setVerbose(bool verbose) = 0;

   /**
    * @brief Set whether to extract debug information
    * @param extract true to extract debug information
    */
   virtual void setExtractDebugInfo(bool extract) = 0;

   /**
    * @brief Set whether to include system libraries
    * @param include true to include system libraries
    */
   virtual void setIncludeSystemLibraries(bool include) = 0;

   /**
    * @brief Set whether to recursively include transitive dependencies
    * @param transitive true to include transitive dependencies, false for direct only
    */
   virtual void setTransitiveDependencies(bool transitive) = 0;

   /**
    * @brief Get the number of components processed
    * @return Number of components
    */
   [[nodiscard]] virtual size_t getComponentCount() const = 0;

   /**
    * @brief Print statistics about the plugin
    */
   virtual void printStatistics() const = 0;

   protected:
   std::unique_ptr<SBOMGenerator> sbomGenerator;             ///< SBOM generator instance
   std::vector<ComponentInfo>     processedComponents;       ///< List of processed components
   bool                           verbose          = false;  ///< Verbose output flag
   bool                           extractDebugInfo = true;   ///< Debug info extraction flag
   bool includeSystemLibraries                     = false;  ///< System library inclusion flag

   /**
    * @brief Add a component to the processed list
    * @param component The component to add
    */
   void addComponent(const ComponentInfo& component);

   /**
    * @brief Update an existing component with new information
    * @param name The component name
    * @param filePath The file path
    * @param symbols The symbols to add
    */
   void updateComponent(const std::string& name, const std::string& filePath,
                        const std::vector<SymbolInfo>& symbols);

   /**
    * @brief Check if a file should be processed
    * @param filePath The file path to check
    * @return true if the file should be processed
    */
   [[nodiscard]] bool shouldProcessFile(const std::string& filePath) const;

   /**
    * @brief Extract component name from file path
    * @param filePath The file path
    * @return The extracted component name
    */
   [[nodiscard]] std::string extractComponentName(const std::string& filePath) const;
};

/**
 * @brief Plugin configuration structure
 */
struct PluginConfig
{
   std::string              outputPath             = "heimdall-sbom.json";  ///< Output file path
   std::string              format                 = "spdx";                ///< Output format
   std::string              cyclonedxVersion       = "1.6";  ///< CycloneDX specification version
   std::string              spdxVersion            = "3.0";  ///< SPDX specification version
   bool                     verbose                = false;  ///< Verbose output flag
   bool                     extractDebugInfo       = true;   ///< Debug info extraction flag
   bool                     includeSystemLibraries = false;  ///< System library inclusion flag
   bool                     generateChecksums      = true;   ///< Checksum generation flag
   bool                     extractMetadata        = true;   ///< Metadata extraction flag
   bool                     transitiveDependencies = true;
   std::vector<std::string> excludePatterns;  ///< File exclusion patterns
   std::vector<std::string> includePatterns;  ///< File inclusion patterns
};

/**
 * @brief Plugin statistics structure
 */
struct PluginStatistics
{
   size_t                    totalFiles          = 0;  ///< Total files processed
   size_t                    objectFiles         = 0;  ///< Object files processed
   size_t                    staticLibraries     = 0;  ///< Static libraries processed
   size_t                    sharedLibraries     = 0;  ///< Shared libraries processed
   size_t                    executables         = 0;  ///< Executables processed
   size_t                    systemLibraries     = 0;  ///< System libraries processed
   size_t                    totalSymbols        = 0;  ///< Total symbols extracted
   size_t                    processedComponents = 0;  ///< Components processed
   size_t                    skippedFiles        = 0;  ///< Files skipped
   std::chrono::milliseconds processingTime{0};        ///< Total processing time
};

/**
 * @brief Namespace containing common plugin utilities
 */
namespace PluginUtils
{

/**
 * @brief Check if a file is an object file
 * @param filePath The file path to check
 * @return true if the file is an object file
 */
bool isObjectFile(const std::string& filePath);

/**
 * @brief Check if a file is a static library
 * @param filePath The file path to check
 * @return true if the file is a static library
 */
bool isStaticLibrary(const std::string& filePath);

/**
 * @brief Check if a file is a shared library
 * @param filePath The file path to check
 * @return true if the file is a shared library
 */
bool isSharedLibrary(const std::string& filePath);

/**
 * @brief Check if a file is an executable
 * @param filePath The file path to check
 * @return true if the file is an executable
 */
bool isExecutable(const std::string& filePath);

/**
 * @brief Normalize a library path
 * @param libraryPath The library path to normalize
 * @return The normalized path
 */
std::string normalizeLibraryPath(const std::string& libraryPath);

/**
 * @brief Resolve a library name to its full path
 * @param libraryName The library name to resolve
 * @return The resolved library path
 */
std::string resolveLibraryPath(const std::string& libraryName);

/**
 * @brief Get the list of library search paths
 * @return Vector of library search paths
 */
std::vector<std::string> getLibrarySearchPaths();

/**
 * @brief Check if a symbol is a system symbol
 * @param symbolName The symbol name to check
 * @return true if the symbol is a system symbol
 */
bool isSystemSymbol(const std::string& symbolName);

/**
 * @brief Check if a symbol is a weak symbol
 * @param symbolName The symbol name to check
 * @return true if the symbol is a weak symbol
 */
bool isWeakSymbol(const std::string& symbolName);

/**
 * @brief Extract version information from a symbol name
 * @param symbolName The symbol name
 * @return The extracted version string
 */
std::string extractSymbolVersion(const std::string& symbolName);

/**
 * @brief Load configuration from a file
 * @param configPath The path to the configuration file
 * @param config The configuration structure to populate
 * @return true if loading was successful
 */
bool loadConfigFromFile(const std::string& configPath, PluginConfig& config);

/**
 * @brief Save configuration to a file
 * @param configPath The path to the configuration file
 * @param config The configuration to save
 * @return true if saving was successful
 */
bool saveConfigToFile(const std::string& configPath, const PluginConfig& config);

/**
 * @brief Parse command line options
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param config The configuration structure to populate
 * @return true if parsing was successful
 */
bool parseCommandLineOptions(int argc, char* argv[], PluginConfig& config);

/**
 * @brief Log an informational message
 * @param message The message to log
 */
void logInfo(const std::string& message);

/**
 * @brief Log a warning message
 * @param message The message to log
 */
void logWarning(const std::string& message);

/**
 * @brief Log an error message
 * @param message The message to log
 */
void logError(const std::string& message);

/**
 * @brief Log a debug message
 * @param message The message to log
 */
void logDebug(const std::string& message);

}  // namespace PluginUtils

}  // namespace heimdall
