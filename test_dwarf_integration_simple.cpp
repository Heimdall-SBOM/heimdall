#include <iostream>
#include <fstream>
#include "src/common/MetadataExtractor.hpp"
#include "src/common/SBOMGenerator.hpp"

int main() {
    std::cout << "=== Heimdall DWARF Integration Test ===" << std::endl;
    
    // Test with our debug-enabled binary
    std::string testBinary = "test_dwarf_sbom";
    
    // Create metadata extractor
    heimdall::MetadataExtractor extractor;
    extractor.setExtractDebugInfo(true);
    extractor.setVerbose(true);
    
    // Process the binary
    heimdall::ComponentInfo component("test_binary", testBinary);
    bool result = extractor.extractMetadata(component);
    
    if (result) {
        std::cout << "✓ Successfully extracted metadata from " << testBinary << std::endl;
        std::cout << "  - File size: " << component.fileSize << " bytes" << std::endl;
        std::cout << "  - Contains debug info: " << (component.containsDebugInfo ? "Yes" : "No") << std::endl;
        std::cout << "  - Symbols: " << component.symbols.size() << std::endl;
        std::cout << "  - Sections: " << component.sections.size() << std::endl;
        
        if (component.containsDebugInfo) {
            std::cout << "\n=== DWARF Debug Information ===" << std::endl;
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
        
        // Generate enhanced SBOM
        heimdall::SBOMGenerator sbomGenerator;
        sbomGenerator.processComponent(component);
        
        // Generate SPDX with DWARF data
        sbomGenerator.setOutputPath("test_dwarf_enhanced.spdx");
        sbomGenerator.setFormat("spdx");
        sbomGenerator.generateSBOM();
        
        // Generate CycloneDX with DWARF data
        sbomGenerator.setOutputPath("test_dwarf_enhanced.cyclonedx.json");
        sbomGenerator.setFormat("cyclonedx");
        sbomGenerator.generateSBOM();
        
        std::cout << "\n✓ Generated enhanced SBOM files:" << std::endl;
        std::cout << "  - test_dwarf_enhanced.spdx" << std::endl;
        std::cout << "  - test_dwarf_enhanced.cyclonedx.json" << std::endl;
        
        // Show SBOM statistics
        sbomGenerator.printStatistics();
        
    } else {
        std::cout << "✗ Failed to extract metadata from " << testBinary << std::endl;
        return 1;
    }
    
    return 0;
} 