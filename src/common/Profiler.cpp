#include "Profiler.hpp"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <utility>

// Static member definitions
heimdall::Profiler* heimdall::Profiler::instance_ = nullptr;
std::mutex          heimdall::Profiler::instance_mutex_;

#ifdef _WIN32
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace heimdall
{

/**
 * @brief Get current process memory usage in bytes
 */
size_t get_current_memory_usage()
{
#ifdef _WIN32
   PROCESS_MEMORY_COUNTERS_EX pmc;
   if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
   {
      return pmc.WorkingSetSize;
   }
   return 0;
#else
   FILE* file = fopen("/proc/self/status", "r");
   if (!file)
   {
      return 0;
   }

   size_t memory_usage = 0;
   char   line[256];

   while (fgets(line, sizeof(line), file))
   {
      if (strncmp(line, "VmRSS:", 6) == 0)
      {
         sscanf(line, "VmRSS: %zu", &memory_usage);
         memory_usage *= 1024;  // Convert KB to bytes
         break;
      }
   }

   fclose(file);
   return memory_usage;
#endif
}

/**
 * @brief Get system total memory in bytes
 */
size_t get_system_total_memory()
{
#ifdef _WIN32
   MEMORYSTATUSEX memInfo;
   memInfo.dwLength = sizeof(MEMORYSTATUSEX);
   if (GlobalMemoryStatusEx(&memInfo))
   {
      return memInfo.ullTotalPhys;
   }
   return 0;
#elif defined(__APPLE__)
   uint64_t total_memory;
   size_t   len = sizeof(total_memory);
   if (sysctlbyname("hw.memsize", &total_memory, &len, nullptr, 0) == 0)
   {
      return static_cast<size_t>(total_memory);
   }
   return 0;
#else
   struct sysinfo si{};
   if (sysinfo(&si) == 0)
   {
      return si.totalram * si.mem_unit;
   }
   return 0;
#endif
}

/**
 * @brief Get system available memory in bytes
 */
size_t get_system_available_memory()
{
#ifdef _WIN32
   MEMORYSTATUSEX memInfo;
   memInfo.dwLength = sizeof(MEMORYSTATUSEX);
   if (GlobalMemoryStatusEx(&memInfo))
   {
      return memInfo.ullAvailPhys;
   }
   return 0;
#elif defined(__APPLE__)
   vm_statistics64_data_t vm_stats;
   mach_msg_type_number_t info_count = HOST_VM_INFO64_COUNT;
   host_t                 host       = mach_host_self();
   if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stats, &info_count) ==
       KERN_SUCCESS)
   {
      return static_cast<size_t>(vm_stats.free_count * vm_page_size);
   }
   return 0;
#else
   struct sysinfo si{};
   if (sysinfo(&si) == 0)
   {
      return si.freeram * si.mem_unit;
   }
   return 0;
#endif
}

/**
 * @brief Performance benchmark runner
 */
class PerformanceBenchmark
{
   private:
   std::string           name_;
   std::function<void()> test_function_;
   int                   iterations_;
   std::vector<double>   times_;
   std::vector<size_t>   memory_usage_;

   public:
   PerformanceBenchmark(std::string name, std::function<void()> func, int iterations = 1)
      : name_(std::move(name)), test_function_(std::move(func)), iterations_(iterations)
   {
   }

   void run()
   {
      times_.clear();
      memory_usage_.clear();

      std::cout << "Running benchmark: " << name_ << " (" << iterations_ << " iterations)"
                << std::endl;

      for (int i = 0; i < iterations_; ++i)
      {
         size_t       memory_before = get_current_memory_usage();

         HighResTimer timer;
         timer.start();

         test_function_();

         timer.stop();

         size_t memory_after = get_current_memory_usage();
         size_t memory_delta = memory_after - memory_before;

         times_.push_back(timer.elapsed_seconds());
         memory_usage_.push_back(memory_delta);

         std::cout << "  Iteration " << (i + 1) << ": " << std::fixed << std::setprecision(6)
                   << timer.elapsed_seconds() << "s, " << memory_delta << " bytes" << std::endl;
      }
   }

