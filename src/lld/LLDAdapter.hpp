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
 * @file LLDAdapter.hpp
 * @brief LLVM LLD linker adapter with full metadata extraction support
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <memory>
#include <string>

namespace heimdall {

/**
 * @brief LLVM LLD linker adapter with full metadata extraction support
 *
 * This class provides an adapter for the LLVM LLD linker,
 * enabling comprehensive SBOM generation during the linking process.
 * It includes full metadata extraction capabilities including DWARF debug
 * information, matching the functionality of the Gold adapter.
 */
class LLDAdapter {
public:
    /**
     * @brief Default constructor
     */
    LLDAdapter();

    /**
     * @brief Destructor
     */
    ~LLDAdapter();

    /**
     * @brief Initialize the LLD adapter
     */
    void initialize();

    /**
     * @brief Process an input file with full metadata extraction
     * @param filePath The path to the input file
     */
    void processInputFile(const std::string& filePath);

    /**
     * @brief Process a library with full metadata extraction
     * @param libraryPath The path to the library file
     */
    void processLibrary(const std::string& libraryPath);

    /**
     * @brief Finalize the LLD adapter and generate SBOM
     */
    void finalize();

    /**
     * @brief Set the output path for the generated SBOM
     * @param path The output file path
     */
    void setOutputPath(const std::string& path);

    /**
     * @brief Set the output format for the generated SBOM
     * @param format The output format (spdx, cyclonedx)
     */
    void setFormat(const std::string& format);

    /**
     * @brief Set the CycloneDX specification version
     * @param version The CycloneDX version (e.g., "1.4", "1.5", "1.6")
     * @note Only applies when format is "cyclonedx"
     */
    void setCycloneDXVersion(const std::string& version);

    /**
     * @brief Set the SPDX specification version
     * @param version The SPDX version (e.g., "2.3", "3.0")
     * @note Only applies when format is "spdx"
     */
    void setSPDXVersion(const std::string& version);

    /**
     * @brief Set verbose output mode
     * @param verbose Whether to enable verbose output
     */
    void setVerbose(bool verbose);

    /**
     * @brief Set whether to extract debug information
     * @param extract Whether to extract DWARF debug information
     */
    void setExtractDebugInfo(bool extract);

    /**
     * @brief Set whether to include system libraries
     * @param include Whether to include system libraries in SBOM
     */
    void setIncludeSystemLibraries(bool include);

    /**
     * @brief Get the number of components processed
     * @return The number of components in the SBOM
     */
    size_t getComponentCount() const;

    /**
     * @brief Print statistics about the processed components
     */
    void printStatistics() const;

private:
    /**
     * @brief Implementation class for LLD adapter
     */
    class Impl;

    std::unique_ptr<Impl> pImpl;  ///< Implementation pointer
};

}  // namespace heimdall
