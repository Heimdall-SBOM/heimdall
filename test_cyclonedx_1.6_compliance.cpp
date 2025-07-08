#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "src/common/SBOMGenerator.hpp"
#include "src/common/ComponentInfo.hpp"

using namespace heimdall;

int main() {
    std::cout << "Testing CycloneDX 1.6 compliance fix..." << std::endl;
    
    // Create a temporary directory for test output
    std::filesystem::path testDir = std::filesystem::temp_directory_path() / "cyclonedx_test";
    std::filesystem::create_directories(testDir);
    
    try {
        // Create a test component
        ComponentInfo testComponent("test-library", "/usr/lib/libtest.so");
        testComponent.version = "1.2.3";
        testComponent.supplier = "Test Organization";
        testComponent.checksum = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        testComponent.containsDebugInfo = true;
        testComponent.functions.push_back("test_function");
        testComponent.sourceFiles.push_back("/src/test.c");
        
        // Test CycloneDX 1.6 generation
        SBOMGenerator generator;
        generator.setFormat("cyclonedx");
        generator.setCycloneDXVersion("1.6");
        generator.setOutputPath((testDir / "test.cyclonedx.json").string());
        generator.processComponent(testComponent);
        generator.generateSBOM();
        
        // Read and display the generated content
        std::ifstream file(testDir / "test.cyclonedx.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open generated SBOM file" << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        std::cout << "Generated CycloneDX 1.6 SBOM:" << std::endl;
        std::cout << content << std::endl;
        
        // Basic validation checks
        bool hasSchemaRef = content.find("\"$schema\"") != std::string::npos;
        bool hasSerialNumber = content.find("\"serialNumber\"") != std::string::npos;
        bool hasUrnUuid = content.find("urn:uuid:") != std::string::npos;
        bool hasToolsComponents = content.find("\"tools\": {") != std::string::npos && 
                                 content.find("\"components\":") != std::string::npos;
        bool hasSupplierObject = content.find("\"supplier\": {") != std::string::npos;
        bool hasLifecycles = content.find("\"lifecycles\"") != std::string::npos;
        bool hasEvidence = content.find("\"evidence\"") != std::string::npos;
        bool hasBomRef = content.find("\"bom-ref\"") != std::string::npos;
        
        std::cout << "\nValidation Results:" << std::endl;
        std::cout << "✓ Schema reference: " << (hasSchemaRef ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ Serial number: " << (hasSerialNumber ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ UUID format: " << (hasUrnUuid ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ Tools components: " << (hasToolsComponents ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ Supplier object: " << (hasSupplierObject ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ Lifecycles: " << (hasLifecycles ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ Evidence field: " << (hasEvidence ? "PASS" : "FAIL") << std::endl;
        std::cout << "✓ BOM references: " << (hasBomRef ? "PASS" : "FAIL") << std::endl;
        
        bool allPassed = hasSchemaRef && hasSerialNumber && hasUrnUuid && 
                        hasToolsComponents && hasSupplierObject && hasLifecycles &&
                        hasEvidence && hasBomRef;
        
        if (allPassed) {
            std::cout << "\n🎉 All CycloneDX 1.6 compliance checks PASSED!" << std::endl;
        } else {
            std::cout << "\n❌ Some CycloneDX 1.6 compliance checks FAILED!" << std::endl;
        }
        
        // Clean up
        std::filesystem::remove_all(testDir);
        
        return allPassed ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        std::filesystem::remove_all(testDir);
        return 1;
    }
}