/*
 * Heimdall Compiler Plugin CMake Example - Logging System Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Thread-safe logging system with multiple severity levels and
 * flexible output formatting for the network simulation demo.
 */

#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    DEBUG = 0,    ///< Debug information for development
    INFO = 1,     ///< General information
    WARNING = 2,  ///< Warning conditions
    ERROR = 3,    ///< Error conditions
    FATAL = 4     ///< Fatal error conditions
};

/**
 * @brief Thread-safe logging system
 * 
 * Singleton pattern logger providing:
 * - Multiple severity levels with filtering
 * - Thread-safe operations with mutex protection
 * - Timestamp formatting for log entries
 * - Optional file output with configurable paths
 * - Statistics tracking for monitoring
 * - Flexible formatting and output customization
 */
class Logger {
private:
    LogLevel min_level_;                          ///< Minimum level to log
    std::atomic<size_t> log_count_;              ///< Total log entries counter
    std::mutex log_mutex_;                       ///< Thread safety mutex
    std::chrono::steady_clock::time_point start_time_; ///< Logger start time
    
    // File logging configuration
    std::string log_file_path_;                  ///< Path to log file
    std::unique_ptr<std::ofstream> log_file_;    ///< File stream for logging
    bool file_logging_enabled_;                  ///< Enable file logging
    
    // Output configuration
    bool show_timestamps_;                       ///< Include timestamps in output
    bool show_thread_id_;                       ///< Include thread ID in output
    bool color_output_;                         ///< Use color codes in console output

public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton logger instance
     */
    static Logger& getInstance();
    
    // Singleton - non-copyable, non-moveable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    /**
     * @brief Set minimum logging level
     * @param level Minimum level to log (levels below this are filtered out)
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief Get current minimum logging level
     * @return Current minimum level
     */
    LogLevel getLevel() const { return min_level_; }
    
    /**
     * @brief Log a message with specified level
     * @param level Severity level of the message
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Log debug message (convenience method)
     * @param message Debug message
     */
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    
    /**
     * @brief Log info message (convenience method)
     * @param message Info message
     */
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    
    /**
     * @brief Log warning message (convenience method)
     * @param message Warning message
     */
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    
    /**
     * @brief Log error message (convenience method)
     * @param message Error message
     */
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    
    /**
     * @brief Log fatal message (convenience method)
     * @param message Fatal error message
     */
    void fatal(const std::string& message) { log(LogLevel::FATAL, message); }
    
    /**
     * @brief Enable or disable file logging
     * @param enabled Enable file logging
     * @param file_path Path to log file (optional, uses default if empty)
     */
    void setFileLogging(bool enabled, const std::string& file_path = "");
    
    /**
     * @brief Enable or disable timestamp display
     * @param enabled Show timestamps in log output
     */
    void setShowTimestamps(bool enabled) { show_timestamps_ = enabled; }
    
    /**
     * @brief Enable or disable thread ID display
     * @param enabled Show thread IDs in log output
     */
    void setShowThreadId(bool enabled) { show_thread_id_ = enabled; }
    
    /**
     * @brief Enable or disable color output
     * @param enabled Use ANSI color codes in console output
     */
    void setColorOutput(bool enabled) { color_output_ = enabled; }
    
    /**
     * @brief Get total number of log entries
     * @return Total log entries since logger creation
     */
    size_t getLogCount() const { return log_count_.load(); }
    
    /**
     * @brief Get logger uptime in seconds
     * @return Seconds since logger initialization
     */
    double getUptime() const;
    
    /**
     * @brief Flush all pending log output
     */
    void flush();

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Logger();
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~Logger();
    
    /**
     * @brief Convert log level to string representation
     * @param level Log level to convert
     * @return String representation of the level
     */
    static std::string levelToString(LogLevel level);
    
    /**
     * @brief Get color code for log level
     * @param level Log level
     * @return ANSI color code string
     */
    static std::string getColorCode(LogLevel level);
    
    /**
     * @brief Get current timestamp as formatted string
     * @return Formatted timestamp string
     */
    std::string getCurrentTimestamp() const;
    
    /**
     * @brief Get current thread ID as string
     * @return Thread ID string
     */
    std::string getCurrentThreadId() const;
    
    /**
     * @brief Format log message with metadata
     * @param level Log level
     * @param message Raw message
     * @return Formatted log message
     */
    std::string formatMessage(LogLevel level, const std::string& message) const;
};

// Global convenience macros for common logging patterns
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)

#endif // CORE_LOGGER_H