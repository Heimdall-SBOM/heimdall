#include <iostream>
#include <string>
#include "src/common/ComponentInfo.hpp"
#include "src/common/Utils.hpp"

int main() {
    std::vector<std::string> testFiles = {
        "/lib64/ld-linux-x86-64.so.2",
        "/lib64/libssl.so.3",
        "/lib64/libcrypto.so.3",
        "/lib64/libelf.so.1",
        "/lib64/libstdc++.so.6",
        "/lib64/libm.so.6",
        "/lib64/libgcc_s.so.1",
        "/lib64/libc.so.6"
    };
    
    for (const auto& testFile : testFiles) {
        std::cout << "\nTesting: " << testFile << std::endl;
        
        if (heimdall::Utils::fileExists(testFile)) {
            std::cout << "  File exists" << std::endl;
            
            // Create ComponentInfo (this should calculate checksum)
            heimdall::ComponentInfo component(heimdall::Utils::getFileName(testFile), testFile);
            
            std::cout << "  Component name: " << component.name << std::endl;
            std::cout << "  Component filePath: " << component.filePath << std::endl;
            std::cout << "  Component checksum: '" << component.checksum << "'" << std::endl;
            std::cout << "  Component checksum length: " << component.checksum.length() << std::endl;
            std::cout << "  Component checksum empty: " << (component.checksum.empty() ? "true" : "false") << std::endl;
            
            // Test direct checksum calculation
            std::string directChecksum = heimdall::Utils::getFileChecksum(testFile);
            std::cout << "  Direct checksum: '" << directChecksum << "'" << std::endl;
            std::cout << "  Direct checksum length: " << directChecksum.length() << std::endl;
            std::cout << "  Checksums match: " << (component.checksum == directChecksum ? "true" : "false") << std::endl;
        } else {
            std::cout << "  File does not exist" << std::endl;
        }
    }
    
    return 0;
} 