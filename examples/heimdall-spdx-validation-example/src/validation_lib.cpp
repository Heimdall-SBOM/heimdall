#include "validation_lib.h"
#include <iostream>

ValidationLib::ValidationLib() : validationCount(0) {
    std::cout << "[ValidationLib] Initialized" << std::endl;
}

ValidationLib::~ValidationLib() {
    std::cout << "[ValidationLib] Destroyed" << std::endl;
}

bool ValidationLib::validate(const std::string& data) {
    validationCount++;
    
    // Simple validation logic
    if (data.empty()) {
        return false;
    }
    
    if (data.find("invalid") != std::string::npos) {
        return false;
    }
    
    return true;
}

int ValidationLib::getValidationCount() const {
    return validationCount;
} 