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
 * @file AdaExtractor.hpp
 * @brief Ada ALI file extractor implementation
 * @author Trevor Bakker
 * @date 2025
 * @version 1.0.0
 *
 * This file provides the AdaExtractor class which implements the IBinaryExtractor
 * interface for extracting metadata from Ada ALI (Ada Library Information) files.
 * It supports comprehensive Ada metadata extraction including package information,
 * dependencies, functions, types, and build configuration.
 *
 * Features:
 * - Ada package information extraction
 * - Function and procedure extraction
 * - Type information extraction
 * - Dependency analysis
 * - Build configuration extraction
 * - Cross-reference analysis
 * - Security and optimization flag detection
 * - Runtime package identification
 *
 * Dependencies:
 * - IBinaryExtractor interface
 * - ComponentInfo structure
 * - File system utilities
 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../interfaces/IBinaryExtractor.hpp"

namespace heimdall
{

/**
 * @brief Structure representing Ada package information
 */
struct AdaPackageInfo
{
   std::string              name;          ///< Package name
   std::string              sourceFile;    ///< Source file (.ads/.adb)
   std::string              aliFile;       ///< ALI file path
   std::string              checksum;      ///< File checksum
   std::string              timestamp;     ///< File timestamp
   std::vector<std::string> functions;     ///< List of functions/procedures
   std::vector<std::string> variables;     ///< List of variables
   std::vector<std::string> types;         ///< List of types
   std::vector<std::string> dependencies;  ///< Package dependencies
   bool isSpecification = false;           ///< Whether this is a spec (.ads) or body (.adb)
   bool isRuntime       = false;           ///< Whether this is a runtime package
};

/**
 * @brief Structure representing Ada function/procedure information
 */
struct AdaFunctionInfo
{
   std::string              name;                 ///< Function name
   std::string              package;              ///< Package containing the function
   std::string              signature;            ///< Function signature with parameters
   std::string              returnType;           ///< Return type (if any)
   std::vector<std::string> parameters;           ///< Parameter types
   bool                     isPublic    = false;  ///< Whether the function is public
   bool                     isProcedure = false;  ///< Whether this is a procedure (no return)
   std::vector<std::string> calls;                ///< Functions this function calls
   std::string              lineNumber;           ///< Line number in source file
   std::string              columnNumber;         ///< Column number in source file
};

/**
 * @brief Structure representing Ada build configuration
 */
struct AdaBuildInfo
{
   std::string                        compilerVersion;     ///< GNAT compiler version
   std::vector<std::string>           runtimeFlags;        ///< Runtime configuration flags
   std::vector<std::string>           compilationFlags;    ///< Compilation flags
   std::string                        targetArchitecture;  ///< Target architecture
   std::string                        buildTimestamp;      ///< Build timestamp
   std::map<std::string, std::string> fileTimestamps;      ///< File timestamps
   std::map<std::string, std::string> fileChecksums;       ///< File checksums
   std::vector<std::string>           securityFlags;       ///< Security-related build flags
   std::vector<std::string>           optimizationFlags;   ///< Optimization flags
};

/**
 * @brief Structure representing Ada cross-reference information
 */
struct AdaCrossReference
{
   std::string callerFunction;  ///< Function making the call
   std::string callerPackage;   ///< Package containing caller
   std::string calledFunction;  ///< Function being called
   std::string calledPackage;   ///< Package containing called function
   std::string callerLine;      ///< Line number in caller
   std::string calledLine;      ///< Line number in called function
   std::string relationship;    ///< Type of relationship (calls, uses, etc.)
};

/**
 * @brief Structure representing Ada type information
 */
struct AdaTypeInfo
{
   std::string              name;               ///< Type name
   std::string              package;            ///< Package containing the type
   std::string              baseType;           ///< Base type (if derived)
   std::vector<std::string> components;         ///< Record components
   std::string              size;               ///< Type size in bits
   std::string              alignment;          ///< Type alignment
   bool                     isPrivate = false;  ///< Whether type is private
   bool                     isLimited = false;  ///< Whether type is limited
   std::string              lineNumber;         ///< Line number in source file
};

/**
 * @brief Ada ALI file extractor implementation
 *
 * This class provides comprehensive Ada ALI file analysis capabilities,
 * implementing the IBinaryExtractor interface. It supports extraction
 * of package information, functions, types, dependencies, and build
 * configuration from Ada ALI files.
 *
 * The extractor can process individual ALI files or scan directories
 * for ALI files and extract metadata from all found files.
 */
class AdaExtractor : public IBinaryExtractor
{
   public:
   /**
    * @brief Default constructor
    */
   AdaExtractor();

