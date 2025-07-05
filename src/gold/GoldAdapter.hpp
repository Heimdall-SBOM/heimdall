/**
 * @file GoldAdapter.hpp
 * @brief GNU Gold linker plugin adapter
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include "../common/PluginInterface.hpp"
#include "../common/ComponentInfo.hpp"
#include "../common/SBOMGenerator.hpp"
#include <memory>
#include <string>
#include <vector>

namespace heimdall {

/**
 * @brief GNU Gold linker plugin adapter
 * 
 * This class implements the PluginInterface for the GNU Gold linker,
 * providing SBOM generation capabilities during the linking process.
 */
class GoldAdapter : public PluginInterface {
public:
    /**
     * @brief Default constructor
     */
    GoldAdapter();
    
    /**
     * @brief Destructor
     */
    ~GoldAdapter() override;
    
    /**
     * @brief Initialize the Gold adapter
     * @return true if initialization was successful
     */
    bool initialize() override;
    
    /**
     * @brief Clean up Gold adapter resources
     */
    void cleanup() override;
    
    /**
     * @brief Process an input file
     * @param filePath The path to the input file
     */
    void processInputFile(const std::string& filePath) override;
    
    /**
     * @brief Process a library file
     * @param libraryPath The path to the library file
     */
    void processLibrary(const std::string& libraryPath) override;
    
    /**
     * @brief Process a symbol
     * @param symbolName The name of the symbol
     * @param address The symbol address
     * @param size The symbol size
     */
    void processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) override;
    
    /**
     * @brief Set the output path for the SBOM
     * @param path The output file path
     */
    void setOutputPath(const std::string& path) override;
    
    /**
     * @brief Set the output format for the SBOM
     * @param format The format (e.g., "spdx", "cyclonedx")
     */
    void setFormat(const std::string& format) override;
    
    /**
     * @brief Generate the SBOM
     */
    void generateSBOM() override;
    
    /**
     * @brief Set verbose output mode
     * @param verbose true to enable verbose output
     */
    void setVerbose(bool verbose) override;
    
    /**
     * @brief Set whether to extract debug information
     * @param extract true to extract debug information
     */
    void setExtractDebugInfo(bool extract) override;
    
    /**
     * @brief Set whether to include system libraries
     * @param include true to include system libraries
     */
    void setIncludeSystemLibraries(bool include) override;
    
    /**
     * @brief Get the number of components processed
     * @return Number of components
     */
    size_t getComponentCount() const override;
    
    /**
     * @brief Print statistics about the Gold adapter
     */
    void printStatistics() const override;
    
private:
    /**
     * @brief Implementation class for Gold adapter
     */
    class Impl {
    public:
        /**
         * @brief Default constructor
         */
        Impl();
        
        /**
         * @brief Destructor
         */
        ~Impl();
        
        /**
         * @brief Initialize the implementation
         * @return true if initialization was successful
         */
        bool initialize();
        
        /**
         * @brief Clean up implementation resources
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
         * @brief Print statistics about the implementation
         */
        void printStatistics() const;
        
        /**
         * @brief Handle Gold plugin events
         * @param pluginName The name of the plugin
         * @param pluginOpt The plugin options
         */
        void handleGoldPlugin(const char* pluginName, const char* pluginOpt);
        
        /**
         * @brief Process a Gold input file
         * @param filePath The path to the input file
         */
        void processGoldInputFile(const char* filePath);
        
        /**
         * @brief Process a Gold library
         * @param libraryPath The path to the library
         */
        void processGoldLibrary(const char* libraryPath);
        
        /**
         * @brief Extract symbols from a file
         * @param filePath The path to the file
         */
        void extractSymbolsFromFile(const std::string& filePath);
        
        /**
         * @brief Process an archive file
         * @param filePath The path to the archive file
         */
        void processArchiveFile(const std::string& filePath);
        
        /**
         * @brief Process an object file
         * @param filePath The path to the object file
         */
        void processObjectFile(const std::string& filePath);
        
        /**
         * @brief Process a shared library
         * @param filePath The path to the shared library
         */
        void processSharedLibrary(const std::string& filePath);
        
        /**
         * @brief Add a component to the processed list
         * @param component The component to add
         */
        void addComponent(const ComponentInfo& component);
        
        /**
         * @brief Extract component name from file path
         * @param filePath The file path
         * @return The extracted component name
         */
        std::string extractComponentName(const std::string& filePath);
        
        // Member variables
        std::vector<std::string> processedFiles;      ///< List of processed files
        std::vector<std::string> processedLibraries;  ///< List of processed libraries
        std::vector<SymbolInfo> currentSymbols;       ///< Current symbols being processed
        std::string currentFile;                      ///< Current file being processed
        bool verbose = false;                         ///< Verbose output flag
        bool extractDebugInfo = true;                 ///< Debug info extraction flag
        bool includeSystemLibraries = false;          ///< System library inclusion flag
        std::unique_ptr<SBOMGenerator> sbomGenerator; ///< SBOM generator instance
    };
    
    std::unique_ptr<Impl> pImpl; ///< Implementation pointer
};

} // namespace heimdall