   void print_statistics() const
   {
      if (times_.empty())
      {
         std::cout << "No benchmark data available" << std::endl;
         return;
      }

      // Calculate statistics
      double total_time   = 0.0;
      size_t total_memory = 0;

      for (double time : times_)
      {
         total_time += time;
      }

      for (size_t memory : memory_usage_)
      {
         total_memory += memory;
      }

      double avg_time   = total_time / times_.size();
      double avg_memory = static_cast<double>(total_memory) / memory_usage_.size();

      // Find min/max
      auto min_time_it   = std::min_element(times_.begin(), times_.end());
      auto max_time_it   = std::max_element(times_.begin(), times_.end());
      auto min_memory_it = std::min_element(memory_usage_.begin(), memory_usage_.end());
      auto max_memory_it = std::max_element(memory_usage_.begin(), memory_usage_.end());

      std::cout << "\n=== Benchmark Statistics: " << name_ << " ===" << std::endl;
      std::cout << "Iterations: " << iterations_ << std::endl;
      std::cout << "Time (seconds):" << std::endl;
      std::cout << "  Average: " << std::fixed << std::setprecision(6) << avg_time << std::endl;
      std::cout << "  Min: " << std::fixed << std::setprecision(6) << *min_time_it << std::endl;
      std::cout << "  Max: " << std::fixed << std::setprecision(6) << *max_time_it << std::endl;
      std::cout << "Memory (bytes):" << std::endl;
      std::cout << "  Average: " << static_cast<size_t>(avg_memory) << std::endl;
      std::cout << "  Min: " << *min_memory_it << std::endl;
      std::cout << "  Max: " << *max_memory_it << std::endl;
   }

   const std::vector<double>& get_times() const
   {
      return times_;
   }
   const std::vector<size_t>& get_memory_usage() const
   {
      return memory_usage_;
   }
};

/**
 * @brief Memory allocation tracker
 */
class MemoryAllocationTracker
{
   private:
   static std::atomic<size_t> total_allocated_;
   static std::atomic<size_t> peak_allocated_;
   static std::atomic<size_t> allocation_count_;
   static std::atomic<size_t> deallocation_count_;

   public:
   static void record_allocation(size_t size)
   {
      total_allocated_ += size;
      allocation_count_++;

      size_t current = total_allocated_.load();
      size_t peak    = peak_allocated_.load();
      while (current > peak && !peak_allocated_.compare_exchange_weak(peak, current))
      {
         // Retry if peak was updated by another thread
      }
   }

   static void record_deallocation(size_t size)
   {
      total_allocated_ -= size;
      deallocation_count_++;
   }

   static size_t get_total_allocated()
   {
      return total_allocated_.load();
   }
   static size_t get_peak_allocated()
   {
      return peak_allocated_.load();
   }
   static size_t get_allocation_count()
   {
      return allocation_count_.load();
   }
   static size_t get_deallocation_count()
   {
      return deallocation_count_.load();
   }

   static void reset()
   {
      total_allocated_    = 0;
      peak_allocated_     = 0;
      allocation_count_   = 0;
      deallocation_count_ = 0;
   }

   static void print_statistics()
   {
      std::cout << "\n=== Memory Allocation Statistics ===" << std::endl;
      std::cout << "Total allocated: " << get_total_allocated() << " bytes" << std::endl;
      std::cout << "Peak allocated: " << get_peak_allocated() << " bytes" << std::endl;
      std::cout << "Allocation count: " << get_allocation_count() << std::endl;
      std::cout << "Deallocation count: " << get_deallocation_count() << std::endl;
      std::cout << "Leaked memory: " << (get_total_allocated() - get_deallocation_count())
                << " bytes" << std::endl;
   }
};

// Static member initialization
std::atomic<size_t> MemoryAllocationTracker::total_allocated_(0);
std::atomic<size_t> MemoryAllocationTracker::peak_allocated_(0);
std::atomic<size_t> MemoryAllocationTracker::allocation_count_(0);
std::atomic<size_t> MemoryAllocationTracker::deallocation_count_(0);

/**
 * @brief CPU usage tracker
 */
class CPUUsageTracker
{
   private:
   std::chrono::high_resolution_clock::time_point last_check_;
   double                                         cpu_usage_percent_;

#ifdef _WIN32
   FILETIME last_idle_time_;
   FILETIME last_kernel_time_;
   FILETIME last_user_time_;
#else
   clock_t last_cpu_time_{};
   clock_t last_sys_time_{};
   clock_t last_user_time_{};
#endif

   public:
   CPUUsageTracker() : cpu_usage_percent_(0.0)
   {
      reset();
   }

