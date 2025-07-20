# Heimdall Linker Integration Rationale

This document explains the technical rationale behind Heimdall's dual approach to linker integration: the **LLD wrapper approach** and the **Gold plugin approach**.

## Overview

Heimdall supports two different linkers with fundamentally different integration approaches:

1. **LLVM LLD**: Uses a wrapper approach with `heimdall-sbom` tool
2. **GNU Gold**: Uses a native plugin interface with `--plugin` and `--plugin-opt`

This dual approach was necessitated by fundamental differences in how these linkers handle plugins and the technical limitations of each platform.

## LLD Wrapper Approach

### Why Not LLD Plugin Interface?

LLVM LLD does **not** support linker plugins in the traditional sense. The plugin interface in LLD is designed for **IR passes** (Intermediate Representation passes), not for linker plugins that can intercept the linking process and generate additional outputs.

#### Technical Limitations

1. **IR Pass Architecture**: LLD's plugin system is built around LLVM's IR pass infrastructure
2. **No Linker Hook Points**: LLD doesn't provide hooks for custom linker behavior
3. **Different Plugin API**: The plugin interface is designed for compiler passes, not linker operations
4. **LLVM 19+ Changes**: The plugin interface was completely redesigned in LLVM 19, breaking compatibility

#### LLVM Version Compatibility

| LLVM Version | Plugin Interface | Heimdall Support | Status |
|--------------|------------------|------------------|--------|
| 7-18        | IR Pass Only     | ❌ Not Compatible | Legacy |
| 19+         | Completely New   | ❌ Not Compatible | Current |

### The Wrapper Solution

Heimdall implements a **wrapper approach** for LLD that works with all LLVM versions:

```bash
# Step 1: Link normally with LLD
g++ -fuse-ld=lld main.o utils.o math.o -o app

# Step 2: Generate SBOM using wrapper tool
heimdall-sbom ../../build-cpp23/lib/heimdall-lld.so app --format spdx --output app.spdx
```

#### How It Works

1. **Normal Linking**: The binary is linked normally using LLD without any plugin interference
2. **Post-Processing**: The `heimdall-sbom` tool analyzes the final binary
3. **Component Extraction**: Extracts all components, dependencies, and metadata
4. **SBOM Generation**: Generates SBOM in the requested format

#### Advantages

- ✅ **Universal Compatibility**: Works with all LLVM versions (7-19+)
- ✅ **No Plugin Dependencies**: No complex plugin loading or version matching
- ✅ **Reliable**: No risk of plugin interface changes breaking functionality
- ✅ **Simple**: Straightforward two-step process
- ✅ **Debuggable**: Easy to troubleshoot and understand

#### Disadvantages

- ❌ **Two-Step Process**: Requires separate linking and SBOM generation
- ❌ **Post-Processing**: Cannot integrate SBOM generation into the linking process
- ❌ **Manual Integration**: Requires explicit calls to `heimdall-sbom`

## Gold Plugin Approach

### Why Gold Plugin Interface?

GNU Gold linker provides a **native plugin interface** designed specifically for linker plugins:

```bash
# Direct plugin integration
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app.spdx \
    main.o utils.o math.o -o app
```

#### Technical Capabilities

1. **Native Plugin API**: Gold provides `--plugin` and `--plugin-opt` options
2. **Linker Hooks**: Plugins can intercept the linking process at various stages
3. **Real-Time Integration**: SBOM generation happens during linking
4. **Single-Step Process**: Linking and SBOM generation in one command

#### Plugin Interface Details

```cpp
// Gold plugin interface (simplified)
class GoldPlugin {
public:
    virtual void onAllocateSection() = 0;
    virtual void onRelocate() = 0;
    virtual void onFinalize() = 0;
    virtual void onWriteOutput() = 0;
};
```

### Gold Plugin Dependencies

The Gold plugin requires specific system libraries:

#### Required Libraries
- **elfutils**: ELF file manipulation library
- **libelf**: ELF object file access library  
- **libdw**: DWARF debugging information library

#### Installation by Platform

**Fedora/RHEL/CentOS:**
```bash
sudo yum install elfutils-devel
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libelf-dev libdw-dev
```

**Arch Linux:**
```bash
sudo pacman -S elfutils
```

#### Common Issues

**Error:** `undefined symbol: elf_nextscn`
- **Cause**: Missing elfutils/libelf libraries
- **Solution**: Install the required development packages

**Error:** `Could not load plugin`
- **Cause**: Plugin dependencies not satisfied
- **Solution**: Install dependencies or use wrapper approach

### Fallback Strategy

Heimdall's Gold plugin implementation includes automatic fallback:

```bash
# Attempt plugin interface first
g++ -fuse-ld=gold -Wl,--plugin=../../build-cpp23/lib/heimdall-gold.so \
    -Wl,--plugin-opt=sbom-output=app.spdx \
    main.o utils.o math.o -o app

# If plugin fails, fall back to wrapper approach
g++ -fuse-ld=gold main.o utils.o math.o -o app
heimdall-sbom ../../build-cpp23/lib/heimdall-gold.so app --format spdx --output app.spdx
```

