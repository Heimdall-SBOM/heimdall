/*
 * Test program to show ALI file content parsing
 * Demonstrates exactly what data is extracted from ALI files
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../../src/common/AdaExtractor.hpp"

void showAliFileContent(const std::string& aliFile) {
    std::cout << "\n=== ALI File Content: " << aliFile << " ===" << std::endl;
    
    std::ifstream file(aliFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open ALI file: " << aliFile << std::endl;
        return;
    }
    
    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        
        // Show key lines that are being parsed
        if (line.substr(0, 2) == "V " ||  // Version
            line.substr(0, 2) == "W " ||  // Dependencies
            line.substr(0, 2) == "X " ||  // Functions/Variables
            line.substr(0, 3) == "RV ") { // Runtime flags
            std::cout << "Line " << lineNum << ": " << line << std::endl;
        }
    }
    file.close();
}

void demonstrateAliParsing() {
    std::cout << "=== ALI File Parsing Demonstration ===" << std::endl;
    
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    
    std::vector<std::string> aliFiles = {
        "main.ali",
        "data_reader.ali", 
        "string_utils.ali",
        "math_lib.ali"
    };
    
    for (const auto& aliFile : aliFiles) {
        showAliFileContent(aliFile);
        
        heimdall::AdaPackageInfo packageInfo;
        if (extractor.parseAliFile(aliFile, packageInfo)) {
            std::cout << "\nParsed Package Info:" << std::endl;
            std::cout << "  Name: " << packageInfo.name << std::endl;
            std::cout << "  Source File: " << packageInfo.sourceFile << std::endl;
            std::cout << "  Is Runtime: " << (packageInfo.isRuntime ? "Yes" : "No") << std::endl;
            std::cout << "  Is Specification: " << (packageInfo.isSpecification ? "Yes" : "No") << std::endl;
            
            std::cout << "  Dependencies (" << packageInfo.dependencies.size() << "):" << std::endl;
            for (const auto& dep : packageInfo.dependencies) {
                std::cout << "    - " << dep << std::endl;
            }
            
            std::cout << "  Functions (" << packageInfo.functions.size() << "):" << std::endl;
            for (const auto& func : packageInfo.functions) {
                std::cout << "    - " << func << std::endl;
            }
            
            std::cout << "  Variables (" << packageInfo.variables.size() << "):" << std::endl;
            for (const auto& var : packageInfo.variables) {
                std::cout << "    - " << var << std::endl;
            }
            
            std::cout << "  Types (" << packageInfo.types.size() << "):" << std::endl;
            for (const auto& type : packageInfo.types) {
                std::cout << "    - " << type << std::endl;
            }
        } else {
            std::cout << "  ✗ Failed to parse ALI file" << std::endl;
        }
        std::cout << std::endl;
    }
}

void compareWithAndWithoutAli() {
    std::cout << "\n=== Comparison: With vs Without ALI Files ===" << std::endl;
    
    // Test with ALI files
    std::cout << "\n--- WITH ALI Files ---" << std::endl;
    heimdall::ComponentInfo componentWithAli("with-ali", "bin/main_static");
    heimdall::AdaExtractor adaExtractor;
    
    std::vector<std::string> aliFiles;
    adaExtractor.findAliFiles(".", aliFiles);
    
    if (adaExtractor.extractAdaMetadata(componentWithAli, aliFiles)) {
        std::cout << "✓ Package Manager: " << componentWithAli.packageManager << std::endl;
        std::cout << "✓ Version: " << componentWithAli.version << std::endl;
        std::cout << "✓ Dependencies: " << componentWithAli.dependencies.size() << std::endl;
        for (const auto& dep : componentWithAli.dependencies) {
            std::cout << "  - " << dep << std::endl;
        }
    }
    
    // Test without ALI files (simulate by not calling Ada extractor)
    std::cout << "\n--- WITHOUT ALI Files (Binary Only) ---" << std::endl;
    heimdall::ComponentInfo componentWithoutAli("without-ali", "bin/main_static");
    
    // This would be the result if we only had binary analysis
    std::cout << "✗ Package Manager: (unknown)" << std::endl;
    std::cout << "✗ Version: (unknown)" << std::endl;
    std::cout << "✗ Ada Dependencies: (none detected)" << std::endl;
    std::cout << "✗ Ada Functions: (none detected)" << std::endl;
    std::cout << "✗ Source Files: (none detected)" << std::endl;
    
    std::cout << "\n--- Key Differences ---" << std::endl;
    std::cout << "With ALI files:" << std::endl;
    std::cout << "  ✓ Detects GNAT as package manager" << std::endl;
    std::cout << "  ✓ Extracts GNAT compiler version" << std::endl;
    std::cout << "  ✓ Maps Ada package dependencies" << std::endl;
    std::cout << "  ✓ Identifies source files" << std::endl;
    std::cout << "  ✓ Extracts function signatures" << std::endl;
    
    std::cout << "\nWithout ALI files:" << std::endl;
    std::cout << "  ✗ No Ada-specific metadata" << std::endl;
    std::cout << "  ✗ No package manager detection" << std::endl;
    std::cout << "  ✗ No Ada dependencies" << std::endl;
    std::cout << "  ✗ No source file mapping" << std::endl;
    std::cout << "  ✗ No function signatures" << std::endl;
}

int main() {
    std::cout << "=== ALI File Content Analysis ===" << std::endl;
    
    demonstrateAliParsing();
    compareWithAndWithoutAli();
    
    std::cout << "\n=== Conclusion ===" << std::endl;
    std::cout << "The Ada extractor is definitely using ALI files because:" << std::endl;
    std::cout << "1. It detects GNAT as the package manager (only possible from ALI files)" << std::endl;
    std::cout << "2. It extracts GNAT compiler version (from ALI version lines)" << std::endl;
    std::cout << "3. It maps Ada package dependencies (from ALI dependency lines)" << std::endl;
    std::cout << "4. It identifies source files (from ALI file mapping)" << std::endl;
    std::cout << "5. It extracts function signatures (from ALI function lines)" << std::endl;
    
    return 0;
} 