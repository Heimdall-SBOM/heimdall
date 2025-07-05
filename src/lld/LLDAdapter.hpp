/**
 * @file LLDAdapter.hpp
 * @brief LLVM LLD linker adapter
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <memory>
#include <string>

namespace heimdall {

/**
 * @brief LLVM LLD linker adapter
 * 
 * This class provides an adapter for the LLVM LLD linker,
 * enabling SBOM generation during the linking process.
 */
class LLDAdapter
{
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
     * @brief Process an input file
     * @param filePath The path to the input file
     */
    void processInputFile(const std::string& filePath);
    
    /**
     * @brief Finalize the LLD adapter
     */
    void finalize();
    
private:
    /**
     * @brief Implementation class for LLD adapter
     */
    class Impl;
    
    std::unique_ptr<Impl> pImpl; ///< Implementation pointer
};

} // namespace heimdall
