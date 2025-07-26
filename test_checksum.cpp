#include <iostream>
#include <string>
#include "src/common/Utils.hpp"

int main() {
    std::string testFile = "/lib64/ld-linux-x86-64.so.2";
    
    std::cout << "Testing checksum calculation for: " << testFile << std::endl;
    
    if (heimdall::Utils::fileExists(testFile)) {
        std::cout << "File exists" << std::endl;
        
        std::string checksum = heimdall::Utils::getFileChecksum(testFile);
        std::cout << "Checksum: '" << checksum << "'" << std::endl;
        std::cout << "Checksum length: " << checksum.length() << std::endl;
        std::cout << "Checksum empty: " << (checksum.empty() ? "true" : "false") << std::endl;
    } else {
        std::cout << "File does not exist" << std::endl;
    }
    
    return 0;
}