## Platform Support Matrix

| Platform | LLD Plugin | LLD Wrapper | Gold Plugin | Gold Wrapper |
|----------|------------|-------------|-------------|--------------|
| macOS ARM64 | ❌ | ✅ | ❌ | ❌ |
| macOS x86_64 | ❌ | ✅ | ❌ | ❌ |
| Linux x86_64 | ❌ | ✅ | ✅ | ✅ |
| Linux ARM64 | ❌ | ✅ | ✅ | ✅ |

### Platform-Specific Rationale

#### macOS
- **LLD Plugin**: Not available (IR pass interface only)
- **LLD Wrapper**: ✅ Primary method
- **Gold Plugin**: Not available (Gold not supported on macOS)
- **Gold Wrapper**: Not available (Gold not supported on macOS)

#### Linux
- **LLD Plugin**: Not available (IR pass interface only)
- **LLD Wrapper**: ✅ Primary method
- **Gold Plugin**: ✅ Available with dependencies
- **Gold Wrapper**: ✅ Fallback when dependencies missing

## Design Decisions

### Why Two Different Approaches?

1. **Technical Reality**: LLD and Gold have fundamentally different plugin architectures
2. **Compatibility**: Each approach maximizes compatibility for its respective linker
3. **Reliability**: Wrapper approach provides consistent behavior across all LLVM versions
4. **User Choice**: Users can choose the approach that works best for their environment

### Plugin vs Wrapper Trade-offs

| Aspect | Plugin Approach | Wrapper Approach |
|--------|----------------|------------------|
| **Integration** | Seamless | Two-step process |
| **Compatibility** | Version-specific | Universal |
| **Dependencies** | Complex | Minimal |
| **Reliability** | Fragile | Robust |
| **Debugging** | Complex | Simple |
| **Performance** | Integrated | Post-processing |

### Future Considerations

#### LLVM LLD
- **Current**: Wrapper approach for all versions
- **Future**: Monitor LLVM plugin interface evolution
- **Recommendation**: Continue with wrapper approach for stability

#### GNU Gold
- **Current**: Plugin approach with wrapper fallback
- **Future**: Maintain plugin support, enhance wrapper capabilities
- **Recommendation**: Support both approaches for maximum compatibility

## Implementation Details

### LLD Wrapper Implementation

```cpp
// src/tools/heimdall-sbom.cpp
int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string pluginPath = argv[1];
    std::string binaryPath = argv[2];
    std::string format = argv[3];
    std::string output = argv[4];
    
    // Load the appropriate plugin
    auto plugin = loadPlugin(pluginPath);
    
    // Analyze the binary
    auto components = plugin->analyzeBinary(binaryPath);
    
    // Generate SBOM
    SBOMGenerator generator;
    generator.setFormat(format);
    generator.setOutputPath(output);
    
    for (const auto& component : components) {
        generator.processComponent(component);
    }
    
    generator.generateSBOM();
    return 0;
}
```

### Gold Plugin Implementation

```cpp
// src/plugins/gold/heimdall-gold.cpp
class HeimdallGoldPlugin : public gold::Plugin {
public:
    void onAllocateSection() override {
        // Track section allocation
    }
    
    void onRelocate() override {
        // Track symbol resolution
    }
    
    void onFinalize() override {
        // Generate SBOM
        generateSBOM();
    }
    
private:
    void generateSBOM() {
        // Collect all components
        auto components = collectComponents();
        
        // Generate SBOM in requested format
        SBOMGenerator generator;
        generator.setFormat(getPluginOption("format"));
        generator.setOutputPath(getPluginOption("sbom-output"));
        
        for (const auto& component : components) {
            generator.processComponent(component);
        }
        
        generator.generateSBOM();
    }
};
```

## Best Practices

### For LLD Users
1. **Use the wrapper approach**: It's the only reliable method
2. **Integrate into build scripts**: Automate the two-step process
3. **Handle errors gracefully**: Check if `heimdall-sbom` is available
4. **Use appropriate formats**: SPDX for compliance, CycloneDX for tooling

### For Gold Users
1. **Try plugin first**: Use the native plugin interface when possible
2. **Install dependencies**: Ensure elfutils/libelf are available
3. **Fallback gracefully**: Use wrapper approach when plugin fails
4. **Test thoroughly**: Verify SBOM generation works in your environment

### For Build System Integration
1. **Detect capabilities**: Check which approaches are available
2. **Provide fallbacks**: Always have a backup method
3. **Document requirements**: Clearly state dependencies and limitations
4. **Test across platforms**: Ensure compatibility on target systems

## Conclusion

Heimdall's dual approach to linker integration reflects the technical reality of modern linkers:

- **LLD Wrapper**: Provides universal compatibility across all LLVM versions
- **Gold Plugin**: Offers seamless integration when dependencies are satisfied

This approach ensures that Heimdall can generate accurate SBOMs regardless of the linker environment, while providing the best possible user experience for each platform.

The wrapper approach, while requiring an extra step, provides the most reliable and maintainable solution for LLD users. The Gold plugin approach, when available, provides the most integrated experience for Linux users who can satisfy the dependency requirements. 