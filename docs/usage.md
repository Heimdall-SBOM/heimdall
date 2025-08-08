# Heimdall Usage Guide

This guide explains how to use Heimdall to generate Software Bill of Materials (SBOMs) when compiling your C or C++ files.

## Overview

Heimdall is a Software Bill of Materials (SBOM) generation tool that works with both LLVM LLD and GNU Gold linkers. It automatically generates accurate SBOMs by analyzing your compiled binaries, capturing all components that actually make it into your final executables.

## Prerequisites

- Heimdall built and installed (see [README.md](../README.md) for build instructions)
- LLVM LLD linker (macOS/Linux) or GNU Gold linker (Linux only)
- Your C++ source files ready to compile

## Method 1: Using LLD (Recommended - works on macOS and Linux)

Heimdall uses a **wrapper approach** with LLD that works with all LLVM versions:

### Step 1: Compile and link application 
```bash
g++ -c file.cpp -o file.o
g++ -fuse-ld=lld file.o -o file
```

### Step 2: Generate SBOM using heimdall-sbom wrapper
```bash
heimdall-sbom /usr/local/lib/heimdall-lld.so file --format spdx --output file.spdx
```

**Or in one step:**
```bash
g++ -c file.cpp -o file.o && \
g++ -fuse-ld=lld file.o -o file && \
heimdall-sbom /usr/local/lib/heimdall-lld.so file --format spdx --output file.spdx
```

## Method 2: Using Gold (Linux only)

Gold supports both plugin interface and wrapper approach:

### Plugin Interface (requires dependencies)
```bash
# Compile to object file
g++ -c file.cpp -o file.o

# Link with Gold plugin
g++ -fuse-ld=gold -Wl,--plugin=/usr/local/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=file.spdx \
    file.o -o file
```

### Heimdall-SBOM Wrapper Approach 
```bash
# Compile to object file
g++ -c file.cpp -o file.o

# Link normally with Gold
g++ -fuse-ld=gold file.o -o file

# Generate SBOM using wrapper
heimdall-sbom /usr/local/lib/heimdall-gold.so file --format spdx --output file.spdx
```

## Method 3: Direct compilation with LLD (macOS/Linux)

You can also compile and link in one step using LLD directly:

```bash
# Compile and link with LLD
g++ -fuse-ld=lld file.cpp -o file

# Generate SBOM using wrapper
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx
```

## Method 4: Using Compiler Plugins (Enhanced SBOM Generation)

Heimdall provides compiler plugins for GCC and Clang that extract metadata during compilation for enhanced SBOM generation. These plugins are **automatically enabled** when using the Heimdall build script.

### GCC Plugin Usage
```bash
# Compile with GCC plugin (automatic when built with Heimdall)
gcc -fplugin=/path/to/heimdall-gcc-plugin.so \
    -fplugin-arg-heimdall-gcc-plugin-output-dir=./sbom \
    -fplugin-arg-heimdall-gcc-plugin-verbose \
    -c source.cpp -o source.o

# Link normally
gcc source.o -o app

# Generate final SBOM combining all metadata
heimdall-sbom /usr/local/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### Clang Plugin Usage
```bash
# Compile with Clang plugin (automatic when built with Heimdall)
clang++ -load /path/to/heimdall-clang-plugin.so \
        -plugin heimdall-sbom \
        -plugin-arg-heimdall-sbom-output-dir=./sbom \
        -plugin-arg-heimdall-sbom-verbose \
        -c source.cpp -o source.o

# Link normally
clang++ source.o -o app

# Generate final SBOM combining all metadata
heimdall-sbom /usr/local/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### Benefits of Compiler Plugins
- **Source-level metadata**: Function names, class definitions, global variables
- **Preprocessor information**: Include files, macro definitions, compiler flags
- **Compilation context**: Compiler version, target architecture, optimization settings
- **Enhanced accuracy**: Captures information not available in binary analysis alone

For detailed information about compiler plugins, see [docs/compiler_plugins.md](compiler_plugins.md).

## Method 5: Using CMake 

Heimdall provides a powerful CMake module for seamless SBOM generation. This module supports:
- Executables and libraries (static/shared/interface)
- Multi-target projects
- Installable projects
- Automatic linker detection (LLD/Gold)

### Quick Integration

1. Add the `cmake/` directory to your `CMAKE_MODULE_PATH`:
   ```cmake
   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
   ```
