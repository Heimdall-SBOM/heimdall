#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "../src/common/SBOMComparator.hpp"
#include "../src/common/SBOMSigner.hpp"

using namespace heimdall;

int main() {
    std::cout << "=== BOM Reference and Dependencies Demo ===\n\n";

    // Create components with BOM references and enhanced fields
    SBOMComponent libA{"libA", "libA-1.0.0", "libA", "1.0.0", "library", "pkg:generic/libA@1.0.0", "MIT", 
                       "Core library providing basic functionality", "required", "com.example", 
                       "application/x-sharedlib", "Copyright 2025 Example Corp", "cpe:2.3:a:example:liba:1.0.0:*:*:*:*:*:*:*:*", 
                       "Example Corp", "Example Corp", "Example Corp", {}, {}, {}};
    
    SBOMComponent libB{"libB", "libB-2.0.0", "libB", "2.0.0", "library", "pkg:generic/libB@2.0.0", "Apache-2.0", 
                       "Advanced library with enhanced features", "required", "com.example", 
                       "application/x-sharedlib", "Copyright 2025 Example Corp", "cpe:2.3:a:example:libb:2.0.0:*:*:*:*:*:*:*:*", 
                       "Example Corp", "Example Corp", "Example Corp", {}, {}, {}};
    
    SBOMComponent app{"app", "app-1.0.0", "myapp", "1.0.0", "application", "pkg:generic/myapp@1.0.0", "GPL-3.0", 
                      "Main application executable", "required", "com.example", 
                      "application/x-executable", "Copyright 2025 Example Corp", "cpe:2.3:a:example:myapp:1.0.0:*:*:*:*:*:*:*:*", 
                      "Example Corp", "Example Corp", "Example Corp", {}, {}, {}};

    // Set up dependencies using BOM references
    libA.dependencies = {};  // libA has no dependencies
    libB.dependencies = {"libA-1.0.0"};  // libB depends on libA
    app.dependencies = {"libA-1.0.0", "libB-2.0.0"};  // app depends on both libA and libB

    std::vector<SBOMComponent> components = {libA, libB, app};

    // Display component information with enhanced fields
    std::cout << "Components:\n";
    for (const auto& comp : components) {
        std::cout << "  - " << comp.name << " v" << comp.version << " (BOM-Ref: " << comp.bomRef << ")\n";
        std::cout << "    Type: " << comp.type << ", Scope: " << comp.scope << "\n";
        std::cout << "    Description: " << comp.description << "\n";
        if (!comp.group.empty()) {
            std::cout << "    Group: " << comp.group << "\n";
        }
        if (!comp.mimeType.empty()) {
            std::cout << "    MIME Type: " << comp.mimeType << "\n";
        }
        if (!comp.copyright.empty()) {
            std::cout << "    Copyright: " << comp.copyright << "\n";
        }
        if (!comp.cpe.empty()) {
            std::cout << "    CPE: " << comp.cpe << "\n";
        }
        if (!comp.supplier.empty()) {
            std::cout << "    Supplier: " << comp.supplier << "\n";
        }
        if (!comp.dependencies.empty()) {
            std::cout << "    Dependencies: ";
            for (size_t i = 0; i < comp.dependencies.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << comp.dependencies[i];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    // Display dependency relationships
    std::cout << "Dependency Relationships:\n";
    for (const auto& comp : components) {
        if (!comp.dependencies.empty()) {
            std::cout << "  " << comp.bomRef << " depends on: ";
            for (size_t i = 0; i < comp.dependencies.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << comp.dependencies[i];
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";

    // Show how BOM references are used for dependency tracking
    std::cout << "BOM Reference Usage:\n";
    std::cout << "  - Each component has a unique BOM reference (bomRef field)\n";
    std::cout << "  - Dependencies are stored as BOM references, not component names\n";
    std::cout << "  - This allows for precise dependency tracking even with version changes\n";
    std::cout << "  - CycloneDX parsers can extract and validate these dependencies\n";
    std::cout << "  - SPDX parsers use SPDXID as the BOM reference\n\n";
    
    // Show enhanced CycloneDX fields
    std::cout << "Enhanced CycloneDX Fields:\n";
    std::cout << "  - description: Detailed component description\n";
    std::cout << "  - scope: Component scope (required/optional/excluded)\n";
    std::cout << "  - group: Component group/organization\n";
    std::cout << "  - mime-type: MIME type of the component\n";
    std::cout << "  - copyright: Copyright information\n";
    std::cout << "  - cpe: Common Platform Enumeration identifier\n";
    std::cout << "  - supplier: Organization that supplied the component\n";
    std::cout << "  - manufacturer: Organization that created the component\n";
    std::cout << "  - publisher: Organization that published the component\n";
    std::cout << "  - externalReferences: Additional reference URLs\n\n";
    
    // Show SBOM signing capabilities
    std::cout << "SBOM Signing Capabilities:\n";
    std::cout << "  - Support for RSA, ECDSA, and Ed25519 signatures\n";
    std::cout << "  - JSON Signature Format (JSF) for CycloneDX 1.6+\n";
    std::cout << "  - Certificate and key ID support\n";
    std::cout << "  - Command-line integration with heimdall-sbom\n";
    std::cout << "  - Example: heimdall-sbom plugin.so binary --format cyclonedx --output sbom.json --sign-key private.key --sign-algorithm RS256\n\n";
    
    // Test canonicalization functionality
    std::cout << "Canonicalization Test:\n";
    heimdall::SBOMSigner signer;
    
    // Create a test SBOM with signature fields
    nlohmann::json testSbom;
    testSbom["bomFormat"] = "CycloneDX";
    testSbom["specVersion"] = "1.6";
    testSbom["version"] = 1;
    testSbom["signature"] = {{"algorithm", "RS256"}, {"signature", "test-signature"}};
    
    testSbom["components"] = nlohmann::json::array();
    nlohmann::json component;
    component["bom-ref"] = "test-component";
    component["name"] = "test";
    component["version"] = "1.0.0";
    component["signature"] = {{"algorithm", "RS256"}, {"signature", "component-signature"}};
    testSbom["components"].push_back(component);
    
         // Test canonicalization with excludes tracking
     std::vector<std::string> excludes;
     std::string canonicalJson = signer.createCanonicalJSON(testSbom, excludes);
     if (signer.verifyCanonicalization(testSbom, canonicalJson)) {
         std::cout << "  ✅ Canonicalization working correctly - all signature fields excluded\n";
         std::cout << "  📋 Excluded fields: ";
         for (size_t i = 0; i < excludes.size(); ++i) {
             if (i > 0) std::cout << ", ";
             std::cout << excludes[i];
         }
         std::cout << "\n";
     } else {
         std::cout << "  ❌ Canonicalization failed: " << signer.getLastError() << "\n";
     }
    
    std::cout << "\n";

    return 0;
} 
