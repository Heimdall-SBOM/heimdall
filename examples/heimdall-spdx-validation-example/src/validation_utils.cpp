#include "validation_utils.h"
#include <iostream>
#include <algorithm>

std::string ValidationUtils::processData(const std::string& data) {
    std::string result = data;
    // Simple processing: convert to uppercase
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool ValidationUtils::validateFormat(const std::string& data) {
    // Simple format validation
    if (data.empty()) {
        return false;
    }
    // Check for basic format requirements
    if (data.length() < 3) {
        return false;
    }
    return true;
}

std::vector<std::string> ValidationUtils::getValidationStats() {
    // Return some dummy stats for demonstration
    return {"Processed: 10", "Valid: 8", "Invalid: 2"};
} 