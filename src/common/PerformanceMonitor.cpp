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
 * @file PerformanceMonitor.cpp
 * @brief Implementation of performance monitoring and metrics collection
 * @author Trevor Bakker
 * @date 2025
 */

#include "PerformanceMonitor.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <thread>

namespace heimdall
{

class PerformanceMonitor::Impl
{
   public:
   Impl() : enabled(true), outputFormat("json") {}

   bool                                                                  enabled;
   std::string                                                           outputFormat;
   std::map<std::string, std::chrono::high_resolution_clock::time_point> activeOperations;
   std::map<std::string, std::vector<PerformanceMetrics>>                operationHistory;
   std::map<std::string, std::map<std::string, double>>                  currentCustomMetrics;

   void startOperationImpl(const std::string& operationName);
   void endOperationImpl(const std::string& operationName, bool success, size_t itemsProcessed,
                         size_t memoryUsage);
   void addCustomMetricImpl(const std::string& metricName, double value);
   PerformanceMetrics              getMetricsImpl(const std::string& operationName) const;
   std::vector<PerformanceMetrics> getAllMetricsImpl() const;
   std::map<std::string, std::map<std::string, double>> getSummaryStatisticsImpl() const;
   void                                                 clearImpl();
   void                                                 setEnabledImpl(bool enabled);
   bool                                                 isEnabledImpl() const;
   void        setOutputFormatImpl(const std::string& format);
   std::string generateReportImpl(const std::string& outputPath) const;
   bool        exportMetricsImpl(const std::string& filePath, const std::string& format) const;
   std::map<std::string, size_t>                    getMemoryUsageStatsImpl() const;
   std::map<std::string, std::chrono::microseconds> getTimingStatsImpl() const;
   void                                             resetImpl();
};

// PerformanceMonitor implementation
PerformanceMonitor::PerformanceMonitor() : pImpl(std::make_unique<Impl>()) {}

PerformanceMonitor::~PerformanceMonitor() = default;

void PerformanceMonitor::startOperation(const std::string& operationName)
{
   pImpl->startOperationImpl(operationName);
}

void PerformanceMonitor::endOperation(const std::string& operationName, bool success,
                                      size_t itemsProcessed, size_t memoryUsage)
{
   pImpl->endOperationImpl(operationName, success, itemsProcessed, memoryUsage);
}

void PerformanceMonitor::addCustomMetric(const std::string& metricName, double value)
{
   pImpl->addCustomMetricImpl(metricName, value);
}

PerformanceMetrics PerformanceMonitor::getMetrics(const std::string& operationName) const
{
   return pImpl->getMetricsImpl(operationName);
}

std::vector<PerformanceMetrics> PerformanceMonitor::getAllMetrics() const
{
   return pImpl->getAllMetricsImpl();
}

std::map<std::string, std::map<std::string, double>> PerformanceMonitor::getSummaryStatistics()
   const
{
   return pImpl->getSummaryStatisticsImpl();
}

void PerformanceMonitor::clear()
{
   pImpl->clearImpl();
}

void PerformanceMonitor::setEnabled(bool enabled)
{
   pImpl->setEnabledImpl(enabled);
}

bool PerformanceMonitor::isEnabled() const
{
   return pImpl->isEnabledImpl();
}

void PerformanceMonitor::setOutputFormat(const std::string& format)
{
   pImpl->setOutputFormatImpl(format);
}

std::string PerformanceMonitor::generateReport(const std::string& outputPath) const
{
   return pImpl->generateReportImpl(outputPath);
}

bool PerformanceMonitor::exportMetrics(const std::string& filePath, const std::string& format) const
{
   return pImpl->exportMetricsImpl(filePath, format);
}

std::map<std::string, size_t> PerformanceMonitor::getMemoryUsageStats() const
{
   return pImpl->getMemoryUsageStatsImpl();
}

std::map<std::string, std::chrono::microseconds> PerformanceMonitor::getTimingStats() const
{
   return pImpl->getTimingStatsImpl();
}

void PerformanceMonitor::reset()
{
   pImpl->resetImpl();
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(PerformanceMonitor& monitor, const std::string& operationName)
   : monitor(monitor), operationName(operationName)
{
   monitor.startOperation(operationName);
}

ScopedTimer::~ScopedTimer()
{
   monitor.endOperation(operationName, success, itemsProcessed, memoryUsage);

   // Add custom metrics
   for (const auto& metric : customMetrics)
   {
      const auto& name = metric.first;
      const auto& value = metric.second;
      monitor.addCustomMetric(name, value);
   }
}

void ScopedTimer::setSuccess(bool success)
{
   this->success = success;
}

void ScopedTimer::setItemsProcessed(size_t count)
{
   this->itemsProcessed = count;
}

void ScopedTimer::setMemoryUsage(size_t bytes)
{
   this->memoryUsage = bytes;
}

void ScopedTimer::addMetric(const std::string& name, double value)
{
   customMetrics[name] = value;
}

// Impl implementation
void PerformanceMonitor::Impl::startOperationImpl(const std::string& operationName)
{
   if (!enabled)
      return;

   auto now                        = std::chrono::high_resolution_clock::now();
   activeOperations[operationName] = now;
   currentCustomMetrics[operationName].clear();
}

void PerformanceMonitor::Impl::endOperationImpl(const std::string& operationName, bool success,
                                                size_t itemsProcessed, size_t memoryUsage)
{
   if (!enabled)
      return;

   auto it = activeOperations.find(operationName);
   if (it == activeOperations.end())
      return;

   auto startTime = it->second;
   auto endTime   = std::chrono::high_resolution_clock::now();
   auto duration  = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

   PerformanceMetrics metrics;
   metrics.operationName  = operationName;
   metrics.executionTime  = duration;
   metrics.memoryUsage    = memoryUsage;
   metrics.itemsProcessed = itemsProcessed;
   metrics.success        = success;
   metrics.customMetrics  = currentCustomMetrics[operationName];

   operationHistory[operationName].push_back(metrics);
   activeOperations.erase(it);
   currentCustomMetrics.erase(operationName);
}

void PerformanceMonitor::Impl::addCustomMetricImpl(const std::string& metricName, double value)
{
   if (!enabled)
      return;

   // Find the most recent active operation
   if (!activeOperations.empty())
   {
      auto lastOperation                              = activeOperations.rbegin()->first;
      currentCustomMetrics[lastOperation][metricName] = value;
   }
}

PerformanceMetrics PerformanceMonitor::Impl::getMetricsImpl(const std::string& operationName) const
{
   auto it = operationHistory.find(operationName);
   if (it != operationHistory.end() && !it->second.empty())
   {
      return it->second.back();  // Return most recent metrics
   }

   return PerformanceMetrics{};
}

std::vector<PerformanceMetrics> PerformanceMonitor::Impl::getAllMetricsImpl() const
{
   std::vector<PerformanceMetrics> allMetrics;
   for (const auto& operation : operationHistory)
   {
      const auto& operationName = operation.first;
      const auto& metrics = operation.second;
      allMetrics.insert(allMetrics.end(), metrics.begin(), metrics.end());
   }
   return allMetrics;
}

std::map<std::string, std::map<std::string, double>>
PerformanceMonitor::Impl::getSummaryStatisticsImpl() const
{
   std::map<std::string, std::map<std::string, double>> summary;

   for (const auto& operation : operationHistory)
   {
      const auto& operationName = operation.first;
      const auto& metrics = operation.second;
      if (metrics.empty())
         continue;

      std::map<std::string, double> stats;

      // Calculate timing statistics
      std::vector<double> times;
      for (const auto& metric : metrics)
      {
         times.push_back(metric.executionTime.count());
      }

      std::sort(times.begin(), times.end());
      stats["count"]          = static_cast<double>(metrics.size());
      stats["min_time_us"]    = times.front();
      stats["max_time_us"]    = times.back();
      stats["avg_time_us"]    = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
      stats["median_time_us"] = times[times.size() / 2];

      // Calculate memory statistics
      std::vector<size_t> memoryUsage;
      for (const auto& metric : metrics)
      {
         memoryUsage.push_back(metric.memoryUsage);
      }

      if (!memoryUsage.empty())
      {
         std::sort(memoryUsage.begin(), memoryUsage.end());
         stats["min_memory_bytes"] = static_cast<double>(memoryUsage.front());
         stats["max_memory_bytes"] = static_cast<double>(memoryUsage.back());
         stats["avg_memory_bytes"] =
            static_cast<double>(std::accumulate(memoryUsage.begin(), memoryUsage.end(), 0ULL)) /
            memoryUsage.size();
         stats["median_memory_bytes"] = static_cast<double>(memoryUsage[memoryUsage.size() / 2]);
      }

      // Calculate success rate
      size_t successCount = 0;
      for (const auto& metric : metrics)
      {
         if (metric.success)
            successCount++;
      }
      stats["success_rate"] = static_cast<double>(successCount) / metrics.size();

      summary[operationName] = stats;
   }

   return summary;
}

void PerformanceMonitor::Impl::clearImpl()
{
   activeOperations.clear();
   operationHistory.clear();
   currentCustomMetrics.clear();
}

void PerformanceMonitor::Impl::setEnabledImpl(bool enabled)
{
   this->enabled = enabled;
}

bool PerformanceMonitor::Impl::isEnabledImpl() const
{
   return enabled;
}

void PerformanceMonitor::Impl::setOutputFormatImpl(const std::string& format)
{
   this->outputFormat = format;
}

std::string PerformanceMonitor::Impl::generateReportImpl(const std::string& outputPath) const
{
   std::stringstream report;

   if (outputFormat == "json")
   {
      report << "{\n";
      report << "  \"performance_report\": {\n";
      report << "    \"timestamp\": \""
             << std::chrono::system_clock::now().time_since_epoch().count() << "\",\n";
      report << "    \"operations\": [\n";

      bool firstOperation = true;
      for (const auto& operation : operationHistory)
      {
         const auto& operationName = operation.first;
         const auto& metrics = operation.second;
         if (!firstOperation)
            report << ",\n";
         firstOperation = false;

         report << "      {\n";
         report << "        \"name\": \"" << operationName << "\",\n";
         report << "        \"executions\": " << metrics.size() << ",\n";

         if (!metrics.empty())
         {
            auto latest = metrics.back();
            report << "        \"latest_execution_time_us\": " << latest.executionTime.count()
                   << ",\n";
            report << "        \"latest_memory_bytes\": " << latest.memoryUsage << ",\n";
            report << "        \"latest_success\": " << (latest.success ? "true" : "false") << "\n";
         }
         report << "      }";
      }

      report << "\n    ]\n";
      report << "  }\n";
      report << "}";
   }
   else if (outputFormat == "csv")
   {
      report << "Operation,Executions,AvgTime_us,MaxTime_us,AvgMemory_bytes,SuccessRate\n";

      for (const auto& operation : operationHistory)
      {
         const auto& operationName = operation.first;
         const auto& metrics = operation.second;
         if (metrics.empty())
            continue;

         double avgTime      = 0.0;
         double maxTime      = 0.0;
         double avgMemory    = 0.0;
         size_t successCount = 0;

         for (const auto& metric : metrics)
         {
            avgTime += metric.executionTime.count();
            maxTime = std::max(maxTime, static_cast<double>(metric.executionTime.count()));
            avgMemory += metric.memoryUsage;
            if (metric.success)
               successCount++;
         }

         avgTime /= metrics.size();
         avgMemory /= metrics.size();
         double successRate = static_cast<double>(successCount) / metrics.size();

         report << operationName << "," << metrics.size() << "," << avgTime << "," << maxTime << ","
                << avgMemory << "," << successRate << "\n";
      }
   }
   else  // text format
   {
      report << "Performance Report\n";
      report << "==================\n\n";

      for (const auto& operation : operationHistory)
      {
         const auto& operationName = operation.first;
         const auto& metrics = operation.second;
         if (metrics.empty())
            continue;

         report << "Operation: " << operationName << "\n";
         report << "  Executions: " << metrics.size() << "\n";

         if (!metrics.empty())
         {
            auto latest = metrics.back();
            report << "  Latest execution time: " << latest.executionTime.count() << " μs\n";
            report << "  Latest memory usage: " << latest.memoryUsage << " bytes\n";
            report << "  Latest success: " << (latest.success ? "Yes" : "No") << "\n";
         }
         report << "\n";
      }
   }

   std::string reportStr = report.str();

   if (!outputPath.empty())
   {
      std::ofstream file(outputPath);
      if (file.is_open())
      {
         file << reportStr;
         file.close();
      }
   }

   return reportStr;
}

bool PerformanceMonitor::Impl::exportMetricsImpl(const std::string& filePath,
                                                 const std::string& format) const
{
   try
   {
      std::ofstream file(filePath);
      if (!file.is_open())
         return false;

      if (format == "json")
      {
         file << "{\n";
         file << "  \"metrics\": [\n";

         bool firstMetric = true;
         for (const auto& operation : operationHistory)
         {
            const auto& operationName = operation.first;
            const auto& metrics = operation.second;
            for (const auto& metric : metrics)
            {
               if (!firstMetric)
                  file << ",\n";
               firstMetric = false;

               file << "    {\n";
               file << "      \"operation\": \"" << metric.operationName << "\",\n";
               file << "      \"execution_time_us\": " << metric.executionTime.count() << ",\n";
               file << "      \"memory_bytes\": " << metric.memoryUsage << ",\n";
               file << "      \"items_processed\": " << metric.itemsProcessed << ",\n";
               file << "      \"success\": " << (metric.success ? "true" : "false") << "\n";
               file << "    }";
            }
         }

         file << "\n  ]\n";
         file << "}";
      }
      else if (format == "csv")
      {
         file << "Operation,ExecutionTime_us,Memory_bytes,ItemsProcessed,Success\n";

         for (const auto& operation : operationHistory)
         {
            const auto& operationName = operation.first;
            const auto& metrics = operation.second;
            for (const auto& metric : metrics)
            {
               file << metric.operationName << "," << metric.executionTime.count() << ","
                    << metric.memoryUsage << "," << metric.itemsProcessed << ","
                    << (metric.success ? "true" : "false") << "\n";
            }
         }
      }

      file.close();
      return true;
   }
   catch (...)
   {
      return false;
   }
}

std::map<std::string, size_t> PerformanceMonitor::Impl::getMemoryUsageStatsImpl() const
{
   std::map<std::string, size_t> stats;

   for (const auto& operation : operationHistory)
   {
      const auto& operationName = operation.first;
      const auto& metrics = operation.second;
      if (metrics.empty())
         continue;

      size_t totalMemory = 0;
      for (const auto& metric : metrics)
      {
         totalMemory += metric.memoryUsage;
      }

      stats[operationName] = totalMemory;
   }

   return stats;
}

std::map<std::string, std::chrono::microseconds> PerformanceMonitor::Impl::getTimingStatsImpl()
   const
{
   std::map<std::string, std::chrono::microseconds> stats;

   for (const auto& operation : operationHistory)
   {
      const auto& operationName = operation.first;
      const auto& metrics = operation.second;
      if (metrics.empty())
         continue;

      std::chrono::microseconds totalTime{0};
      for (const auto& metric : metrics)
      {
         totalTime += metric.executionTime;
      }

      stats[operationName] = totalTime;
   }

   return stats;
}

void PerformanceMonitor::Impl::resetImpl()
{
   clearImpl();
}

}  // namespace heimdall