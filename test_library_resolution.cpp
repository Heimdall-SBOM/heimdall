#include <iostream>
#include <vector>
#include <string>
#include "src/common/Utils.hpp"

int main() {
    std::cout << "=== Testing Library Resolution ===" << std::endl;
    
    // Test library names that should be found on Ubuntu
    std::vector<std::string> testLibs = {
        "libssl.so.3",
        "libcrypto.so.3", 
        "libc.so.6",
        "libpthread.so.0",
        "libssl.so",
        "libcrypto.so",
        "libc.so",
        "libpthread.so"
    };
    
    for (const auto& lib : testLibs) {
        std::string resolved = heimdall::Utils::resolveLibraryPath(lib);
        if (!resolved.empty()) {
            std::cout << "✓ " << lib << " -> " << resolved << std::endl;
        } else {
            std::cout << "✗ " << lib << " -> NOT FOUND" << std::endl;
        }
    }
    
    return 0;
} 