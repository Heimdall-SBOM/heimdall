#!/usr/bin/env python3
"""
Heimdall Profiling Visualizer

This script reads JSON profiling results from Heimdall and generates
various graphical representations of the performance data.

Usage:
    python profiling_visualizer.py <json_file> [options]
"""

import json
import argparse
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import seaborn as sns
import pandas as pd
import numpy as np
from pathlib import Path
import sys
from typing import Dict, List, Any, Optional
import warnings

# Suppress matplotlib warnings
warnings.filterwarnings('ignore', category=UserWarning, module='matplotlib')

class ProfilingVisualizer:
    def __init__(self, json_file: str):
        """Initialize the visualizer with JSON profiling data."""
        self.json_file = json_file
        self.data = self._load_data()
        self.df = self._create_dataframe()
        
    def _load_data(self) -> Dict[str, Any]:
        """Load and parse the JSON profiling data."""
        try:
            with open(self.json_file, 'r') as f:
                data = json.load(f)
            return data
        except FileNotFoundError:
            print(f"Error: File '{self.json_file}' not found.")
            sys.exit(1)
        except json.JSONDecodeError:
            print(f"Error: Invalid JSON in file '{self.json_file}'.")
            sys.exit(1)
    
    def _create_dataframe(self) -> pd.DataFrame:
        """Create a pandas DataFrame from the profiling data."""
        sessions = self.data.get('profiler_results', {}).get('sessions', [])
        
        if not sessions:
            print("Warning: No sessions found in profiling data.")
            return pd.DataFrame()
        
        # Extract data for DataFrame
        records = []
        for session in sessions:
            record = {
                'session_name': session.get('name', 'Unknown'),
                'elapsed_seconds': session.get('elapsed_seconds', 0),
                'elapsed_milliseconds': session.get('elapsed_milliseconds', 0),
                'peak_memory': session.get('peak_memory', 0),
                'metrics': session.get('metrics', {})
            }
            
            # Flatten metrics into separate columns
            for key, value in record['metrics'].items():
                record[f'metric_{key}'] = value
            
            records.append(record)
        
        df = pd.DataFrame(records)
        
        # Convert memory to MB for better readability
        if 'peak_memory' in df.columns:
            df['peak_memory_mb'] = df['peak_memory'] / (1024 * 1024)
        
        return df
    
    def create_execution_time_chart(self, output_file: Optional[str] = None):
        """Create a bar chart showing execution times for each session."""
        if self.df.empty:
            print("No data available for visualization.")
            return
        
        plt.figure(figsize=(12, 8))
        
        # Group by session name and calculate statistics
        session_stats = self.df.groupby('session_name').agg({
            'elapsed_seconds': ['mean', 'min', 'max', 'count']
        }).round(6)
        
        # Create bar chart
        session_names = session_stats.index
        mean_times = session_stats[('elapsed_seconds', 'mean')]
        
        bars = plt.bar(range(len(session_names)), mean_times, 
                      color=sns.color_palette("husl", len(session_names)))
        
        # Add error bars showing min/max
        for i, session in enumerate(session_names):
            min_time = session_stats.loc[session, ('elapsed_seconds', 'min')]
            max_time = session_stats.loc[session, ('elapsed_seconds', 'max')]
            mean_time = session_stats.loc[session, ('elapsed_seconds', 'mean')]
            
            plt.errorbar(i, mean_time, yerr=[[mean_time - min_time], [max_time - mean_time]], 
                        fmt='none', color='black', capsize=5)
        
        plt.title('Execution Time by Session', fontsize=16, fontweight='bold')
        plt.xlabel('Session Name', fontsize=12)
        plt.ylabel('Execution Time (seconds)', fontsize=12)
        plt.xticks(range(len(session_names)), session_names, rotation=45, ha='right')
        plt.grid(axis='y', alpha=0.3)
        
        # Add value labels on bars
        for i, (bar, mean_time) in enumerate(zip(bars, mean_times)):
            plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + max(mean_times) * 0.01,
                    f'{mean_time:.6f}s', ha='center', va='bottom', fontsize=8)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Execution time chart saved to: {output_file}")
        else:
            plt.show()
        
        plt.close()
    
    def create_memory_usage_chart(self, output_file: Optional[str] = None):
        """Create a chart showing memory usage patterns."""
        if self.df.empty or 'peak_memory_mb' not in self.df.columns:
            print("No memory data available for visualization.")
            return
        
        plt.figure(figsize=(12, 8))
        
        # Group by session name
        memory_stats = self.df.groupby('session_name')['peak_memory_mb'].agg(['mean', 'max']).round(2)
        
        # Create bar chart
        session_names = memory_stats.index
        mean_memory = memory_stats['mean']
        max_memory = memory_stats['max']
        
        x = np.arange(len(session_names))
        width = 0.35
        
        bars1 = plt.bar(x - width/2, mean_memory, width, label='Mean Memory', alpha=0.8)
        bars2 = plt.bar(x + width/2, max_memory, width, label='Peak Memory', alpha=0.8)
        
        plt.title('Memory Usage by Session', fontsize=16, fontweight='bold')
        plt.xlabel('Session Name', fontsize=12)
        plt.ylabel('Memory Usage (MB)', fontsize=12)
        plt.xticks(x, session_names, rotation=45, ha='right')
        plt.legend()
        plt.grid(axis='y', alpha=0.3)
        
        # Add value labels
        for bar in bars1:
            height = bar.get_height()
            plt.text(bar.get_x() + bar.get_width()/2, height + max(max_memory) * 0.01,
                    f'{height:.2f}MB', ha='center', va='bottom', fontsize=8)
        
        for bar in bars2:
            height = bar.get_height()
            plt.text(bar.get_x() + bar.get_width()/2, height + max(max_memory) * 0.01,
                    f'{height:.2f}MB', ha='center', va='bottom', fontsize=8)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Memory usage chart saved to: {output_file}")
        else:
            plt.show()
        
        plt.close()
    
    def create_metrics_heatmap(self, output_file: Optional[str] = None):
        """Create a heatmap showing metrics across sessions."""
        if self.df.empty:
            print("No data available for metrics heatmap.")
            return
        
        # Extract metric columns
        metric_columns = [col for col in self.df.columns if col.startswith('metric_')]
        
        if not metric_columns:
            print("No metrics found in the data.")
            return
        
        # Create pivot table for metrics
        metrics_data = self.df[['session_name'] + metric_columns].copy()
        
        # Filter out non-numeric columns to avoid aggregation errors
        numeric_columns = []
        for col in metric_columns:
            converted = pd.to_numeric(metrics_data[col], errors='coerce')
            # Only include if at least one value is not NaN and all non-NaN values are numeric
            if not converted.isnull().all():
                # If all non-NaN values are numeric, include
                if converted.notnull().all():
                    numeric_columns.append(col)
        
        if not numeric_columns:
            print("No numeric metrics found in the data.")
            return
        
        # Group by session_name and calculate mean only for numeric columns
        metrics_data = metrics_data[['session_name'] + numeric_columns].copy()
        for col in numeric_columns:
            metrics_data[col] = pd.to_numeric(metrics_data[col], errors='coerce')
        metrics_data = metrics_data.groupby('session_name').mean()
        
        # Remove 'metric_' prefix from column names
        metrics_data.columns = [col.replace('metric_', '') for col in metrics_data.columns]
        
        plt.figure(figsize=(max(8, len(numeric_columns) * 2), max(6, len(metrics_data) * 0.8)))
        
        # Create heatmap
        sns.heatmap(metrics_data, annot=True, fmt='.2f', cmap='YlOrRd', 
                   cbar_kws={'label': 'Metric Value'})
        
        plt.title('Metrics Heatmap by Session', fontsize=16, fontweight='bold')
        plt.xlabel('Metrics', fontsize=12)
        plt.ylabel('Session Name', fontsize=12)
        plt.xticks(rotation=45, ha='right')
        plt.yticks(rotation=0)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Metrics heatmap saved to: {output_file}")
        else:
            plt.show()
        
        plt.close()
    
    def create_timeline_chart(self, output_file: Optional[str] = None):
        """Create a timeline chart showing session execution order."""
        if self.df.empty:
            print("No data available for timeline visualization.")
            return
        
        plt.figure(figsize=(14, 8))
        
        # Create timeline data
        timeline_data = []
        current_time = 0
        
        for _, row in self.df.iterrows():
            session_name = row['session_name']
            duration = row['elapsed_seconds']
            
            timeline_data.append({
                'session': session_name,
                'start': current_time,
                'end': current_time + duration,
                'duration': duration
            })
            
            current_time += duration
        
        # Create timeline visualization
        colors = plt.cm.Set3(np.linspace(0, 1, len(timeline_data)))
        
        for i, (data, color) in enumerate(zip(timeline_data, colors)):
            plt.barh(i, data['duration'], left=data['start'], 
                    color=color, alpha=0.8, edgecolor='black', linewidth=0.5)
            
            # Add session name label
            plt.text(data['start'] + data['duration']/2, i, 
                    f"{data['session']}\n({data['duration']:.6f}s)", 
                    ha='center', va='center', fontsize=8, fontweight='bold')
        
        plt.title('Session Execution Timeline', fontsize=16, fontweight='bold')
        plt.xlabel('Time (seconds)', fontsize=12)
        plt.ylabel('Session', fontsize=12)
        plt.yticks(range(len(timeline_data)), [d['session'] for d in timeline_data])
        plt.grid(axis='x', alpha=0.3)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Timeline chart saved to: {output_file}")
        else:
            plt.show()
        
        plt.close()
    
    def create_performance_summary(self, output_file: Optional[str] = None):
        """Create a comprehensive performance summary dashboard."""
        if self.df.empty:
            print("No data available for performance summary.")
            return
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
        
        # 1. Execution time distribution
        session_stats = self.df.groupby('session_name')['elapsed_seconds'].agg(['mean', 'std']).round(6)
        bars1 = ax1.bar(range(len(session_stats)), session_stats['mean'], 
                        yerr=session_stats['std'], capsize=5, alpha=0.8)
        ax1.set_title('Execution Time Distribution', fontweight='bold')
        ax1.set_xlabel('Session')
        ax1.set_ylabel('Time (seconds)')
        ax1.set_xticks(range(len(session_stats)))
        ax1.set_xticklabels(session_stats.index, rotation=45, ha='right')
        ax1.grid(axis='y', alpha=0.3)
        
        # 2. Memory usage (if available)
        if 'peak_memory_mb' in self.df.columns:
            memory_stats = self.df.groupby('session_name')['peak_memory_mb'].mean()
            bars2 = ax2.bar(range(len(memory_stats)), memory_stats, alpha=0.8)
            ax2.set_title('Memory Usage', fontweight='bold')
            ax2.set_xlabel('Session')
            ax2.set_ylabel('Memory (MB)')
            ax2.set_xticks(range(len(memory_stats)))
            ax2.set_xticklabels(memory_stats.index, rotation=45, ha='right')
            ax2.grid(axis='y', alpha=0.3)
        else:
            ax2.text(0.5, 0.5, 'No memory data available', 
                    ha='center', va='center', transform=ax2.transAxes)
            ax2.set_title('Memory Usage', fontweight='bold')
        
        # 3. Session count
        session_counts = self.df['session_name'].value_counts()
        bars3 = ax3.bar(range(len(session_counts)), session_counts.values, alpha=0.8)
        ax3.set_title('Session Execution Count', fontweight='bold')
        ax3.set_xlabel('Session')
        ax3.set_ylabel('Count')
        ax3.set_xticks(range(len(session_counts)))
        ax3.set_xticklabels(session_counts.index, rotation=45, ha='right')
        ax3.grid(axis='y', alpha=0.3)
        
        # 4. Total statistics
        total_time = self.df['elapsed_seconds'].sum()
        total_sessions = len(self.df)
        avg_time = self.df['elapsed_seconds'].mean()
        
        stats_text = f"""
        Total Execution Time: {total_time:.6f}s
        Total Sessions: {total_sessions}
        Average Time per Session: {avg_time:.6f}s
        """
        
        if 'peak_memory_mb' in self.df.columns:
            total_memory = self.df['peak_memory_mb'].sum()
            avg_memory = self.df['peak_memory_mb'].mean()
            stats_text += f"""
        Total Memory Used: {total_memory:.2f}MB
        Average Memory per Session: {avg_memory:.2f}MB
        """
        
        ax4.text(0.1, 0.5, stats_text, fontsize=12, fontfamily='monospace',
                verticalalignment='center', transform=ax4.transAxes)
        ax4.set_title('Performance Summary', fontweight='bold')
        ax4.axis('off')
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Performance summary saved to: {output_file}")
        else:
            plt.show()
        
        plt.close()
    
    def generate_all_charts(self, output_dir: str = "profiling_charts"):
        """Generate all charts and save them to the specified directory."""
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
        
        print(f"Generating charts in directory: {output_path}")
        
        # Generate all chart types
        self.create_execution_time_chart(output_path / "execution_time.png")
        self.create_memory_usage_chart(output_path / "memory_usage.png")
        self.create_metrics_heatmap(output_path / "metrics_heatmap.png")
        self.create_timeline_chart(output_path / "timeline.png")
        self.create_performance_summary(output_path / "performance_summary.png")
        
        print(f"All charts generated successfully in: {output_path}")
    
    def print_summary(self):
        """Print a text summary of the profiling data."""
        if self.df.empty:
            print("No data available for summary.")
            return
        
        print("\n" + "="*60)
        print("HEIMDALL PROFILING SUMMARY")
        print("="*60)
        
        # Basic statistics
        total_sessions = len(self.df)
        total_time = self.df['elapsed_seconds'].sum()
        avg_time = self.df['elapsed_seconds'].mean()
        
        print(f"Total Sessions: {total_sessions}")
        print(f"Total Execution Time: {total_time:.6f} seconds")
        print(f"Average Time per Session: {avg_time:.6f} seconds")
        
        if 'peak_memory_mb' in self.df.columns:
            total_memory = self.df['peak_memory_mb'].sum()
            avg_memory = self.df['peak_memory_mb'].mean()
            print(f"Total Memory Used: {total_memory:.2f} MB")
            print(f"Average Memory per Session: {avg_memory:.2f} MB")
        
        # Session breakdown
        print("\nSession Breakdown:")
        session_stats = self.df.groupby('session_name').agg({
            'elapsed_seconds': ['count', 'mean', 'min', 'max'],
            'peak_memory_mb': ['mean', 'max'] if 'peak_memory_mb' in self.df.columns else []
        }).round(6)
        
        for session in session_stats.index:
            count = session_stats.loc[session, ('elapsed_seconds', 'count')]
            mean_time = session_stats.loc[session, ('elapsed_seconds', 'mean')]
            min_time = session_stats.loc[session, ('elapsed_seconds', 'min')]
            max_time = session_stats.loc[session, ('elapsed_seconds', 'max')]
            
            print(f"  {session}:")
            print(f"    Count: {count}")
            print(f"    Time: {mean_time:.6f}s (min: {min_time:.6f}s, max: {max_time:.6f}s)")
            
            if 'peak_memory_mb' in self.df.columns:
                mean_memory = session_stats.loc[session, ('peak_memory_mb', 'mean')]
                max_memory = session_stats.loc[session, ('peak_memory_mb', 'max')]
                print(f"    Memory: {mean_memory:.2f}MB (peak: {max_memory:.2f}MB)")
        
        print("="*60)


