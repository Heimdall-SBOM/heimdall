/*
 * Heimdall Compiler Plugin CMake Example - Application Core Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Implementation of the Application class providing complete application
 * lifecycle management and system monitoring capabilities.
 */

#include "Application.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <thread>

#ifdef __linux__
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

Application::Application(const std::string& name, const std::string& version)
    : name_(name), version_(version), initialized_(false), running_(false), 
      peak_memory_kb_(0), config_(std::make_unique<AppConfig>()) {
    start_time_ = std::chrono::steady_clock::now();
}

Application::~Application() {
    if (running_.load()) {
        shutdown();
    }
}

bool Application::initialize() {
    if (initialized_.load()) {
        return true; // Already initialized
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Initializing application: " + name_ + " v" + version_);
    
    try {
        // Initialize system resources
        if (!initializeSystem()) {
            logger.log(LogLevel::ERROR, "Failed to initialize system resources");
            return false;
        }
        
        // Initialize monitoring
        initializeMonitoring();
        
        // Set initial state
        initialized_.store(true);
        running_.store(true);
        
        logger.log(LogLevel::INFO, "Application initialized successfully");
        logger.log(LogLevel::DEBUG, "System info: " + getSystemInfo());
        
        return true;
        
    } catch (const std::exception& e) {
        logger.log(LogLevel::ERROR, "Exception during initialization: " + std::string(e.what()));
        return false;
    }
}

void Application::shutdown() {
    if (!running_.load()) {
        return; // Already shut down
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::INFO, "Shutting down application...");
    
    running_.store(false);
    
    try {
        // Cleanup system resources
        cleanupSystem();
        
        logger.log(LogLevel::INFO, "Application shutdown completed successfully");
        
    } catch (const std::exception& e) {
        logger.log(LogLevel::ERROR, "Exception during shutdown: " + std::string(e.what()));
    }
}

double Application::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - start_time_;
    return std::chrono::duration<double>(duration).count();
}

std::string Application::getPeakMemoryUsage() const {
    size_t peak_kb = peak_memory_kb_.load();
    double peak_mb = peak_kb / 1024.0;
    
    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << peak_mb;
    return oss.str();
}

void Application::updateMemoryUsage(size_t current_kb) {
    size_t current_peak = peak_memory_kb_.load();
    while (current_kb > current_peak && 
           !peak_memory_kb_.compare_exchange_weak(current_peak, current_kb)) {
        // Retry if another thread updated the value
    }
}

void Application::setLogging(bool enable_logging) {
    config_->enable_logging = enable_logging;
    
    auto& logger = Logger::getInstance();
    if (enable_logging) {
        logger.log(LogLevel::DEBUG, "Logging enabled");
    }
}

void Application::setMemoryLimit(size_t max_memory_mb) {
    config_->max_memory_mb = max_memory_mb;
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Memory limit set to " + std::to_string(max_memory_mb) + " MB");
}

std::string Application::getSystemInfo() {
    std::ostringstream info;
    
#ifdef __linux__
    // Get system information on Linux
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        info << "System: Linux, ";
        info << "RAM: " << (sys_info.totalram / 1024 / 1024) << " MB, ";
        info << "CPUs: " << std::thread::hardware_concurrency();
    } else {
        info << "System: Linux (details unavailable)";
    }
#else
    info << "System: Unknown, CPUs: " << std::thread::hardware_concurrency();
#endif
    
    return info.str();
}

size_t Application::getCurrentMemoryUsage() {
#ifdef __linux__
    // Read memory usage from /proc/self/status
    std::ifstream file("/proc/self/status");
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string label, value, unit;
            iss >> label >> value >> unit;
            return std::stoull(value); // Return in KB
        }
    }
#endif
    return 0; // Unable to determine
}

bool Application::initializeSystem() {
    auto& logger = Logger::getInstance();
    
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Set initial memory usage
    updateMemoryUsage(getCurrentMemoryUsage());
    
    logger.log(LogLevel::DEBUG, "System resources initialized");
    return true;
}

void Application::cleanupSystem() {
    auto& logger = Logger::getInstance();
    
    // Perform any necessary cleanup
    // In a real application, this might include:
    // - Closing database connections
    // - Cleaning up temporary files
    // - Releasing shared resources
    
    logger.log(LogLevel::DEBUG, "System resources cleaned up");
}

void Application::initializeMonitoring() {
    auto& logger = Logger::getInstance();
    
    if (!config_->enable_monitoring) {
        return;
    }
    
    // In a real application, this might start monitoring threads
    // For this demo, we just log the initialization
    logger.log(LogLevel::DEBUG, "Performance monitoring initialized");
    logger.log(LogLevel::DEBUG, "Worker threads configured: " + std::to_string(config_->worker_threads));
    logger.log(LogLevel::DEBUG, "Memory limit: " + std::to_string(config_->max_memory_mb) + " MB");
}