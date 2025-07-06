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
#include "../common/SBOMGenerator.hpp"
#include "../common/ComponentInfo.hpp"
#include "../common/MetadataExtractor.hpp"
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
     * @brief Get the number of components processed
     * @return Number of components
     */
    size_t getComponentCount() const;
    
    /**
     * @brief Print statistics about the Gold adapter
     */
    void printStatistics() const;
    
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
        void setFormat(const std::string& format);
        void generateSBOM();
        void setVerbose(bool verbose);
        void setExtractDebugInfo(bool extract);
        void setIncludeSystemLibraries(bool include);
        size_t getComponentCount() const;
        void printStatistics() const;
    private:
        std::unique_ptr<SBOMGenerator> sbomGenerator;
        std::vector<std::string> processedFiles;
        std::vector<std::string> processedLibraries;
        std::string outputPath;
        std::string format;
        bool verbose = false;
        bool extractDebugInfo = true;
        bool includeSystemLibraries = false;
    };
    std::unique_ptr<Impl> pImpl;
};

} // namespace heimdall
