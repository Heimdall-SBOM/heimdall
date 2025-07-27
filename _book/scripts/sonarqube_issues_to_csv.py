#!/usr/bin/env python3
"""
SonarQube Issues to CSV Exporter

This script queries SonarQube for open issues in the Heimdall project
and exports them to a CSV file for analysis.

Usage:
    python sonarqube_issues_to_csv.py [--sonar-url URL] [--token TOKEN] [--output FILE]

Requirements:
    pip install requests pandas
"""

import argparse
import csv
import json
import sys
from datetime import datetime
from typing import Dict, List, Optional
import requests
import pandas as pd

# Constants for file extensions
EXCEL_EXTENSION = '.xlsx'
CSV_EXTENSION = '.csv'


class SonarQubeIssuesExporter:
    """Exports SonarQube issues to CSV format."""
    
    def __init__(self, sonar_url: str, token: str, project_key: str, organization_key: str):
        """
        Initialize the SonarQube issues exporter.
        
        Args:
            sonar_url: SonarQube server URL
            token: SonarQube authentication token
            project_key: Project key in SonarQube
            organization_key: Organization key in SonarQube
        """
        self.sonar_url = sonar_url.rstrip('/')
        self.token = token
        self.project_key = project_key
        self.organization_key = organization_key
        self.session = requests.Session()
        self.session.auth = (token, '')
        
    def get_issues(self, page_size: int = 500) -> List[Dict]:
        """
        Fetch all open issues from SonarQube.
        
        Args:
            page_size: Number of issues per page
            
        Returns:
            List of issue dictionaries
        """
        issues = []
        page = 1
        
        print(f"Fetching issues for project: {self.project_key}")
        print(f"Organization: {self.organization_key}")
        
        while True:
            url = f"{self.sonar_url}/api/issues/search"
            params = {
                'componentKeys': self.project_key,
                'organization': self.organization_key,
                'statuses': 'OPEN',
                'ps': page_size,
                'p': page,
                'facets': 'severities,types,rules'
            }
            
            try:
                response = self.session.get(url, params=params)
                response.raise_for_status()
                data = response.json()
                
                if 'issues' not in data:
                    print(f"Warning: No 'issues' field in response: {data}")
                    break
                    
                page_issues = data['issues']
                if not page_issues:
                    break
                    
                issues.extend(page_issues)
                print(f"Fetched page {page}: {len(page_issues)} issues")
                
                # Check if we've reached the end
                total = data.get('total', 0)
                if len(issues) >= total:
                    break
                    
                page += 1
                
            except requests.exceptions.RequestException as e:
                print(f"Error fetching issues: {e}")
                break
                
        print(f"Total issues fetched: {len(issues)}")
        return issues
    
    def format_issue_for_csv(self, issue: Dict) -> Dict:
        """
        Format a SonarQube issue for CSV export.
        
        Args:
            issue: Raw issue dictionary from SonarQube API
            
        Returns:
            Formatted issue dictionary for CSV
        """
        # Extract basic issue information
        formatted = {
            'key': issue.get('key', ''),
            'rule': issue.get('rule', ''),
            'severity': issue.get('severity', ''),
            'component': issue.get('component', ''),
            'project': issue.get('project', ''),
            'line': issue.get('line', ''),
            'message': issue.get('message', ''),
            'status': issue.get('status', ''),
            'type': issue.get('type', ''),
            'effort': issue.get('effort', ''),
            'debt': issue.get('debt', ''),
            'author': issue.get('author', ''),
            'tags': ','.join(issue.get('tags', [])),
            'creationDate': issue.get('creationDate', ''),
            'updateDate': issue.get('updateDate', ''),
            'closeDate': issue.get('closeDate', ''),
        }
        
        # Extract file path from component
        component = issue.get('component', '')
        if component:
            # Remove project key prefix to get relative file path
            if component.startswith(self.project_key + ':'):
                formatted['file_path'] = component[len(self.project_key + ':'):]
            else:
                formatted['file_path'] = component
        else:
            formatted['file_path'] = ''
            
        # Format dates
        for date_field in ['creationDate', 'updateDate', 'closeDate']:
            if formatted[date_field]:
                try:
                    # Convert ISO date to readable format
                    date_obj = datetime.fromisoformat(formatted[date_field].replace('Z', '+00:00'))
                    formatted[date_field] = date_obj.strftime('%Y-%m-%d %H:%M:%S')
                except ValueError:
                    pass  # Keep original format if parsing fails
                    
        return formatted
    
    def export_to_csv(self, issues: List[Dict], output_file: str):
        """
        Export issues to CSV file.
        
        Args:
            issues: List of issue dictionaries
            output_file: Output CSV file path
        """
        if not issues:
            print("No issues to export")
            return
            
        # Format issues for CSV
        formatted_issues = [self.format_issue_for_csv(issue) for issue in issues]
        
        # Define CSV columns
        columns = [
            'key', 'rule', 'severity', 'type', 'component', 'project', 'file_path', 'line',
            'message', 'status', 'effort', 'debt', 'author', 'tags',
            'creationDate', 'updateDate', 'closeDate'
        ]
        
        # Write to CSV
        try:
            with open(output_file, 'w', newline='', encoding='utf-8') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=columns)
                writer.writeheader()
                writer.writerows(formatted_issues)
                
            print(f"Successfully exported {len(formatted_issues)} issues to {output_file}")
            
        except IOError as e:
            print(f"Error writing CSV file: {e}")
            sys.exit(1)
    
    def export_to_excel(self, issues: List[Dict], output_file: str):
        """
        Export issues to Excel file using pandas.
        
        Args:
            issues: List of issue dictionaries
            output_file: Output Excel file path
        """
        if not issues:
            print("No issues to export")
            return
            
        try:
            # Format issues for Excel
            formatted_issues = [self.format_issue_for_csv(issue) for issue in issues]
            
            # Create DataFrame
            df = pd.DataFrame(formatted_issues)
            
            # Write to Excel
            df.to_excel(output_file, index=False, engine='openpyxl')
            print(f"Successfully exported {len(formatted_issues)} issues to {output_file}")
            
        except Exception as e:
            print(f"Error writing Excel file: {e}")
            print("Falling back to CSV export...")
            csv_file = output_file.replace(EXCEL_EXTENSION, CSV_EXTENSION)
            self.export_to_csv(issues, csv_file)


