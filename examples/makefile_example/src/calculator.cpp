/*
 * Heimdall Compiler Plugin Example - Calculator Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Calculator class implementation with various mathematical operations
 * for demonstrating compiler plugin metadata collection.
 */

#include "calculator.h"
#include "math/operations.h"
#include <cmath>

Calculator::Calculator() : operation_count_(0) {
    // Constructor implementation
}

double Calculator::add(double a, double b) const {
    ++operation_count_;
    return MathOperations::add(a, b);
}

double Calculator::subtract(double a, double b) const {
    ++operation_count_;
    return MathOperations::subtract(a, b);
}

double Calculator::multiply(double a, double b) const {
    ++operation_count_;
    return MathOperations::multiply(a, b);
}

double Calculator::divide(double a, double b) const {
    ++operation_count_;
    if (std::abs(b) < EPSILON) {
        throw std::runtime_error("Division by zero");
    }
    return MathOperations::divide(a, b);
}

double Calculator::sqrt(double x) const {
    ++operation_count_;
    if (x < 0.0) {
        throw std::runtime_error("Square root of negative number");
    }
    return std::sqrt(x);
}

double Calculator::power(double base, double exponent) const {
    ++operation_count_;
    return std::pow(base, exponent);
}

double Calculator::sin(double angle) const {
    ++operation_count_;
    return std::sin(angle);
}

double Calculator::cos(double angle) const {
    ++operation_count_;
    return std::cos(angle);
}

double Calculator::ln(double x) const {
    ++operation_count_;
    if (x <= 0.0) {
        throw std::runtime_error("Natural logarithm of non-positive number");
    }
    return std::log(x);
}