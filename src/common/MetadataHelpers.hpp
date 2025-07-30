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
 * @file MetadataHelpers.hpp
 * @brief Helper functions for metadata extraction
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include "ComponentInfo.hpp"

namespace heimdall
{

/**
 * @brief Helper namespace for metadata extraction functions
 *
 * This namespace provides helper functions that are used by tests
 * and provide backward compatibility with the old API.
 */
namespace MetadataHelpers
{
/**
 * @brief Check if a file is an ELF binary
 * @param filePath Path to the file to check
 * @return true if the file is an ELF binary
 */
bool isELF(const std::string& filePath);

/**
 * @brief Check if a file is a Mach-O binary
 * @param filePath Path to the file to check
 * @return true if the file is a Mach-O binary
 */
bool isMachO(const std::string& filePath);

/**
 * @brief Check if a file is a PE binary
 * @param filePath Path to the file to check
 * @return true if the file is a PE binary
 */
bool isPE(const std::string& filePath);

/**
 * @brief Extract source files from DWARF debug information
 * @param filePath Path to the binary file
 * @param sourceFiles Vector to populate with source file paths
 * @return true if extraction was successful
 */
bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles);

/**
 * @brief Extract compile units from DWARF debug information
 * @param filePath Path to the binary file
 * @param compileUnits Vector to populate with compile unit names
 * @return true if extraction was successful
 */
bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits);

/**
 * @brief Extract debug information from a file
 * @param filePath Path to the binary file
 * @param component Component to populate with debug information
 * @return true if extraction was successful
 */
bool extractDebugInfo(const std::string& filePath, ComponentInfo& component);

/**
 * @brief Extract ELF build ID from a file
 * @param filePath Path to the ELF file
 * @param buildId String to populate with build ID
 * @return true if extraction was successful
 */
bool extractELFBuildId(const std::string& filePath, std::string& buildId);

/**
 * @brief Set test mode for metadata extraction
 * @param enabled Whether to enable test mode
 */
void setTestMode(bool enabled);

}  // namespace MetadataHelpers

}  // namespace heimdall