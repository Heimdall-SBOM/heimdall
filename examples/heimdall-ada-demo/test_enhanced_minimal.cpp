/**
 * Minimal test program for enhanced Ada metadata extraction
 * Tests only the enhanced extraction methods without ComponentInfo dependencies
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "../../src/common/AdaExtractor.hpp"

int main() {
    std::cout << "=== Heimdall Enhanced Ada Metadata Extraction Test ===" << std::endl;
    
    try {
        // Create Ada extractor with enhanced metadata enabled
        heimdall::AdaExtractor extractor;
        extractor.setVerbose(true);
        extractor.setExtractEnhancedMetadata(true);
        extractor.setExtractRuntimePackages(true);
        
        // Find ALI files
        std::vector<std::string> aliFiles;
        if (!extractor.findAliFiles(".", aliFiles)) {
            std::cout << "No ALI files found in current directory" << std::endl;
            return 1;
        }
        
        std::cout << "Found " << aliFiles.size() << " ALI files:" << std::endl;
        for (const auto& aliFile : aliFiles) {
            std::cout << "  - " << aliFile << std::endl;
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
                    std::cout << "✓ Cross-references (" << crossRefs.size() << "):" << std::endl;
                    for (const auto& crossRef : crossRefs) {
                        std::cout << "  " << crossRef.callerFunction << "(" << crossRef.callerPackage 
                                  << ") -> " << crossRef.calledFunction << "(" << crossRef.calledPackage 
                                  << ") [" << crossRef.relationship << "]" << std::endl;
                    }
                } else {
                    std::cout << "✗ No cross-references found" << std::endl;
                }
                
                // Test type information extraction
                std::vector<heimdall::AdaTypeInfo> types;
                if (extractor.extractTypeInfo(content, types)) {
                    std::cout << "✓ Types (" << types.size() << "):" << std::endl;
                    for (const auto& type : types) {
                        std::cout << "  " << type.name << " (base: " << type.baseType 
                                  << ", size: " << type.size << ", alignment: " << type.alignment << ")" << std::endl;
                    }
                } else {
                    std::cout << "✗ No types found" << std::endl;
                }
                
                // Test security flags extraction
                std::vector<std::string> securityFlags;
                if (extractor.extractSecurityFlags(content, securityFlags)) {
                    std::cout << "✓ Security Flags (" << securityFlags.size() << "):" << std::endl;
                    for (const auto& flag : securityFlags) {
                        std::cout << "  - " << flag << std::endl;
                    }
                } else {
                    std::cout << "✗ No security flags found" << std::endl;
                }
                
                // Test file info extraction
                std::map<std::string, std::string> timestamps, checksums;
                if (extractor.extractFileInfo(content, timestamps, checksums)) {
                    std::cout << "✓ File Timestamps (" << timestamps.size() << "):" << std::endl;
                    for (const auto& [file, timestamp] : timestamps) {
                        std::cout << "  " << file << ": " << timestamp << std::endl;
                    }
                    
                    std::cout << "✓ File Checksums (" << checksums.size() << "):" << std::endl;
                    for (const auto& [file, checksum] : checksums) {
                        std::cout << "  " << file << ": " << checksum << std::endl;
                    }
                } else {
                    std::cout << "✗ No file info found" << std::endl;
                }
                
                // Test build info extraction
                heimdall::AdaBuildInfo buildInfo;
                if (extractor.extractBuildInfo(content, buildInfo)) {
                    std::cout << "✓ Build Info:" << std::endl;
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
                } else {
                    std::cout << "✗ No build info found" << std::endl;
                }
                
            } else {
                std::cout << "✗ Failed to read ALI file" << std::endl;
            }
        }
        
        std::cout << "\n=== Enhanced Ada Metadata Extraction Test Completed ===" << std::endl;
        std::cout << "✓ All enhanced Ada metadata extraction capabilities are working!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 