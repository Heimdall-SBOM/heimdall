#include "validator.h"
#include <iostream>

Validator::Validator() : validationCount(0) {
    std::cout << "[Validator] Initialized" << std::endl;
}

Validator::~Validator() {
    std::cout << "[Validator] Destroyed" << std::endl;
}

bool Validator::validateInput(const std::string& input) {
    validationCount++;
    
    // Simple validation logic
    if (input.empty()) {
        return false;
    }
    
    if (input.find("invalid") != std::string::npos) {
        return false;
    }
    
    return true;
}

bool Validator::validateScenario(const std::string& scenario) {
    validationCount++;
    
    // Scenario validation logic
    if (scenario.find("valid") != std::string::npos) {
        return true;
    }
    
    if (scenario.find("invalid") != std::string::npos) {
        return false;
    }
    
    // Default to valid for edge cases
    return true;
}

int Validator::getValidationCount() const {
    return validationCount;
} 