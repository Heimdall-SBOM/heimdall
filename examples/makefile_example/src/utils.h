/*
 * Heimdall Compiler Plugin Example - Utility Functions Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Utility functions for data processing, timing, and vector operations
 * to demonstrate compiler plugin metadata collection.
 */

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <chrono>

/**
 * @brief Utility functions for the calculator example
 * 
 * This namespace provides helper functions that demonstrate:
 * - STL container operations
 * - Algorithm usage
 * - Time measurement
 * - Statistical calculations
 */
namespace Utils {
    /**
     * @brief Print vector contents to console
     * @param vec Vector to print
     */
    void printVector(const std::vector<double>& vec);
    
    /**
     * @brief Calculate average of vector elements
     * @param vec Input vector
     * @return Average value
     * @throws std::runtime_error if vector is empty
     */
    double calculateAverage(const std::vector<double>& vec);
    
    /**
     * @brief Find maximum element in vector
     * @param vec Input vector
     * @return Maximum value
     * @throws std::runtime_error if vector is empty
     */
    double findMaximum(const std::vector<double>& vec);
    
    /**
     * @brief Check if vector is sorted in ascending order
     * @param vec Input vector
     * @return true if sorted, false otherwise
     */
    bool isSorted(const std::vector<double>& vec);
    
    /**
     * @brief Get current timestamp as seconds since epoch
     * @return Current time in seconds (double precision)
     */
    double getCurrentTimestamp();
    
    /**
     * @brief Generate random double in given range
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Random double in [min, max)
     */
    double generateRandom(double min, double max);
    
    /**
     * @brief Sort vector in-place using std::sort
     * @param vec Vector to sort
     */
    void sortVector(std::vector<double>& vec);
}

// Global utility functions for convenience
using Utils::printVector;
using Utils::calculateAverage;
using Utils::findMaximum;
using Utils::isSorted;
using Utils::getCurrentTimestamp;

#endif // UTILS_H