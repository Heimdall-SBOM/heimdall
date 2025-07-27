# LLD Integration Rationale for Heimdall SBOM Generation

*Heimdall Project*  
*Last updated: July 2025*

## Introduction

Heimdall aims to generate Software Bill of Materials (SBOMs) for binaries produced by linkers such as GNU Gold and LLVM LLD. While Gold provides a plugin API that calls user hooks during linking, LLD does not natively support this style of integration. This document explains the rationale behind the current wrapper approach and discusses what would be required for deeper integration with LLD.

## Current Solution: Wrapper Approach

### How It Works

- The build system or user invokes a wrapper script instead of calling LLD directly.
- The script first runs LLD to link the program as usual.
- After linking, the script loads the Heimdall plugin and calls its SBOM generation functions, passing in the output binary and relevant input files.
- The plugin generates the SBOM for the linked output.

### Advantages

- **No need to patch or recompile LLD.**
- **Works with any stock LLD installation.**
- **Simple to maintain and deploy.**
- **SBOM is always up-to-date with the actual linked output.**

### Limitations

- The plugin is not called during the linking process, so per-file or per-library callbacks are not available.
- The user must use the wrapper script instead of invoking LLD directly.

## Deeper Integration: Modifying LLD

### What Would Be Required

- Modify LLD's source code (e.g., in `lld/ELF/Driver.cpp`) to load the Heimdall plugin at runtime.
- Add hooks in LLD's file and library processing loops to call plugin functions such as `heimdall_process_input_file` and `heimdall_finalize`.
- Recompile LLD with these changes.
- Distribute and use the custom LLD binary.

### Advantages

- **True automatic SBOM generation:** The plugin is called for each file and library as they are processed by LLD.
- **No need for a wrapper script.**
- **Closer to Gold plugin experience.**

### Limitations

- **Requires patching and recompiling LLD.**
- **Not compatible with stock LLD installations.**
- **Maintenance burden:** Must keep the patch up-to-date with LLD releases.

## Comparison

| Approach | Requires LLD Patch | Works with Stock LLD |
|----------|-------------------|---------------------|
| Wrapper Script | No | Yes |
| Deeper Integration (LLD Patch) | Yes | No |

## Conclusion

The wrapper approach is the most practical solution for SBOM generation with LLD today, as it requires no changes to the linker and works with any standard installation. For projects that require true per-file integration, a custom-patched LLD is possible but comes with significant maintenance and deployment costs.

## References

- [Heimdall Project](https://github.com/heimdall)
- [LLVM LLD Documentation](https://lld.llvm.org/)
- [Gold Plugin API](https://sourceware.org/binutils/docs-2.39/gold/Plugin.html) 