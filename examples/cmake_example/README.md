# Heimdall Compiler Plugin CMake Example

This example demonstrates how to use Heimdall's compiler plugins with a CMake-based C++ project. The example creates a sophisticated multi-module application with network simulation capabilities and shows how compiler metadata is captured and integrated into SBOMs.

## Project Structure

```
cmake_example/
├── README.md                # This file
├── CMakeLists.txt          # Main CMake configuration
├── src/
│   ├── main.cpp            # Main application entry point
│   ├── core/
│   │   ├── Application.cpp  # Application core implementation
│   │   ├── Application.h    # Application core header
│   │   ├── Logger.cpp      # Logging system implementation
│   │   └── Logger.h        # Logging system header
│   └── network/
│       ├── Server.cpp      # Network server implementation
│       ├── Server.h        # Network server header
│       ├── Client.cpp      # Network client implementation
│       └── Client.h        # Network client header
└── build/                  # Build output directory (created during build)
    ├── obj/                # Object files (CMake managed)
    ├── metadata/           # Compiler metadata from plugins
    └── sbom/               # Generated SBOMs
```

## Features Demonstrated

- Multi-module C++ project with separate core and network components
- CMake build system with compiler detection and plugin integration
- Advanced C++ features (inheritance, templates, smart pointers, threading)
- Network simulation with client-server architecture
- Logging system with different severity levels
- Compiler plugin integration for both GCC and Clang
- Enhanced SBOM generation with complete dependency tracking

## Prerequisites

1. **Built Heimdall**: Ensure Heimdall is built with compiler plugins enabled
   ```bash
   cd /path/to/heimdall
   mkdir -p build && cd build
   cmake .. -DBUILD_COMPILER_PLUGINS=ON
   make -j$(nproc)
   ```

2. **CMake**: CMake 3.16+ for modern C++ support
3. **Compiler Support**:
   - For GCC: GCC 7+ with plugin development headers
   - For Clang: Clang 10+ with plugin development support

## Usage Instructions

### 1. Configure Build

CMake will automatically detect your compiler and configure the appropriate Heimdall plugin:

```bash
# Create build directory
mkdir build && cd build

# Configure with GCC
cmake .. -DCMAKE_CXX_COMPILER=g++

# Or configure with Clang
cmake .. -DCMAKE_CXX_COMPILER=clang++

# Set custom Heimdall path if needed
cmake .. -DHEIMDALL_ROOT=/path/to/heimdall/build
```

### 2. Build the Project

```bash
# Build with compiler metadata collection
make

# This will:
# 1. Compile all source files with Heimdall compiler plugin
# 2. Collect metadata in build/metadata/
# 3. Link the executable
# 4. Generate enhanced SBOM with compiler + linker metadata
```

### 3. Run the Application

```bash
# Execute the network simulation
./network_simulator

# The program will demonstrate:
# - Server startup and client connections
# - Message passing between components
# - Logging system with different severity levels
# - Resource management and cleanup
```

### 4. Examine Generated Files

**Compiler Metadata:**
```bash
# View collected compiler metadata
ls -la build/metadata/
cat build/metadata/*.json
```

**Enhanced SBOM:**
```bash
# View the generated SBOM
cat build/sbom/network_simulator.cdx.json

# The SBOM includes:
# - All source and header files with integrity hashes
# - CMake-generated build configuration
# - Compiler flags and optimization settings
# - Library dependencies and system includes
# - License detection results for all components
```

## CMake Integration Features

### Automatic Compiler Detection
```cmake
# Detects GCC or Clang and sets appropriate plugin
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_TYPE "gcc")
    set(HEIMDALL_PLUGIN "${HEIMDALL_ROOT}/src/compiler/gcc/libheimdall-gcc-plugin.so")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_TYPE "clang") 
    set(HEIMDALL_PLUGIN "${HEIMDALL_ROOT}/src/compiler/clang/libheimdall-clang-plugin.so")
endif()
```

### Plugin Configuration
```cmake
# Adds compiler flags for Heimdall plugin integration
if(COMPILER_TYPE STREQUAL "gcc")
    target_compile_options(${TARGET_NAME} PRIVATE
        -fplugin=${HEIMDALL_PLUGIN}
        -fplugin-arg-heimdall-gcc-plugin-output-dir=${METADATA_DIR}
    )
elseif(COMPILER_TYPE STREQUAL "clang")
    target_compile_options(${TARGET_NAME} PRIVATE
        -Xclang -load -Xclang ${HEIMDALL_PLUGIN}
        -Xclang -plugin -Xclang heimdall-metadata-collector
    )
endif()
```

### SBOM Generation Target
```cmake
# Custom target for SBOM generation
add_custom_target(sbom
    COMMAND ${SBOM_GENERATOR} ${LINKER_PLUGIN} $<TARGET_FILE:${TARGET_NAME}>
            --format cyclonedx --output ${SBOM_DIR}/${TARGET_NAME}.cdx.json
            --metadata-dir ${METADATA_DIR} --verbose
    DEPENDS ${TARGET_NAME}
    COMMENT "Generating enhanced SBOM with compiler metadata"
)
```

## Available Targets

- `make` - Build the complete project with metadata collection
- `make sbom` - Generate enhanced SBOM from metadata
- `make clean` - Remove all build artifacts
- `make install` - Install the application (if configured)

## Expected Output

### Compilation Phase:
```
[Heimdall GCC Plugin] Processing main.cpp
[Heimdall GCC Plugin] Detected includes: core/Application.h, network/Server.h
[Heimdall GCC Plugin] Metadata written to build/metadata/main_cpp.json
[Heimdall GCC Plugin] Processing core/Application.cpp
[Heimdall GCC Plugin] Processing network/Server.cpp
```

### Application Execution:
```
=== Network Simulator Demo ===
[INFO] Application starting up...
[INFO] Server listening on port 8080
[INFO] Client connecting to server
[INFO] Message sent: "Hello from client"
[INFO] Server received: "Hello from client"
[INFO] Response sent: "Message acknowledged"
[DEBUG] Connection statistics: 1 active, 0 pending
[INFO] Application shutting down gracefully
```

### SBOM Generation:
```
Enhanced SBOM generated with 18 components
- Source files: 7
- Header files: 6  
- System headers: 5
- Unique licenses detected: 2
- Build configuration entries: 12
```

## Troubleshooting

**CMake Configuration Issues:**
- Ensure CMake 3.16+ is installed
- Verify `HEIMDALL_ROOT` path is correct in CMakeLists.txt
- Check that compiler plugins were built successfully

**Plugin Loading Errors:**
- Verify compiler plugin shared libraries exist and have correct permissions
- Ensure compiler version supports plugin interface
- Check CMake output for plugin path resolution

**SBOM Generation Failures:**
- Confirm heimdall-sbom tool is built and executable
- Verify metadata directory contains JSON files from compilation
- Check that all required plugins are accessible

## Customization

This example can be extended by:
- Adding more modules (database, GUI, etc.)
- Including external libraries (Boost, OpenSSL, etc.)
- Implementing unit tests with GoogleTest
- Adding more sophisticated network protocols
- Including configuration files and resource management
- Implementing multi-threading and async operations

The CMake build system automatically handles:
- Dependency tracking for all modules
- Compiler plugin integration regardless of chosen compiler
- Metadata collection from all compilation units
- Enhanced SBOM generation with complete project analysis

This example provides a robust foundation for integrating Heimdall's compiler plugin system into modern CMake-based C++ projects.