   /**
    * @brief Destructor
    */
   ~AdaExtractor() override;

   /**
    * @brief Copy constructor
    * @param other The AdaExtractor to copy from
    */
   AdaExtractor(const AdaExtractor& other);

   /**
    * @brief Move constructor
    * @param other The AdaExtractor to move from
    */
   AdaExtractor(AdaExtractor&& other) noexcept;

   /**
    * @brief Copy assignment operator
    * @param other The AdaExtractor to copy from
    * @return Reference to this AdaExtractor
    */
   AdaExtractor& operator=(const AdaExtractor& other);

   /**
    * @brief Move assignment operator
    * @param other The AdaExtractor to move from
    * @return Reference to this AdaExtractor
    */
   AdaExtractor& operator=(AdaExtractor&& other) noexcept;

   // IBinaryExtractor interface implementation
   /**
    * @brief Extract symbol information from Ada ALI file
    * @param filePath Path to the ALI file
    * @param symbols Output vector to store extracted symbols (functions/procedures)
    * @return true if symbols were successfully extracted
    */
   bool extractSymbols(const std::string& filePath, std::vector<SymbolInfo>& symbols) override;

   /**
    * @brief Extract section information from Ada ALI file
    * @param filePath Path to the ALI file
    * @param sections Output vector to store extracted sections (packages)
    * @return true if sections were successfully extracted
    */
   bool extractSections(const std::string& filePath, std::vector<SectionInfo>& sections) override;

   /**
    * @brief Extract version information from Ada ALI file
    * @param filePath Path to the ALI file
    * @param version Output string to store version information
    * @return true if version was successfully extracted
    */
   bool extractVersion(const std::string& filePath, std::string& version) override;

   /**
    * @brief Extract dependency information from Ada binary file
    *
    * @param filePath Path to the Ada binary file
    * @return Vector of dependency strings (library names)
    */
   std::vector<std::string> extractDependencies(const std::string& filePath) override;

   /**
    * @brief Extract function names from DWARF debug information
    *
    * @param filePath Path to the Ada binary file
    * @param functions Output vector to store extracted function names
    * @return true if functions were successfully extracted
    * @return false if extraction failed or no functions found
    */
   bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions) override;

   /**
    * @brief Extract compile unit information from DWARF debug information
    *
    * @param filePath Path to the Ada binary file
    * @param compileUnits Output vector to store extracted compile unit names
    * @return true if compile units were successfully extracted
    * @return false if extraction failed or no compile units found
    */
   bool extractCompileUnits(const std::string&        filePath,
                            std::vector<std::string>& compileUnits) override;

   /**
    * @brief Extract source file information from DWARF debug information
    *
    * @param filePath Path to the Ada binary file
    * @param sourceFiles Output vector to store extracted source file names
    * @return true if source files were successfully extracted
    * @return false if extraction failed or no source files found
    */
   bool extractSourceFiles(const std::string&        filePath,
                           std::vector<std::string>& sourceFiles) override;

   /**
    * @brief Check if the extractor can handle the given file format
    * @param filePath Path to the file
    * @return true if this extractor can process the file
    */
   bool canHandle(const std::string& filePath) const override;

