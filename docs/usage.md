# Heimdall Usage Guide

This guide explains how to use Heimdall to generate Software Bill of Materials (SBOMs) when compiling your C++ files.

## Overview

Heimdall is a Software Bill of Materials (SBOM) generation tool that works with both LLVM LLD and GNU Gold linkers. It automatically generates accurate SBOMs by analyzing your compiled binaries, capturing all components that actually make it into your final executables.

## Prerequisites

- Heimdall built and installed (see [README.md](../README.md) for build instructions)
- LLVM LLD linker (macOS/Linux) or GNU Gold linker (Linux only)
- Your C++ source files ready to compile

## Method 1: Using LLD (Recommended - works on macOS and Linux)

Heimdall uses a **wrapper approach** with LLD that works with all LLVM versions:

### Step 1: Compile your source file to object file
```bash
g++ -c file.cpp -o file.o
```

### Step 2: Link with LLD (normal linking)
```bash
g++ -fuse-ld=lld file.o -o file
```

### Step 3: Generate SBOM using heimdall-sbom wrapper
```bash
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx
```

**Or in one step:**
```bash
g++ -c file.cpp -o file.o && \
g++ -fuse-ld=lld file.o -o file && \
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx
```

## Method 2: Using Gold (Linux only)

Gold supports both plugin interface and wrapper approach:

### Plugin Interface (requires dependencies)
```bash
# Compile to object file
g++ -c file.cpp -o file.o

# Link with Gold plugin
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=file.spdx \
    file.o -o file
```

### Wrapper Approach (fallback if plugin fails)
```bash
# Compile to object file
g++ -c file.cpp -o file.o

# Link normally with Gold
g++ -fuse-ld=gold file.o -o file

# Generate SBOM using wrapper
heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so file --format spdx --output file.spdx
```

## Method 3: Direct compilation with LLD (macOS/Linux)

You can also compile and link in one step using LLD directly:

```bash
# Compile and link with LLD
g++ -fuse-ld=lld file.cpp -o file

# Generate SBOM using wrapper
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx
```

## Method 4: Using CMake (if you have a CMake project)

```cmake
# In your CMakeLists.txt
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

add_executable(myapp file.cpp)

# Add SBOM generation as post-build step
add_custom_command(TARGET myapp POST_BUILD
    COMMAND heimdall-sbom ${HEIMDALL_LLD} $<TARGET_FILE:myapp> 
            --format spdx --output ${CMAKE_BINARY_DIR}/myapp.spdx
    COMMENT "Generating SBOM for myapp"
)
```

## heimdall-sbom Tool Options

You can customize the SBOM generation with various options:

### Output Format
```bash
# Generate SPDX format
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx

# Generate CycloneDX format
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format cyclonedx --output file.cyclonedx.json
```

### Verbose Output
```bash
# Enable verbose output for debugging
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx --verbose
```

### Component Name
```bash
# Specify custom component name
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx --name "My Custom App"
```

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
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so test --format spdx --output test.spdx

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
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### With External Libraries
```bash
# Compile with external library
g++ -c main.cpp -o main.o

# Link with library and LLD
g++ -fuse-ld=lld main.o -L/usr/local/lib -lmylib -o app

# Generate SBOM
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so app --format spdx --output app.spdx
```

### Using Gold with Multiple Files
```bash
# Compile files
g++ -c file1.cpp -o file1.o
g++ -c file2.cpp -o file2.o

# Link with Gold (plugin interface)
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app.spdx \
    file1.o file2.o -o app

# Or use wrapper approach if plugin fails
g++ -fuse-ld=gold file1.o file2.o -o app
heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so app --format spdx --output app.spdx
```

## Platform-Specific Notes

### macOS
- Use LLD linker (Gold is not available on macOS)
- Plugin file extension is `.dylib`
- Example: `../../build-cpp23/lib/heimdall-lld.dylib`
- **Wrapper approach only** - no plugin interface available

