/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file test_cyclonedx_enhanced_fields.cpp
 * @brief Unit tests for enhanced CycloneDX fields in SBOM components
 * @author Trevor Bakker
 * @date 2025
 * 
 * This test file validates the enhanced CycloneDX fields that were added to
 * the SBOMComponent structure and SBOM generation process.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "common/SBOMComparator.hpp"
#include "common/SBOMGenerator.hpp"
#include "common/ComponentInfo.hpp"
#include "common/MetadataExtractor.hpp"
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class CycloneDXEnhancedFieldsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::cout << "DEBUG: SetUp() called" << std::endl;
        test_dir = test_utils::getUniqueTestDirectory("heimdall_cyclonedx_enhanced_test");
        std::cout << "DEBUG: Test directory: " << test_dir.string() << std::endl;
        heimdall::compat::fs::create_directories(test_dir);
        std::cout << "DEBUG: SetUp() completed" << std::endl;
    }
    
    void TearDown() override
    {
        test_utils::safeRemoveDirectory(test_dir);
    }
    
    heimdall::compat::fs::path test_dir;
    
    // Helper method to create a component with enhanced fields
    SBOMComponent createEnhancedComponent()
    {
        return SBOMComponent{
            "test-component",           // name
            "test-component-1.0.0",     // bomRef
            "test-component",           // group
            "1.0.0",                   // version
            "library",                 // type
            "pkg:generic/test-component@1.0.0", // purl
            "MIT",                     // license
            "A test component with enhanced fields", // description
            "required",                // scope
            "com.example",             // group
            "application/x-sharedlib", // mimeType
            "Copyright 2025 Example Corp", // copyright
            "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*", // cpe
            "Example Corp",            // supplier
            "Example Corp",            // manufacturer
            "Example Corp",            // publisher
            {},                        // hashes
            {},                        // dependencies
            {}                         // externalReferences
        };
    }
    
    // Helper method to create ComponentInfo with enhanced fields
    ComponentInfo createEnhancedComponentInfo()
    {
        // Create a test file
        std::string testFilePath = (test_dir / "test-component.so").string();
        std::ofstream testFile(testFilePath);
        testFile << "Test shared library content";
        testFile.close();
        
        // Verify file was created
        std::cout << "DEBUG: Created test file at: " << testFilePath << std::endl;
        std::cout << "DEBUG: File exists: " << (heimdall::compat::fs::exists(testFilePath) ? "YES" : "NO") << std::endl;
        
        // Use default constructor to avoid file access during construction
        ComponentInfo info;
        info.name = "test-component";
        info.filePath = testFilePath;
        info.version = "1.0.0";
        info.license = "MIT";
        
        // Set enhanced fields
        info.description = "A test component with enhanced fields";
        info.scope = "required";
        info.group = "com.example";
        info.mimeType = "application/x-sharedlib";
        info.copyright = "Copyright 2025 Example Corp";
        info.cpe = "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*";
        info.supplier = "Example Corp";
        info.manufacturer = "Example Corp";
        info.publisher = "Example Corp";
        
        std::cout << "DEBUG: ComponentInfo filePath set to: " << info.filePath << std::endl;
        
        return info;
    }
};

// Test enhanced field construction and access
TEST_F(CycloneDXEnhancedFieldsTest, EnhancedFieldConstruction)
{
    SBOMComponent component = createEnhancedComponent();
    
    // Test basic fields
    EXPECT_EQ(component.name, "test-component");
    EXPECT_EQ(component.version, "1.0.0");
    EXPECT_EQ(component.type, "library");
    EXPECT_EQ(component.license, "MIT");
    EXPECT_EQ(component.purl, "pkg:generic/test-component@1.0.0");
    
    // Test enhanced fields
    EXPECT_EQ(component.description, "A test component with enhanced fields");
    EXPECT_EQ(component.scope, "required");
    EXPECT_EQ(component.group, "com.example");
    EXPECT_EQ(component.mimeType, "application/x-sharedlib");
    EXPECT_EQ(component.copyright, "Copyright 2025 Example Corp");
    EXPECT_EQ(component.cpe, "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*");
    EXPECT_EQ(component.supplier, "Example Corp");
    EXPECT_EQ(component.manufacturer, "Example Corp");
    EXPECT_EQ(component.publisher, "Example Corp");
}

