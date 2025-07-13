#!/usr/bin/env python3
"""
Heimdall Profiling Visualization Demo

This script demonstrates the complete profiling and visualization workflow.
It shows how to run benchmarks and generate charts automatically.
"""

import subprocess
import sys
import os
import json
from pathlib import Path

def run_command(cmd, description):
    """Run a command and handle errors."""
    print(f"\n🔄 {description}")
    print(f"Running: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        print(f"✅ {description} completed successfully")
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"❌ {description} failed:")
        print(f"Error: {e.stderr}")
        return None

def check_dependencies():
    """Check if required dependencies are installed."""
    print("🔍 Checking dependencies...")
    
    try:
        import matplotlib.pyplot as plt
        import seaborn as sns
        import pandas as pd
        import numpy as np
        print("✅ All Python dependencies are installed")
        return True
    except ImportError as e:
        print(f"❌ Missing dependency: {e}")
        print("Installing dependencies...")
        return run_command([sys.executable, "install_visualizer.py"], "Installing dependencies") is not None

def find_binary_for_testing():
    """Find a suitable binary for testing."""
    # Look for common system binaries
    test_binaries = [
        "/usr/bin/ls",
        "/usr/bin/cat",
        "/usr/bin/echo",
        "/bin/ls",
        "/bin/cat"
    ]
    
    for binary in test_binaries:
        if os.path.exists(binary):
            return binary
    
    print("⚠️  No suitable test binary found")
    print("Please provide a binary path as argument")
    return None

def run_demo(binary_path=None):
    """Run the complete profiling and visualization demo."""
    print("🚀 Heimdall Profiling Visualization Demo")
    print("=" * 50)
    
    # Check dependencies
    if not check_dependencies():
        print("❌ Failed to install dependencies")
        return False
    
    # Find test binary
    if not binary_path:
        binary_path = find_binary_for_testing()
        if not binary_path:
            return False
    
    print(f"📁 Using test binary: {binary_path}")
    
    # Step 1: Run benchmark
    benchmark_cmd = [
        "./heimdall-benchmark",
        "--iterations", "3",
        "--output", "demo_results.json",
        binary_path
    ]
    
    benchmark_output = run_command(benchmark_cmd, "Running benchmark")
    if not benchmark_output:
        return False
    
    # Step 2: Check if results were generated
    if not os.path.exists("demo_results.json"):
        print("❌ Benchmark results not found")
        return False
    
    print(f"📊 Benchmark results saved to: demo_results.json")
    
    # Step 3: Generate visualizations
    viz_cmd = [
        sys.executable, "../tools/profiling_visualizer.py",
        "demo_results.json",
        "--output-dir", "demo_charts"
    ]
    
    viz_output = run_command(viz_cmd, "Generating visualizations")
    if not viz_output:
        return False
    
    # Step 4: Show summary
    summary_cmd = [
        sys.executable, "../tools/profiling_visualizer.py",
        "demo_results.json"
    ]
    
    summary_output = run_command(summary_cmd, "Generating summary")
    
    # Step 5: List generated files
    print("\n📁 Generated files:")
    if os.path.exists("demo_charts"):
        for file in os.listdir("demo_charts"):
            if file.endswith(".png"):
                print(f"  📈 demo_charts/{file}")
    
    print("\n🎉 Demo completed successfully!")
    print("\nNext steps:")
    print("1. View the generated charts in the 'demo_charts' directory")
    print("2. Analyze the benchmark results in 'demo_results.json'")
    print("3. Run custom visualizations:")
    print("   python ../tools/profiling_visualizer.py demo_results.json --chart-type time")
    print("   python ../tools/profiling_visualizer.py demo_results.json --chart-type memory")
    
    return True

def main():
    """Main function."""
    if len(sys.argv) > 1:
        binary_path = sys.argv[1]
    else:
        binary_path = None
    
    success = run_demo(binary_path)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 