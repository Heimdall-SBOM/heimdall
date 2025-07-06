#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "src/common/MetadataExtractor.hpp"

int main() {
    std::cout << "=== Heimdall Library Detection Debug ===" << std::endl;
    
    // Test with a simple binary that uses OpenSSL
    std::string testBinary = "/tmp/test_binary";
    
    // Check if test binary exists
    if (!std::filesystem::exists(testBinary)) {
        std::cout << "Test binary not found at " << testBinary << std::endl;
        return 1;
    }
    
    std::cout << "Test binary found: " << testBinary << std::endl;
    
    // Extract dependencies
    std::vector<std::string> deps = heimdall::MetadataHelpers::detectDependencies(testBinary);
    
    std::cout << "Found " << deps.size() << " dependencies:" << std::endl;
    for (const auto& dep : deps) {
        std::cout << "  - " << dep << std::endl;
    }
    
    // Try to resolve library paths
    std::vector<std::string> libPaths = {
        "/usr/lib", "/usr/local/lib", "/opt/local/lib", "/opt/homebrew/lib",
        "/lib", "/lib64", "/usr/lib64", "/usr/lib/x86_64-linux-gnu"
    };
    
    std::cout << "\n=== Library Path Resolution ===" << std::endl;
    for (const auto& dep : deps) {
        std::cout << "Resolving: " << dep << std::endl;
        
        // Check if it's an absolute path
        if (!dep.empty() && dep[0] == '/') {
            if (std::filesystem::exists(dep)) {
                std::cout << "  Found at: " << dep << std::endl;
            } else {
                std::cout << "  Not found at: " << dep << std::endl;
            }
        } else {
            // Search in library paths
            bool found = false;
            for (const auto& libDir : libPaths) {
                std::string candidate = libDir + "/" + dep;
                if (std::filesystem::exists(candidate)) {
                    std::cout << "  Found at: " << candidate << std::endl;
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "  Not found in any library path" << std::endl;
            }
        }
    }
    
    // Check what libraries are actually available on the system
    std::cout << "\n=== System Library Check ===" << std::endl;
    std::vector<std::string> expectedLibs = {"libssl.so", "libcrypto.so", "libc.so", "libpthread.so"};
    
    for (const auto& lib : expectedLibs) {
        std::cout << "Looking for: " << lib << std::endl;
        bool found = false;
        for (const auto& libDir : libPaths) {
            std::string candidate = libDir + "/" + lib;
            if (std::filesystem::exists(candidate)) {
                std::cout << "  Found at: " << candidate << std::endl;
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "  Not found" << std::endl;
        }
    }
    
    return 0;
} 