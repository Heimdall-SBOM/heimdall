#pragma once

#include <string>
#include <vector>

/**
 * @brief Validation utilities for SPDX testing
 */
class ValidationUtils {
public:
    /**
     * @brief Process validation data
     * @param data Input data to process
     * @return Processed result
     */
    static std::string processData(const std::string& data);
    
    /**
     * @brief Validate data format
     * @param data Data to validate
     * @return true if format is valid
     */
    static bool validateFormat(const std::string& data);
    
    /**
     * @brief Get validation statistics
     * @return Vector of validation results
     */
    static std::vector<std::string> getValidationStats();
}; 