   /**
    * @brief Get the name of the binary format this extractor handles
    * @return String identifier for the binary format
    */
   std::string getFormatName() const override;

   /**
    * @brief Get the priority of this extractor
    * @return Priority value (higher values have higher priority)
    */
   int getPriority() const override;

   // Ada-specific methods
   /**
    * @brief Extract comprehensive Ada metadata from ALI files
    * @param component ComponentInfo to populate with Ada metadata
    * @param aliFiles Vector of ALI file paths to process
    * @return true if metadata was successfully extracted
    */
   bool extractAdaMetadata(ComponentInfo& component, const std::vector<std::string>& aliFiles);

   /**
    * @brief Parse a single ALI file and extract package information
    * @param aliFilePath Path to the ALI file
    * @param packageInfo Output package information
    * @return true if package information was successfully extracted
    */
   bool parseAliFile(const std::string& aliFilePath, AdaPackageInfo& packageInfo);

   /**
    * @brief Find ALI files in a directory
    * @param directory Directory to search
    * @param aliFiles Output vector of found ALI file paths
    * @return true if ALI files were found
    */
   bool findAliFiles(const std::string& directory, std::vector<std::string>& aliFiles);

   /**
    * @brief Check if a file is an ALI file
    * @param filePath Path to the file
    * @return true if the file is an ALI file
    */
   bool isAliFile(const std::string& filePath) const;

   /**
    * @brief Extract dependencies from ALI file content
    * @param content ALI file content
    * @param dependencies Output vector of dependencies
    * @return true if dependencies were successfully extracted
    */
   bool extractDependencies(const std::string& content, std::vector<std::string>& dependencies);
   bool extractSourceFilesFromContent(const std::string& content, std::string& sourceFile);

   /**
    * @brief Extract functions from ALI file content
    * @param content ALI file content
    * @param functions Output vector of function information
    * @return true if functions were successfully extracted
    */
   bool extractFunctions(const std::string& content, std::vector<AdaFunctionInfo>& functions);

   /**
    * @brief Extract build information from ALI file content
    * @param content ALI file content
    * @param buildInfo Output build information
    * @return true if build information was successfully extracted
    */
   bool extractBuildInfo(const std::string& content, AdaBuildInfo& buildInfo);

   /**
    * @brief Extract cross-references from ALI file content
    * @param content ALI file content
    * @param crossRefs Output vector of cross-reference information
    * @return true if cross-references were successfully extracted
    */
   bool extractCrossReferences(const std::string&              content,
                               std::vector<AdaCrossReference>& crossRefs);

   /**
    * @brief Extract type information from ALI file content
    * @param content ALI file content
    * @param types Output vector of type information
    * @return true if type information was successfully extracted
    */
   bool extractTypeInfo(const std::string& content, std::vector<AdaTypeInfo>& types);

   /**
    * @brief Extract security flags from ALI file content
    * @param content ALI file content
    * @param securityFlags Output vector of security flags
    * @return true if security flags were successfully extracted
    */
   bool extractSecurityFlags(const std::string& content, std::vector<std::string>& securityFlags);

   /**
    * @brief Extract file information from ALI file content
    * @param content ALI file content
    * @param timestamps Output map of file timestamps
    * @param checksums Output map of file checksums
    * @return true if file information was successfully extracted
    */
   bool extractFileInfo(const std::string& content, std::map<std::string, std::string>& timestamps,
                        std::map<std::string, std::string>& checksums);

   /**
    * @brief Generate call graph from cross-references
    * @param crossRefs Vector of cross-reference information
    * @return Generated call graph as string
    */
   std::string generateCallGraph(const std::vector<AdaCrossReference>& crossRefs);

   /**
    * @brief Check if a package is a runtime package
    * @param packageName Package name to check
    * @return true if the package is a runtime package
    */
   bool isRuntimePackage(const std::string& packageName) const;

