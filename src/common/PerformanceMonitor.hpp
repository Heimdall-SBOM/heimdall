/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file PerformanceMonitor.hpp
 * @brief Performance monitoring and metrics collection for Heimdall
 * @author Trevor Bakker
 * @date 2025
 *
 * This module provides performance monitoring capabilities for tracking
 * execution times, memory usage, and other performance metrics across
 * the modular architecture.
 */

#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief Performance metrics for a single operation
 */
struct PerformanceMetrics
{
   std::string                   operationName;           ///< Name of the operation
   std::chrono::microseconds     executionTime;           ///< Execution time in microseconds
   size_t                        memoryUsage    = 0;      ///< Memory usage in bytes
   size_t                        itemsProcessed = 0;      ///< Number of items processed
   bool                          success        = false;  ///< Whether operation succeeded
   std::string                   errorMessage;            ///< Error message if failed
   std::map<std::string, double> customMetrics;           ///< Custom metrics
};

/**
 * @brief Performance monitoring and metrics collection
 *
 * This class provides comprehensive performance monitoring capabilities
 * for tracking execution times, memory usage, and other metrics across
 * the modular architecture. It supports hierarchical timing, custom
 * metrics, and performance reporting.
 */
class PerformanceMonitor
{
   public:
   /**
    * @brief Default constructor
    */
   PerformanceMonitor();

   /**
    * @brief Destructor
    */
   ~PerformanceMonitor();

   /**
    * @brief Start timing an operation
    * @param operationName Name of the operation to time
    */
   void startOperation(const std::string& operationName);

   /**
    * @brief End timing an operation
    * @param operationName Name of the operation to end
    * @param success Whether the operation succeeded
    * @param itemsProcessed Number of items processed
    * @param memoryUsage Memory usage in bytes
    */
   void endOperation(const std::string& operationName, bool success = true,
                     size_t itemsProcessed = 0, size_t memoryUsage = 0);

   /**
    * @brief Add a custom metric to the current operation
    * @param metricName Name of the metric
    * @param value Metric value
    */
   void addCustomMetric(const std::string& metricName, double value);

   /**
    * @brief Get performance metrics for a specific operation
    * @param operationName Name of the operation
    * @return Performance metrics for the operation
    */
   PerformanceMetrics getMetrics(const std::string& operationName) const;

   /**
    * @brief Get all performance metrics
    * @return Vector of all performance metrics
    */
   std::vector<PerformanceMetrics> getAllMetrics() const;

   /**
    * @brief Get summary statistics
    * @return Map of operation names to summary statistics
    */
   std::map<std::string, std::map<std::string, double>> getSummaryStatistics() const;

   /**
    * @brief Clear all metrics
    */
   void clear();

   /**
    * @brief Enable or disable performance monitoring
    * @param enabled Whether to enable monitoring
    */
   void setEnabled(bool enabled);

   /**
    * @brief Check if performance monitoring is enabled
    * @return true if monitoring is enabled
    */
   bool isEnabled() const;

   /**
    * @brief Set the output format for reports
    * @param format Output format ("json", "csv", "text")
    */
   void setOutputFormat(const std::string& format);

   /**
    * @brief Generate a performance report
    * @param outputPath Path to save the report (optional)
    * @return Performance report as string
    */
   std::string generateReport(const std::string& outputPath = "") const;

   /**
    * @brief Export metrics to file
    * @param filePath Path to save the metrics
    * @param format Output format ("json", "csv")
    * @return true if export was successful
    */
   bool exportMetrics(const std::string& filePath, const std::string& format = "json") const;

   /**
    * @brief Get memory usage statistics
    * @return Map of memory usage statistics
    */
   std::map<std::string, size_t> getMemoryUsageStats() const;

   /**
    * @brief Get timing statistics
    * @return Map of timing statistics
    */
   std::map<std::string, std::chrono::microseconds> getTimingStats() const;

   /**
    * @brief Reset all statistics
    */
   void reset();

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

/**
 * @brief RAII wrapper for automatic operation timing
 */
class ScopedTimer
{
   public:
   /**
    * @brief Constructor - starts timing
    * @param monitor Performance monitor instance
    * @param operationName Name of the operation
    */
   ScopedTimer(PerformanceMonitor& monitor, const std::string& operationName);

   /**
    * @brief Destructor - ends timing
    */
   ~ScopedTimer();

   /**
    * @brief Set success status
    * @param success Whether operation succeeded
    */
   void setSuccess(bool success);

   /**
    * @brief Set number of items processed
    * @param count Number of items
    */
   void setItemsProcessed(size_t count);

   /**
    * @brief Set memory usage
    * @param bytes Memory usage in bytes
    */
   void setMemoryUsage(size_t bytes);

   /**
    * @brief Add custom metric
    * @param name Metric name
    * @param value Metric value
    */
   void addMetric(const std::string& name, double value);

   private:
   PerformanceMonitor&           monitor;
   std::string                   operationName;
   bool                          success        = true;
   size_t                        itemsProcessed = 0;
   size_t                        memoryUsage    = 0;
   std::map<std::string, double> customMetrics;
};

}  // namespace heimdall