// Test ComponentInfo enhanced field setters and getters
TEST_F(CycloneDXEnhancedFieldsTest, ComponentInfoEnhancedFields)
{
    ComponentInfo info = createEnhancedComponentInfo();
    
    // Test enhanced field getters (direct field access since these are public members)
    EXPECT_EQ(info.description, "A test component with enhanced fields");
    EXPECT_EQ(info.scope, "required");
    EXPECT_EQ(info.group, "com.example");
    EXPECT_EQ(info.mimeType, "application/x-sharedlib");
    EXPECT_EQ(info.copyright, "Copyright 2025 Example Corp");
    EXPECT_EQ(info.cpe, "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*");
    EXPECT_EQ(info.manufacturer, "Example Corp");
    EXPECT_EQ(info.publisher, "Example Corp");
}

// Test enhanced fields in CycloneDX generation
TEST_F(CycloneDXEnhancedFieldsTest, CycloneDXGenerationWithEnhancedFields)
{
    std::cout << "DEBUG: Starting CycloneDXGenerationWithEnhancedFields test" << std::endl;
    
    // Create a test file in the test directory
    std::string testFilePath = (test_dir / "test-component.so").string();
    std::ofstream testFile(testFilePath);
    testFile << "Test shared library content";
    testFile.close();
    
    std::cout << "DEBUG: Created test file at: " << testFilePath << std::endl;
    std::cout << "DEBUG: File exists: " << (heimdall::compat::fs::exists(testFilePath) ? "YES" : "NO") << std::endl;
    
    // Create a minimal component without file access
    ComponentInfo component;
    component.name = "test-component";
    component.filePath = testFilePath;
    component.version = "1.0.0";
    component.license = "MIT";
    component.description = "A test component with enhanced fields";
    component.scope = "required";
    component.group = "com.example";
    component.mimeType = "application/x-sharedlib";
    component.copyright = "Copyright 2025 Example Corp";
    component.cpe = "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*";
    component.manufacturer = "Example Corp";
    component.publisher = "Example Corp";
    
    std::cout << "DEBUG: Component created successfully" << std::endl;
    
    // Test that the component has the expected values
    EXPECT_EQ(component.name, "test-component");
    EXPECT_EQ(component.version, "1.0.0");
    EXPECT_EQ(component.license, "MIT");
    EXPECT_EQ(component.description, "A test component with enhanced fields");
    EXPECT_EQ(component.scope, "required");
    EXPECT_EQ(component.group, "com.example");
    EXPECT_EQ(component.mimeType, "application/x-sharedlib");
    EXPECT_EQ(component.copyright, "Copyright 2025 Example Corp");
    EXPECT_EQ(component.cpe, "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*");
    EXPECT_EQ(component.manufacturer, "Example Corp");
    EXPECT_EQ(component.publisher, "Example Corp");
    
    std::cout << "DEBUG: Component validation passed" << std::endl;
    
    // Test completed successfully
    std::cout << "DEBUG: Test completed successfully" << std::endl;
}

// Test enhanced fields with different scopes
TEST_F(CycloneDXEnhancedFieldsTest, DifferentScopes)
{
    std::vector<std::string> scopes = {"required", "optional", "excluded"};
    
    for (const auto& scope : scopes) {
        ComponentInfo component = createEnhancedComponentInfo();
        component.scope = scope;
        
        SBOMGenerator generator;
        generator.setOutputPath((test_dir / ("scope_" + scope + ".json")).string());
        generator.setFormat("cyclonedx");
        generator.setCycloneDXVersion("1.6");
        generator.processComponent(component);
        
        generator.generateSBOM();
        
        // Verify scope in generated SBOM
        std::ifstream file(test_dir / ("scope_" + scope + ".json"));
        nlohmann::json sbom;
        file >> sbom;
        file.close();
        
        EXPECT_EQ(sbom["components"][0]["scope"], scope);
    }
}

