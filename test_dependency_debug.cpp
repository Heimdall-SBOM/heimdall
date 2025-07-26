#include <iostream>
#include <string>
#include <vector>
#include "src/common/MetadataExtractor.hpp"

int main() {
    std::string filePath = "lib/heimdall-lld.so";
    
    std::cout << "Testing dependency detection for: " << filePath << std::endl;
    
    // Test dependency detection
    std::vector<std::string> deps = heimdall::MetadataHelpers::detectDependencies(filePath);
    
    std::cout << "Detected " << deps.size() << " dependencies:" << std::endl;
    for (const auto& dep : deps) {
        std::cout << "  - " << dep << std::endl;
    }
    
    // Check if libz.so.1 is in the list
    bool found_libz = false;
    for (const auto& dep : deps) {
        if (dep.find("libz") != std::string::npos) {
            found_libz = true;
            std::cout << "Found libz dependency: " << dep << std::endl;
        }
    }
    
    if (!found_libz) {
        std::cout << "WARNING: libz.so.1 not found in detected dependencies!" << std::endl;
    }
    
    return 0;
} 