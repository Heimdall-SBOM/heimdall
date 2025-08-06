/*
 * Heimdall Compiler Plugin CMake Example - Main Application
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Network simulation application demonstrating advanced C++ features
 * for comprehensive compiler plugin metadata collection.
 * 
 * Author: Heimdall Team
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "core/Application.h"
#include "core/Logger.h"

/**
 * @brief Main application entry point
 * 
 * Demonstrates a sophisticated application with:
 * - Application lifecycle management
 * - Logging system with multiple severity levels
 * - Resource management and error handling
 * - Modern C++ features (smart pointers, RAII, etc.)
 */
int main() {
    std::cout << "=== Heimdall Compiler Plugin CMake Demo ===" << std::endl;
    std::cout << "Network Simulation with Advanced C++ Features" << std::endl;
    std::cout << std::endl;
    
    try {
        // Initialize the logging system
        auto& logger = Logger::getInstance();
        logger.setLevel(LogLevel::DEBUG);
        logger.log(LogLevel::INFO, "Application starting up...");
        
        // Create application instance
        auto app = std::make_unique<Application>("NetworkSimulator", "1.0.0");
        
        if (!app->initialize()) {
            logger.log(LogLevel::ERROR, "Failed to initialize application");
            return 1;
        }
        
        // Demonstrate application features
        logger.log(LogLevel::INFO, "Demonstrating application features...");
        
        // Simulate some work
        logger.log(LogLevel::DEBUG, "Processing simulation data...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Update memory usage tracking
        app->updateMemoryUsage(Application::getCurrentMemoryUsage());
        
        // Simulate multiple operations
        std::vector<std::string> operations = {
            "Data processing",
            "Network initialization", 
            "Resource allocation",
            "Configuration loading",
            "Service startup"
        };
        
        for (const auto& op : operations) {
            logger.log(LogLevel::INFO, "Executing: " + op);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            app->updateMemoryUsage(Application::getCurrentMemoryUsage());
        }
        
        // Application statistics
        logger.log(LogLevel::INFO, "=== Application Statistics ===");
        logger.log(LogLevel::INFO, "Application: " + app->getName() + " v" + app->getVersion());
        logger.log(LogLevel::INFO, "Uptime: " + std::to_string(app->getUptime()) + " seconds");
        logger.log(LogLevel::INFO, "Peak memory: " + app->getPeakMemoryUsage() + " MB");
        logger.log(LogLevel::INFO, "Operations completed: " + std::to_string(operations.size()));
        
        // Demonstrate error handling
        try {
            logger.log(LogLevel::DEBUG, "Testing error handling...");
            throw std::runtime_error("Simulated error for demonstration");
        } catch (const std::exception& e) {
            logger.log(LogLevel::WARNING, "Caught expected error: " + std::string(e.what()));
        }
        
        // Graceful shutdown
        logger.log(LogLevel::INFO, "Initiating graceful shutdown...");
        app->shutdown();
        logger.log(LogLevel::INFO, "Application shutdown complete");
        
        // Performance and resource usage summary
        logger.log(LogLevel::DEBUG, "=== Performance Summary ===");
        logger.log(LogLevel::DEBUG, "Peak memory usage: " + app->getPeakMemoryUsage() + " MB");
        logger.log(LogLevel::DEBUG, "Total runtime: " + std::to_string(app->getUptime()) + " seconds");
        logger.log(LogLevel::DEBUG, "Log entries generated: " + std::to_string(logger.getLogCount()));
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal application error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 2;
    }
    
    std::cout << std::endl;
    std::cout << "=== Demo Complete ===" << std::endl;
    std::cout << "Check build/metadata/ for compiler plugin output" << std::endl;
    std::cout << "Check build/sbom/ for enhanced SBOM with compiler metadata" << std::endl;
    std::cout << "Run 'make sbom' to generate SBOM if not done automatically" << std::endl;
    
    return 0;
}