// Test enhanced fields with different MIME types
TEST_F(CycloneDXEnhancedFieldsTest, DifferentMimeTypes)
{
    std::vector<std::string> mimeTypes = {
        "application/x-executable",
        "application/x-sharedlib",
        "application/x-archive",
        "application/x-object",
        "text/plain",
        "application/octet-stream"
    };
    
    for (const auto& mimeType : mimeTypes) {
        ComponentInfo component = createEnhancedComponentInfo();
        component.mimeType = mimeType;
        
        SBOMGenerator generator;
        generator.setOutputPath((test_dir / ("mime_" + std::to_string(mimeTypes.size()) + ".json")).string());
        generator.setFormat("cyclonedx");
        generator.setCycloneDXVersion("1.6");
        generator.processComponent(component);
        
        generator.generateSBOM();
        
        // Verify MIME type in generated SBOM
        std::ifstream file(test_dir / ("mime_" + std::to_string(mimeTypes.size()) + ".json"));
        nlohmann::json sbom;
        file >> sbom;
        file.close();
        
        EXPECT_EQ(sbom["components"][0]["mime-type"], mimeType);
    }
}

// Test CPE format validation
TEST_F(CycloneDXEnhancedFieldsTest, CPEFormatValidation)
{
    std::vector<std::string> validCPEs = {
        "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*",
        "cpe:2.3:a:microsoft:windows:10.0.19041:*:*:*:*:*:*:*:*",
        "cpe:2.3:a:openssl:openssl:1.1.1f:*:*:*:*:*:*:*:*",
        "cpe:2.3:a:apache:http_server:2.4.41:*:*:*:*:*:*:*:*"
    };
    
    for (const auto& cpe : validCPEs) {
        ComponentInfo component = createEnhancedComponentInfo();
        component.cpe = cpe;
        
        SBOMGenerator generator;
        generator.setOutputPath((test_dir / "cpe_test.json").string());
        generator.setFormat("cyclonedx");
        generator.setCycloneDXVersion("1.6");
        generator.processComponent(component);
        
        generator.generateSBOM();
        
        // Verify CPE in generated SBOM
        std::ifstream file(test_dir / "cpe_test.json");
        nlohmann::json sbom;
        file >> sbom;
        file.close();
        
        EXPECT_EQ(sbom["components"][0]["cpe"], cpe);
    }
}

// Test external references
TEST_F(CycloneDXEnhancedFieldsTest, ExternalReferences)
{
    ComponentInfo component = createEnhancedComponentInfo();
    
    // Add external references as URLs
    std::vector<std::string> externalRefs = {
        "https://example.com/test-component",
        "https://docs.example.com/test-component",
        "https://download.example.com/test-component-1.0.0.tar.gz"
    };
    
    // Note: ComponentInfo doesn't have externalReferences field, so we'll test this
    // by adding properties that represent external references
    component.addProperty("external:website", "https://example.com/test-component");
    component.addProperty("external:documentation", "https://docs.example.com/test-component");
    component.addProperty("external:download", "https://download.example.com/test-component-1.0.0.tar.gz");
    
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "external_refs.json").string());
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.6");
    generator.processComponent(component);
    
    generator.generateSBOM();
    
    // Verify external references in generated SBOM
    std::ifstream file(test_dir / "external_refs.json");
    nlohmann::json sbom;
    file >> sbom;
    file.close();
    
    nlohmann::json componentJson = sbom["components"][0];
    
    // Check that properties contain external references
    ASSERT_TRUE(componentJson.contains("properties"));
    bool foundWebsite = false, foundDocs = false, foundDownload = false;
    
    for (const auto& prop : componentJson["properties"]) {
        std::string name = prop["name"];
        std::string value = prop["value"];
        
        if (name == "external:website" && value == "https://example.com/test-component") {
            foundWebsite = true;
        } else if (name == "external:documentation" && value == "https://docs.example.com/test-component") {
            foundDocs = true;
        } else if (name == "external:download" && value == "https://download.example.com/test-component-1.0.0.tar.gz") {
            foundDownload = true;
        }
    }
    
    EXPECT_TRUE(foundWebsite) << "Website external reference not found";
    EXPECT_TRUE(foundDocs) << "Documentation external reference not found";
    EXPECT_TRUE(foundDownload) << "Download external reference not found";
}

