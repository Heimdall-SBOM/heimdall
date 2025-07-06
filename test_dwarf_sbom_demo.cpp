#include <iostream>
#include <fstream>
#include "src/common/MetadataExtractor.hpp"
#include "src/common/SBOMGenerator.hpp"

int main() {
    std::cout << "=== Heimdall DWARF-Enhanced SBOM Demo ===" << std::endl;
    
    // Create metadata extractor with debug info enabled
    heimdall::MetadataExtractor extractor;
    extractor.setExtractDebugInfo(true);
    extractor.setVerbose(true);
    
    // Process our test binary
    heimdall::ComponentInfo component("test_dwarf_sbom", "test_dwarf_sbom");
    bool result = extractor.extractMetadata(component);
    
    if (result) {
        std::cout << "✓ Successfully extracted metadata" << std::endl;
        std::cout << "  - File: " << component.filePath << std::endl;
        std::cout << "  - Size: " << component.fileSize << " bytes" << std::endl;
        std::cout << "  - Contains debug info: " << (component.containsDebugInfo ? "Yes" : "No") << std::endl;
        
        if (component.containsDebugInfo) {
            std::cout << "  - Source files: " << component.sourceFiles.size() << std::endl;
            for (const auto& source : component.sourceFiles) {
                std::cout << "    * " << source << std::endl;
            }
            
            std::cout << "  - Functions: " << component.functions.size() << std::endl;
            for (const auto& func : component.functions) {
                std::cout << "    * " << func << std::endl;
            }
            
            std::cout << "  - Compile units: " << component.compileUnits.size() << std::endl;
            for (const auto& unit : component.compileUnits) {
                std::cout << "    * " << unit << std::endl;
            }
        }
        
        // Generate SBOM
        heimdall::SBOMGenerator sbomGenerator;
        sbomGenerator.processComponent(component);
        sbomGenerator.setOutputPath("test_dwarf_sbom.spdx");
        sbomGenerator.setFormat("spdx");
        sbomGenerator.generateSBOM();
        
        std::cout << "\n✓ Generated SPDX SBOM: test_dwarf_sbom.spdx" << std::endl;
        
        // Also generate CycloneDX
        sbomGenerator.setOutputPath("test_dwarf_sbom.cyclonedx.json");
        sbomGenerator.setFormat("cyclonedx");
        sbomGenerator.generateSBOM();
        
        std::cout << "✓ Generated CycloneDX SBOM: test_dwarf_sbom.cyclonedx.json" << std::endl;
        
        // Show statistics
        sbomGenerator.printStatistics();
        
    } else {
        std::cout << "✗ Failed to extract metadata" << std::endl;
        return 1;
    }
    
    return 0;
} 