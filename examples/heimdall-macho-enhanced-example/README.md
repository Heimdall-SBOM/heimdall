# Enhanced Mach-O Analysis Example

This example demonstrates the enhanced Mach-O parsing capabilities of the Heimdall SBOM tool. It shows how to extract comprehensive metadata from macOS executables, including:

## Features Demonstrated

### 1. **Enhanced Version Extraction**
- **LC_ID_DYLIB version** - Extracts version from dynamic library commands
- **LC_SOURCE_VERSION** - Extracts source code version information
- **Version symbols** - Fallback to version-related symbols

### 2. **Code Signing Information**
- **Signer identity** - Code signer information
- **Team ID** - Development team identifier
- **Certificate hash** - Certificate fingerprint
- **Hardened runtime** - Security runtime detection
- **Ad-hoc signing** - Ad-hoc signature detection

### 3. **Build Configuration**
- **Target platform** - iOS, macOS, tvOS, watchOS
- **Minimum OS version** - Required OS version
- **SDK version** - Build SDK information
- **Simulator detection** - Simulator vs device builds

### 4. **Platform Information**
- **Architecture detection** - x86_64, arm64, arm, etc.
- **Platform flags** - Platform-specific features
- **Simulator builds** - Simulator vs device detection

### 5. **Multi-Architecture Support**
- **Fat binary parsing** - Universal binary support
- **Architecture enumeration** - All contained architectures
- **Architecture metadata** - CPU types, offsets, sizes

### 6. **Framework Dependencies**
- **Framework detection** - .framework dependencies
- **Dynamic library analysis** - All linked libraries
- **Dependency categorization** - Framework vs library separation

### 7. **Security Features**
- **Entitlements extraction** - App entitlements
- **Hardened runtime** - Security runtime detection
- **Code signing validation** - Signature verification

## Building the Example

```bash
# From the project root
mkdir build && cd build
cmake ..
make macho-enhanced-example
```

## Usage

```bash
# Analyze a macOS executable
./macho-enhanced-example /usr/bin/ls

# Analyze a macOS application
./macho-enhanced-example /Applications/Safari.app/Contents/MacOS/Safari

# Analyze a framework
./macho-enhanced-example /System/Library/Frameworks/Cocoa.framework/Cocoa
```

## Example Output

```
=== Enhanced Mach-O Analysis ===
File: /usr/bin/ls

✓ File is a Mach-O binary
✓ Basic metadata extracted successfully
✓ Enhanced Mach-O metadata extracted successfully

=== Analysis Results ===
File Type: Executable
File Size: 45632 bytes
SHA256: a1b2c3d4e5f6...
UUID: 12345678-9ABC-DEF0-1122-334455667788

--- Platform Information ---
Architecture: x86_64
Platform: macos

--- Build Configuration ---
Target Platform: macos
Minimum OS Version: 10.15.0
SDK Version: 12.3.0

--- Code Signing Information ---
Ad-hoc Signed: No
Hardened Runtime: Yes

--- Architectures ---
Architecture: x86_64
  CPU Type: 0x1000007
  CPU Subtype: 0x3
  Offset: 0
  Size: 45632 bytes
  Alignment: 4096

--- Dependencies ---
Dependency: /usr/lib/libSystem.B.dylib
Dependency: /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation

--- Framework Dependencies ---
Framework: /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation

--- Entitlements ---
Entitlement: com.apple.security.hardened-runtime

--- Symbols (first 10) ---
Symbol: _main (global) (defined)
Symbol: _usage (global) (defined)
Symbol: _listxattr (global) (defined)
...

--- Sections (first 10) ---
Section: __text (4096 bytes)
Section: __data (1024 bytes)
Section: __bss (512 bytes)
...

=== Analysis Complete ===
```

## Benefits for SBOM Generation

This enhanced Mach-O parsing provides several key benefits for SBOM generation:

1. **Comprehensive Dependency Analysis** - Identifies all linked libraries and frameworks
2. **Security Compliance** - Code signing and entitlement information for security audits
3. **Build Reproducibility** - SDK versions and build configuration for reproducible builds
4. **Platform Compatibility** - Architecture and platform information for deployment planning
5. **Version Tracking** - Multiple version sources for accurate component identification

## Integration with SBOM Standards

The extracted metadata can be mapped to SBOM standards:

- **CycloneDX**: Component metadata, dependencies, properties
- **SPDX**: Package information, relationships, external references
- **SWID**: Software identification, version information

## Technical Details

The implementation uses native macOS APIs and Mach-O parsing to extract:

- **Load Commands**: LC_UUID, LC_ID_DYLIB, LC_SOURCE_VERSION, LC_BUILD_VERSION
- **Header Information**: CPU types, flags, file types
- **Symbol Tables**: Function and variable symbols
- **Section Data**: Code and data sections
- **Code Signing**: CMS signatures and entitlements

This provides a comprehensive view of macOS executables for security analysis, compliance, and software supply chain management. 