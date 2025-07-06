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
