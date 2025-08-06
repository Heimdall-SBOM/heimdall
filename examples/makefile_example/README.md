# Heimdall Compiler Plugin Makefile Example

This example demonstrates how to use Heimdall's compiler plugins with a traditional Makefile-based C++ project. The example creates a complex multi-file C++ application with dependencies and shows how compiler metadata is captured and integrated into SBOMs.

## Project Structure

```
makefile_example/
├── README.md           # This file
├── Makefile            # Main build configuration
├── src/
│   ├── main.cpp        # Main application entry point
│   ├── calculator.cpp  # Calculator implementation
│   ├── calculator.h    # Calculator header
│   ├── utils.cpp       # Utility functions
│   ├── utils.h         # Utility header
│   └── math/
│       ├── operations.cpp  # Math operations
│       └── operations.h    # Math operations header
└── build/              # Build output directory (created during build)
    ├── obj/            # Object files
    ├── metadata/       # Compiler metadata from plugins
    └── sbom/           # Generated SBOMs
```

## Features Demonstrated

- Multi-file C++ project with headers and source files
- Nested directory structure (`src/math/`)
- Header dependencies between modules
- Compiler plugin integration for both GCC and Clang
- Metadata collection during compilation
- Enhanced SBOM generation with compiler metadata
- Makefile-based build system with plugin configuration

## Prerequisites

1. **Built Heimdall**: Ensure Heimdall is built with compiler plugins enabled
   ```bash
   cd /path/to/heimdall
   mkdir -p build && cd build
   cmake .. -DBUILD_COMPILER_PLUGINS=ON
   make -j$(nproc)
   ```

2. **Compiler Support**: 
   - For GCC: GCC 7+ with plugin development headers
   - For Clang: Clang 10+ with plugin development support

## Usage Instructions

### 1. Configure Compiler
Edit the `Makefile` to set your preferred compiler:
```makefile
# Choose your compiler (gcc or clang)
CXX = g++        # For GCC with Heimdall GCC plugin
# CXX = clang++  # For Clang with Heimdall Clang plugin
```

### 2. Set Heimdall Path
Update the `HEIMDALL_ROOT` variable in the Makefile to point to your Heimdall installation:
```makefile
HEIMDALL_ROOT = /path/to/heimdall/build
```

### 3. Build the Project
```bash
# Clean any previous builds
make clean

# Build the project with compiler metadata collection
make all

# This will:
# 1. Compile all source files with Heimdall compiler plugin
# 2. Collect metadata in build/metadata/
# 3. Link the executable
# 4. Generate enhanced SBOM with compiler + linker metadata
```

### 4. Run the Application
```bash
# Execute the built program
./build/calculator

# The program will demonstrate various mathematical operations
```

### 5. Examine Generated Files

**Compiler Metadata:**
```bash
# View collected compiler metadata
ls -la build/metadata/
cat build/metadata/*.json
```

**Enhanced SBOM:**
```bash
# View the generated SBOM
cat build/sbom/calculator.cdx.json

# The SBOM includes:
# - Source files with hashes and license information
# - Header dependencies with system/project classification  
# - Compiler version and build flags
# - License detection results
# - File integrity checksums
```

## Makefile Targets

- `make all` - Complete build with metadata collection and SBOM generation
- `make compile` - Compile source files with compiler plugins
- `make link` - Link object files into executable
- `make sbom` - Generate enhanced SBOM from metadata
- `make clean` - Remove all build artifacts
- `make help` - Show available targets and usage

## Expected Output

When running the example, you should see:

1. **Compilation Phase:**
   ```
   [Heimdall GCC Plugin] Processing main.cpp
   [Heimdall GCC Plugin] Detected includes: calculator.h, utils.h
   [Heimdall GCC Plugin] Metadata written to build/metadata/main_cpp.json
   ```

2. **SBOM Generation:**
   ```
   Enhanced SBOM generated with 12 components
   - Source files: 6
   - Include files: 4  
   - System headers: 2
   - Unique licenses detected: 3
   ```

3. **Final SBOM Content:**
   - Complete dependency graph
   - File integrity hashes for all sources
   - License identification for project and system files
   - Compiler environment details
   - Build reproducibility information

## Troubleshooting

**Plugin Not Found:**
- Verify `HEIMDALL_ROOT` path is correct
- Check that Heimdall was built with `BUILD_COMPILER_PLUGINS=ON`
- Ensure compiler plugin shared libraries exist

**Permission Errors:**
- Verify write permissions for `build/` directory
- Check that metadata directory can be created

**Compilation Errors:**
- Ensure your compiler version supports the plugin interface
- Check that all required headers are available

## Customization

You can modify this example by:
- Adding more source files to `src/`
- Changing compiler flags in the Makefile
- Adjusting SBOM output format (CycloneDX/SPDX)
- Adding custom license headers to see license detection
- Including external dependencies

This example provides a complete foundation for integrating Heimdall's compiler plugin system into Makefile-based C++ projects.