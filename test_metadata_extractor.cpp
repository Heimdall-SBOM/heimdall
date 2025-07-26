#include <iostream>
#include <string>
#include "src/common/ComponentInfo.hpp"
#include "src/common/Utils.hpp"
#include "src/common/MetadataExtractor.hpp"

int main() {
    std::string testFile = "/lib64/ld-linux-x86-64.so.2";
    
    std::cout << "Testing MetadataExtractor with: " << testFile << std::endl;
    
    if (heimdall::Utils::fileExists(testFile)) {
        std::cout << "File exists" << std::endl;
        
        // Create ComponentInfo (this should calculate checksum)
        heimdall::ComponentInfo component(heimdall::Utils::getFileName(testFile), testFile);
        
        std::cout << "Before MetadataExtractor:" << std::endl;
        std::cout << "  Component checksum: '" << component.checksum << "'" << std::endl;
        std::cout << "  Component checksum length: " << component.checksum.length() << std::endl;
        std::cout << "  Component checksum empty: " << (component.checksum.empty() ? "true" : "false") << std::endl;
        
        // Test MetadataExtractor
        heimdall::MetadataExtractor extractor;
        bool result = extractor.extractMetadata(component);
        
        std::cout << "After MetadataExtractor:" << std::endl;
        std::cout << "  Extraction result: " << (result ? "success" : "failed") << std::endl;
        std::cout << "  Component checksum: '" << component.checksum << "'" << std::endl;
        std::cout << "  Component checksum length: " << component.checksum.length() << std::endl;
        std::cout << "  Component checksum empty: " << (component.checksum.empty() ? "true" : "false") << std::endl;
        
        // Test direct checksum calculation again
        std::string directChecksum = heimdall::Utils::getFileChecksum(testFile);
        std::cout << "  Direct checksum: '" << directChecksum << "'" << std::endl;
        std::cout << "  Checksums match: " << (component.checksum == directChecksum ? "true" : "false") << std::endl;
    } else {
        std::cout << "File does not exist" << std::endl;
    }
    
    return 0;
} 