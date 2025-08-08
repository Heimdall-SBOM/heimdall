/*
 * Heimdall Compiler Plugin CMake Example - Application Core Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Application management class providing lifecycle control, resource
 * management, and system monitoring for the network simulation demo.
 */

#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include <string>
#include <memory>
#include <chrono>
#include <atomic>

/**
 * @brief Core application management class
 * 
 * This class demonstrates modern C++ application design patterns including:
 * - RAII resource management
 * - Lifecycle management with proper initialization/cleanup
 * - Performance monitoring and statistics collection
 * - Thread-safe operations with atomic variables
 * - Exception safety and error handling
 */
class Application {
private:
    std::string name_;                                    ///< Application name
    std::string version_;                                 ///< Application version
    std::chrono::steady_clock::time_point start_time_;   ///< Application start time
    std::atomic<bool> initialized_;                      ///< Initialization state
    std::atomic<bool> running_;                          ///< Running state
    std::atomic<size_t> peak_memory_kb_;                 ///< Peak memory usage in KB
    
    // Configuration and state
    struct AppConfig {
        bool enable_logging = true;
        bool enable_monitoring = true;
        size_t max_memory_mb = 1024;
        int worker_threads = 4;
        std::string log_file_path = "app.log";
    };
    
    std::unique_ptr<AppConfig> config_;                  ///< Application configuration

public:
    /**
     * @brief Constructor with application details
     * @param name Application name
     * @param version Application version string
     */
    Application(const std::string& name, const std::string& version);
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Application();
    
    // Non-copyable but moveable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = default;
    Application& operator=(Application&&) = default;
    
    /**
     * @brief Initialize the application
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Check if application is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized_.load(); }
    
    /**
     * @brief Check if application is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_.load(); }
    
    /**
     * @brief Shutdown the application gracefully
     */
    void shutdown();
    
    /**
     * @brief Get application name
     * @return Application name string
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief Get application version
     * @return Version string
     */
    const std::string& getVersion() const { return version_; }
    
    /**
     * @brief Get application uptime in seconds
     * @return Uptime in seconds since initialization
     */
    double getUptime() const;
    
    /**
     * @brief Get peak memory usage
     * @return Peak memory usage as formatted string
     */
    std::string getPeakMemoryUsage() const;
    
    /**
     * @brief Update memory usage tracking
     * @param current_kb Current memory usage in KB
     */
    void updateMemoryUsage(size_t current_kb);
    
    /**
     * @brief Get application configuration
     * @return Reference to configuration object
     */
    const AppConfig& getConfig() const { return *config_; }
    
    /**
     * @brief Set configuration parameter
     * @param enable_logging Enable/disable logging
     */
    void setLogging(bool enable_logging);
    
    /**
     * @brief Set maximum memory limit
     * @param max_memory_mb Maximum memory in megabytes
     */
    void setMemoryLimit(size_t max_memory_mb);
    
    /**
     * @brief Get system information string
     * @return Formatted system information
     */
    static std::string getSystemInfo();
    
    /**
     * @brief Get current memory usage of the process
     * @return Current memory usage in KB
     */
    static size_t getCurrentMemoryUsage();

private:
    /**
     * @brief Initialize system resources
     * @return true if successful
     */
    bool initializeSystem();
    
    /**
     * @brief Cleanup system resources
     */
    void cleanupSystem();
    
    /**
     * @brief Initialize performance monitoring
     */
    void initializeMonitoring();
};

#endif // CORE_APPLICATION_H