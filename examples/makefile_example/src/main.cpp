/*
 * Heimdall Compiler Plugin Example - Main Application
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * This file demonstrates a complex C++ application for testing
 * Heimdall's compiler plugin system with SBOM generation.
 * 
 * Author: Heimdall Team
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "calculator.h"
#include "utils.h"

/**
 * @brief Main application entry point
 * 
 * Demonstrates various mathematical operations using the Calculator class
 * and utility functions to show compiler plugin metadata collection.
 */
int main() {
    std::cout << "=== Heimdall Compiler Plugin Demo ===" << std::endl;
    std::cout << "Complex C++ Application with SBOM Generation" << std::endl;
    std::cout << std::endl;
    
    try {
        // Initialize calculator with some test data
        auto calc = std::make_unique<Calculator>();
        
        // Demonstrate basic arithmetic operations
        std::cout << "1. Basic Arithmetic Operations:" << std::endl;
        std::cout << "   Addition: 15 + 25 = " << calc->add(15.0, 25.0) << std::endl;
        std::cout << "   Subtraction: 100 - 42 = " << calc->subtract(100.0, 42.0) << std::endl;
        std::cout << "   Multiplication: 7 * 8 = " << calc->multiply(7.0, 8.0) << std::endl;
        std::cout << "   Division: 144 / 12 = " << calc->divide(144.0, 12.0) << std::endl;
        std::cout << std::endl;
        
        // Test utility functions
        std::cout << "2. Utility Functions:" << std::endl;
        std::vector<double> test_data = {1.5, 2.8, 3.2, 4.9, 5.1, 6.7};
        std::cout << "   Test data: ";
        printVector(test_data);
        std::cout << "   Average: " << calculateAverage(test_data) << std::endl;
        std::cout << "   Maximum: " << findMaximum(test_data) << std::endl;
        std::cout << "   Is sorted: " << (isSorted(test_data) ? "Yes" : "No") << std::endl;
        std::cout << std::endl;
        
        // Demonstrate advanced operations
        std::cout << "3. Advanced Mathematical Operations:" << std::endl;
        std::cout << "   Square root of 64: " << calc->sqrt(64.0) << std::endl;
        std::cout << "   2^10 = " << calc->power(2.0, 10.0) << std::endl;
        std::cout << "   sin(π/2) ≈ " << calc->sin(3.14159 / 2.0) << std::endl;
        std::cout << "   Natural log of e ≈ " << calc->ln(2.71828) << std::endl;
        std::cout << std::endl;
        
        // Test error handling
        std::cout << "4. Error Handling:" << std::endl;
        try {
            std::cout << "   Division by zero: " << calc->divide(10.0, 0.0) << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "   Caught expected error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Memory and performance test
        std::cout << "5. Memory and Performance:" << std::endl;
        std::vector<double> large_dataset;
        large_dataset.reserve(10000);
        
        // Generate test data
        for (int i = 0; i < 10000; ++i) {
            large_dataset.push_back(static_cast<double>(i) * 0.1);
        }
        
        auto start_time = getCurrentTimestamp();
        double avg = calculateAverage(large_dataset);
        auto end_time = getCurrentTimestamp();
        
        std::cout << "   Processed " << large_dataset.size() << " elements" << std::endl;
        std::cout << "   Average: " << avg << std::endl;
        std::cout << "   Processing time: " << (end_time - start_time) << " seconds" << std::endl;
        std::cout << std::endl;
        
        // Statistics summary
        std::cout << "6. Application Statistics:" << std::endl;
        std::cout << "   Calculator operations performed: 8" << std::endl;
        std::cout << "   Utility functions called: 6" << std::endl;
        std::cout << "   Vector operations: 3" << std::endl;
        std::cout << "   Error handling tests: 1" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "=== Demo Complete ===" << std::endl;
    std::cout << "Check build/metadata/ for compiler plugin output" << std::endl;
    std::cout << "Check build/sbom/ for enhanced SBOM with compiler metadata" << std::endl;
    
    return 0;
}