#pragma once

#include <string>

/**
 * @brief Validation library component for SPDX testing
 */
class ValidationLib {
public:
    ValidationLib();
    ~ValidationLib();
    
    /**
     * @brief Validate input data
     * @param data Input data to validate
     * @return true if validation passes, false otherwise
     */
    bool validate(const std::string& data);
    
    /**
     * @brief Get validation statistics
     * @return Number of validations performed
     */
    int getValidationCount() const;

private:
    int validationCount;
}; 