### Linux
- Can use either LLD or Gold linker
- Plugin file extension is `.so`
- Example: `../../build-cpp23/lib/heimdall-lld.so` or `../../build-cpp23/lib/heimdall-gold.so`
- **Gold plugin interface** available with dependencies, **wrapper approach** as fallback

## Quick Reference

| Platform | Linker | Approach | Command |
|----------|--------|----------|---------|
| macOS | LLD | Wrapper | `heimdall-sbom ../../build-cpp23/lib/heimdall-lld.dylib file --format spdx --output file.spdx` |
| Linux | LLD | Wrapper | `heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so file --format spdx --output file.spdx` |
| Linux | Gold | Plugin | `g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so -Wl,--plugin-opt=sbom-output=file.spdx file.o -o file` |
| Linux | Gold | Wrapper | `heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so file --format spdx --output file.spdx` |

## Troubleshooting

### heimdall-sbom not found:
```bash
Error: heimdall-sbom: command not found
```
**Solution:** Make sure Heimdall is built and the tool is available at `../../build-cpp23/src/tools/heimdall-sbom`

### Plugin not found error:
```bash
Error: Could not load plugin
```
**Solution:** This is expected for LLD (uses wrapper approach). For Gold, install dependencies or use wrapper approach.

### LLD not found:
```bash
Error: ld.lld: command not found
```
**Solution:** Install LLVM/LLD or use Gold linker instead.

### Gold not found (Linux):
```bash
Error: ld.gold: command not found
```
**Solution:** Install `binutils-gold` package for your distribution.

### Gold plugin dependencies missing:
```bash
Error: undefined symbol: elf_nextscn
```
**Solution:** Install dependencies or use wrapper approach:
```bash
# Fedora/RHEL/CentOS
sudo yum install elfutils-devel

# Ubuntu/Debian
sudo apt-get install libelf-dev libdw-dev
```

### Permission denied:
```bash
Error: Cannot write to output file
```
**Solution:** Check write permissions for the output directory.

### Missing dependencies:
```bash
Error: LLVM libraries not found
```
**Solution:** Install LLVM development packages for your distribution.

## Best Practices

1. **Always compile to object files first** before linking
2. **Use the correct plugin path** for your platform and build standard
3. **Enable verbose output** when debugging: `--verbose`
4. **Check the generated SBOM** to ensure it contains the expected information
5. **Use appropriate output formats** (SPDX for compliance, CycloneDX for tooling)
6. **Test both approaches** on Linux (plugin and wrapper)
7. **Handle plugin failures gracefully** by falling back to wrapper approach

## Integration with Build Systems

### Makefile Example
```makefile
CXX = g++
LDFLAGS += -fuse-ld=lld

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so $@ --format spdx --output $@.spdx
```

### CMake Example
```cmake
find_library(HEIMDALL_LLD heimdall-lld REQUIRED)

add_executable(myapp main.cpp utils.cpp)

# Add SBOM generation as post-build step
add_custom_command(TARGET myapp POST_BUILD
    COMMAND heimdall-sbom ${HEIMDALL_LLD} $<TARGET_FILE:myapp> 
            --format spdx --output ${CMAKE_BINARY_DIR}/myapp.spdx
    COMMENT "Generating SBOM for myapp"
)
```

## Format-Specific Examples

Heimdall provides dedicated usage examples for generating SBOMs in specific formats:

- **SPDX Example:** See `examples/heimdall-usage-spdx-example` for a minimal project and script that generates only SPDX SBOMs. Run `./run_example.sh` in that directory to build and generate SPDX output.
- **CycloneDX Example:** See `examples/heimdall-usage-cyclonedx-example` for a minimal project and script that generates only CycloneDX SBOMs. Run `./run_example.sh` in that directory to build and generate CycloneDX output.

Each example demonstrates both the LLD and Gold linker approaches, and will fall back to the wrapper method if plugin dependencies are missing. See the respective `README.md` in each example directory for details and troubleshooting.

## Technical Details

For detailed information about the technical rationale behind the LLD wrapper approach and Gold plugin approach, see [docs/rationale.md](rationale.md).

This guide should help you get started with using Heimdall for SBOM generation in your C++ projects! 