2. Include the Heimdall modules:
   ```cmake
   include(HeimdallConfig)
   include(HeimdallSBOM)
   ```
3. Add your targets and enable SBOM generation:
   ```cmake
   add_executable(myapp main.cpp)
   heimdall_enable_sbom(myapp FORMAT spdx-2.3 VERBOSE ON)
   ```
## Method 6: Using make

```makefile
CXX = g++
LDFLAGS += -fuse-ld=lld

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	heimdall-sbom /usr/local/lib/heimdall-lld.so $@ --format spdx --output $@.spdx
```



See [`cmake/templates/cmake-sbom-template.cmake`](../cmake/templates/cmake-sbom-template.cmake) for a ready-to-use template.

### Advanced CMake Examples

Heimdall includes advanced CMake module examples:

| Example Directory | Description |
|-------------------|-------------|
| `heimdall-cmake-module-example` | Multi-target (executable + static lib) |
| `heimdall-cmake-sharedlib-example` | Shared library + executable |
| `heimdall-cmake-interface-example` | Interface (header-only) + implementation + executable |
| `heimdall-cmake-install-example` | Installable static lib + executable + headers |


## Complete Example

Here's a complete example with a simple C++ file:

```bash
# Create a simple test file
cat > test.cpp << 'EOF'
#include <iostream>
#include <string>

int main() {
    std::string message = "Hello, Heimdall!";
    std::cout << message << std::endl;
    return 0;
}
EOF

# Compile to object file
g++ -c test.cpp -o test.o

# Link with LLD
g++ -fuse-ld=lld test.o -o test

# Generate SBOM using wrapper
heimdall-sbom /usr/local/lib/heimdall-lld.so test --format spdx --output test.spdx

# Run the program
./test

# View the generated SBOM
cat test.spdx
```

## Advanced Examples

### Multiple Source Files
```bash
# Compile multiple source files
g++ -c main.cpp -o main.o
g++ -c utils.cpp -o utils.o
g++ -c math.cpp -o math.o

# Link with LLD
g++ -fuse-ld=lld main.o utils.o math.o -o app

# Generate SBOM
heimdall-sbom /usr/local/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### With External Libraries
```bash
# Compile with external library
g++ -c main.cpp -o main.o

# Link with library and LLD
g++ -fuse-ld=lld main.o -L/usr/local/lib -lmylib -o app

# Generate SBOM
heimdall-sbom /usr/local/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### Using Gold with Multiple Files
```bash
# Compile files
g++ -c file1.cpp -o file1.o
g++ -c file2.cpp -o file2.o

# Link with Gold (plugin interface)
g++ -fuse-ld=gold -Wl,--plugin=./usr/local/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app.spdx \
    file1.o file2.o -o app

# Or use wrapper approach if plugin fails
g++ -fuse-ld=gold file1.o file2.o -o app
heimdall-sbom /usr/local/lib//heimdall-gold.so app --format spdx --output app.spdx
```

## Platform-Specific Notes

### macOS
- Use LLD linker (Gold is not available on macOS)
- Plugin file extension is `.dylib`
- Example: `/usr/local/lib/heimdall-lld.dylib`
- **Wrapper approach only** - no plugin interface available

### Linux
- Can use either LLD or Gold linker
- Plugin file extension is `.so`
- Example: `/usr/local/lib/heimdall-lld.so` or `/usr/local/lib/heimdall-gold.so`
- **Gold plugin interface** available with dependencies, **wrapper approach** as fallback

## Quick Reference

| Platform | Linker | Approach | Command |
|----------|--------|----------|---------|
| macOS | LLD | Wrapper | `heimdall-sbom /usr/local/lib/heimdall-lld.dylib file --format spdx --output file.spdx` |
| Linux | LLD | Wrapper | `heimdall-sbom /usr/local/lib/heimdall-lld.so file --format spdx --output file.spdx` |
| Linux | Gold | Plugin | `g++ -fuse-ld=gold -Wl,--plugin=/usr/local/lib/heimdall-gold.so -Wl,--plugin-opt=sbom-output=file.spdx file.o -o file` |
| Linux | Gold | Wrapper | `heimdall-sbom ./usr/local/lib/heimdall-gold.so file --format spdx --output file.spdx` |


## Integration with Build Systems

### Makefile Example
```makefile
CXX = g++
LDFLAGS += -fuse-ld=lld

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	heimdall-sbom /usr/local/lib/heimdall-lld.so $@ --format spdx --output $@.spdx
```

