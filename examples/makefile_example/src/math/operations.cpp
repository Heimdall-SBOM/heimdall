/*
 * Heimdall Compiler Plugin Example - Mathematical Operations Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Implementation of mathematical operations demonstrating modular
 * design and nested directory structure for compiler plugin testing.
 */

#include "operations.h"
#include <cmath>
#include <stdexcept>

namespace MathOperations {
    double add(double a, double b) {
        return a + b;
    }
    
    double subtract(double a, double b) {
        return a - b;
    }
    
    double multiply(double a, double b) {
        return a * b;
    }
    
    double divide(double dividend, double divisor) {
        // Note: Division by zero check handled by caller (Calculator class)
        return dividend / divisor;
    }
    
    double absolute(double x) {
        return x < 0.0 ? -x : x;
    }
    
    bool isNearZero(double x, double epsilon) {
        return absolute(x) < epsilon;
    }
    
    double factorial(int n) {
        if (n < 0) {
            throw std::runtime_error("Factorial is not defined for negative numbers");
        }
        if (n > 170) {
            throw std::runtime_error("Factorial too large for double precision");
        }
        
        double result = 1.0;
        for (int i = 2; i <= n; ++i) {
            result *= i;
        }
        return result;
    }
    
    int gcd(int a, int b) {
        // Use absolute values for GCD calculation
        a = a < 0 ? -a : a;
        b = b < 0 ? -b : b;
        
        // Euclidean algorithm
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    bool isPrime(int n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        
        // Check for divisors from 5 up to sqrt(n)
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                return false;
            }
        }
        return true;
    }
}