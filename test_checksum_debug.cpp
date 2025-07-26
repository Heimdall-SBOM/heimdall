#include <iostream>
#include <string>
#include "src/common/ComponentInfo.hpp"
#include "src/common/Utils.hpp"

int main() {
    std::string testFile = "/lib64/libc.so.6";
    std::cout << "Testing checksum calculation for: " << testFile << std::endl;
    
    if (heimdall::Utils::fileExists(testFile)) {
        std::cout << "File exists" << std::endl;
        
        // Test direct checksum calculation
        std::string directChecksum = heimdall::Utils::getFileChecksum(testFile);
        std::cout << "Direct checksum: '" << directChecksum << "'" << std::endl;
        
        // Test ComponentInfo constructor
        heimdall::ComponentInfo component(heimdall::Utils::getFileName(testFile), testFile);
        std::cout << "Component checksum: '" << component.checksum << "'" << std::endl;
        
        // Test if they match
        if (component.checksum == directChecksum) {
            std::cout << "SUCCESS: Checksums match!" << std::endl;
        } else {
            std::cout << "ERROR: Checksums don't match!" << std::endl;
        }
    } else {
        std::cout << "File does not exist" << std::endl;
    }
    
    return 0;
} 