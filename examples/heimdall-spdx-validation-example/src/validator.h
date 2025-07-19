#pragma once

#include <string>

/**
 * @brief Validator component for SPDX testing
 */
class Validator {
public:
    Validator();
    ~Validator();
    
    /**
     * @brief Validate input
     * @param input Input to validate
     * @return true if valid, false otherwise
     */
    bool validateInput(const std::string& input);
    
    /**
     * @brief Validate scenario
     * @param scenario Scenario to validate
     * @return true if scenario is valid
     */
    bool validateScenario(const std::string& scenario);
    
    /**
     * @brief Get validation statistics
     * @return Number of validations performed
     */
    int getValidationCount() const;

private:
    int validationCount;
}; 