#pragma once

#include <algorithm>
#include <chrono>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

namespace heimdall {

/**
 * @brief High-resolution timer for performance measurements
 */
class HighResTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point end_time_;
    bool is_running_;

public:
    HighResTimer() : is_running_(false) {}
    
    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        is_running_ = true;
    }
    
    void stop() {
        if (is_running_) {
            end_time_ = std::chrono::high_resolution_clock::now();
            is_running_ = false;
        }
    }
    
    template<typename Duration = std::chrono::microseconds>
    auto elapsed() const -> typename Duration::rep {
        auto end = is_running_ ? std::chrono::high_resolution_clock::now() : end_time_;
        return std::chrono::duration_cast<Duration>(end - start_time_).count();
    }
    
    double elapsed_seconds() const {
        return elapsed<std::chrono::duration<double>>();
    }
    
    double elapsed_milliseconds() const {
        return elapsed<std::chrono::milliseconds>();
    }
    
    double elapsed_microseconds() const {
        return elapsed<std::chrono::microseconds>();
    }
    
    bool is_running() const { return is_running_; }
};

/**
 * @brief Memory usage tracker
 */
class MemoryTracker {
private:
    size_t peak_memory_;
    size_t current_memory_;
    
public:
    MemoryTracker() : peak_memory_(0), current_memory_(0) {}
    
    void update_memory_usage(size_t bytes) {
        current_memory_ = bytes;
        peak_memory_ = std::max(bytes, peak_memory_);
    }
    
    size_t get_current_memory() const { return current_memory_; }
    size_t get_peak_memory() const { return peak_memory_; }
    
    void reset() {
        peak_memory_ = 0;
        current_memory_ = 0;
    }
};

/**
 * @brief System resource usage tracker
 */
class SystemResourceTracker {
private:
    struct ResourceUsage {
        double cpu_percent;
        size_t memory_kb;
        size_t virtual_memory_kb;
    };
    
    ResourceUsage last_usage_{};
    
public:
    SystemResourceTracker() {
        reset();
    }
    
    void reset() {
        last_usage_ = {0.0, 0, 0};
    }
    
    ResourceUsage get_current_usage() {
        ResourceUsage usage = {0.0, 0, 0};
        
#ifdef _WIN32
        FILETIME idle_time, kernel_time, user_time;
        if (GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
            // Calculate CPU usage (simplified)
            usage.cpu_percent = 0.0; // Would need more complex calculation
        }
        
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            usage.memory_kb = pmc.WorkingSetSize / 1024;
            usage.virtual_memory_kb = pmc.PrivateUsage / 1024;
        }
#else
        struct rusage rusage_data{};
        if (getrusage(RUSAGE_SELF, &rusage_data) == 0) {
            usage.memory_kb = rusage_data.ru_maxrss;
            // CPU usage calculation would need more complex tracking
            usage.cpu_percent = 0.0;
        }
#endif
        
        return usage;
    }
    
    ResourceUsage get_delta_usage() {
        auto current = get_current_usage();
        auto delta = current;
        delta.memory_kb -= last_usage_.memory_kb;
        delta.virtual_memory_kb -= last_usage_.virtual_memory_kb;
        last_usage_ = current;
        return delta;
    }
};

/**
 * @brief Performance measurement session
 */
class PerformanceSession {
private:
    std::string name_;
    HighResTimer timer_;
    MemoryTracker memory_tracker_;
    SystemResourceTracker resource_tracker_;
    std::map<std::string, double> metrics_;
    
public:
    explicit PerformanceSession(std::string  name) : name_(std::move(name)) {
        timer_.start();
        resource_tracker_.reset();
    }
    
    void add_metric(const std::string& key, double value) {
        metrics_[key] = value;
    }
    
    void stop() {
        timer_.stop();
    }
    
    double get_elapsed_seconds() const {
        return timer_.elapsed_seconds();
    }
    
    double get_elapsed_milliseconds() const {
        return timer_.elapsed_milliseconds();
    }
    
    const std::string& get_name() const { return name_; }
    const std::map<std::string, double>& get_metrics() const { return metrics_; }
    const MemoryTracker& get_memory_tracker() const { return memory_tracker_; }
};

/**
 * @brief Main profiler class for managing performance measurements
 */
class Profiler {
private:
    static Profiler* instance_;
    static std::mutex instance_mutex_;
    
    std::map<std::string, std::shared_ptr<PerformanceSession>> active_sessions_;
    std::vector<std::shared_ptr<PerformanceSession>> completed_sessions_;
    std::mutex sessions_mutex_;
    
    bool enabled_;
    std::string output_file_;
    
