/*
 * Heimdall Compiler Plugin Example - Utility Functions Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Implementation of utility functions for data processing, timing,
 * and vector operations demonstrating compiler plugin capabilities.
 */

#include "utils.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <random>

namespace Utils {
    void printVector(const std::vector<double>& vec) {
        std::cout << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << vec[i];
        }
        std::cout << "]" << std::endl;
    }
    
    double calculateAverage(const std::vector<double>& vec) {
        if (vec.empty()) {
            throw std::runtime_error("Cannot calculate average of empty vector");
        }
        
        double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
        return sum / vec.size();
    }
    
    double findMaximum(const std::vector<double>& vec) {
        if (vec.empty()) {
            throw std::runtime_error("Cannot find maximum of empty vector");
        }
        
        return *std::max_element(vec.begin(), vec.end());
    }
    
    bool isSorted(const std::vector<double>& vec) {
        return std::is_sorted(vec.begin(), vec.end());
    }
    
    double getCurrentTimestamp() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<double>(duration).count();
    }
    
    double generateRandom(double min, double max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(min, max);
        return dis(gen);
    }
    
    void sortVector(std::vector<double>& vec) {
        std::sort(vec.begin(), vec.end());
    }
}