   /**
    * @brief Extract package name from ALI file path
    * @param aliFilePath Path to the ALI file
    * @return Extracted package name
    */
   std::string extractPackageName(const std::string& aliFilePath) const;

   /**
    * @brief Extract source file path from ALI file path
    * @param aliFilePath Path to the ALI file
    * @return Extracted source file path
    */
   std::string extractSourceFilePath(const std::string& aliFilePath) const;

   /**
    * @brief Check if a flag is a security flag
    * @param flag Flag to check
    * @return true if the flag is a security flag
    */
   bool isSecurityFlag(const std::string& flag) const;

   /**
    * @brief Check if a flag is an optimization flag
    * @param flag Flag to check
    * @return true if the flag is an optimization flag
    */
   bool isOptimizationFlag(const std::string& flag) const;

   // Configuration methods
   /**
    * @brief Set verbose output mode
    * @param verbose Whether to enable verbose output
    */
   void setVerbose(bool verbose);

   /**
    * @brief Set whether to extract runtime packages
    * @param extract Whether to extract runtime packages
    */
   void setExcludeRuntimePackages(bool exclude);

   /**
    * @brief Set whether to extract enhanced metadata
    * @param extract Whether to extract enhanced metadata
    */
   void setExtractEnhancedMetadata(bool extract);

   /**
    * @brief Set test mode (for unit testing)
    * @param enabled Whether to enable test mode
    */
   static void setTestMode(bool enabled);

   /**
    * @brief Check if test mode is enabled
    * @return true if test mode is enabled
    */
   static bool isTestMode();

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   // Private helper methods
   /**
    * @brief Parse version line from ALI file
    * @param line Line to parse
    * @param buildInfo Build info to populate
    * @return true if version was successfully parsed
    */
   bool parseVersionLine(const std::string& line, AdaBuildInfo& buildInfo);

   /**
    * @brief Parse dependency line from ALI file
    * @param line Line to parse
    * @param packageInfo Package info to populate
    * @return true if dependency was successfully parsed
    */
   bool parseDependencyLine(const std::string& line, AdaPackageInfo& packageInfo);

   /**
    * @brief Parse function line from ALI file
    * @param line Line to parse
    * @param functions Functions vector to populate
    * @return true if function was successfully parsed
    */
   bool parseFunctionLine(const std::string& line, std::vector<AdaFunctionInfo>& functions);

   /**
    * @brief Parse variable line from ALI file
    * @param line Line to parse
    * @param variables Variables vector to populate
    * @return true if variable was successfully parsed
    */
   bool parseVariableLine(const std::string& line, std::vector<std::string>& variables);

   /**
    * @brief Parse type line from ALI file
    * @param line Line to parse
    * @param types Types vector to populate
    * @return true if type was successfully parsed
    */
   bool parseTypeLine(const std::string& line, std::vector<std::string>& types);

   /**
    * @brief Parse cross-reference line from ALI file
    * @param line Line to parse
    * @param crossRefs Cross-references vector to populate
    * @return true if cross-reference was successfully parsed
    */
   bool parseCrossReferenceLine(const std::string& line, std::vector<AdaCrossReference>& crossRefs);

   /**
    * @brief Parse build flag line from ALI file
    * @param line Line to parse
    * @param buildInfo Build info to populate
    * @return true if build flag was successfully parsed
    */
   bool parseBuildFlagLine(const std::string& line, AdaBuildInfo& buildInfo);

   /**
    * @brief Parse file info line from ALI file
    * @param line Line to parse
    * @param timestamps Timestamps map to populate
    * @param checksums Checksums map to populate
    * @return true if file info was successfully parsed
    */
   bool parseFileInfoLine(const std::string& line, std::map<std::string, std::string>& timestamps,
                          std::map<std::string, std::string>& checksums);
};

}  // namespace heimdall