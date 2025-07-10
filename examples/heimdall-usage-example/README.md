# Heimdall Usage Example

This directory contains example C++ files that demonstrate how to use Heimdall as a linker plugin to generate SBOMs during compilation.

## Prerequisites

- Heimdall built and installed (see main [README.md](../../README.md) for build instructions)
- LLVM LLD linker (macOS/Linux) or GNU Gold linker (Linux only)
- GCC or Clang compiler

## Quick Start

1. **Navigate to this directory:**
   ```bash
   cd examples/heimdall-usage-example
   ```

2. **Run the example script:**
   ```bash
   ./run_example.sh
   ```

This will compile the example files and generate SBOMs using both LLD and Gold linkers (if available).

## Manual Steps

### Step 1: Compile source files to object files
```bash
g++ -c main.cpp -o main.o
g++ -c utils.cpp -o utils.o
g++ -c math.cpp -o math.o
```

### Step 2: Link with LLD and Heimdall plugin
```bash
g++ -fuse-ld=lld -Wl,--plugin-opt=load:../../build-cpp23/lib/heimdall-lld.so \
    -Wl,--plugin-opt=sbom-output:app-lld-sbom.json \
    main.o utils.o math.o -o app-lld
```

### Step 3: Link with Gold and Heimdall plugin (Linux only)
```bash
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app-gold-sbom.json \
    main.o utils.o math.o -o app-gold
```

**Note:** Gold plugin may fail due to missing dependencies (elfutils, libelf). The examples will automatically fall back to the wrapper approach if the plugin fails.

### Step 4: Run the programs and view SBOMs
```bash
# Run the LLD version
./app-lld

# Run the Gold version (if available)
./app-gold

# View the generated SBOMs
cat app-lld-sbom.json
cat app-gold-sbom.json
```

## Files in this Example

- `main.cpp` - Main program with string operations
- `utils.cpp` - Utility functions
- `math.cpp` - Mathematical functions
- `utils.h` - Header file for utils
- `math.h` - Header file for math
- `run_example.sh` - Automated script to run the example
- `README.md` - This file

## Gold Plugin Dependencies

The Gold plugin requires specific system libraries that may not be available on all systems:

- **elfutils** - ELF file manipulation library
- **libelf** - ELF object file access library
- **libdw** - DWARF debugging information library

**Common error:** `undefined symbol: elf_nextscn`

**Solutions:**
1. **Install dependencies** (if available):
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libelf-dev libdw-dev
   
   # Fedora/RHEL/CentOS
   sudo yum install elfutils-devel
   ```

2. **Use the wrapper approach** (recommended):
   ```bash
   # Link normally
   g++ -fuse-ld=gold main.o utils.o math.o -o app-gold
   
   # Generate SBOM using wrapper
   heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so app-gold --format spdx --output app-gold-sbom.json
   ```

The examples automatically detect plugin failures and fall back to the wrapper approach.

## Expected Output

After running the example, you should see:

1. **Compiled executables:**
   - `app-lld` (linked with LLD)
   - `app-gold` (linked with Gold, Linux only)

2. **Generated SBOM files:**
   - `app-lld-sbom.json` (SBOM from LLD linking)
   - `app-gold-sbom.json` (SBOM from Gold linking)

3. **Program output:**
   ```
   Hello from Heimdall example!
   String: Hello, World!
   Length: 13
   Math: 2 + 3 = 5
   Math: 10 * 5 = 50
   ```

## Troubleshooting

### Plugin not found
If you get "Could not load plugin" errors, make sure:
1. Heimdall is built (run `../../scripts/build.sh --standard 23 --compiler gcc --tests`)
2. You're using the correct path to the plugin
3. The plugin file exists in the build directory

### Linker not found
- **LLD not found:** Install LLVM/LLD or use Gold instead
- **Gold not found:** Install `binutils-gold` package

### Gold Plugin Dependencies
- **Error:** `undefined symbol: elf_nextscn`
- **Cause:** Missing elfutils/libelf libraries
- **Solution:** Install dependencies or use wrapper approach
  ```bash
  # Fedora/RHEL/CentOS
  sudo yum install elfutils-devel
  
  # Ubuntu/Debian
  sudo apt-get install libelf-dev libdw-dev
  ```

### Platform-specific notes
- **macOS:** Use LLD only (Gold is not available)
- **Linux:** Can use either LLD or Gold

### Plugin Compatibility

Heimdall supports different approaches for different linkers:

#### LLVM LLD (Wrapper Approach)
- **All LLD versions:** Use `heimdall-sbom` wrapper tool
- **No plugin interface:** LLD plugin interface is for IR passes, not linker plugins
- **Recommended:** Always use wrapper approach for LLD

#### GNU Gold (Plugin Interface)
- **Plugin interface:** `--plugin` and `--plugin-opt` options
- **Dependencies:** Requires elfutils/libelf libraries
- **Fallback:** Automatically falls back to wrapper approach if plugin fails
- **Error:** `undefined symbol: elf_nextscn` indicates missing dependencies

#### Checking Your System
```bash
# Check LLD version
ld.lld --version

# Check Gold version  
ld.gold --version

# Check if plugins are compatible
./run_example.sh
```

#### Installing Gold Plugin Dependencies

**For Gold Plugin Support:**

**Fedora/RHEL/CentOS:**
```bash
sudo yum install elfutils-devel
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libelf-dev libdw-dev
```

**After installing dependencies:**
```bash
# Test Gold plugin
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=test.json main.o -o test
```

**Note:** The wrapper approach works regardless of plugin dependencies.

## Advanced Usage

### Generate different SBOM formats
```