def main():
    """Main function to run the SonarQube issues exporter."""
    parser = argparse.ArgumentParser(
        description='Export SonarQube issues to CSV/Excel file',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python sonarqube_issues_to_csv.py --token YOUR_TOKEN
  python sonarqube_issues_to_csv.py --sonar-url https://sonarcloud.io --token YOUR_TOKEN --output issues.xlsx
  python sonarqube_issues_to_csv.py --token YOUR_TOKEN --output heimdall_issues.csv
        """
    )
    
    parser.add_argument(
        '--sonar-url',
        default='https://sonarcloud.io',
        help='SonarQube server URL (default: https://sonarcloud.io)'
    )
    
    parser.add_argument(
        '--token',
        required=True,
        help='SonarQube authentication token'
    )
    
    parser.add_argument(
        '--project-key',
        default='Heimdall-SBOM_heimdall',
        help='Project key in SonarQube (default: Heimdall-SBOM_heimdall)'
    )
    
    parser.add_argument(
        '--organization-key',
        default='heimdall-sbom',
        help='Organization key in SonarQube (default: heimdall-sbom)'
    )
    
    parser.add_argument(
        '--output',
        default=f'heimdall_sonarqube_issues_{datetime.now().strftime("%Y%m%d_%H%M%S")}.csv',
        help='Output file path (default: heimdall_sonarqube_issues_YYYYMMDD_HHMMSS.csv)'
    )
    
    parser.add_argument(
        '--format',
        choices=['csv', 'excel'],
        default='csv',
        help='Output format (default: csv)'
    )
    
    args = parser.parse_args()
    
    # Create exporter
    exporter = SonarQubeIssuesExporter(
        sonar_url=args.sonar_url,
        token=args.token,
        project_key=args.project_key,
        organization_key=args.organization_key
    )
    
    # Fetch issues
    print("Fetching SonarQube issues...")
    issues = exporter.get_issues()
    
    if not issues:
        print("No open issues found in the project")
        return
    
    # Export to file
    if args.format == 'excel':
        if not args.output.endswith(EXCEL_EXTENSION):
            args.output = args.output.replace(CSV_EXTENSION, EXCEL_EXTENSION)
        exporter.export_to_excel(issues, args.output)
    else:
        if not args.output.endswith(CSV_EXTENSION):
            args.output = args.output.replace(EXCEL_EXTENSION, CSV_EXTENSION)
        exporter.export_to_csv(issues, args.output)
    
    # Print summary
    print("\n" + "="*50)
    print("EXPORT SUMMARY")
    print("="*50)
    print(f"Project: {args.project_key}")
    print(f"Organization: {args.organization_key}")
    print(f"Total Issues: {len(issues)}")
    print(f"Output File: {args.output}")
    
    # Count by severity
    severity_counts = {}
    type_counts = {}
    for issue in issues:
        severity = issue.get('severity', 'UNKNOWN')
        issue_type = issue.get('type', 'UNKNOWN')
        severity_counts[severity] = severity_counts.get(severity, 0) + 1
        type_counts[issue_type] = type_counts.get(issue_type, 0) + 1
    
    print(f"\nIssues by Severity:")
    for severity, count in sorted(severity_counts.items()):
        print(f"  {severity}: {count}")
    
    print(f"\nIssues by Type:")
    for issue_type, count in sorted(type_counts.items()):
        print(f"  {issue_type}: {count}")


if __name__ == '__main__':
    main() 