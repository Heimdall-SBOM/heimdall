#include <iostream>
#include <string>
#include "src/common/ComponentInfo.hpp"
#include "src/common/Utils.hpp"
#include "src/common/SBOMGenerator.hpp"

int main() {
    std::string testFile = "/lib64/ld-linux-x86-64.so.2";
    
    std::cout << "Testing SBOM generation with: " << testFile << std::endl;
    
    if (heimdall::Utils::fileExists(testFile)) {
        std::cout << "File exists" << std::endl;
        
        // Create ComponentInfo (this should calculate checksum)
        heimdall::ComponentInfo component(heimdall::Utils::getFileName(testFile), testFile);
        
        std::cout << "After ComponentInfo creation:" << std::endl;
        std::cout << "  Component checksum: '" << component.checksum << "'" << std::endl;
        std::cout << "  Component checksum length: " << component.checksum.length() << std::endl;
        std::cout << "  Component checksum empty: " << (component.checksum.empty() ? "true" : "false") << std::endl;
        
        // Create SBOMGenerator and process the component
        heimdall::SBOMGenerator sbomGenerator;
        sbomGenerator.processComponent(component);
        
        std::cout << "After SBOM processing:" << std::endl;
        std::cout << "  Component count: " << sbomGenerator.getComponentCount() << std::endl;
        
        // Try to get the component back from the SBOMGenerator
        // Note: This is a simplified test since we can't directly access the internal components
        // But we can check if the component was processed
        
        // Generate SPDX to see what happens
        sbomGenerator.setOutputPath("test_debug.spdx");
        sbomGenerator.setFormat("spdx");
        sbomGenerator.generateSBOM();
        
        std::cout << "SBOM generated. Check test_debug.spdx for results." << std::endl;
    } else {
        std::cout << "File does not exist" << std::endl;
    }
    
    return 0;
} 