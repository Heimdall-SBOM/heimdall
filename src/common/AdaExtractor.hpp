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
 * @brief Ada ALI file parser for extracting metadata from Ada applications
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include "ComponentInfo.hpp"
#include "../compat/compatibility.hpp"

namespace heimdall {

/**
 * @brief Structure representing Ada package information
 */
struct AdaPackageInfo {
    std::string name;                    ///< Package name
    std::string sourceFile;              ///< Source file (.ads/.adb)
    std::string aliFile;                 ///< ALI file path
    std::string checksum;                ///< File checksum
    std::string timestamp;               ///< File timestamp
    std::vector<std::string> functions;  ///< List of functions/procedures
    std::vector<std::string> variables;  ///< List of variables
    std::vector<std::string> types;      ///< List of types
    std::vector<std::string> dependencies; ///< Package dependencies
    bool isSpecification = false;        ///< Whether this is a spec (.ads) or body (.adb)
    bool isRuntime = false;              ///< Whether this is a runtime package
};

/**
 * @brief Structure representing Ada function/procedure information
 */
struct AdaFunctionInfo {
    std::string name;                    ///< Function name
    std::string package;                 ///< Package containing the function
    std::string signature;               ///< Function signature with parameters
    std::string returnType;              ///< Return type (if any)
    std::vector<std::string> parameters; ///< Parameter types
    bool isPublic = false;               ///< Whether the function is public
    bool isProcedure = false;            ///< Whether this is a procedure (no return)
    std::vector<std::string> calls;      ///< Functions this function calls
    std::string lineNumber;              ///< Line number in source file
    std::string columnNumber;            ///< Column number in source file
};

/**
 * @brief Structure representing Ada build configuration
 */
struct AdaBuildInfo {
    std::string compilerVersion;         ///< GNAT compiler version
    std::vector<std::string> runtimeFlags; ///< Runtime configuration flags
    std::vector<std::string> compilationFlags; ///< Compilation flags
    std::string targetArchitecture;      ///< Target architecture
    std::string buildTimestamp;          ///< Build timestamp
    std::map<std::string, std::string> fileTimestamps; ///< File timestamps
    std::map<std::string, std::string> fileChecksums;  ///< File checksums
    std::vector<std::string> securityFlags; ///< Security-related build flags
    std::vector<std::string> optimizationFlags; ///< Optimization flags
};

/**
 * @brief Structure representing Ada cross-reference information
 */
struct AdaCrossReference {
    std::string callerFunction;          ///< Function making the call
    std::string callerPackage;           ///< Package containing caller
    std::string calledFunction;          ///< Function being called
    std::string calledPackage;           ///< Package containing called function
    std::string callerLine;              ///< Line number in caller
    std::string calledLine;              ///< Line number in called function
    std::string relationship;            ///< Type of relationship (calls, uses, etc.)
};

/**
 * @brief Structure representing Ada type information
 */
struct AdaTypeInfo {
    std::string name;                    ///< Type name
    std::string package;                 ///< Package containing the type
    std::string baseType;                ///< Base type (if derived)
    std::vector<std::string> components; ///< Record components
    std::string size;                    ///< Type size in bits
    std::string alignment;               ///< Type alignment
    bool isPrivate = false;              ///< Whether type is private
    bool isLimited = false;              ///< Whether type is limited
    std::string lineNumber;              ///< Line number in source file
};

/**
 * @brief Ada ALI file parser for extracting metadata from Ada applications
 *
 * This class provides functionality to parse Ada ALI files and extract
 * comprehensive metadata including package dependencies, function signatures,
 * build configuration, runtime information, cross-references, and type details.
 */
class AdaExtractor {
public:
    /**
     * @brief Default constructor
     */
    AdaExtractor();

    /**
     * @brief Destructor
     */
    ~AdaExtractor();

    /**
     * @brief Extract Ada metadata from ALI files
     * @param component The component to extract metadata from
     * @param aliFiles Vector of ALI file paths to parse
     * @return true if extraction was successful
     */
    bool extractAdaMetadata(ComponentInfo& component, 
                          const std::vector<std::string>& aliFiles);

    /**
     * @brief Parse a single ALI file
     * @param aliFilePath Path to the ALI file
     * @param packageInfo Output package information
     * @return true if parsing was successful
     */
    bool parseAliFile(const std::string& aliFilePath, AdaPackageInfo& packageInfo);

    /**
     * @brief Extract dependencies from ALI file content
     * @param content ALI file content as string
     * @param dependencies Output vector of dependencies
     * @return true if extraction was successful
     */
    bool extractDependencies(const std::string& content, 
                           std::vector<std::string>& dependencies);

    /**
     * @brief Extract functions from ALI file content
     * @param content ALI file content as string
     * @param functions Output vector of function information
     * @return true if extraction was successful
     */
    bool extractFunctions(const std::string& content, 
                         std::vector<AdaFunctionInfo>& functions);

    /**
     * @brief Extract build configuration from ALI file content
     * @param content ALI file content as string
     * @param buildInfo Output build information
     * @return true if extraction was successful
     */
    bool extractBuildInfo(const std::string& content, AdaBuildInfo& buildInfo);

    /**
     * @brief Extract cross-references from ALI file content
     * @param content ALI file content as string
     * @param crossRefs Output vector of cross-reference information
     * @return true if extraction was successful
     */
    bool extractCrossReferences(const std::string& content,
                              std::vector<AdaCrossReference>& crossRefs);