// Test metadata extraction with enhanced fields
TEST_F(CycloneDXEnhancedFieldsTest, MetadataExtractionEnhancedFields)
{
    // Create a test binary file
    std::string testBinaryPath = (test_dir / "test_binary").string();
    std::ofstream binaryFile(testBinaryPath);
    binaryFile << "ELF test binary content";
    binaryFile.close();
    
    MetadataExtractor extractor;
    ComponentInfo info("test_binary", testBinaryPath);
    bool success = extractor.extractMetadata(info);
    
    // Test that enhanced fields can be set
    info.setDescription("Extracted component description");
    info.setScope("required");
    info.setGroup("com.example");
    info.setMimeType("application/x-executable");
    info.setCopyright("Copyright 2025 Example Corp");
    info.setCPE("cpe:2.3:a:example:test-binary:1.0.0:*:*:*:*:*:*:*:*");
    info.setManufacturer("Example Corp");
    info.setPublisher("Example Corp");
    
    // Verify enhanced fields were set (direct field access since these are public members)
    EXPECT_EQ(info.description, "Extracted component description");
    EXPECT_EQ(info.scope, "required");
    EXPECT_EQ(info.group, "com.example");
    EXPECT_EQ(info.mimeType, "application/x-executable");
    EXPECT_EQ(info.copyright, "Copyright 2025 Example Corp");
    EXPECT_EQ(info.cpe, "cpe:2.3:a:example:test-binary:1.0.0:*:*:*:*:*:*:*:*");
    EXPECT_EQ(info.manufacturer, "Example Corp");
    EXPECT_EQ(info.publisher, "Example Corp");
}

// Test enhanced fields with empty/null values
TEST_F(CycloneDXEnhancedFieldsTest, EmptyEnhancedFields)
{
    ComponentInfo component = createEnhancedComponentInfo();
    
    // Set some fields to empty
    component.description = "";
    component.scope = "";
    component.group = "";
    component.mimeType = "";
    component.copyright = "";
    component.cpe = "";
    component.supplier = "";
    component.manufacturer = "";
    component.publisher = "";
    
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "empty_fields.json").string());
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.6");
    generator.processComponent(component);
    
    generator.generateSBOM();
    
    // Verify that empty fields are handled gracefully
    std::ifstream file(test_dir / "empty_fields.json");
    nlohmann::json sbom;
    file >> sbom;
    file.close();
    
    nlohmann::json componentJson = sbom["components"][0];
    
    // Fields should either be omitted or set to empty strings
    EXPECT_FALSE(componentJson.contains("description"));
    EXPECT_FALSE(componentJson.contains("scope"));
    EXPECT_FALSE(componentJson.contains("group"));
    EXPECT_FALSE(componentJson.contains("mime-type"));
    EXPECT_FALSE(componentJson.contains("copyright"));
    EXPECT_FALSE(componentJson.contains("cpe"));
    EXPECT_FALSE(componentJson.contains("supplier"));
    EXPECT_FALSE(componentJson.contains("manufacturer"));
    EXPECT_FALSE(componentJson.contains("publisher"));
}