def main():
    parser = argparse.ArgumentParser(
        description="Heimdall Profiling Visualizer - Generate charts from JSON profiling data"
    )
    parser.add_argument("json_file", help="Path to the JSON profiling results file")
    parser.add_argument("--output-dir", default="profiling_charts", 
                       help="Output directory for charts (default: profiling_charts)")
    parser.add_argument("--chart-type", choices=['all', 'time', 'memory', 'metrics', 'timeline', 'summary'],
                       default='all', help="Type of chart to generate (default: all)")
    parser.add_argument("--show", action='store_true', 
                       help="Show charts interactively (in addition to saving)")
    
    args = parser.parse_args()
    
    # Check if required packages are available
    try:
        import matplotlib.pyplot as plt
        import seaborn as sns
        import pandas as pd
        import numpy as np
    except ImportError as e:
        print(f"Error: Required package not found: {e}")
        print("Please install required packages:")
        print("pip install matplotlib seaborn pandas numpy")
        sys.exit(1)
    
    # Create visualizer
    visualizer = ProfilingVisualizer(args.json_file)
    
    # Print summary
    visualizer.print_summary()
    
    # Generate charts based on type
    if args.chart_type == 'all':
        visualizer.generate_all_charts(args.output_dir)
    elif args.chart_type == 'time':
        visualizer.create_execution_time_chart()
    elif args.chart_type == 'memory':
        visualizer.create_memory_usage_chart()
    elif args.chart_type == 'metrics':
        visualizer.create_metrics_heatmap()
    elif args.chart_type == 'timeline':
        visualizer.create_timeline_chart()
    elif args.chart_type == 'summary':
        visualizer.create_performance_summary()
    
    # Show charts if requested
    if args.show:
        plt.show()


if __name__ == "__main__":
    main() 