/*
 * Test program to demonstrate Ada ALI file integration
 * Shows exactly what metadata is extracted from ALI files
 */

#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/AdaExtractor.hpp"
#include "../../src/common/ComponentInfo.hpp"
#include "../../src/common/MetadataExtractor.hpp"

void printComponentInfo(const heimdall::ComponentInfo& component, const std::string& title) {
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
    std::cout << "=== Ada ALI File Integration Test ===" << std::endl;
    
    // Test 1: Extract metadata WITH ALI files
    std::cout << "\n--- Test 1: With ALI Files ---" << std::endl;
    
    heimdall::MetadataExtractor extractor;
    extractor.setVerbose(true);
    
    // Find ALI files
    std::vector<std::string> aliFiles;
    if (extractor.findAdaAliFiles(".", aliFiles)) {
        std::cout << "Found " << aliFiles.size() << " ALI files:" << std::endl;
        for (const auto& aliFile : aliFiles) {
            std::cout << "  - " << aliFile << std::endl;
        }
        
        // Create component and extract metadata
        heimdall::ComponentInfo componentWithAli("heimdall-ada-demo-with-ali", "bin/main_static");
        
        // Extract Ada metadata
        if (extractor.extractAdaMetadata(componentWithAli, aliFiles)) {
            std::cout << "✓ Successfully extracted Ada metadata" << std::endl;
            printComponentInfo(componentWithAli, "Component WITH ALI Files");
        } else {
            std::cout << "✗ Failed to extract Ada metadata" << std::endl;
        }
    } else {
        std::cout << "✗ No ALI files found" << std::endl;
    }
    
    // Test 2: Extract metadata WITHOUT ALI files (binary only)
    std::cout << "\n--- Test 2: Without ALI Files (Binary Only) ---" << std::endl;
    
    heimdall::ComponentInfo componentWithoutAli("heimdall-ada-demo-without-ali", "bin/main_static");
    
    // Extract only binary metadata (no ALI files)
    if (extractor.extractMetadata(componentWithoutAli)) {
        std::cout << "✓ Successfully extracted binary metadata" << std::endl;
        printComponentInfo(componentWithoutAli, "Component WITHOUT ALI Files");
    } else {
        std::cout << "✗ Failed to extract binary metadata" << std::endl;
    }
    
    // Test 3: Direct Ada extractor test
    std::cout << "\n--- Test 3: Direct Ada Extractor ---" << std::endl;
    
    heimdall::AdaExtractor adaExtractor;
    adaExtractor.setVerbose(true);
    adaExtractor.setExtractRuntimePackages(true);
    
    heimdall::ComponentInfo directComponent("direct-ada-test", "bin/main_static");
    
    if (adaExtractor.extractAdaMetadata(directComponent, aliFiles)) {
        std::cout << "✓ Direct Ada extractor successful" << std::endl;
        printComponentInfo(directComponent, "Direct Ada Extractor Result");
    } else {
        std::cout << "✗ Direct Ada extractor failed" << std::endl;
    }
    
    // Test 4: Show ALI file content analysis
    std::cout << "\n--- Test 4: ALI File Content Analysis ---" << std::endl;
    
    for (const auto& aliFile : aliFiles) {
        std::cout << "\nAnalyzing: " << aliFile << std::endl;
        
        heimdall::AdaPackageInfo packageInfo;
        if (adaExtractor.parseAliFile(aliFile, packageInfo)) {
            std::cout << "  Package: " << packageInfo.name << std::endl;
            std::cout << "  Source: " << packageInfo.sourceFile << std::endl;
            std::cout << "  Is Runtime: " << (packageInfo.isRuntime ? "Yes" : "No") << std::endl;
            std::cout << "  Is Spec: " << (packageInfo.isSpecification ? "Yes" : "No") << std::endl;
            std::cout << "  Dependencies: " << packageInfo.dependencies.size() << std::endl;
            std::cout << "  Functions: " << packageInfo.functions.size() << std::endl;
            std::cout << "  Variables: " << packageInfo.variables.size() << std::endl;
            std::cout << "  Types: " << packageInfo.types.size() << std::endl;
        } else {
            std::cout << "  ✗ Failed to parse ALI file" << std::endl;
        }
    }
    
    std::cout << "\n=== Integration Test Complete ===" << std::endl;
    return 0;
} 