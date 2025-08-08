/*
 * Heimdall Compiler Plugin CMake Example - Logging System Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Complete implementation of the thread-safe logging system with
 * multiple output formats and severity level filtering.
 */

#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : min_level_(LogLevel::INFO), log_count_(0), log_file_path_("app.log"),
      file_logging_enabled_(false), show_timestamps_(true), show_thread_id_(false),
      color_output_(true) {
    start_time_ = std::chrono::steady_clock::now();
}

Logger::~Logger() {
    if (log_file_ && log_file_->is_open()) {
        log_file_->flush();
        log_file_->close();
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    min_level_ = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    // Quick check without lock for performance
    if (level < min_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // Double-check after acquiring lock
    if (level < min_level_) {
        return;
    }
    
    // Format the message
    std::string formatted = formatMessage(level, message);
    
    // Output to console
    if (color_output_) {
        std::cout << getColorCode(level) << formatted << "\033[0m" << std::endl;
    } else {
        std::cout << formatted << std::endl;
    }
    
    // Output to file if enabled
    if (file_logging_enabled_ && log_file_ && log_file_->is_open()) {
        *log_file_ << formatted << std::endl;
        log_file_->flush();
    }
    
    // Increment log counter
    log_count_.fetch_add(1);
}

void Logger::setFileLogging(bool enabled, const std::string& file_path) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (!file_path.empty()) {
        log_file_path_ = file_path;
    }
    
    if (enabled && !file_logging_enabled_) {
        // Enable file logging
        log_file_ = std::make_unique<std::ofstream>(log_file_path_, std::ios::app);
        if (log_file_->is_open()) {
            file_logging_enabled_ = true;
            *log_file_ << "\n=== Logger Session Started ===" << std::endl;
        }
    } else if (!enabled && file_logging_enabled_) {
        // Disable file logging
        if (log_file_ && log_file_->is_open()) {
            *log_file_ << "=== Logger Session Ended ===" << std::endl;
            log_file_->close();
        }
        file_logging_enabled_ = false;
        log_file_.reset();
    }
}

double Logger::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - start_time_;
    return std::chrono::duration<double>(duration).count();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::cout.flush();
    
    if (log_file_ && log_file_->is_open()) {
        log_file_->flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:               return "UNKNOWN";
    }
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "\033[36m";    // Cyan
        case LogLevel::INFO:    return "\033[32m";    // Green
        case LogLevel::WARNING: return "\033[33m";    // Yellow
        case LogLevel::ERROR:   return "\033[31m";    // Red
        case LogLevel::FATAL:   return "\033[35m";    // Magenta
        default:               return "";
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::getCurrentThreadId() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

std::string Logger::formatMessage(LogLevel level, const std::string& message) const {
    std::ostringstream oss;
    
    // Add timestamp if enabled
    if (show_timestamps_) {
        oss << "[" << getCurrentTimestamp() << "] ";
    }
    
    // Add log level
    oss << "[" << std::setw(7) << std::left << levelToString(level) << "] ";
    
    // Add thread ID if enabled
    if (show_thread_id_) {
        oss << "[" << getCurrentThreadId().substr(0, 8) << "] ";
    }
    
    // Add the actual message
    oss << message;
    
    return oss.str();
}