/*
 * Heimdall Compiler Plugin Example - Calculator Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Calculator class definition with various mathematical operations
 * for demonstrating compiler plugin metadata collection.
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdexcept>
#include "math/operations.h"

/**
 * @brief Calculator class providing mathematical operations
 * 
 * This class demonstrates various C++ features that will be captured
 * by Heimdall's compiler plugins, including:
 * - Class definitions and method declarations
 * - Template usage and STL dependencies  
 * - Exception handling
 * - Include dependencies
 */
class Calculator {
private:
    static constexpr double EPSILON = 1e-10;  ///< Precision threshold
    mutable int operation_count_;             ///< Operations performed counter

public:
    /**
     * @brief Default constructor
     */
    Calculator();
    
    /**
     * @brief Destructor
     */
    virtual ~Calculator() = default;
    
    // Basic arithmetic operations
    
    /**
     * @brief Addition operation
     * @param a First operand
     * @param b Second operand
     * @return Sum of a and b
     */
    double add(double a, double b) const;
    
    /**
     * @brief Subtraction operation
     * @param a First operand (minuend)
     * @param b Second operand (subtrahend)
     * @return Difference of a and b
     */
    double subtract(double a, double b) const;
    
    /**
     * @brief Multiplication operation
     * @param a First operand
     * @param b Second operand
     * @return Product of a and b
     */
    double multiply(double a, double b) const;
    
    /**
     * @brief Division operation
     * @param a Dividend
     * @param b Divisor
     * @return Quotient of a and b
     * @throws std::runtime_error if divisor is zero
     */
    double divide(double a, double b) const;
    
    // Advanced mathematical operations
    
    /**
     * @brief Square root operation
     * @param x Input value
     * @return Square root of x
     * @throws std::runtime_error if x is negative
     */
    double sqrt(double x) const;
    
    /**
     * @brief Power operation
     * @param base Base value
     * @param exponent Exponent value
     * @return base raised to the power of exponent
     */
    double power(double base, double exponent) const;
    
    /**
     * @brief Sine trigonometric function
     * @param angle Angle in radians
     * @return Sine of the angle
     */
    double sin(double angle) const;
    
    /**
     * @brief Cosine trigonometric function
     * @param angle Angle in radians
     * @return Cosine of the angle
     */
    double cos(double angle) const;
    
    /**
     * @brief Natural logarithm
     * @param x Input value
     * @return Natural logarithm of x
     * @throws std::runtime_error if x <= 0
     */
    double ln(double x) const;
    
    // Utility methods
    
    /**
     * @brief Get the number of operations performed
     * @return Number of operations since construction
     */
    int getOperationCount() const { return operation_count_; }
    
    /**
     * @brief Reset the operation counter
     */
    void resetCounter() { operation_count_ = 0; }
    
    /**
     * @brief Check if two doubles are approximately equal
     * @param a First value
     * @param b Second value  
     * @return true if values are within EPSILON of each other
     */
    static bool isEqual(double a, double b) {
        return std::abs(a - b) < EPSILON;
    }
    
    /**
     * @brief Get precision epsilon value
     * @return Current epsilon value used for comparisons
     */
    static double getEpsilon() { return EPSILON; }
};

#endif // CALCULATOR_H