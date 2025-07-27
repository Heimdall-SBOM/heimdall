# Heimdall Profiling and Performance Measurement

This document describes the profiling and performance measurement capabilities built into Heimdall.

## Overview

Heimdall includes a comprehensive profiling system that allows you to:

- Measure execution time of specific operations
- Track memory usage and allocation patterns
- Monitor system resource utilization
- Generate performance reports and benchmarks
- Export results in JSON format for further analysis

## Building with Profiling Support

To enable profiling support, build Heimdall with the `--profiling` and `--benchmarks` flags:

```bash
./scripts/build.sh --standard 17 --compiler gcc --profiling --benchmarks
```

This will:
- Enable the profiling library
- Build the benchmark tools
- Include performance measurement utilities
- Add profiling examples

## Core Components

### 1. Profiler Class

The main profiling interface is provided by the `heimdall::Profiler` class:

```cpp
#include "common/Profiler.hpp"

// Enable profiling
heimdall::Profiler::get_instance().enable(true);

// Start a profiling session
auto session = heimdall::Profiler::get_instance().start_session("my_operation");

// Add custom metrics
session->add_metric("items_processed", 42);
session->add_metric("memory_allocated", 1024);

// End the session
heimdall::Profiler::get_instance().end_session("my_operation");

// Print summary
heimdall::Profiler::get_instance().print_summary();

// Export results
heimdall::Profiler::get_instance().export_results("results.json");
```

### 2. RAII Wrapper

For automatic session management, use the `ScopedProfilerSession`:

```cpp
{
    heimdall::ScopedProfilerSession session("my_operation");
    // ... your code here ...
    // Session automatically ends when scope exits
}
```

### 3. Convenience Macros

Use the provided macros for easy profiling:

```cpp
// Profile a function
HEIMDALL_PROFILE_FUNCTION();

// Profile a specific session
HEIMDALL_PROFILE_SESSION("my_session");

// Profile a code block
HEIMDALL_PROFILE_BLOCK("my_block");
```

## Performance Utilities

### Memory Tracking

```cpp
#include "common/Profiler.hpp"

// Get current memory usage
size_t memory_usage = heimdall::get_current_memory_usage();

// Print system memory information
heimdall::performance_utils::print_system_memory_info();

// Enable memory allocation tracking
heimdall::performance_utils::enable_memory_tracking();
```

### Benchmarking

```cpp
#include "common/Profiler.hpp"

// Run a benchmark
heimdall::performance_utils::run_benchmark("My Benchmark", []() {
    // Your benchmark code here
}, 5); // 5 iterations
```

## Tools

### 1. Performance Benchmark Tool

The `heimdall-benchmark` tool provides comprehensive performance testing:

```bash
# Basic usage
./heimdall-benchmark /path/to/binary

# With options
./heimdall-benchmark --iterations 5 --output results.json /path/to/binary

# Disable profiling
./heimdall-benchmark --no-profiling /path/to/binary
```

The benchmark tool performs:
- Metadata extraction benchmarks
- DWARF extraction benchmarks
- SBOM generation benchmarks
- Memory usage analysis
- System resource monitoring

### 2. Profiling Example

The `profiling_example` demonstrates various profiling techniques:

```bash
# Run basic profiling
./profiling_example --basic /path/to/binary

# Run SBOM profiling
./profiling_example --sbom /path/to/binary

# Run memory profiling
./profiling_example --memory /path/to/binary

# Run comprehensive profiling
./profiling_example --comprehensive /path/to/binary
```

### 3. Profiling Visualizer

The Python-based `profiling_visualizer.py` tool generates graphical representations of profiling data:

```bash
# Install dependencies
python tools/install_visualizer.py

# Generate all charts
python tools/profiling_visualizer.py results.json

# Generate specific chart types
python tools/profiling_visualizer.py results.json --chart-type time
python tools/profiling_visualizer.py results.json --chart-type memory
python tools/profiling_visualizer.py results.json --chart-type metrics
python tools/profiling_visualizer.py results.json --chart-type timeline
python tools/profiling_visualizer.py results.json --chart-type summary

# Show charts interactively
python tools/profiling_visualizer.py results.json --show

# Custom output directory
python tools/profiling_visualizer.py results.json --output-dir my_charts
```

The visualizer generates:
- **Execution Time Charts**: Bar charts with error bars showing min/max times
- **Memory Usage Charts**: Mean and peak memory usage per session
- **Metrics Heatmaps**: Custom metrics visualization across sessions
- **Timeline Charts**: Session execution order and duration
- **Performance Summary**: Comprehensive dashboard with multiple charts

For detailed usage instructions, see `tools/README.md`.

## Integration Examples

### Profiling SBOM Generation

```cpp
#include "common/Profiler.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/MetadataExtractor.hpp"
#include "common/ComponentInfo.hpp"

void profile_sbom_generation(const std::string& binary_path) {
    HEIMDALL_PROFILE_SESSION("sbom_generation");
    
    ComponentInfo component;
    component.filePath = binary_path;
    
    MetadataExtractor extractor;
    if (extractor.extractMetadata(component)) {
        SBOMGenerator generator;
        generator.processComponent(component);
        generator.generateSBOM();
        
        // Add metrics
        auto session = heimdall::Profiler::get_instance().start_session("sbom_generation");
        if (session) {
            session->add_metric("components_count", 1);
            session->add_metric("sections_count", component.getSectionCount());
        }
    }
}
```

