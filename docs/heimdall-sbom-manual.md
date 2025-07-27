# Heimdall SBOM - Dynamic SBOM Generator Loader

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Commands Reference](#commands-reference)
5. [Supported Formats](#supported-formats)
6. [Plugin System](#plugin-system)
7. [Examples](#examples)
8. [Advanced Usage](#advanced-usage)
9. [Troubleshooting](#troubleshooting)
10. [Integration](#integration)

## Overview

`heimdall-sbom` is a dynamic SBOM generator loader that provides a lightweight command-line interface for generating Software Bills of Materials from binary files using Heimdall plugins. It dynamically loads linker plugins and generates SBOMs in multiple formats without requiring the full Heimdall library to be linked.

### Key Features

- **Dynamic Plugin Loading**: Loads Heimdall plugins at runtime using `dlopen`/`dlsym`
- **Multi-format Support**: SPDX (2.3, 3.0, 3.0.0, 3.0.1) and CycloneDX (1.4, 1.5, 1.6)
- **Lightweight**: Minimal dependencies, only requires the plugin shared library
- **Flexible Configuration**: Configurable output formats, versions, and dependency handling
- **Cross-platform**: Works on Linux and macOS with ELF and Mach-O binaries
- **Batch Processing**: Process multiple binaries with different configurations

### Use Cases

- **Standalone SBOM Generation**: Generate SBOMs without full Heimdall installation
- **CI/CD Integration**: Lightweight SBOM generation in build pipelines
- **Plugin Testing**: Test Heimdall plugins independently
- **Binary Analysis**: Analyze existing binaries for SBOM generation
- **Format Conversion**: Convert between different SBOM formats

## Installation

### Prerequisites

- C++17 or later compiler
- CMake 3.15 or later
- Git
- Dynamic linking support (`dlopen`/`dlsym`)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Heimdall-SBOM/heimdall.git
cd heimdall

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)

# The heimdall-sbom tool will be available at:
# build/heimdall-sbom
```

### Verifying Installation

```bash
./build/heimdall-sbom --help
# or
./build/heimdall-sbom
```

## Quick Start

### Basic SBOM Generation

```bash
# Generate CycloneDX SBOM from a binary
heimdall-sbom /path/to/heimdall-lld.so myapp --format cyclonedx --output myapp.cdx.json

# Generate SPDX SBOM from a binary
heimdall-sbom /path/to/heimdall-lld.so myapp --format spdx --output myapp.spdx

# Use default format (SPDX 2.3)
heimdall-sbom /path/to/heimdall-lld.so myapp --output myapp.sbom
```

### Using Different Plugin Types

```bash
# Use LLD plugin
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp --format cyclonedx-1.6

# Use Gold plugin
heimdall-sbom /usr/lib/heimdall/heimdall-gold.so myapp --format spdx-3.0
```

## Commands Reference

### Basic Syntax

```bash
heimdall-sbom <plugin_path> <binary_path> [options]
```

### Required Arguments

- `<plugin_path>` - Path to the Heimdall plugin shared library
- `<binary_path>` - Path to the binary file to analyze

### Options

#### Format Options

- `--format <format>` - Specify SBOM format
  - **SPDX formats**: `spdx`, `spdx-2.3`, `spdx-3.0`, `spdx-3.0.0`, `spdx-3.0.1`
  - **CycloneDX formats**: `cyclonedx`, `cyclonedx-1.4`, `cyclonedx-1.5`, `cyclonedx-1.6`
  - **Default**: `spdx-2.3`

#### Output Options

- `--output <path>` - Output file path (default: `sbom.json`)
- `--cyclonedx-version <version>` - CycloneDX specification version (default: `1.6`)
- `--spdx-version <version>` - SPDX specification version (default: `2.3`)

#### Dependency Options

- `--no-transitive-dependencies` - Include only direct dependencies (default: include all transitive dependencies)

#### Examples

```bash
# Basic usage with defaults
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp

# Specify format and output
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx-1.6 \
    --output myapp.cdx.json

# SPDX with specific version
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format spdx-3.0 \
    --output myapp.spdx

# Only direct dependencies
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx \
    --no-transitive-dependencies \
    --output myapp-direct.cdx.json
```

## Supported Formats

### CycloneDX Support

| Version | Status | Features |
|---------|--------|----------|
| 1.4 | ✅ Full Support | Basic component structure, metadata |
| 1.5 | ✅ Full Support | Enhanced validation, additional fields |
| 1.6 | ✅ Full Support | Latest features, complete validation |

**Generated Fields:**
- `bomFormat`, `specVersion`, `version`, `metadata`
- Component information (name, version, type, purl)
- Dependencies and relationships
- File hashes and integrity information
- License information
- Supplier and author details

### SPDX Support

| Version | Status | Format | Features |
|---------|--------|--------|----------|
| 2.3 | ✅ Full Support | Tag-value | Complete validation |
| 3.0 | ✅ Full Support | JSON | Complete validation |
| 3.0.0 | ✅ Full Support | JSON | Complete validation |
| 3.0.1 | ✅ Full Support | JSON | Complete validation |

**Generated Fields:**
- `SPDXVersion`, `DataLicense`, `DocumentName`
- Package and file information
- License expressions and identifiers
- Checksums and integrity information
- Relationships between components
- Creator and creation information

## Plugin System

### Plugin Architecture

`heimdall-sbom` uses dynamic loading to interface with Heimdall plugins:

```mermaid
graph TB
    subgraph "heimdall-sbom"
        A[Main Program]
        B[Dynamic Loader]
        C[Plugin Interface]
    end
    
    subgraph "Plugin"
        D[LLD Plugin]
        E[Gold Plugin]
    end
    
    subgraph "Binary Analysis"
        F[Metadata Extraction]
        G[Dependency Resolution]
        H[SBOM Generation]
    end
    
    A --> B
    B --> C
    C --> D
    C --> E
    D --> F
    E --> F
    F --> G
    G --> H
```

### Required Plugin Functions

The tool expects the following functions to be exported by the plugin:

```cpp
// Plugin initialization
int onload(void* context);

// Format configuration
int heimdall_set_format(const char* format);
int heimdall_set_cyclonedx_version(const char* version);
int heimdall_set_spdx_version(const char* version);

// Output configuration
int heimdall_set_output_path(const char* path);
int heimdall_set_transitive_dependencies(int enabled);

// Processing
int heimdall_process_input_file(const char* path);

// Cleanup
void heimdall_finalize();
```

### Plugin Loading Process

1. **Library Loading**: Uses `dlopen()` to load the plugin shared library
2. **Symbol Resolution**: Uses `dlsym()` to resolve required function symbols
3. **Initialization**: Calls `onload()` to initialize the plugin
4. **Configuration**: Sets format, version, and output parameters
5. **Processing**: Calls `process_input_file()` to analyze the binary
6. **Finalization**: Calls `finalize()` to generate the SBOM
7. **Cleanup**: Unloads the plugin library

## Examples

### Basic Examples

#### Generate CycloneDX 1.6 SBOM
```bash
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx-1.6 \
    --output myapp.cdx.json
```

**Expected Output:**
```
Processing myapp with heimdall-lld.so...
SBOM generated successfully: myapp.cdx.json
```

#### Generate SPDX 3.0 JSON SBOM
```bash
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format spdx-3.0 \
    --output myapp.spdx.json
```

#### Generate SPDX 2.3 Tag-Value SBOM
```bash
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format spdx-2.3 \
    --output myapp.spdx
```

### Advanced Examples

#### Process Multiple Binaries
```bash
#!/bin/bash
# process-binaries.sh

PLUGIN="/usr/lib/heimdall/heimdall-lld.so"
FORMAT="cyclonedx-1.6"

for binary in *.exe *.bin; do
    if [ -f "$binary" ]; then
        echo "Processing $binary..."
        heimdall-sbom "$PLUGIN" "$binary" \
            --format "$FORMAT" \
            --output "${binary%.*}.cdx.json"
    fi
done
```

#### Batch Processing with Different Formats
```bash
#!/bin/bash
# batch-sbom.sh

PLUGIN="/usr/lib/heimdall/heimdall-lld.so"
BINARY="myapp"

# Generate multiple formats
heimdall-sbom "$PLUGIN" "$BINARY" --format cyclonedx-1.6 --output "$BINARY.cdx.json"
heimdall-sbom "$PLUGIN" "$BINARY" --format spdx-3.0 --output "$BINARY.spdx.json"
heimdall-sbom "$PLUGIN" "$BINARY" --format spdx-2.3 --output "$BINARY.spdx"

echo "Generated SBOMs:"
ls -la "$BINARY".*
```

#### Compare Direct vs Transitive Dependencies
```bash
# Generate SBOM with all dependencies
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx-1.6 \
    --output myapp-all.cdx.json

# Generate SBOM with only direct dependencies
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx-1.6 \
    --no-transitive-dependencies \
    --output myapp-direct.cdx.json

# Compare the results
diff myapp-all.cdx.json myapp-direct.cdx.json
```

## Advanced Usage

### Plugin Development

#### Creating a Custom Plugin

```cpp
// my_plugin.cpp
#include <iostream>

extern "C" {
    int onload(void* context) {
        std::cout << "My plugin loaded successfully" << std::endl;
        return 0;
    }
    
    int heimdall_set_format(const char* format) {
        std::cout << "Setting format: " << format << std::endl;
        return 0;
    }
    
    int heimdall_set_output_path(const char* path) {
        std::cout << "Setting output path: " << path << std::endl;
        return 0;
    }
    
    int heimdall_process_input_file(const char* path) {
        std::cout << "Processing file: " << path << std::endl;
        // Your SBOM generation logic here
        return 0;
    }
    
    void heimdall_finalize() {
        std::cout << "Finalizing SBOM generation" << std::endl;
    }
}
```

#### Building the Plugin

```bash
# Compile the plugin
g++ -shared -fPIC my_plugin.cpp -o my_plugin.so

# Use with heimdall-sbom
heimdall-sbom ./my_plugin.so myapp --format cyclonedx --output myapp.cdx.json
```

### Integration with Build Systems

#### CMake Integration

```cmake
# CMakeLists.txt
find_package(Heimdall REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp ${HEIMDALL_LIBRARIES})

# Custom target for SBOM generation
add_custom_target(sbom
    COMMAND heimdall-sbom
        ${HEIMDALL_PLUGIN_PATH}/heimdall-lld.so
        $<TARGET_FILE:myapp>
        --format cyclonedx-1.6
        --output ${CMAKE_BINARY_DIR}/myapp.cdx.json
    DEPENDS myapp
    COMMENT "Generating SBOM for myapp"
)
```

#### Makefile Integration

```makefile
# Makefile
HEIMDALL_PLUGIN = /usr/lib/heimdall/heimdall-lld.so
SBOM_FORMAT = cyclonedx-1.6

myapp: main.cpp
	g++ -o myapp main.cpp

sbom: myapp
	heimdall-sbom $(HEIMDALL_PLUGIN) myapp \
		--format $(SBOM_FORMAT) \
		--output myapp.cdx.json

.PHONY: sbom
```

### CI/CD Integration

#### GitHub Actions

```yaml
name: Generate SBOM
on: [push, pull_request]

jobs:
  sbom:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Build Application
        run: |
          g++ -o myapp main.cpp
      
      - name: Install Heimdall
        run: |
          git clone https://github.com/Heimdall-SBOM/heimdall.git
          cd heimdall
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
      
      - name: Generate SBOM
        run: |
          ./heimdall/build/heimdall-sbom \
            ./heimdall/build/lib/heimdall-lld.so \
            myapp \
            --format cyclonedx-1.6 \
            --output myapp.cdx.json
      
      - name: Upload SBOM
        uses: actions/upload-artifact@v3
        with:
          name: sbom
          path: myapp.cdx.json
```

#### Jenkins Pipeline

```groovy
pipeline {
    agent any
    
    stages {
        stage('Build') {
            steps {
                sh 'g++ -o myapp main.cpp'
            }
        }
        
        stage('Generate SBOM') {
            steps {
                sh '''
                    git clone https://github.com/Heimdall-SBOM/heimdall.git
                    cd heimdall
                    mkdir build && cd build
                    cmake ..
                    make -j$(nproc)
                    
                    ../../heimdall-sbom \
                        ./lib/heimdall-lld.so \
                        ../../myapp \
                        --format cyclonedx-1.6 \
                        --output ../../myapp.cdx.json
                '''
            }
        }
        
        stage('Archive SBOM') {
            steps {
                archiveArtifacts artifacts: 'myapp.cdx.json'
            }
        }
    }
}
```

## Troubleshooting

### Common Issues

#### 1. "Failed to load plugin"
**Cause:** Plugin file doesn't exist or has incorrect permissions
**Solution:** Check plugin path and permissions
```bash
ls -la /usr/lib/heimdall/heimdall-lld.so
chmod 755 /usr/lib/heimdall/heimdall-lld.so
```

#### 2. "Failed to get function symbols"
**Cause:** Plugin doesn't export required functions
**Solution:** Ensure plugin implements all required functions
```bash
nm -D /usr/lib/heimdall/heimdall-lld.so | grep heimdall
```

#### 3. "Failed to initialize plugin"
**Cause:** Plugin initialization failed
**Solution:** Check plugin logs and dependencies
```bash
# Check plugin dependencies
ldd /usr/lib/heimdall/heimdall-lld.so
```

#### 4. "Failed to process binary"
**Cause:** Binary file doesn't exist or is not supported
**Solution:** Verify binary path and format
```bash
file myapp
ls -la myapp
```

### Debug Mode

Enable verbose output for debugging:
```bash
# Set environment variable for verbose output
export HEIMDALL_VERBOSE=1
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp --format cyclonedx
```

### Error Codes

| Exit Code | Meaning |
|-----------|---------|
| 0 | Success |
| 1 | Plugin loading failed |
| 2 | Plugin initialization failed |
| 3 | Binary processing failed |
| 4 | Invalid command line arguments |

### Performance Tips

1. **Use absolute paths** for plugins and binaries to avoid path resolution issues
2. **Pre-load plugins** in long-running processes to avoid repeated loading overhead
3. **Use appropriate formats** for your use case (JSON for programmatic processing, tag-value for human reading)
4. **Consider dependency scope** - use `--no-transitive-dependencies` for faster processing when only direct dependencies are needed

## Integration

### With Other Tools

#### Integration with CycloneDX Tools
```bash
# Generate SBOM with heimdall-sbom
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format cyclonedx-1.6 \
    --output myapp.cdx.json

# Validate with cyclonedx-cli
cyclonedx-cli validate myapp.cdx.json

# Convert to other formats
cyclonedx-cli convert --input-format json --output-format xml myapp.cdx.json
```

#### Integration with SPDX Tools
```bash
# Generate SPDX SBOM
heimdall-sbom /usr/lib/heimdall/heimdall-lld.so myapp \
    --format spdx-3.0 \
    --output myapp.spdx.json

# Validate with SPDX tools
spdx-tools-validate myapp.spdx.json
```

### API Integration

The tool can be integrated into custom scripts and applications:

```python
import subprocess
import json

def generate_sbom(plugin_path, binary_path, format='cyclonedx-1.6', output_path='sbom.json'):
    cmd = [
        'heimdall-sbom',
        plugin_path,
        binary_path,
        '--format', format,
        '--output', output_path
    ]
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0, result.stdout, result.stderr

# Usage
success, output, errors = generate_sbom(
    '/usr/lib/heimdall/heimdall-lld.so',
    'myapp',
    'cyclonedx-1.6',
    'myapp.cdx.json'
)

if success:
    print("SBOM generated successfully")
else:
    print(f"Error: {errors}")
```

### Configuration Files

Create configuration files for repeated operations:

```json
{
  "defaults": {
    "plugin_path": "/usr/lib/heimdall/heimdall-lld.so",
    "format": "cyclonedx-1.6",
    "include_transitive": true
  },
  "formats": {
    "cyclonedx": {
      "versions": ["1.4", "1.5", "1.6"],
      "default": "1.6"
    },
    "spdx": {
      "versions": ["2.3", "3.0", "3.0.0", "3.0.1"],
      "default": "2.3"
    }
  },
  "output": {
    "directory": "./sboms",
    "naming": "{binary}.{format}.{version}.json"
  }
}
```

### Shell Scripts

#### Automated SBOM Generation Script
```bash
#!/bin/bash
# generate-sboms.sh

PLUGIN_PATH="${HEIMDALL_PLUGIN:-/usr/lib/heimdall/heimdall-lld.so}"
OUTPUT_DIR="${SBOM_OUTPUT_DIR:-./sboms}"
FORMATS=("cyclonedx-1.6" "spdx-3.0" "spdx-2.3")

if [ $# -eq 0 ]; then
    echo "Usage: $0 <binary1> [binary2] ..."
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

for binary in "$@"; do
    if [ ! -f "$binary" ]; then
        echo "Warning: $binary not found, skipping"
        continue
    fi
    
    echo "Processing $binary..."
    
    for format in "${FORMATS[@]}"; do
        output_file="$OUTPUT_DIR/$(basename "$binary").${format//-/.}.json"
        
        echo "  Generating $format SBOM..."
        if heimdall-sbom "$PLUGIN_PATH" "$binary" \
            --format "$format" \
            --output "$output_file"; then
            echo "    ✅ Generated: $output_file"
        else
            echo "    ❌ Failed to generate: $output_file"
        fi
    done
done

echo "SBOM generation complete. Files saved to: $OUTPUT_DIR"
```

## Conclusion

`heimdall-sbom` provides a lightweight, flexible interface for generating SBOMs from binary files using Heimdall plugins. Its dynamic loading approach makes it ideal for integration into existing toolchains and CI/CD pipelines without requiring full Heimdall installation.

For more information, see:
- [Heimdall User Guide](heimdall-users-guide.md)
- [Heimdall Validate Manual](heimdall-validate-manual.md)
- [Plugin Development Guide](heimdall-developers-guide.md)
- [CycloneDX Specification](https://cyclonedx.org/specification/)
- [SPDX Specification](https://spdx.github.io/spdx-spec/) 