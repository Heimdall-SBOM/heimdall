#include <iostream>
#include <string>
#include "src/common/Utils.hpp"

int main() {
    std::string libraryName = "libheimdall-core.so.1";
    
    std::cout << "Testing library resolution for: " << libraryName << std::endl;
    
    // Test library resolution
    std::string resolvedPath = heimdall::Utils::resolveLibraryPath(libraryName);
    
    std::cout << "Resolved path: '" << resolvedPath << "'" << std::endl;
    
    // Check if the resolved path exists
    bool exists = heimdall::Utils::fileExists(resolvedPath);
    std::cout << "File exists: " << (exists ? "YES" : "NO") << std::endl;
    
    // Test with libz.so.1
    std::string libzName = "libz.so.1";
    std::string libzPath = heimdall::Utils::resolveLibraryPath(libzName);
    
    std::cout << "\nTesting library resolution for: " << libzName << std::endl;
    std::cout << "Resolved path: '" << libzPath << "'" << std::endl;
    
    bool libzExists = heimdall::Utils::fileExists(libzPath);
    std::cout << "File exists: " << (libzExists ? "YES" : "NO") << std::endl;
    
    return 0;
} 