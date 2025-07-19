/*
 * Test program to show ALI file usage and data extraction
 */

#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/AdaExtractor.hpp"
#include "../../src/common/ComponentInfo.hpp"

void testIndividualAliFiles() {
    std::cout << "=== Testing Individual ALI Files ===" << std::endl;
    
    std::vector<std::string> aliFiles = {
        "main.ali",
        "data_reader.ali", 
        "string_utils.ali",
        "math_lib.ali"
    };
    
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    
    for (const auto& aliFile : aliFiles) {
        std::cout << "\n--- Testing: " << aliFile << " ---" << std::endl;
        
        heimdall::AdaPackageInfo packageInfo;
        if (extractor.parseAliFile(aliFile, packageInfo)) {
            std::cout << "✓ Successfully parsed " << aliFile << std::endl;
            std::cout << "  Package: " << packageInfo.name << std::endl;
            std::cout << "  Source File: " << packageInfo.sourceFile << std::endl;
            std::cout << "  Is Runtime: " << (packageInfo.isRuntime ? "Yes" : "No") << std::endl;
            std::cout << "  Dependencies: " << packageInfo.dependencies.size() << std::endl;
            for (const auto& dep : packageInfo.dependencies) {
                std::cout << "    - " << dep << std::endl;
            }
        } else {
            std::cout << "✗ Failed to parse " << aliFile << std::endl;
        }
    }
}

void testAllAliFilesTogether() {
    std::cout << "\n=== Testing All ALI Files Together ===" << std::endl;
    
    std::vector<std::string> aliFiles = {
        "main.ali",
        "data_reader.ali", 
        "string_utils.ali",
        "math_lib.ali"
    };
    
    heimdall::ComponentInfo component("test-all-ali", "bin/main_static");
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    
    if (extractor.extractAdaMetadata(component, aliFiles)) {
        std::cout << "✓ Successfully extracted metadata from all ALI files" << std::endl;
        std::cout << "  Package Manager: " << component.packageManager << std::endl;
        std::cout << "  Version: " << component.version << std::endl;
        std::cout << "  Dependencies: " << component.dependencies.size() << std::endl;
        std::cout << "  Source Files: " << component.sourceFiles.size() << std::endl;
        
        std::cout << "\nAll Dependencies:" << std::endl;
        for (const auto& dep : component.dependencies) {
            std::cout << "  - " << dep << std::endl;
        }
        
        std::cout << "\nAll Source Files:" << std::endl;
        for (const auto& src : component.sourceFiles) {
            std::cout << "  - " << src << std::endl;
        }
    } else {
        std::cout << "✗ Failed to extract metadata from ALI files" << std::endl;
    }
}

void testAliFileDiscovery() {
    std::cout << "\n=== Testing ALI File Discovery ===" << std::endl;
    
    heimdall::AdaExtractor extractor;
    std::vector<std::string> discoveredAliFiles;
    
    if (extractor.findAliFiles(".", discoveredAliFiles)) {
        std::cout << "✓ Found " << discoveredAliFiles.size() << " ALI files:" << std::endl;
        for (const auto& aliFile : discoveredAliFiles) {
            std::cout << "  - " << aliFile << std::endl;
        }
    } else {
        std::cout << "✗ Failed to discover ALI files" << std::endl;
    }
}

void compareWithAndWithoutSpecificAliFiles() {
    std::cout << "\n=== Comparing With/Without Specific ALI Files ===" << std::endl;
    
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    
    // Test with all ALI files
    std::vector<std::string> allAliFiles = {
        "main.ali",
        "data_reader.ali", 
        "string_utils.ali",
        "math_lib.ali"
    };
    
    heimdall::ComponentInfo componentAll("all-ali", "bin/main_static");
    if (extractor.extractAdaMetadata(componentAll, allAliFiles)) {
        std::cout << "With ALL ALI files:" << std::endl;
        std::cout << "  Dependencies: " << componentAll.dependencies.size() << std::endl;
        std::cout << "  Source Files: " << componentAll.sourceFiles.size() << std::endl;
    }
    
    // Test with only main.ali
    std::vector<std::string> mainOnly = {"main.ali"};
    heimdall::ComponentInfo componentMain("main-only", "bin/main_static");
    if (extractor.extractAdaMetadata(componentMain, mainOnly)) {
        std::cout << "\nWith ONLY main.ali:" << std::endl;
        std::cout << "  Dependencies: " << componentMain.dependencies.size() << std::endl;
        std::cout << "  Source Files: " << componentMain.sourceFiles.size() << std::endl;
    }
    
    // Test with only string_utils.ali
    std::vector<std::string> stringUtilsOnly = {"string_utils.ali"};
    heimdall::ComponentInfo componentStringUtils("string-utils-only", "bin/main_static");
    if (extractor.extractAdaMetadata(componentStringUtils, stringUtilsOnly)) {
        std::cout << "\nWith ONLY string_utils.ali:" << std::endl;
        std::cout << "  Dependencies: " << componentStringUtils.dependencies.size() << std::endl;
        std::cout << "  Source Files: " << componentStringUtils.sourceFiles.size() << std::endl;
    }
}

int main() {
    std::cout << "=== ALI File Usage Analysis ===" << std::endl;
    
    testIndividualAliFiles();
    testAllAliFilesTogether();
    testAliFileDiscovery();
    compareWithAndWithoutSpecificAliFiles();
    
    std::cout << "\n=== Conclusion ===" << std::endl;
    std::cout << "The Ada extractor processes ALL ALI files it finds and:" << std::endl;
    std::cout << "1. Extracts package information from each ALI file" << std::endl;
    std::cout << "2. Collects dependencies from all ALI files" << std::endl;
    std::cout << "3. Gathers source file names from all ALI files" << std::endl;
    std::cout << "4. Merges all the metadata into a single component" << std::endl;
    
    return 0;
} 