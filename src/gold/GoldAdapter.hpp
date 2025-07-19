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
 * @file GoldAdapter.hpp
 * @brief GNU Gold linker plugin adapter
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../common/MetadataExtractor.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/Utils.hpp"

namespace heimdall {

/**
 * @brief GNU Gold linker plugin adapter
 *
 * This class provides basic file processing capabilities for the GNU Gold linker,
 * without using LLVM-dependent heimdall-core classes.
 */
class GoldAdapter {
public:
    /**
     * @brief Default constructor
     */
    GoldAdapter();

    /**
     * @brief Destructor
     */
    ~GoldAdapter();

    /**
     * @brief Initialize the Gold adapter
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Clean up Gold adapter resources
     */
    void cleanup();

    /**
     * @brief Process an input file
     * @param filePath The path to the input file
     */
    void processInputFile(const std::string& filePath);

    /**
     * @brief Process a library file
     * @param libraryPath The path to the library file
     */
    void processLibrary(const std::string& libraryPath);

    /**
     * @brief Process a symbol
     * @param symbolName The name of the symbol
     * @param address The symbol address
     * @param size The symbol size
     */
    void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size);

    /**
     * @brief Set the output path for the SBOM
     * @param path The output file path
     */
    void setOutputPath(const std::string& path);

    /**
     * @brief Set the output format for the SBOM
     * @param format The format (e.g., "spdx", "cyclonedx")
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
     * @brief Generate the SBOM
     */
    void generateSBOM();

    /**
     * @brief Set verbose output mode
     * @param verbose true to enable verbose output
     */
    void setVerbose(bool verbose);

    /**
     * @brief Set whether to extract debug information
     * @param extract true to extract debug information
     */
    void setExtractDebugInfo(bool extract);

    /**
     * @brief Set whether to include system libraries
     * @param include true to include system libraries
     */
    void setIncludeSystemLibraries(bool include);

    /**
     * @brief Set whether to suppress warnings (for test mode)
     * @param suppress true to suppress warnings
     */
    void setSuppressWarnings(bool suppress);

    /**
     * @brief Get the number of components processed
     * @return Number of components
     */
    [[nodiscard]] size_t getComponentCount() const;

    /**
     * @brief Print statistics about the Gold adapter
     */
    void printStatistics() const;

    /**
     * @brief Finalize the Gold adapter
     */
    void finalize();

    /**
     * @brief Get the list of processed files
     * @return Vector of processed file paths
     */
    [[nodiscard]] std::vector<std::string> getProcessedFiles() const;

    /**
     * @brief Get the list of processed libraries
     * @return Vector of processed library paths
     */
    [[nodiscard]] std::vector<std::string> getProcessedLibraries() const;

    /**
     * @brief Get the list of processed symbols
     * @return Vector of processed symbol names
     */
    [[nodiscard]] std::vector<std::string> getProcessedSymbols() const;

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

    /**
     * @brief Process multiple files in parallel
     * @param filePaths Vector of file paths to process
     */
    void processFilesParallel(const std::vector<std::string>& filePaths);

private:
    class Impl {
    public:
        Impl();
        ~Impl();
        bool initialize();
        void cleanup();
        void processInputFile(const std::string& filePath);
        void processLibrary(const std::string& libraryPath);
        void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size);
        void setOutputPath(const std::string& path);
        void setFormat(const std::string& fmt);
        void setCycloneDXVersion(const std::string& version);
        void setSPDXVersion(const std::string& version);
        void generateSBOM();
        void setVerbose(bool verbose);
        void setExtractDebugInfo(bool extract);
        void setIncludeSystemLibraries(bool include);
        void setSuppressWarnings(bool suppress);
        [[nodiscard]] size_t getComponentCount() const;
        void printStatistics() const;
        void processFilesParallel(const std::vector<std::string>& filePaths);

        // Getter methods for accessing private members
        const std::vector<std::string>& getProcessedFiles() const { return processedFiles; }
        const std::vector<std::string>& getProcessedLibraries() const { return processedLibraries; }

    private:
        std::unique_ptr<SBOMGenerator> sbomGenerator;
        std::vector<std::string> processedFiles;
        std::vector<std::string> processedLibraries;
        std::string outputPath;
        std::string format;
        std::string cyclonedxVersion;
        bool verbose = false;
        bool extractDebugInfo = true;
        bool includeSystemLibraries = false;
        bool suppressWarnings = false;
    };
    std::unique_ptr<Impl> pImpl;
};

}  // namespace heimdall
