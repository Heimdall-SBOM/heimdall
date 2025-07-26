#include <iostream>
#include <string>
#include "src/common/ComponentInfo.hpp"

int main() {
    std::string filePath = "/usr/lib64/libz.so.1";
    
    std::cout << "Testing ComponentInfo constructor with path: " << filePath << std::endl;
    
    // Create component and see what happens
    heimdall::ComponentInfo component("libz.so.1", filePath);
    
    std::cout << "Component filePath: '" << component.filePath << "'" << std::endl;
    std::cout << "Component checksum: '" << component.checksum << "'" << std::endl;
    std::cout << "Checksum length: " << component.checksum.length() << std::endl;
    std::cout << "Checksum empty: " << (component.checksum.empty() ? "YES" : "NO") << std::endl;
    
    return 0;
} 