#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <regex>
#include "SBOMGenerator.hpp"
#include "ComponentInfo.hpp"

using namespace heimdall;

class CycloneDXAllVersionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create some test components
        component1 = ComponentInfo("test-library", "/path/to/test-library.so");
        component1.version = "1.0.0";
        component1.fileType = FileType::SharedLibrary;
        component1.checksum = "abc123def456789";
        component1.supplier = "Test Supplier";
        component1.downloadLocation = "https://example.com/test-library";
        component1.homepage = "https://example.com";
        component1.license = "MIT";
        component1.containsDebugInfo = true;
        component1.functions = {"function1", "function2"};
        component1.sourceFiles = {"/src/test.cpp", "/src/lib.cpp"};
        
        component2 = ComponentInfo("another-lib", "/path/to/another-lib.a");
        component2.version = "2.1.0";
        component2.fileType = FileType::StaticLibrary;
        component2.checksum = "def456abc123789";
        component2.supplier = "Another Supplier";
        component2.downloadLocation = "https://example.com/another-lib";
        component2.homepage = "https://another-example.com";
        component2.license = "Apache-2.0";
    }

    ComponentInfo component1;
    ComponentInfo component2;
};

TEST_F(CycloneDXAllVersionsTest, TestCycloneDX13Generation) {
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.3");
    
    // Add components
    generator.processComponent(component1);
    generator.processComponent(component2);
    
    // Generate to string (using impl access via friendship)
    std::string output = generator.pImpl->generateCycloneDXDocument();
    
    // Verify basic structure
    EXPECT_TRUE(output.find("\"bomFormat\": \"CycloneDX\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"specVersion\": \"1.3\"") != std::string::npos);
    
    // CycloneDX 1.3 should NOT have $schema field
    EXPECT_TRUE(output.find("\"$schema\"") == std::string::npos);
    
    // Should have serialNumber for 1.3+
    EXPECT_TRUE(output.find("\"serialNumber\": \"urn:uuid:") != std::string::npos);
    
    // Should use simple tools array (not tools.components)
    EXPECT_TRUE(output.find("\"tools\": [") != std::string::npos);
    EXPECT_TRUE(output.find("\"vendor\": \"Heimdall Project\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"tools\": {") == std::string::npos);
    
    // Components should have version field (required in 1.3)
    EXPECT_TRUE(output.find("\"version\": \"1.0.0\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"version\": \"2.1.0\"") != std::string::npos);
    
    // Supplier should be string in 1.3 (not object)
    std::regex supplier_string_regex(R"("supplier":\s*"[^"]+")");
    EXPECT_TRUE(std::regex_search(output, supplier_string_regex));
    
    // Should NOT have evidence field (not available in 1.3)
    EXPECT_TRUE(output.find("\"evidence\"") == std::string::npos);
    
    // Should NOT have lifecycles (not available in 1.3)
    EXPECT_TRUE(output.find("\"lifecycles\"") == std::string::npos);
    
    std::cout << "CycloneDX 1.3 output sample:\n" << output.substr(0, 500) << "...\n";
}

TEST_F(CycloneDXAllVersionsTest, TestCycloneDX14Generation) {
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.4");
    
    // Add components
    generator.processComponent(component1);
    generator.processComponent(component2);
    
    // Generate to string
    std::string output = generator.pImpl->generateCycloneDXDocument();
    
    // Verify basic structure
    EXPECT_TRUE(output.find("\"bomFormat\": \"CycloneDX\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"specVersion\": \"1.4\"") != std::string::npos);
    
    // CycloneDX 1.4 SHOULD have $schema field
    EXPECT_TRUE(output.find("\"$schema\": \"http://cyclonedx.org/schema/bom-1.4.schema.json\"") != std::string::npos);
    
    // Should have serialNumber for 1.4+
    EXPECT_TRUE(output.find("\"serialNumber\": \"urn:uuid:") != std::string::npos);
    
    // Should use simple tools array (not tools.components)
    EXPECT_TRUE(output.find("\"tools\": [") != std::string::npos);
    EXPECT_TRUE(output.find("\"vendor\": \"Heimdall Project\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"tools\": {") == std::string::npos);
    
    // Components may have version field (optional in 1.4)
    EXPECT_TRUE(output.find("\"version\": \"1.0.0\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"version\": \"2.1.0\"") != std::string::npos);
    
    // Supplier should be object in 1.4+ (not string)
    std::regex supplier_object_regex(R"("supplier":\s*\{[^}]*"name":\s*"[^"]+")");
    EXPECT_TRUE(std::regex_search(output, supplier_object_regex));
    
    // Should NOT have evidence field (not available in 1.4)
    EXPECT_TRUE(output.find("\"evidence\"") == std::string::npos);
    
    // Should NOT have lifecycles (not available in 1.4)
    EXPECT_TRUE(output.find("\"lifecycles\"") == std::string::npos);
    
    std::cout << "CycloneDX 1.4 output sample:\n" << output.substr(0, 500) << "...\n";
}

TEST_F(CycloneDXAllVersionsTest, TestCycloneDX15Generation) {
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.5");
    
    // Add components
    generator.processComponent(component1);
    generator.processComponent(component2);
    
    // Generate to string
    std::string output = generator.pImpl->generateCycloneDXDocument();
    
    // Verify basic structure
    EXPECT_TRUE(output.find("\"bomFormat\": \"CycloneDX\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"specVersion\": \"1.5\"") != std::string::npos);
    
    // CycloneDX 1.5 SHOULD have $schema field
    EXPECT_TRUE(output.find("\"$schema\": \"http://cyclonedx.org/schema/bom-1.5.schema.json\"") != std::string::npos);
    
    // Should have serialNumber for 1.5+
    EXPECT_TRUE(output.find("\"serialNumber\": \"urn:uuid:") != std::string::npos);
    
    // Should use tools.components structure (1.5+)
    EXPECT_TRUE(output.find("\"tools\": {") != std::string::npos);
    EXPECT_TRUE(output.find("\"components\": [") != std::string::npos);
    EXPECT_TRUE(output.find("\"tools\": [") == std::string::npos);
    
    // Supplier should be object in 1.5+
    std::regex supplier_object_regex(R"("supplier":\s*\{[^}]*"name":\s*"[^"]+")");
    EXPECT_TRUE(std::regex_search(output, supplier_object_regex));
    
    // SHOULD have evidence field (available in 1.5+)
    EXPECT_TRUE(output.find("\"evidence\"") != std::string::npos);
    
    // SHOULD have lifecycles (available in 1.5+)
    EXPECT_TRUE(output.find("\"lifecycles\"") != std::string::npos);
    
    std::cout << "CycloneDX 1.5 output sample:\n" << output.substr(0, 500) << "...\n";
}

TEST_F(CycloneDXAllVersionsTest, TestCyclonDX16Generation) {
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.6");
    
    // Add components
    generator.processComponent(component1);
    generator.processComponent(component2);
    
    // Generate to string
    std::string output = generator.pImpl->generateCycloneDXDocument();
    
    // Verify basic structure
    EXPECT_TRUE(output.find("\"bomFormat\": \"CycloneDX\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"specVersion\": \"1.6\"") != std::string::npos);
    
    // CycloneDX 1.6 SHOULD have $schema field
    EXPECT_TRUE(output.find("\"$schema\": \"http://cyclonedx.org/schema/bom-1.6.schema.json\"") != std::string::npos);
    
    // Should have serialNumber for 1.6+
    EXPECT_TRUE(output.find("\"serialNumber\": \"urn:uuid:") != std::string::npos);
    
    // Should use tools.components structure (1.6+)
    EXPECT_TRUE(output.find("\"tools\": {") != std::string::npos);
    EXPECT_TRUE(output.find("\"components\": [") != std::string::npos);
    EXPECT_TRUE(output.find("\"tools\": [") == std::string::npos);
    
    // Supplier should be object in 1.6+
    std::regex supplier_object_regex(R"("supplier":\s*\{[^}]*"name":\s*"[^"]+")");
    EXPECT_TRUE(std::regex_search(output, supplier_object_regex));
    
    // SHOULD have evidence field (available in 1.6+)
    EXPECT_TRUE(output.find("\"evidence\"") != std::string::npos);
    
    // SHOULD have lifecycles (available in 1.6+)
    EXPECT_TRUE(output.find("\"lifecycles\"") != std::string::npos);
    
    std::cout << "CycloneDX 1.6 output sample:\n" << output.substr(0, 500) << "...\n";
}

TEST_F(CycloneDXAllVersionsTest, TestVersionSpecificEvidenceStructure) {
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    
    // Test 1.5 callstack structure (should have module field)
    generator.setCycloneDXVersion("1.5");
    generator.processComponent(component1);
    std::string output15 = generator.pImpl->generateCycloneDXDocument();
    
    if (output15.find("\"evidence\"") != std::string::npos) {
        // CycloneDX 1.5 callstack frames should have module field
        if (output15.find("\"callstack\"") != std::string::npos) {
            EXPECT_TRUE(output15.find("\"module\"") != std::string::npos);
        }
    }
    
    // Test 1.6 callstack structure (function field without requiring module)
    generator.setCycloneDXVersion("1.6");
    std::string output16 = generator.pImpl->generateCycloneDXDocument();
    
    if (output16.find("\"evidence\"") != std::string::npos) {
        // CycloneDX 1.6 callstack frames can have function field
        if (output16.find("\"callstack\"") != std::string::npos) {
            EXPECT_TRUE(output16.find("\"function\"") != std::string::npos);
        }
    }
}

TEST_F(CycloneDXAllVersionsTest, TestVersionCrossCompatibility) {
    // Test that changing versions produces different outputs
    SBOMGenerator generator;
    generator.setFormat("cyclonedx");
    generator.processComponent(component1);
    
    generator.setCycloneDXVersion("1.3");
    std::string output13 = generator.pImpl->generateCycloneDXDocument();
    
    generator.setCycloneDXVersion("1.4");
    std::string output14 = generator.pImpl->generateCycloneDXDocument();
    
    generator.setCycloneDXVersion("1.5");
    std::string output15 = generator.pImpl->generateCycloneDXDocument();
    
    generator.setCycloneDXVersion("1.6");
    std::string output16 = generator.pImpl->generateCycloneDXDocument();
    
    // Outputs should be different due to version-specific features
    EXPECT_NE(output13, output14);
    EXPECT_NE(output14, output15);
    EXPECT_NE(output15, output16);
    
    // Version differences:
    // 1.3: no $schema, string supplier, simple tools
    // 1.4: has $schema, object supplier, simple tools  
    // 1.5: has $schema, object supplier, tools.components, evidence, lifecycles
    // 1.6: has $schema, object supplier, tools.components, evidence, lifecycles
    
    EXPECT_TRUE(output13.find("\"$schema\"") == std::string::npos);
    EXPECT_TRUE(output14.find("\"$schema\"") != std::string::npos);
    EXPECT_TRUE(output15.find("\"$schema\"") != std::string::npos);
    EXPECT_TRUE(output16.find("\"$schema\"") != std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}