    /**
     * @brief Extract type information from ALI file content
     * @param content ALI file content as string
     * @param types Output vector of type information
     * @return true if extraction was successful
     */
    bool extractTypeInfo(const std::string& content,
                        std::vector<AdaTypeInfo>& types);

    /**
     * @brief Extract security-related build flags
     * @param content ALI file content as string
     * @param securityFlags Output vector of security flags
     * @return true if extraction was successful
     */
    bool extractSecurityFlags(const std::string& content,
                            std::vector<std::string>& securityFlags);

    /**
     * @brief Extract file timestamps and checksums
     * @param content ALI file content as string
     * @param timestamps Output map of file timestamps
     * @param checksums Output map of file checksums
     * @return true if extraction was successful
     */
    bool extractFileInfo(const std::string& content,
                        std::map<std::string, std::string>& timestamps,
                        std::map<std::string, std::string>& checksums);

    /**
     * @brief Generate function call graph from cross-references
     * @param crossRefs Vector of cross-references
     * @return String representation of call graph
     */
    std::string generateCallGraph(const std::vector<AdaCrossReference>& crossRefs);

    /**
     * @brief Check if a file is an Ada ALI file
     * @param filePath Path to the file
     * @return true if the file is an ALI file
     */
    bool isAliFile(const std::string& filePath);

    /**
     * @brief Find ALI files in a directory
     * @param directory Directory to search
     * @param aliFiles Output vector of ALI file paths
     * @return true if search was successful
     */
    bool findAliFiles(const std::string& directory, 
                     std::vector<std::string>& aliFiles);

    /**
     * @brief Set verbose output mode
     * @param verbose true to enable verbose output
     */
    void setVerbose(bool verbose);

    /**
     * @brief Set whether to extract runtime packages
     * @param extract true to extract runtime packages
     */
    void setExtractRuntimePackages(bool extract);

    /**
     * @brief Set whether to extract enhanced metadata
     * @param extract true to extract enhanced metadata (cross-refs, types, etc.)
     */
    void setExtractEnhancedMetadata(bool extract);

private:
    /**
     * @brief Parse ALI file version line
     * @param line The version line
     * @param buildInfo Build info to update
     * @return true if parsing was successful
     */
    bool parseVersionLine(const std::string& line, AdaBuildInfo& buildInfo);

    /**
     * @brief Parse ALI file dependency lines
     * @param line The dependency line
     * @param packageInfo Package info to update
     * @return true if parsing was successful
     */
    bool parseDependencyLine(const std::string& line, AdaPackageInfo& packageInfo);

    /**
     * @brief Parse ALI file function lines
     * @param line The function line
     * @param functions Functions vector to update
     * @return true if parsing was successful
     */
    bool parseFunctionLine(const std::string& line, 
                          std::vector<AdaFunctionInfo>& functions);

    /**
     * @brief Parse ALI file variable lines
     * @param line The variable line
     * @param variables Variables vector to update
     * @return true if parsing was successful
     */
    bool parseVariableLine(const std::string& line, 
                          std::vector<std::string>& variables);

    /**
     * @brief Parse ALI file type lines
     * @param line The type line
     * @param types Types vector to update
     * @return true if parsing was successful
     */
    bool parseTypeLine(const std::string& line, 
                      std::vector<std::string>& types);

    /**
     * @brief Parse ALI file cross-reference lines (G lines)
     * @param line The cross-reference line
     * @param crossRefs Cross-references vector to update
     * @return true if parsing was successful
     */
    bool parseCrossReferenceLine(const std::string& line,
                               std::vector<AdaCrossReference>& crossRefs);

    /**
     * @brief Parse ALI file build flag lines (RV lines)
     * @param line The build flag line
     * @param buildInfo Build info to update
     * @return true if parsing was successful
     */
    bool parseBuildFlagLine(const std::string& line, AdaBuildInfo& buildInfo);

    /**
     * @brief Parse ALI file file info lines (D lines)
     * @param line The file info line
     * @param timestamps Timestamps map to update
     * @param checksums Checksums map to update
     * @return true if parsing was successful
     */
    bool parseFileInfoLine(const std::string& line,
                          std::map<std::string, std::string>& timestamps,
                          std::map<std::string, std::string>& checksums);

    /**
     * @brief Check if a package is a runtime package
     * @param packageName Package name to check
     * @return true if it's a runtime package
     */
    bool isRuntimePackage(const std::string& packageName);

    /**
     * @brief Extract package name from ALI file path
     * @param aliFilePath ALI file path
     * @return Package name
     */
    std::string extractPackageName(const std::string& aliFilePath);

    /**
     * @brief Extract source file path from ALI file path
     * @param aliFilePath ALI file path
     * @return Source file path
     */
    std::string extractSourceFilePath(const std::string& aliFilePath);

    /**
     * @brief Check if a build flag is security-related
     * @param flag Build flag to check
     * @return true if it's a security-related flag
     */
    bool isSecurityFlag(const std::string& flag);

    /**
     * @brief Check if a build flag is optimization-related
     * @param flag Build flag to check
     * @return true if it's an optimization flag
     */
    bool isOptimizationFlag(const std::string& flag);

    bool verbose = false;                    ///< Verbose output flag
    bool extractRuntimePackages = false;     ///< Whether to extract runtime packages
    bool extractEnhancedMetadata = false;    ///< Whether to extract enhanced metadata
    std::vector<std::string> runtimePackages; ///< List of known runtime packages
    std::set<std::string> securityFlags;    ///< Set of known security-related flags
    std::set<std::string> optimizationFlags; ///< Set of known optimization flags
};

} // namespace heimdall 