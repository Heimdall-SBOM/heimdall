# Heimdall Profiling Visualizer

This directory contains Python-based visualization tools for Heimdall profiling data.

## Overview

The profiling visualizer reads JSON output from Heimdall's profiling system and generates various charts and graphs to help analyze performance data.

## Installation

### Automatic Installation

```bash
python tools/install_visualizer.py
```

### Manual Installation

```bash
pip install matplotlib seaborn pandas numpy
```

## Usage

### Basic Usage

```bash
python tools/profiling_visualizer.py <json_file>
```

### Examples

```bash
# Generate all charts from benchmark results
python tools/profiling_visualizer.py heimdall_benchmark_results.json

# Generate specific chart types
python tools/profiling_visualizer.py results.json --chart-type time
python tools/profiling_visualizer.py results.json --chart-type memory
python tools/profiling_visualizer.py results.json --chart-type metrics

# Show charts interactively
python tools/profiling_visualizer.py results.json --show

# Custom output directory
python tools/profiling_visualizer.py results.json --output-dir my_charts
```

## Chart Types

### 1. Execution Time Chart (`--chart-type time`)
- Bar chart showing execution times for each session
- Includes error bars showing min/max times
- Useful for identifying performance bottlenecks

### 2. Memory Usage Chart (`--chart-type memory`)
- Shows mean and peak memory usage per session
- Helps identify memory-intensive operations
- Displays memory usage in MB

### 3. Metrics Heatmap (`--chart-type metrics`)
- Heatmap showing custom metrics across sessions
- Useful for comparing different performance aspects
- Automatically handles any custom metrics in the data

### 4. Timeline Chart (`--chart-type timeline`)
- Shows execution order and duration of sessions
- Helps understand the flow of operations
- Useful for identifying sequential vs parallel operations

### 5. Performance Summary (`--chart-type summary`)
- Comprehensive dashboard with multiple charts
- Includes execution time distribution, memory usage, session counts
- Provides overall performance statistics

## Output

Charts are saved as high-resolution PNG files (300 DPI) in the specified output directory. The default directory is `profiling_charts/`.

### Generated Files

When using `--chart-type all`, the following files are created:

- `execution_time.png` - Execution time analysis
- `memory_usage.png` - Memory usage patterns
- `metrics_heatmap.png` - Custom metrics visualization
- `timeline.png` - Session execution timeline
- `performance_summary.png` - Comprehensive dashboard

## Integration with Heimdall

### 1. Run Benchmarks

```bash
# Build with profiling enabled
./scripts/build.sh --standard 17 --compiler gcc --profiling --benchmarks

# Run benchmarks
cd build-cpp17
./src/tools/heimdall-benchmark /path/to/binary
```

### 2. Generate Visualizations

```bash
# Generate charts from the results
python tools/profiling_visualizer.py heimdall_benchmark_results.json
```

### 3. Analyze Results

The visualizer provides both console output and graphical charts:

```bash
# View text summary
python tools/profiling_visualizer.py results.json

# Generate charts
python tools/profiling_visualizer.py results.json --chart-type all
```

## Customization

### Adding Custom Metrics

The visualizer automatically detects and visualizes any custom metrics in the JSON data:

```cpp
// In your C++ code
session->add_metric("components_count", 42);
session->add_metric("memory_allocated", 1024);
session->add_metric("custom_metric", 3.14);
```

These will appear in the metrics heatmap and summary charts.

### Chart Customization

You can modify the visualizer code to customize:

- Chart colors and styles
- Figure sizes and layouts
- Output formats (PNG, PDF, SVG)
- Additional chart types

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   ```bash
   python tools/install_visualizer.py
   ```

2. **No Data in Charts**
   - Ensure the JSON file contains valid profiling data
   - Check that sessions have meaningful names and metrics

3. **Memory Charts Not Generated**
   - Memory tracking must be enabled in the profiling data
   - Check that `peak_memory` values are present in the JSON

4. **Display Issues**
   - Use `--show` flag for interactive display
   - Check that matplotlib backend is properly configured

### Debug Mode

For detailed debugging, you can modify the visualizer to print data:

```python
# Add to profiling_visualizer.py for debugging
print("DataFrame shape:", self.df.shape)
print("Columns:", self.df.columns.tolist())
print("Sample data:", self.df.head())
```

## Advanced Usage

### Batch Processing

```bash
# Process multiple result files
for file in results/*.json; do
    python tools/profiling_visualizer.py "$file" --output-dir "charts/$(basename "$file" .json)"
done
```

### Custom Analysis

```python
# Use the visualizer programmatically
from profiling_visualizer import ProfilingVisualizer

viz = ProfilingVisualizer("results.json")
viz.print_summary()
viz.create_execution_time_chart("custom_chart.png")
```

## Dependencies

- **matplotlib** - Core plotting library
- **seaborn** - Statistical data visualization
- **pandas** - Data manipulation and analysis
- **numpy** - Numerical computing

## License

This visualizer is part of the Heimdall project and follows the same license terms. 