#pragma once

#include <string>

/**
 * @brief Data processor component for SPDX testing
 */
class Processor {
public:
    Processor();
    ~Processor();
    
    /**
     * @brief Process data
     * @param data Input data to process
     * @return Processed result
     */
    std::string processData(const std::string& data);
    
    /**
     * @brief Get processing statistics
     * @return Number of items processed
     */
    int getProcessedCount() const;

private:
    int processedCount;
}; 