#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <numeric>

#include "common/Profiler.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/MetadataExtractor.hpp"
#include "common/Utils.hpp"
#include "common/ComponentInfo.hpp"

#if LLVM_DWARF_AVAILABLE
#include "common/DWARFExtractor.hpp"
#endif

using namespace heimdall;

/**
 * @brief Performance benchmark for SBOM generation
 */
class SBOMGenerationBenchmark {
private:
    std::string test_binary_path_;
    int iterations_;
    
public:
    SBOMGenerationBenchmark(const std::string& binary_path, int iterations = 5)
        : test_binary_path_(binary_path), iterations_(iterations) {}
    
    void run() {
        std::cout << "\n=== SBOM Generation Benchmark ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Iterations: " << iterations_ << std::endl;
        
        auto benchmark_func = [this]() {
            HEIMDALL_PROFILE_SESSION("sbom_generation");
            
            try {
                ComponentInfo component;
                component.filePath = test_binary_path_;
                
                MetadataExtractor extractor;
                if (extractor.extractMetadata(component)) {
                    SBOMGenerator generator;
                    generator.processComponent(component);
                    generator.generateSBOM();
                    
                    // Add some metrics
                    auto session = Profiler::get_instance().start_session("sbom_generation");
                    if (session) {
                        session->add_metric("components_count", 1);
                        session->add_metric("sbom_size_bytes", 1024); // Placeholder
                    }
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during SBOM generation: " << e.what() << std::endl;
            }
        };
        
        heimdall::performance_utils::run_benchmark("SBOM Generation", benchmark_func, iterations_);
    }
};

/**
 * @brief Performance benchmark for metadata extraction
 */
class MetadataExtractionBenchmark {
private:
    std::string test_binary_path_;
    int iterations_;
    
public:
    MetadataExtractionBenchmark(const std::string& binary_path, int iterations = 5)
        : test_binary_path_(binary_path), iterations_(iterations) {}
    
    void run() {
        std::cout << "\n=== Metadata Extraction Benchmark ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Iterations: " << iterations_ << std::endl;
        
        auto benchmark_func = [this]() {
            HEIMDALL_PROFILE_SESSION("metadata_extraction");
            
            try {
                ComponentInfo component;
                component.filePath = test_binary_path_;
                
                MetadataExtractor extractor;
                if (extractor.extractMetadata(component)) {
                    // Add metrics
                    auto session = Profiler::get_instance().start_session("metadata_extraction");
                    if (session) {
                        session->add_metric("components_count", 1);
                        session->add_metric("sections_count", component.getSectionCount());
                    }
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during metadata extraction: " << e.what() << std::endl;
            }
        };
        
        heimdall::performance_utils::run_benchmark("Metadata Extraction", benchmark_func, iterations_);
    }
};

/**
 * @brief Performance benchmark for DWARF extraction
 */
class DWARFExtractionBenchmark {
private:
    std::string test_binary_path_;
    int iterations_;
    
public:
    DWARFExtractionBenchmark(const std::string& binary_path, int iterations = 5)
        : test_binary_path_(binary_path), iterations_(iterations) {}
    
    void run() {
        std::cout << "\n=== DWARF Extraction Benchmark ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Iterations: " << iterations_ << std::endl;
        
        auto benchmark_func = [this]() {
            HEIMDALL_PROFILE_SESSION("dwarf_extraction");
            
            try {
                ComponentInfo component;
                component.filePath = test_binary_path_;
                
                MetadataExtractor extractor;
                if (extractor.extractDebugInfo(component)) {
                    // Add metrics
                    auto session = Profiler::get_instance().start_session("dwarf_extraction");
                    if (session) {
                        session->add_metric("dwarf_entries_count", component.functions.size());
                        session->add_metric("compile_units_count", component.compileUnits.size());
                    }
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during DWARF extraction: " << e.what() << std::endl;
            }
        };
        
        heimdall::performance_utils::run_benchmark("DWARF Extraction", benchmark_func, iterations_);
    }
};

/**
 * @brief Memory usage benchmark
 */
class MemoryUsageBenchmark {
private:
    std::string test_binary_path_;
    int iterations_;
    
public:
    MemoryUsageBenchmark(const std::string& binary_path, int iterations = 3)
        : test_binary_path_(binary_path), iterations_(iterations) {}
    
    void run() {
        std::cout << "\n=== Memory Usage Benchmark ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Iterations: " << iterations_ << std::endl;
        
        for (int i = 0; i < iterations_; ++i) {
            std::cout << "\nIteration " << (i + 1) << ":" << std::endl;
            
            size_t memory_before = heimdall::get_current_memory_usage();
            
            HEIMDALL_PROFILE_SESSION("memory_usage_test");
            
            try {
                ComponentInfo component;
                component.filePath = test_binary_path_;
                
                MetadataExtractor extractor;
                if (extractor.extractMetadata(component)) {
                    SBOMGenerator generator;
                    generator.processComponent(component);
                    generator.generateSBOM();
                    
                    size_t memory_after = heimdall::get_current_memory_usage();
                    size_t memory_delta = memory_after - memory_before;
                    
                    std::cout << "  Memory before: " << (memory_before / (1024 * 1024)) << " MB" << std::endl;
                    std::cout << "  Memory after: " << (memory_after / (1024 * 1024)) << " MB" << std::endl;
                    std::cout << "  Memory delta: " << (memory_delta / (1024 * 1024)) << " MB" << std::endl;
                    std::cout << "  Components processed: " << 1 << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during memory usage test: " << e.what() << std::endl;
            }
        }
    }
};

/**
 * @brief System resource benchmark
 */
class SystemResourceBenchmark {
private:
    std::string test_binary_path_;
    int iterations_;
    
public:
    SystemResourceBenchmark(const std::string& binary_path, int iterations = 3)
        : test_binary_path_(binary_path), iterations_(iterations) {}
    
