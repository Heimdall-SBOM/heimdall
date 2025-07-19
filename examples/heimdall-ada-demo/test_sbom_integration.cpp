/*
 * Test program to demonstrate SBOM integration with Ada metadata
 */

#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/MetadataExtractor.hpp"
#include "../../src/common/ComponentInfo.hpp"

void printComponentDetails(const heimdall::ComponentInfo& component, const std::string& title) {
    std::cout << "\n=== " << title << " ===" << std::endl;
    std::cout << "Name: " << component.name << std::endl;
    std::cout << "Package Manager: " << component.packageManager << std::endl;
    std::cout << "Version: " << component.version << std::endl;
    std::cout << "File Type: " << (int)component.fileType << std::endl;
    
    std::cout << "\nDependencies (" << component.dependencies.size() << "):" << std::endl;
    for (const auto& dep : component.dependencies) {
        std::cout << "  - " << dep << std::endl;
    }
    
    std::cout << "\nSource Files (" << component.sourceFiles.size() << "):" << std::endl;
    for (const auto& src : component.sourceFiles) {
        std::cout << "  - " << src << std::endl;
    }
    
    std::cout << "\nFunctions (" << component.functions.size() << "):" << std::endl;
    for (const auto& func : component.functions) {
        std::cout << "  - " << func << std::endl;
    }
    
    std::cout << "\nSymbols (" << component.symbols.size() << "):" << std::endl;
    for (const auto& symbol : component.symbols) {
        std::cout << "  - " << symbol.name << " (size: " << symbol.size << ")" << std::endl;
    }
}

int main() {
    std::cout << "=== SBOM Integration Test ===" << std::endl;
    
    // Test 1: Extract metadata with Ada integration
    std::cout << "\n--- Test 1: Full Metadata Extraction with Ada ---" << std::endl;
    
    heimdall::MetadataExtractor extractor;
    extractor.setVerbose(true);
    
    heimdall::ComponentInfo component("heimdall-ada-demo", "bin/main_static");
    
    if (extractor.extractMetadata(component)) {
        std::cout << "✓ Successfully extracted metadata" << std::endl;
        printComponentDetails(component, "Component with Ada Integration");
    } else {
        std::cout << "✗ Failed to extract metadata" << std::endl;
    }
    
    // Test 2: Direct Ada extraction
    std::cout << "\n--- Test 2: Direct Ada Extraction ---" << std::endl;
    
    heimdall::ComponentInfo adaComponent("direct-ada-test", "bin/main_static");
    
    std::vector<std::string> aliFiles;
    if (extractor.findAdaAliFiles(".", aliFiles)) {
        std::cout << "Found " << aliFiles.size() << " ALI files:" << std::endl;
        for (const auto& aliFile : aliFiles) {
            std::cout << "  - " << aliFile << std::endl;
        }
        
        if (extractor.extractAdaMetadata(adaComponent, aliFiles)) {
            std::cout << "✓ Successfully extracted Ada metadata" << std::endl;
            printComponentDetails(adaComponent, "Direct Ada Component");
        } else {
            std::cout << "✗ Failed to extract Ada metadata" << std::endl;
        }
    } else {
        std::cout << "✗ No ALI files found" << std::endl;
    }
    
    // Test 3: Compare with and without Ada
    std::cout << "\n--- Test 3: Comparison ---" << std::endl;
    
    std::cout << "With Ada integration:" << std::endl;
    std::cout << "  Package Manager: " << component.packageManager << std::endl;
    std::cout << "  Version: " << component.version << std::endl;
    std::cout << "  Source Files: " << component.sourceFiles.size() << std::endl;
    std::cout << "  Dependencies: " << component.dependencies.size() << std::endl;
    
    std::cout << "\nDirect Ada extraction:" << std::endl;
    std::cout << "  Package Manager: " << adaComponent.packageManager << std::endl;
    std::cout << "  Version: " << adaComponent.version << std::endl;
    std::cout << "  Source Files: " << adaComponent.sourceFiles.size() << std::endl;
    std::cout << "  Dependencies: " << adaComponent.dependencies.size() << std::endl;
    
    return 0;
} 