### Profiling Memory Usage

```cpp
#include "common/Profiler.hpp"

void profile_memory_usage() {
    size_t memory_before = heimdall::get_current_memory_usage();
    
    HEIMDALL_PROFILE_SESSION("memory_intensive_operation");
    
    // Your memory-intensive code here
    std::vector<std::string> large_data;
    for (int i = 0; i < 10000; ++i) {
        large_data.push_back("Test string " + std::to_string(i));
    }
    
    size_t memory_after = heimdall::get_current_memory_usage();
    size_t memory_delta = memory_after - memory_before;
    
    std::cout << "Memory delta: " << memory_delta << " bytes" << std::endl;
}
```

## Output Formats

### Console Output

The profiler provides detailed console output:

```
=== Heimdall Profiler Summary ===
Total sessions: 3

Session: metadata_extraction
  Time: 0.004s (4ms)
  Peak Memory: 524288 bytes
  Metrics:
    components_count: 1
    sections_count: 15

Session: sbom_generation
  Time: 0.003s (3ms)
  Peak Memory: 0 bytes
  Metrics:
    sbom_size_bytes: 1024

=== Summary ===
Total time: 0.007s
Total peak memory: 524288 bytes
Average time per session: 0.002s
```

### JSON Export

Results are exported in JSON format:

```json
{
  "profiler_results": {
    "total_sessions": 3,
    "sessions": [
      {
        "name": "metadata_extraction",
        "elapsed_seconds": 0.00463847,
        "elapsed_milliseconds": 4,
        "peak_memory": 524288,
        "metrics": {
          "components_count": 1,
          "sections_count": 15
        }
      }
    ]
  }
}
```

### Visualization Workflow

The complete profiling workflow includes graphical analysis:

1. **Run Benchmarks**: Generate profiling data
   ```bash
   ./heimdall-benchmark /path/to/binary
   ```

2. **Generate Charts**: Create visual representations
   ```bash
   python tools/profiling_visualizer.py heimdall_benchmark_results.json
   ```

3. **Analyze Results**: Review both console and graphical output
   - Console output provides detailed statistics
   - Charts show trends and patterns
   - Performance summary gives overview

4. **Custom Analysis**: Use specific chart types for focused analysis
   ```bash
   # Focus on execution times
   python tools/profiling_visualizer.py results.json --chart-type time
   
   # Analyze memory patterns
   python tools/profiling_visualizer.py results.json --chart-type memory
   
   # Compare custom metrics
   python tools/profiling_visualizer.py results.json --chart-type metrics
   ```

## Performance Tips

### 1. Minimize Profiling Overhead

- Only enable profiling when needed
- Use RAII wrappers for automatic cleanup
- Avoid profiling in tight loops

### 2. Meaningful Session Names

- Use descriptive session names
- Include context in session names
- Use consistent naming conventions

### 3. Useful Metrics

- Track relevant business metrics
- Include size/complexity measures
- Monitor resource usage patterns

### 4. Analysis

- Compare multiple runs
- Look for outliers
- Analyze trends over time
- Use the JSON output for detailed analysis

## Configuration Options

### Build Options

- `--profiling`: Enable profiling support
- `--benchmarks`: Build benchmark tools
- `--no-profiling`: Disable profiling (for production builds)

### Runtime Options

- Enable/disable profiling at runtime
- Configure output file locations
- Set custom metrics and thresholds

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure profiling is enabled during build
2. **Missing Symbols**: Check that all required headers are included
3. **Performance Impact**: Disable profiling in production builds
4. **Memory Leaks**: Use RAII wrappers for automatic cleanup

### Debug Tips

- Enable verbose output for detailed information
- Check system memory usage during profiling
- Monitor CPU usage during benchmarks
- Validate JSON output format

## Future Enhancements

Planned improvements include:

- CPU profiling with call graphs
- Network I/O profiling
- Database query profiling
- Integration with external profiling tools
- Real-time monitoring dashboard
- Performance regression detection
- Automated performance testing

## API Reference

### Profiler Class

```cpp
class Profiler {
public:
    static Profiler& get_instance();
    void enable(bool enabled = true);
    bool is_enabled() const;
    void set_output_file(const std::string& filename);
    std::shared_ptr<PerformanceSession> start_session(const std::string& name);
    void end_session(const std::string& name);
    void clear_sessions();
    void export_results(const std::string& filename = "");
    void print_summary();
};
```

### PerformanceSession Class

```cpp
class PerformanceSession {
public:
    void add_metric(const std::string& key, double value);
    void stop();
    double get_elapsed_seconds() const;
    double get_elapsed_milliseconds() const;
    const std::string& get_name() const;
    const std::map<std::string, double>& get_metrics() const;
};
```

### Utility Functions

```cpp
namespace performance_utils {
    void run_benchmark(const std::string& name, std::function<void()> func, int iterations);
    void print_system_memory_info();
    void enable_memory_tracking();
    void print_performance_report();
}

size_t get_current_memory_usage();
```