    Profiler() : enabled_(false) {}
    
public:
    static Profiler& get_instance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        if (!instance_) {
            instance_ = new Profiler();
        }
        return *instance_;
    }
    
    void enable(bool enabled = true) {
        enabled_ = enabled;
    }
    
    bool is_enabled() const { return enabled_; }
    
    void set_output_file(const std::string& filename) {
        output_file_ = filename;
    }
    
    std::shared_ptr<PerformanceSession> start_session(const std::string& name) {
        if (!enabled_) {
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto session = std::make_shared<PerformanceSession>(name);
        active_sessions_[name] = session;
        return session;
    }
    
    void end_session(const std::string& name) {
        if (!enabled_) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = active_sessions_.find(name);
        if (it != active_sessions_.end()) {
            it->second->stop();
            completed_sessions_.push_back(it->second);
            active_sessions_.erase(it);
        }
    }
    
    void clear_sessions() {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        active_sessions_.clear();
        completed_sessions_.clear();
    }
    
    void export_results(const std::string& filename = "") {
        if (!enabled_ || completed_sessions_.empty()) {
            return;
        }
        
        std::string output_file = filename.empty() ? output_file_ : filename;
        if (output_file.empty()) {
            output_file = "heimdall_profiler_results.json";
        }
        
        std::ofstream file(output_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
            return;
        }
        
        file << "{\n";
        file << "  \"profiler_results\": {\n";
        file << "    \"total_sessions\": " << completed_sessions_.size() << ",\n";
        file << "    \"sessions\": [\n";
        
        for (size_t i = 0; i < completed_sessions_.size(); ++i) {
            const auto& session = completed_sessions_[i];
            file << "      {\n";
            file << "        \"name\": \"" << session->get_name() << "\",\n";
            file << "        \"elapsed_seconds\": " << session->get_elapsed_seconds() << ",\n";
            file << "        \"elapsed_milliseconds\": " << session->get_elapsed_milliseconds() << ",\n";
            file << "        \"peak_memory\": " << session->get_memory_tracker().get_peak_memory() << ",\n";
            file << "        \"metrics\": {\n";
            
            const auto& metrics = session->get_metrics();
            size_t metric_count = 0;
            for (const auto& metric : metrics) {
                file << "          \"" << metric.first << "\": " << metric.second;
                if (++metric_count < metrics.size()) {
                    file << ",";
                }
                file << "\n";
            }
            file << "        }\n";
            file << "      }";
            if (i < completed_sessions_.size() - 1) {
                file << ",";
            }
            file << "\n";
        }
        
        file << "    ]\n";
        file << "  }\n";
        file << "}\n";
        
        std::cout << "Profiler results exported to: " << output_file << std::endl;
    }
    
    void print_summary() {
        if (!enabled_ || completed_sessions_.empty()) {
            return;
        }
        
        std::cout << "\n=== Heimdall Profiler Summary ===" << std::endl;
        std::cout << "Total sessions: " << completed_sessions_.size() << std::endl;
        
        double total_time = 0.0;
        size_t total_peak_memory = 0;
        
        for (const auto& session : completed_sessions_) {
            total_time += session->get_elapsed_seconds();
            total_peak_memory += session->get_memory_tracker().get_peak_memory();
            
            std::cout << "\nSession: " << session->get_name() << std::endl;
            std::cout << "  Time: " << session->get_elapsed_seconds() << "s (" 
                     << session->get_elapsed_milliseconds() << "ms)" << std::endl;
            std::cout << "  Peak Memory: " << session->get_memory_tracker().get_peak_memory() << " bytes" << std::endl;
            
            if (!session->get_metrics().empty()) {
                std::cout << "  Metrics:" << std::endl;
                for (const auto& metric : session->get_metrics()) {
                    std::cout << "    " << metric.first << ": " << metric.second << std::endl;
                }
            }
        }
        
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "Total time: " << total_time << "s" << std::endl;
        std::cout << "Total peak memory: " << total_peak_memory << " bytes" << std::endl;
        std::cout << "Average time per session: " << (total_time / completed_sessions_.size()) << "s" << std::endl;
    }
};

/**
 * @brief RAII wrapper for automatic session management
 */
class ScopedProfilerSession {
private:
    std::string name_;
    std::shared_ptr<PerformanceSession> session_;
    
public:
    explicit ScopedProfilerSession(const std::string& name) 
        : name_(name), session_(Profiler::get_instance().start_session(name)) {}
    
    ~ScopedProfilerSession() {
        if (session_) {
            try {
                Profiler::get_instance().end_session(name_);
            } catch (const std::exception& e) {
                // Optionally log: std::cerr << "Profiler exception: " << e.what() << std::endl;
            } catch (...) {
                // Optionally log: std::cerr << "Unknown exception in Profiler" << std::endl;
            }
        }
    }
    
    void add_metric(const std::string& key, double value) {
        if (session_) {
            session_->add_metric(key, value);
        }
    }
};

/**
 * @brief Macro for easy profiling
 */
#define HEIMDALL_PROFILE_SESSION(name) \
    heimdall::ScopedProfilerSession __heimdall_profile_session__(name)

#define HEIMDALL_PROFILE_FUNCTION() \
    heimdall::ScopedProfilerSession __heimdall_profile_function__(__FUNCTION__)

#define HEIMDALL_PROFILE_BLOCK(name) \
    heimdall::ScopedProfilerSession __heimdall_profile_block__(name)

/**
 * @brief Performance monitoring utilities
 */
namespace performance_utils {

/**
 * @brief Run a performance benchmark
 */
void run_benchmark(const std::string& name, std::function<void()> func, int iterations);

/**
 * @brief Get current system memory information
 */
void print_system_memory_info();

/**
 * @brief Enable memory allocation tracking
 */
void enable_memory_tracking();

/**
 * @brief Print comprehensive performance report
 */
void print_performance_report();

} // namespace performance_utils

/**
 * @brief Get current process memory usage in bytes
 */
size_t get_current_memory_usage();

} // namespace heimdall 
