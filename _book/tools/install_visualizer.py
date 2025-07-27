#!/usr/bin/env python3
"""
Installation script for Heimdall Profiling Visualizer dependencies
"""

import subprocess
import sys
import os

def install_requirements():
    """Install required Python packages for the visualizer."""
    print("Installing Heimdall Profiling Visualizer dependencies...")
    
    # Get the directory of this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    requirements_file = os.path.join(script_dir, "requirements.txt")
    
    try:
        # Install requirements
        subprocess.check_call([
            sys.executable, "-m", "pip", "install", "-r", requirements_file
        ])
        print("✅ Dependencies installed successfully!")
        print("\nYou can now use the visualizer:")
        print("  python tools/profiling_visualizer.py <json_file>")
        print("\nExample:")
        print("  python tools/profiling_visualizer.py heimdall_benchmark_results.json")
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Error installing dependencies: {e}")
        print("\nYou can try installing manually:")
        print("  pip install matplotlib seaborn pandas numpy")
        sys.exit(1)
    except FileNotFoundError:
        print("❌ pip not found. Please install pip first.")
        sys.exit(1)

def check_dependencies():
    """Check if all required packages are already installed."""
    required_packages = ['matplotlib', 'seaborn', 'pandas', 'numpy']
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print(f"Missing packages: {', '.join(missing_packages)}")
        return False
    else:
        print("✅ All dependencies are already installed!")
        return True

if __name__ == "__main__":
    print("Heimdall Profiling Visualizer - Dependency Installer")
    print("=" * 50)
    
    if check_dependencies():
        print("\nVisualizer is ready to use!")
    else:
        install_requirements() 