/*
 * Test program for Ada ALI file extractor
 * Demonstrates the Ada metadata extraction capabilities
 */

#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/AdaExtractor.hpp"
#include "../../src/common/ComponentInfo.hpp"

int main() {
    std::cout << "=== Heimdall Ada ALI Extractor Test ===" << std::endl;
    
    // Create Ada extractor
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    extractor.setExtractRuntimePackages(true);
    
    // Find ALI files in current directory
    std::vector<std::string> aliFiles;
    if (!extractor.findAliFiles(".", aliFiles)) {
        std::cerr << "Failed to find ALI files" << std::endl;
        return 1;
    }
    
    std::cout << "Found " << aliFiles.size() << " ALI files:" << std::endl;
    for (const auto& aliFile : aliFiles) {
        std::cout << "  - " << aliFile << std::endl;
    }
    
    // Create a component to store extracted metadata
    heimdall::ComponentInfo component("heimdall-ada-demo", "examples/heimdall-ada-demo/bin/main_static");
    
    // Extract Ada metadata
    if (!extractor.extractAdaMetadata(component, aliFiles)) {
        std::cerr << "Failed to extract Ada metadata" << std::endl;
        return 1;
    }
    
    // Display extracted metadata
    std::cout << "\n=== Extracted Ada Metadata ===" << std::endl;
    std::cout << "Component Name: " << component.name << std::endl;
    std::cout << "Package Manager: " << component.packageManager << std::endl;
    std::cout << "Version: " << component.version << std::endl;
    
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
    
    std::cout << "\nSections (" << component.sections.size() << "):" << std::endl;
    for (const auto& section : component.sections) {
        std::cout << "  - " << section.name << " (size: " << section.size << ")" << std::endl;
    }
    
    std::cout << "\n=== Test Completed Successfully ===" << std::endl;
    return 0;
} 