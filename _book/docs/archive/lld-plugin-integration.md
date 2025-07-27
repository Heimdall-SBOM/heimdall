# LLD Plugin Integration Guide

## Overview

LLD (LLVM Linker) supports plugins, but the integration is different from Gold's plugin system. This guide shows how to make LLD call our SBOM generation callbacks during the linking process.

## Method 1: LLD Native Plugin Interface

### Plugin Structure

LLD plugins need to implement specific entry points that LLD calls during linking:

```cpp
// LLD Plugin Entry Points
extern "C" {
    // Called when plugin is loaded
    void heimdall_lld_plugin_init() {
        // Initialize our plugin
        onload(nullptr);
    }
    
    // Called when plugin is unloaded
    void heimdall_lld_plugin_cleanup() {
        // Generate final SBOM
        heimdall_finalize();
        onunload();
    }
    
    // Called for each input file
    void heimdall_lld_process_file(const char* filePath) {
        heimdall_process_input_file(filePath);
    }
    
    // Called for each library
    void heimdall_lld_process_library(const char* libraryPath) {
        heimdall_process_library(libraryPath);
    }
}
```

### Usage

```bash
# Load plugin during linking
clang -fuse-ld=lld -Wl,--load-pass-plugin=./heimdall-lld.dylib main.o -o program
```

## Method 2: LLVM Pass Plugin Integration

LLD supports LLVM pass plugins that can be called during the IR processing phase:

```cpp
// LLVM Pass Plugin
class HeimdallPass : public llvm::ModulePass {
public:
    bool runOnModule(llvm::Module& M) override {
        // Process module and generate SBOM
        std::string moduleName = M.getModuleIdentifier();
        heimdall_process_input_file(moduleName.c_str());
        return false; // Don't modify the module
    }
};

extern "C" {
    llvm::PassPluginLibraryInfo getPassPluginInfo() {
        return {
            LLVM_PLUGIN_API_VERSION, "HeimdallSBOM", "v1.0.0",
            [](llvm::PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef Name, llvm::ModulePassManager& MPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        if (Name == "heimdall-sbom") {
                            MPM.addPass(HeimdallPass());
                            return true;
                        }
                        return false;
                    });
            }
        };
    }
}
```

### Usage

```bash
# Use as LLVM pass plugin
clang -fuse-ld=lld -Wl,--load-pass-plugin=./heimdall-lld.dylib \
     -Wl,--pass-plugin-arg=heimdall-sbom main.o -o program
```

## Method 3: Environment-Based Integration

Set environment variables to trigger plugin functionality:

```bash
export HEIMDALL_PLUGIN_PATH="./heimdall-lld.dylib"
export HEIMDALL_SBOM_OUTPUT="sbom.json"
export HEIMDALL_VERBOSE=1

clang -fuse-ld=lld main.o -o program
```

## Method 4: Wrapper Script Approach

Create a wrapper script that calls our plugin after linking:

```bash
#!/bin/bash
# lld-with-sbom.sh

# Run LLD linking
ld64.lld "$@"

# After linking, generate SBOM
if [[ -f "$HEIMDALL_PLUGIN_PATH" ]]; then
    # Load plugin and process linked files
    ./heimdall-sbom-generator "$OUTPUT_FILE"
fi
```

## Method 5: LLD Source Code Integration (Advanced)

For complete integration, modify LLD source code to call our hooks:

```cpp
// In LLD source code (lld/ELF/Driver.cpp)
void link(ArrayRef<const char *> args) {
    // ... existing linking code ...
    
    // Call our plugin hooks
    if (auto *plugin = getPlugin("heimdall")) {
        for (const auto &file : files) {
            plugin->processFile(file->getName());
        }
        plugin->finalize();
    }
}
```

## Current Status

✅ **What Works:**
- Plugin loads successfully with `--load-pass-plugin`
- Manual SBOM generation works
- Plugin functions are callable

⚠️ **What Needs Work:**
- LLD doesn't automatically call our file processing hooks
- Need to integrate with LLD's internal linking process
- Requires either LLD source modification or wrapper approach

## Recommended Approach

For now, use **Method 4 (Wrapper Script)** as it provides the best balance of functionality and ease of use:

```bash
# Use the wrapper script
./scripts/lld-with-sbom.sh main.o sbom.json
```

This approach:
1. Runs LLD linking normally
2. Automatically generates SBOM after linking
3. Processes all linked files
4. Works with existing LLD installations

## Future Improvements

To enable true automatic SBOM generation, we would need to:

1. **Modify LLD source code** to call our plugin hooks
2. **Implement LLD's internal plugin API** properly
3. **Create a patch** for LLD that adds SBOM generation support
4. **Submit upstream** to the LLVM project

## Testing

Test the integration:

```bash
# Run the test script
./scripts/lld-plugin-test.sh

# Check results
ls -la test_output/*.json
cat test_output/lld-sbom.json
```

This demonstrates that while LLD can load our plugin, true automatic callback integration requires deeper LLD integration. 