    void run() {
        std::cout << "\n=== System Resource Benchmark ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Iterations: " << iterations_ << std::endl;
        
        heimdall::performance_utils::print_system_memory_info();
        
        for (int i = 0; i < iterations_; ++i) {
            std::cout << "\nIteration " << (i + 1) << ":" << std::endl;
            
            HEIMDALL_PROFILE_SESSION("system_resource_test");
            
            try {
                ComponentInfo component;
                component.filePath = test_binary_path_;
                
                MetadataExtractor extractor;
                if (extractor.extractMetadata(component)) {
                    SBOMGenerator generator;
                    generator.processComponent(component);
                    generator.generateSBOM();
                    
                    // Print current system state
                    heimdall::performance_utils::print_system_memory_info();
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during system resource test: " << e.what() << std::endl;
            }
        }
    }
};

/**
 * @brief Comprehensive performance test suite
 */
class PerformanceTestSuite {
private:
    std::string test_binary_path_;
    bool enable_profiling_;
    bool enable_memory_tracking_;
    std::string output_file_;
    
public:
    PerformanceTestSuite(const std::string& binary_path, bool profiling = true, bool memory_tracking = true)
        : test_binary_path_(binary_path), enable_profiling_(profiling), enable_memory_tracking_(memory_tracking) {}
    
    void set_output_file(const std::string& filename) {
        output_file_ = filename;
    }
    
    void run_all_tests() {
        std::cout << "=== Heimdall Performance Test Suite ===" << std::endl;
        std::cout << "Test binary: " << test_binary_path_ << std::endl;
        std::cout << "Profiling enabled: " << (enable_profiling_ ? "Yes" : "No") << std::endl;
        std::cout << "Memory tracking enabled: " << (enable_memory_tracking_ ? "Yes" : "No") << std::endl;
        
        // Enable profiling if requested
        if (enable_profiling_) {
            Profiler::get_instance().enable(true);
            if (!output_file_.empty()) {
                Profiler::get_instance().set_output_file(output_file_);
            }
        }
        
        // Enable memory tracking if requested
        if (enable_memory_tracking_) {
            heimdall::performance_utils::enable_memory_tracking();
        }
        
        // Run individual benchmarks
        MetadataExtractionBenchmark metadata_benchmark(test_binary_path_, 3);
        metadata_benchmark.run();
        
        DWARFExtractionBenchmark dwarf_benchmark(test_binary_path_, 3);
        dwarf_benchmark.run();
        
        SBOMGenerationBenchmark sbom_benchmark(test_binary_path_, 3);
        sbom_benchmark.run();
        
        MemoryUsageBenchmark memory_benchmark(test_binary_path_, 2);
        memory_benchmark.run();
        
        SystemResourceBenchmark resource_benchmark(test_binary_path_, 2);
        resource_benchmark.run();
        
        // Print comprehensive report
        if (enable_profiling_) {
            heimdall::performance_utils::print_performance_report();
            Profiler::get_instance().export_results();
        }
        
        std::cout << "\n=== Performance Test Suite Complete ===" << std::endl;
    }
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] <binary_path>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --iterations <N>     Number of iterations per test (default: 3)" << std::endl;
    std::cout << "  --no-profiling       Disable profiling" << std::endl;
    std::cout << "  --no-memory-tracking Disable memory tracking" << std::endl;
    std::cout << "  --output <file>      Output file for results (default: heimdall_benchmark_results.json)" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " /path/to/test/binary" << std::endl;
    std::cout << "  " << program_name << " --iterations 5 --output results.json /path/to/test/binary" << std::endl;
    std::cout << "  " << program_name << " --no-profiling /path/to/test/binary" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string binary_path;
    int iterations = 3;
    bool enable_profiling = true;
    bool enable_memory_tracking = true;
    std::string output_file = "heimdall_benchmark_results.json";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--iterations" && i + 1 < argc) {
            iterations = std::stoi(argv[++i]);
        } else if (arg == "--no-profiling") {
            enable_profiling = false;
        } else if (arg == "--no-memory-tracking") {
            enable_memory_tracking = false;
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            binary_path = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (binary_path.empty()) {
        std::cerr << "Error: No binary path specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // Check if binary exists
    std::ifstream file(binary_path);
    if (!file.good()) {
        std::cerr << "Error: Binary file not found: " << binary_path << std::endl;
        return 1;
    }
    
    try {
        PerformanceTestSuite test_suite(binary_path, enable_profiling, enable_memory_tracking);
        test_suite.set_output_file(output_file);
        test_suite.run_all_tests();
        
        std::cout << "\nBenchmark completed successfully!" << std::endl;
        if (enable_profiling) {
            std::cout << "Results saved to: " << output_file << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error during benchmark: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 