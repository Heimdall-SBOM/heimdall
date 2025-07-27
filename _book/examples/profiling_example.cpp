#include <chrono>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "common/ComponentInfo.hpp"
#include "common/MetadataExtractor.hpp"
#include "common/Profiler.hpp"
#include "common/SBOMGenerator.hpp"

using namespace heimdall;

/**
 * @brief Example demonstrating how to use the profiling system
 */
class ProfilingExample
{
   private:
   std::string test_binary_path_;

   public:
       ProfilingExample(std::string binary_path) : test_binary_path_(std::move(binary_path)) {}

       void run_basic_profiling()
       {
          std::cout << "=== Basic Profiling Example ===" << std::endl;

          // Enable the profiler
          Profiler::get_instance().enable(true);

          // Profile a simple operation
          {
             HEIMDALL_PROFILE_SESSION("simple_operation");

             // Simulate some work
             std::this_thread::sleep_for(std::chrono::milliseconds(100));

             // Add custom metrics
             auto session = Profiler::get_instance().start_session("simple_operation");
             if (session)
             {
                session->add_metric("items_processed", 42);
                session->add_metric("memory_allocated", 1024);
             }
          }

          // Profile multiple operations
          for (int i = 0; i < 3; ++i)
          {
             std::string session_name = "iteration_" + std::to_string(i);
             HEIMDALL_PROFILE_SESSION(session_name);

             // Simulate different workloads
             std::this_thread::sleep_for(std::chrono::milliseconds(50 + i * 25));

             auto session = Profiler::get_instance().start_session(session_name);
             if (session)
             {
                session->add_metric("iteration", i);
                session->add_metric("workload_factor", 1.0 + i * 0.5);
             }
          }

          // Print results
          Profiler::get_instance().print_summary();
       }

   void run_sbom_profiling()
   {
      std::cout << "\n=== SBOM Generation Profiling ===" << std::endl;

      try
      {
         // Profile metadata extraction
         {
            HEIMDALL_PROFILE_SESSION("metadata_extraction");

            ComponentInfo component;
            component.filePath = test_binary_path_;

            MetadataExtractor extractor;
            if (extractor.extractMetadata(component))
            {
               auto session = Profiler::get_instance().start_session("metadata_extraction");
               if (session)
               {
                  session->add_metric("components_count", 1);
                  session->add_metric("sections_count", component.getSectionCount());
               }
            }
         }

         // Profile SBOM generation
         {
            HEIMDALL_PROFILE_SESSION("sbom_generation");

            ComponentInfo component;
            component.filePath = test_binary_path_;

            MetadataExtractor extractor;
            if (extractor.extractMetadata(component))
            {
               SBOMGenerator generator;
               generator.processComponent(component);
               generator.generateSBOM();

               auto session = Profiler::get_instance().start_session("sbom_generation");
               if (session)
               {
                  session->add_metric("sbom_size_bytes", 1024);  // Placeholder
                  session->add_metric("components_count", 1);
               }
            }
         }
      }
      catch (const std::exception& e)
      {
         std::cerr << "Error during SBOM profiling: " << e.what() << std::endl;
      }
   }

   void run_memory_profiling()
   {
      std::cout << "\n=== Memory Usage Profiling ===" << std::endl;

      // Print initial memory state
      heimdall::performance_utils::print_system_memory_info();

      // Profile memory-intensive operations
      for (int i = 0; i < 3; ++i)
      {
         std::string session_name = "memory_test_" + std::to_string(i);
         HEIMDALL_PROFILE_SESSION(session_name);

         // Allocate some memory
         std::vector<std::string> strings;
         strings.reserve(1000);
         for (int j = 0; j < 1000; ++j)
         {
            strings.push_back("Test string " + std::to_string(j));
         }

         // Simulate work
         std::this_thread::sleep_for(std::chrono::milliseconds(100));

         auto session = Profiler::get_instance().start_session(session_name);
         if (session)
         {
            session->add_metric("strings_allocated", strings.size());
            session->add_metric(
               "total_string_length",
               std::accumulate(strings.begin(), strings.end(), 0,
                               [](int sum, const std::string& s) { return sum + s.length(); }));
         }
      }

      // Print final memory state
      heimdall::performance_utils::print_system_memory_info();
   }

   void run_comprehensive_profiling()
   {
      std::cout << "\n=== Comprehensive Profiling Example ===" << std::endl;

      // Enable memory tracking
      heimdall::performance_utils::enable_memory_tracking();

      // Run all profiling examples
      run_basic_profiling();
      run_sbom_profiling();
      run_memory_profiling();

      // Export results
      Profiler::get_instance().export_results("profiling_example_results.json");

      // Print comprehensive report
      heimdall::performance_utils::print_performance_report();
   }
};

void print_usage(const char* program_name)
{
   std::cout << "Usage: " << program_name << " [OPTIONS] <binary_path>" << std::endl;
   std::cout << std::endl;
   std::cout << "Options:" << std::endl;
   std::cout << "  --basic              Run basic profiling example" << std::endl;
   std::cout << "  --sbom               Run SBOM generation profiling" << std::endl;
   std::cout << "  --memory             Run memory usage profiling" << std::endl;
   std::cout << "  --comprehensive      Run all profiling examples" << std::endl;
   std::cout << "  --help               Show this help message" << std::endl;
   std::cout << std::endl;
   std::cout << "Examples:" << std::endl;
   std::cout << "  " << program_name << " --basic /path/to/binary" << std::endl;
   std::cout << "  " << program_name << " --comprehensive /path/to/binary" << std::endl;
}

int main(int argc, char* argv[])
{
   if (argc < 3)
   {
      print_usage(argv[0]);
      return 1;
   }

   std::string binary_path;
   bool        run_basic         = false;
   bool        run_sbom          = false;
   bool        run_memory        = false;
   bool        run_comprehensive = false;

   // Parse command line arguments
   for (int i = 1; i < argc; ++i)
   {
      std::string arg = argv[i];

      if (arg == "--help")
      {
         print_usage(argv[0]);
         return 0;
      }
      else if (arg == "--basic")
      {
         run_basic = true;
      }
      else if (arg == "--sbom")
      {
         run_sbom = true;
      }
      else if (arg == "--memory")
      {
         run_memory = true;
      }
      else if (arg == "--comprehensive")
      {
         run_comprehensive = true;
      }
      else if (arg[0] != '-')
      {
         binary_path = arg;
      }
      else
      {
         std::cerr << "Unknown option: " << arg << std::endl;
         print_usage(argv[0]);
         return 1;
      }
   }

   if (binary_path.empty())
   {
      std::cerr << "Error: No binary path specified" << std::endl;
      print_usage(argv[0]);
      return 1;
   }

   // Check if binary exists
   std::ifstream file(binary_path);
   if (!file.good())
   {
      std::cerr << "Error: Binary file not found: " << binary_path << std::endl;
      return 1;
   }

   try
   {
      ProfilingExample example(binary_path);

      if (run_comprehensive)
      {
         example.run_comprehensive_profiling();
      }
      else
      {
         if (run_basic)
         {
            example.run_basic_profiling();
         }
         if (run_sbom)
         {
            example.run_sbom_profiling();
         }
         if (run_memory)
         {
            example.run_memory_profiling();
         }
      }

      std::cout << "\nProfiling example completed successfully!" << std::endl;
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error during profiling: " << e.what() << std::endl;
      return 1;
   }

   return 0;
}