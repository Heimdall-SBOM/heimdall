# SBOM Refactoring: Clean Architecture for SPDX and CycloneDX

## Overview

This document describes the refactored SBOM architecture that provides clean separation of concerns between SPDX and CycloneDX formats, making the codebase more maintainable, extensible, and testable.

## Problem Statement

The original SBOM implementation had several issues:

1. **Monolithic Design**: `SBOMGenerator.cpp` was a massive 2315-line file handling both SPDX and CycloneDX
2. **Code Duplication**: Similar logic repeated across different SPDX versions (2.3, 3.0.0, 3.0.1)
3. **Tight Coupling**: Format-specific logic mixed with general SBOM generation
4. **Hard to Maintain**: Large files made it difficult to modify individual formats
5. **Inconsistent Interfaces**: Different approaches for different formats

## Solution: Clean Architecture

### 1. Interface-Based Design

The new architecture uses clean interfaces to separate concerns:

```cpp
// Abstract base for all SBOM format handlers
class ISBOMFormatHandler {
    virtual std::string generateSBOM(...) = 0;
    virtual ValidationResult validateContent(...) = 0;
    virtual std::string getFormatName() const = 0;
    // ...
};

// SPDX-specific interface
class ISPDXHandler : public ISBOMFormatHandler {
    virtual void setVersion(const std::string& version) = 0;
    virtual std::string generateComponentEntry(...) = 0;
    // ...
};

// CycloneDX-specific interface
class ICycloneDXHandler : public ISBOMFormatHandler {
    virtual void setVersion(const std::string& version) = 0;
    virtual std::string generateComponentEntry(...) = 0;
    // ...
};
```

### 2. Modular Format Handlers

Each format and version has its own dedicated handler:

#### SPDX Handlers
- `SPDX2_3Handler` - Handles SPDX 2.3 tag-value format
- `SPDX3_0_0Handler` - Handles SPDX 3.0.0 JSON format
- `SPDX3_0_1Handler` - Handles SPDX 3.0.1 JSON format

#### CycloneDX Handlers
- `CycloneDX1_4Handler` - Handles CycloneDX 1.4 format
- `CycloneDX1_5Handler` - Handles CycloneDX 1.5 format
- `CycloneDX1_6Handler` - Handles CycloneDX 1.6 format

### 3. Factory Pattern

A factory class creates the appropriate handler based on format and version:

```cpp
class SBOMFormatFactory {
    static std::unique_ptr<ISPDXHandler> createSPDXHandler(const std::string& version);
    static std::unique_ptr<ICycloneDXHandler> createCycloneDXHandler(const std::string& version);
    static std::unique_ptr<ISBOMFormatHandler> createHandler(const std::string& format, const std::string& version);
};
```

### 4. Clean SBOM Generator

The new `SBOMGeneratorV2` is much simpler and focused:

```cpp
class SBOMGeneratorV2 {
    void processComponent(const ComponentInfo& component);
    bool generateSBOM();
    std::string generateSBOMContent();
    void setFormat(const std::string& format);
    // ...
};
```

## File Structure

```
src/common/
├── SBOMFormats.hpp              # Interface definitions
├── SBOMFormatFactory.cpp        # Factory implementation
├── SPDXHandler.hpp              # SPDX handler interfaces
├── SPDX2_3Handler.cpp           # SPDX 2.3 implementation
├── SPDX3_0_0Handler.cpp         # SPDX 3.0.0 implementation
├── SPDX3_0_1Handler.cpp         # SPDX 3.0.1 implementation
├── CycloneDXHandler.hpp         # CycloneDX handler interfaces
├── CycloneDX1_4Handler.cpp      # CycloneDX 1.4 implementation
├── CycloneDX1_5Handler.cpp      # CycloneDX 1.5 implementation
├── CycloneDX1_6Handler.cpp      # CycloneDX 1.6 implementation
├── SBOMGeneratorV2.hpp          # Clean SBOM generator interface
└── SBOMGeneratorV2.cpp          # Clean SBOM generator implementation
```

## Benefits

### 1. **Maintainability**
- Each format handler is focused on a single responsibility
- Changes to one format don't affect others
- Easier to understand and modify individual components

### 2. **Extensibility**
- Adding new formats requires implementing the interface
- Adding new versions requires creating a new handler class
- No changes needed to the main generator

### 3. **Testability**
- Each handler can be tested independently
- Mock handlers can be easily created for testing
- Unit tests are more focused and reliable

### 4. **Code Reuse**
- Common functionality shared through base classes
- Utilities can be reused across different formats
- Consistent interfaces across all handlers

### 5. **Performance**
- Only the required handler is loaded
- No unnecessary code execution
- Better memory usage

## Usage Examples

### Basic Usage

```cpp
// Create generator
heimdall::SBOMGeneratorV2 generator;

// Set format and version
generator.setFormat("spdx");
generator.setSPDXVersion("2.3");

// Add components
ComponentInfo component("myapp", "/path/to/myapp");
generator.processComponent(component);

// Generate SBOM
generator.setOutputPath("myapp.spdx");
generator.generateSBOM();
```

### Advanced Usage

```cpp
// Create specific handler directly
auto spdxHandler = heimdall::SBOMFormatFactory::createSPDXHandler("3.0.1");

// Generate content without file I/O
std::string content = spdxHandler->generateSBOM(components, metadata);

// Validate content
auto result = spdxHandler->validateContent(content);
if (!result.isValid) {
    for (const auto& error : result.errors) {
        std::cerr << "Error: " << error << std::endl;
    }
}
```

### Adding New Formats

```cpp
// 1. Implement the interface
class MyCustomHandler : public heimdall::ISBOMFormatHandler {
    std::string generateSBOM(...) override { /* implementation */ }
    ValidationResult validateContent(...) override { /* implementation */ }
    // ... other required methods
};

// 2. Add to factory
std::unique_ptr<ISBOMFormatHandler> SBOMFormatFactory::createHandler(const std::string& format) {
    if (format == "mycustom") {
        return std::make_unique<MyCustomHandler>();
    }
    // ... existing code
}
```

## Migration Strategy

### Phase 1: Parallel Implementation
- Implement new architecture alongside existing code
- Add feature flags to switch between implementations
- Maintain backward compatibility

### Phase 2: Gradual Migration
- Update plugins to use new `SBOMGeneratorV2`
- Migrate tools one by one
- Update tests to use new architecture

### Phase 3: Cleanup
- Remove old `SBOMGenerator.cpp`
- Remove feature flags
- Update documentation

## Testing Strategy

### Unit Tests
- Test each handler independently
- Mock dependencies for isolated testing
- Test format-specific edge cases

### Integration Tests
- Test factory creation
- Test end-to-end SBOM generation
- Test format validation

### Compatibility Tests
- Ensure new implementation produces identical output
- Test backward compatibility
- Validate against official schemas

## Future Enhancements

### 1. **Plugin System**
- Allow external format handlers
- Dynamic loading of format plugins
- Third-party format support

### 2. **Performance Optimizations**
- Lazy loading of handlers
- Caching of generated content
- Parallel processing of components

### 3. **Enhanced Validation**
- Schema-based validation
- Custom validation rules
- Validation plugins

### 4. **Format Conversion**
- Convert between formats
- Lossless transformations
- Format comparison tools

## Conclusion

The refactored SBOM architecture provides a clean, maintainable, and extensible foundation for handling multiple SBOM formats. The separation of concerns makes the code easier to understand, test, and modify, while the interface-based design allows for easy extension and customization.

This architecture positions Heimdall for future growth and ensures that adding new SBOM formats or versions will be straightforward and won't impact existing functionality. 