   void reset()
   {
      last_check_ = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
      FILETIME idle_time, kernel_time, user_time;
      GetSystemTimes(&idle_time, &kernel_time, &user_time);
      last_idle_time_   = idle_time;
      last_kernel_time_ = kernel_time;
      last_user_time_   = user_time;
#else
      last_cpu_time_ = clock();
      // For Linux, we'd need to read /proc/stat for system-wide CPU usage
      // This is a simplified version
#endif
   }

   double get_cpu_usage()
   {
      auto now = std::chrono::high_resolution_clock::now();
      auto elapsed =
         std::chrono::duration_cast<std::chrono::milliseconds>(now - last_check_).count();

      if (elapsed < 100)
      {  // Don't check too frequently
         return cpu_usage_percent_;
      }

#ifdef _WIN32
      FILETIME idle_time, kernel_time, user_time;
      GetSystemTimes(&idle_time, &kernel_time, &user_time);

      ULARGE_INTEGER idle, kernel, user;
      idle.LowPart    = idle_time.dwLowDateTime;
      idle.HighPart   = idle_time.dwHighDateTime;
      kernel.LowPart  = kernel_time.dwLowDateTime;
      kernel.HighPart = kernel_time.dwHighDateTime;
      user.LowPart    = user_time.dwLowDateTime;
      user.HighPart   = user_time.dwHighDateTime;

      ULARGE_INTEGER last_idle, last_kernel, last_user;
      last_idle.LowPart    = last_idle_time_.dwLowDateTime;
      last_idle.HighPart   = last_idle_time_.dwHighDateTime;
      last_kernel.LowPart  = last_kernel_time_.dwLowDateTime;
      last_kernel.HighPart = last_kernel_time_.dwHighDateTime;
      last_user.LowPart    = last_user_time_.dwLowDateTime;
      last_user.HighPart   = last_user_time_.dwHighDateTime;

      ULONGLONG kernel_diff = kernel.QuadPart - last_kernel.QuadPart;
      ULONGLONG user_diff   = user.QuadPart - last_user.QuadPart;
      ULONGLONG idle_diff   = idle.QuadPart - last_idle.QuadPart;

      ULONGLONG total_diff = kernel_diff + user_diff;
      if (total_diff > 0)
      {
         cpu_usage_percent_ = 100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
      }

      last_idle_time_   = idle_time;
      last_kernel_time_ = kernel_time;
      last_user_time_   = user_time;
#else
      // Simplified CPU usage calculation for Linux
      clock_t current_cpu_time = clock();
      double  cpu_time_diff    = static_cast<double>(current_cpu_time - last_cpu_time_);
      cpu_usage_percent_       = (cpu_time_diff / CLOCKS_PER_SEC) * 100.0;
      last_cpu_time_           = current_cpu_time;
#endif

      last_check_ = now;
      return cpu_usage_percent_;
   }
};

/**
 * @brief Performance monitoring utilities
 */
namespace performance_utils
{

/**
 * @brief Run a performance benchmark
 */
void run_benchmark(const std::string& name, std::function<void()> func, int iterations)
{
   PerformanceBenchmark benchmark(name, std::move(func), iterations);
   benchmark.run();
   benchmark.print_statistics();
}

/**
 * @brief Get current system memory information
 */
void print_system_memory_info()
{
   size_t total_memory           = get_system_total_memory();
   size_t available_memory       = get_system_available_memory();
   size_t used_memory            = total_memory - available_memory;
   size_t current_process_memory = get_current_memory_usage();

   std::cout << "\n=== System Memory Information ===" << std::endl;
   std::cout << "Total system memory: " << (total_memory / (1024 * 1024)) << " MB" << std::endl;
   std::cout << "Available system memory: " << (available_memory / (1024 * 1024)) << " MB"
             << std::endl;
   std::cout << "Used system memory: " << (used_memory / (1024 * 1024)) << " MB" << std::endl;
   std::cout << "Current process memory: " << (current_process_memory / (1024 * 1024)) << " MB"
             << std::endl;
   std::cout << "Memory usage percentage: " << std::fixed << std::setprecision(2)
             << (static_cast<double>(used_memory) / total_memory * 100.0) << "%" << std::endl;
}

/**
 * @brief Enable memory allocation tracking
 */
void enable_memory_tracking()
{
   // This would require overriding global new/delete operators
   // For now, we'll just enable the profiler
   Profiler::get_instance().enable(true);
}

/**
 * @brief Print comprehensive performance report
 */
void print_performance_report()
{
   print_system_memory_info();
   MemoryAllocationTracker::print_statistics();
   Profiler::get_instance().print_summary();
}

}  // namespace performance_utils

}  // namespace heimdall