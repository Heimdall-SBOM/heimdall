#include "../common/SBOMValidator.hpp"
#include <iostream>

int main() {
    std::string spdx_content = R"(
SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
SPDXID: SPDXRef-DOCUMENT
DocumentName: Test Document
DocumentNamespace: https://spdx.org/spdxdocs/test
Creator: Organization: Test Org
Created: 2024-01-01T00:00:00Z

PackageName: test-package
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-test
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/test
)";

    auto validator = heimdall::SBOMValidatorFactory::createValidator("spdx");
    if (!validator) {
        std::cout << "Failed to create validator\n";
        return 1;
    }

    auto result = validator->validateContent(spdx_content);
    std::cout << "Valid: " << (result.isValid ? "true" : "false") << "\n";
    std::cout << "Format: " << result.metadata["format"] << "\n";
    std::cout << "Version: " << result.metadata["version"] << "\n";
    
    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  " << error << "\n";
        }
    }
    
    if (!result.warnings.empty()) {
        std::cout << "Warnings:\n";
        for (const auto& warning : result.warnings) {
            std::cout << "  " << warning << "\n";
        }
    }

    return 0;
} 