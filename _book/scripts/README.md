# Scripts

This directory contains utility scripts for the Heimdall project.

## SonarQube Issues Exporter

The `sonarqube_issues_to_csv.py` script exports open issues from SonarQube to CSV or Excel format for analysis.

### Setup

1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Get your SonarQube authentication token:
   - Go to your SonarQube profile settings
   - Generate a new token
   - Copy the token for use with the script

### Usage

Basic usage with default settings:
```bash
python sonarqube_issues_to_csv.py --token YOUR_SONARQUBE_TOKEN
```

Export to Excel format:
```bash
python sonarqube_issues_to_csv.py --token YOUR_TOKEN --format excel --output heimdall_issues.xlsx
```

Custom output file:
```bash
python sonarqube_issues_to_csv.py --token YOUR_TOKEN --output my_issues.csv
```

### Command Line Options

- `--token`: SonarQube authentication token (required)
- `--sonar-url`: SonarQube server URL (default: https://sonarcloud.io)
- `--project-key`: Project key in SonarQube (default: Heimdall-SBOM_heimdall)
- `--organization-key`: Organization key in SonarQube (default: heimdall-sbom)
- `--output`: Output file path (default: timestamped CSV file)
- `--format`: Output format: csv or excel (default: csv)

### Output Format

The script exports the following fields for each issue:

- `key`: Unique issue identifier
- `rule`: Rule that triggered the issue
- `severity`: Issue severity (BLOCKER, CRITICAL, MAJOR, MINOR, INFO)
- `type`: Issue type (BUG, VULNERABILITY, CODE_SMELL)
- `component`: Full component path
- `file_path`: Relative file path
- `line`: Line number where issue occurs
- `message`: Issue description
- `status`: Issue status
- `effort`: Estimated effort to fix
- `debt`: Technical debt
- `author`: Issue author
- `tags`: Comma-separated tags
- `creationDate`: When issue was created
- `updateDate`: When issue was last updated
- `closeDate`: When issue was closed (if applicable)

### Example Output

The script will display a summary including:
- Total number of issues
- Breakdown by severity
- Breakdown by issue type
- Output file location 