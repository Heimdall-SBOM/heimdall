/*
 * Heimdall Compiler Plugin Example - Mathematical Operations Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Mathematical operations module demonstrating nested directory
 * structure and modular design for compiler plugin testing.
 */

#ifndef MATH_OPERATIONS_H
#define MATH_OPERATIONS_H

/**
 * @brief Mathematical operations namespace
 * 
 * This namespace provides core mathematical functions used by
 * the Calculator class. It demonstrates:
 * - Nested namespace organization
 * - Static function definitions
 * - Mathematical computations
 * - Error handling for edge cases
 */
namespace MathOperations {
    /**
     * @brief Perform addition operation
     * @param a First operand
     * @param b Second operand
     * @return Sum of a and b
     */
    double add(double a, double b);
    
    /**
     * @brief Perform subtraction operation
     * @param a Minuend
     * @param b Subtrahend
     * @return Difference of a and b
     */
    double subtract(double a, double b);
    
    /**
     * @brief Perform multiplication operation
     * @param a First factor
     * @param b Second factor
     * @return Product of a and b
     */
    double multiply(double a, double b);
    
    /**
     * @brief Perform division operation
     * @param dividend Value to be divided
     * @param divisor Value to divide by
     * @return Quotient of dividend and divisor
     * @note Does not check for division by zero (handled by caller)
     */
    double divide(double dividend, double divisor);
    
    /**
     * @brief Calculate absolute value
     * @param x Input value
     * @return Absolute value of x
     */
    double absolute(double x);
    
    /**
     * @brief Check if a number is approximately zero
     * @param x Value to check
     * @param epsilon Tolerance for comparison
     * @return true if x is within epsilon of zero
     */
    bool isNearZero(double x, double epsilon = 1e-10);
    
    /**
     * @brief Calculate factorial (iterative implementation)
     * @param n Non-negative integer
     * @return Factorial of n
     * @throws std::runtime_error if n is negative or too large
     */
    double factorial(int n);
    
    /**
     * @brief Calculate greatest common divisor
     * @param a First integer
     * @param b Second integer
     * @return GCD of a and b using Euclidean algorithm
     */
    int gcd(int a, int b);
    
    /**
     * @brief Check if a number is prime
     * @param n Integer to check
     * @return true if n is prime, false otherwise
     */
    bool isPrime(int n);
}

#endif // MATH_OPERATIONS_H