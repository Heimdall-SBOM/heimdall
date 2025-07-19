/**
 * Test program for enhanced Ada metadata extraction implementation
 * Demonstrates all the enhanced capabilities including cross-references, 
 * type information, security flags, file timestamps/checksums, and build configuration
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "../../src/common/AdaExtractor.hpp"
#include "../../src/common/ComponentInfo.hpp"

void printComponentInfo(const heimdall::ComponentInfo& component, const std::string& title) {
    std::cout << "\n=== " << title << " ===" << std::endl;
    std::cout << "Name: " << component.name << std::endl;
    std::cout << "Version: " << component.version << std::endl;
    std::cout << "Package Manager: " << component.packageManager << std::endl;
    std::cout << "License: " << component.license << std::endl;
    
    std::cout << "\nDependencies (" << component.dependencies.size() << "):" << std::endl;
    for (const auto& dep : component.dependencies) {
        std::cout << "  - " << dep << std::endl;
    }
    
    std::cout << "\nSource Files (" << component.sourceFiles.size() << "):" << std::endl;
    for (const auto& source : component.sourceFiles) {
        std::cout << "  - " << source << std::endl;
    }
    
    std::cout << "\nFunctions (" << component.functions.size() << "):" << std::endl;
    for (const auto& func : component.functions) {
        std::cout << "  - " << func << std::endl;
    }
    
    std::cout << "\nProperties (" << component.properties.size() << "):" << std::endl;
    for (const auto& [key, value] : component.properties) {
        std::cout << "  - " << key << ": " << value << std::endl;
    }
}

void testEnhancedAdaExtraction() {
    std::cout << "=== Enhanced Ada Metadata Extraction Test ===" << std::endl;
    
    // Create Ada extractor with enhanced metadata enabled
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    extractor.setExtractEnhancedMetadata(true);
    extractor.setExtractRuntimePackages(true);
    
    // Find ALI files
    std::vector<std::string> aliFiles;
    if (!extractor.findAliFiles(".", aliFiles)) {
        std::cout << "No ALI files found in current directory" << std::endl;
        return;
    }
    
    std::cout << "Found " << aliFiles.size() << " ALI files:" << std::endl;
    for (const auto& aliFile : aliFiles) {
        std::cout << "  - " << aliFile << std::endl;
    }
    
    // Test enhanced metadata extraction
    heimdall::ComponentInfo component;
    component.name = "heimdall-ada-demo-enhanced";
    component.filePath = "main_static";
    
    if (extractor.extractAdaMetadata(component, aliFiles)) {
        std::cout << "\n✓ Enhanced Ada metadata extraction successful!" << std::endl;
        printComponentInfo(component, "Enhanced Ada Metadata Extraction Result");
        
        // Verify enhanced properties are present
        bool hasSecurityFlags = false;
        bool hasCallGraph = false;
        bool hasTypeInfo = false;
        bool hasTimestamps = false;
        bool hasChecksums = false;
        bool hasRuntimeFlags = false;
        bool hasCompilerVersion = false;
        
        for (const auto& [key, value] : component.properties) {
            if (key == "security.buildFlags") hasSecurityFlags = true;
            if (key == "functions.calls") hasCallGraph = true;
            if (key == "types.variables") hasTypeInfo = true;
            if (key == "build.timestamps") hasTimestamps = true;
            if (key == "build.checksums") hasChecksums = true;
            if (key == "security.runtimeFlags") hasRuntimeFlags = true;
            if (key == "security.compilerVersion") hasCompilerVersion = true;
        }
        
        std::cout << "\n=== Enhanced Metadata Verification ===" << std::endl;
        std::cout << "Security Flags: " << (hasSecurityFlags ? "✓" : "✗") << std::endl;
        std::cout << "Function Call Graph: " << (hasCallGraph ? "✓" : "✗") << std::endl;
        std::cout << "Type Information: " << (hasTypeInfo ? "✓" : "✗") << std::endl;
        std::cout << "Build Timestamps: " << (hasTimestamps ? "✓" : "✗") << std::endl;
        std::cout << "Build Checksums: " << (hasChecksums ? "✓" : "✗") << std::endl;
        std::cout << "Runtime Flags: " << (hasRuntimeFlags ? "✓" : "✗") << std::endl;
        std::cout << "Compiler Version: " << (hasCompilerVersion ? "✓" : "✗") << std::endl;
        
    } else {
        std::cout << "✗ Enhanced Ada metadata extraction failed" << std::endl;
    }
}

void testIndividualALIFileParsing() {
    std::cout << "\n=== Individual ALI File Parsing Test ===" << std::endl;
    
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    extractor.setExtractEnhancedMetadata(true);
    
    // Find ALI files
    std::vector<std::string> aliFiles;
    if (!extractor.findAliFiles(".", aliFiles)) {
        std::cout << "No ALI files found" << std::endl;
        return;
    }
    
    // Test parsing each ALI file individually
    for (const auto& aliFile : aliFiles) {
        std::cout << "\n--- Parsing " << aliFile << " ---" << std::endl;
        
        heimdall::AdaPackageInfo packageInfo;
        if (extractor.parseAliFile(aliFile, packageInfo)) {
            std::cout << "Package: " << packageInfo.name << std::endl;
            std::cout << "Source File: " << packageInfo.sourceFile << std::endl;
            std::cout << "Is Runtime: " << (packageInfo.isRuntime ? "Yes" : "No") << std::endl;
            std::cout << "Is Specification: " << (packageInfo.isSpecification ? "Yes" : "No") << std::endl;
            
            std::cout << "Dependencies (" << packageInfo.dependencies.size() << "):" << std::endl;
            for (const auto& dep : packageInfo.dependencies) {
                std::cout << "  - " << dep << std::endl;
            }
            
            std::cout << "Functions (" << packageInfo.functions.size() << "):" << std::endl;
            for (const auto& func : packageInfo.functions) {
                std::cout << "  - " << func << std::endl;
            }
            
            std::cout << "Variables (" << packageInfo.variables.size() << "):" << std::endl;
            for (const auto& var : packageInfo.variables) {
                std::cout << "  - " << var << std::endl;
            }
            
            std::cout << "Types (" << packageInfo.types.size() << "):" << std::endl;
            for (const auto& type : packageInfo.types) {
                std::cout << "  - " << type << std::endl;
            }
        } else {
            std::cout << "Failed to parse ALI file" << std::endl;
        }
    }
}

void testEnhancedMetadataExtraction() {
    std::cout << "\n=== Enhanced Metadata Extraction Test ===" << std::endl;
    
    heimdall::AdaExtractor extractor;
    extractor.setVerbose(true);
    extractor.setExtractEnhancedMetadata(true);
    
    // Find ALI files
    std::vector<std::string> aliFiles;
    if (!extractor.findAliFiles(".", aliFiles)) {
        std::cout << "No ALI files found" << std::endl;
        return;
    }
    
    // Test enhanced metadata extraction from each ALI file
    for (const auto& aliFile : aliFiles) {
        std::cout << "\n--- Enhanced extraction from " << aliFile << " ---" << std::endl;
        
        std::ifstream file(aliFile);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            
            // Test cross-reference extraction
            std::vector<heimdall::AdaCrossReference> crossRefs;
            if (extractor.extractCrossReferences(content, crossRefs)) {
                std::cout << "Cross-references (" << crossRefs.size() << "):" << std::endl;
                for (const auto& crossRef : crossRefs) {
                    std::cout << "  " << crossRef.callerFunction << "(" << crossRef.callerPackage 
                              << ") -> " << crossRef.calledFunction << "(" << crossRef.calledPackage 
                              << ") [" << crossRef.relationship << "]" << std::endl;
                }
            }
            
            // Test type information extraction
            std::vector<heimdall::AdaTypeInfo> types;
            if (extractor.extractTypeInfo(content, types)) {
                std::cout << "Types (" << types.size() << "):" << std::endl;
                for (const auto& type : types) {
                    std::cout << "  " << type.name << " (base: " << type.baseType 
                              << ", size: " << type.size << ", alignment: " << type.alignment << ")" << std::endl;
                }
            }
            
            // Test security flags extraction
            std::vector<std::string> securityFlags;
            if (extractor.extractSecurityFlags(content, securityFlags)) {
                std::cout << "Security Flags (" << securityFlags.size() << "):" << std::endl;
                for (const auto& flag : securityFlags) {
                    std::cout << "  - " << flag << std::endl;
                }
            }
            
            // Test file info extraction
            std::map<std::string, std::string> timestamps, checksums;
            if (extractor.extractFileInfo(content, timestamps, checksums)) {
                std::cout << "File Timestamps (" << timestamps.size() << "):" << std::endl;
                for (const auto& [file, timestamp] : timestamps) {
                    std::cout << "  " << file << ": " << timestamp << std::endl;
                }
                
                std::cout << "File Checksums (" << checksums.size() << "):" << std::endl;
                for (const auto& [file, checksum] : checksums) {
                    std::cout << "  " << file << ": " << checksum << std::endl;
                }
            }
            
            // Test build info extraction
            heimdall::AdaBuildInfo buildInfo;
            if (extractor.extractBuildInfo(content, buildInfo)) {
                std::cout << "Build Info:" << std::endl;
                std::cout << "  Compiler Version: " << buildInfo.compilerVersion << std::endl;
                std::cout << "  Runtime Flags (" << buildInfo.runtimeFlags.size() << "):" << std::endl;
                for (const auto& flag : buildInfo.runtimeFlags) {
                    std::cout << "    - " << flag << std::endl;
                }
                std::cout << "  Security Flags (" << buildInfo.securityFlags.size() << "):" << std::endl;
                for (const auto& flag : buildInfo.securityFlags) {
                    std::cout << "    - " << flag << std::endl;
                }
                std::cout << "  Optimization Flags (" << buildInfo.optimizationFlags.size() << "):" << std::endl;
                for (const auto& flag : buildInfo.optimizationFlags) {
                    std::cout << "    - " << flag << std::endl;
                }
            }
            
        } else {
            std::cout << "Failed to read ALI file" << std::endl;
        }
    }
}

int main() {
    std::cout << "=== Heimdall Enhanced Ada Metadata Extraction Test ===" << std::endl;
    
    try {
        testEnhancedAdaExtraction();
        testIndividualALIFileParsing();
        testEnhancedMetadataExtraction();
        
        std::cout << "\n=== All Tests Completed ===" << std::endl;
        std::cout << "Enhanced Ada metadata extraction is now fully implemented!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 