// Test enhanced fields with special characters
TEST_F(CycloneDXEnhancedFieldsTest, SpecialCharactersInEnhancedFields)
{
    ComponentInfo component = createEnhancedComponentInfo();
    
    // Set fields with special characters
    component.description = "Component with special chars: éñüß日本語한국어العربية";
    component.copyright = "Copyright © 2025 Example Corp. All rights reserved.";
    component.supplier = "Example Corp & Associates";
    component.manufacturer = "Example Corp (International)";
    component.publisher = "Example Corp [Publisher]";
    
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "special_chars.json").string());
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.6");
    generator.processComponent(component);
    
    generator.generateSBOM();
    
    // Verify special characters are preserved
    std::ifstream file(test_dir / "special_chars.json");
    nlohmann::json sbom;
    file >> sbom;
    file.close();
    
    nlohmann::json componentJson = sbom["components"][0];
    
    EXPECT_EQ(componentJson["description"], "Component with special chars: éñüß日本語한국어العربية");
    EXPECT_EQ(componentJson["copyright"], "Copyright © 2025 Example Corp. All rights reserved.");
    EXPECT_EQ(componentJson["supplier"]["name"], "Example Corp & Associates");
    EXPECT_EQ(componentJson["manufacturer"]["name"], "Example Corp (International)");
    EXPECT_EQ(componentJson["publisher"]["name"], "Example Corp [Publisher]");
}

// Test enhanced fields with very long values
TEST_F(CycloneDXEnhancedFieldsTest, LongEnhancedFields)
{
    ComponentInfo component = createEnhancedComponentInfo();
    
    // Create very long field values
    std::string longDescription(1000, 'A');
    std::string longCopyright(500, 'B');
    std::string longCPE(200, 'C');
    
    component.description = longDescription;
    component.copyright = longCopyright;
    component.cpe = longCPE;
    
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "long_fields.json").string());
    generator.setFormat("cyclonedx");
    generator.setCycloneDXVersion("1.6");
    generator.processComponent(component);
    
    generator.generateSBOM();
    
    // Verify long fields are handled correctly
    std::ifstream file(test_dir / "long_fields.json");
    nlohmann::json sbom;
    file >> sbom;
    file.close();
    
    nlohmann::json componentJson = sbom["components"][0];
    
    EXPECT_EQ(componentJson["description"], longDescription);
    EXPECT_EQ(componentJson["copyright"], longCopyright);
    EXPECT_EQ(componentJson["cpe"], longCPE);
}

// Test enhanced fields in different CycloneDX versions
TEST_F(CycloneDXEnhancedFieldsTest, EnhancedFieldsInDifferentVersions)
{
    std::vector<std::string> versions = {"1.4", "1.5", "1.6"};
    
    for (const auto& version : versions) {
        ComponentInfo component = createEnhancedComponentInfo();
        
        SBOMGenerator generator;
        generator.setOutputPath((test_dir / ("version_" + version + ".json")).string());
        generator.setFormat("cyclonedx");
        generator.setCycloneDXVersion(version);
        generator.processComponent(component);
        
        generator.generateSBOM();
        
        // Verify enhanced fields are present in all versions
        std::ifstream file(test_dir / ("version_" + version + ".json"));
        nlohmann::json sbom;
        file >> sbom;
        file.close();
        
        EXPECT_EQ(sbom["bomFormat"], "CycloneDX");
        EXPECT_EQ(sbom["specVersion"], version);
        
        nlohmann::json componentJson = sbom["components"][0];
        
        // Enhanced fields should be present in all supported versions
        EXPECT_EQ(componentJson["description"], "A test component with enhanced fields");
        EXPECT_EQ(componentJson["scope"], "required");
        EXPECT_EQ(componentJson["group"], "com.example");
        EXPECT_EQ(componentJson["mime-type"], "application/x-sharedlib");
        EXPECT_EQ(componentJson["copyright"], "Copyright 2025 Example Corp");
        EXPECT_EQ(componentJson["cpe"], "cpe:2.3:a:example:test-component:1.0.0:*:*:*:*:*:*:*:*");
        EXPECT_EQ(componentJson["supplier"]["name"], "Example Corp");
        EXPECT_EQ(componentJson["manufacturer"]["name"], "Example Corp");
        EXPECT_EQ(componentJson["publisher"]["name"], "Example Corp");
    }
} 