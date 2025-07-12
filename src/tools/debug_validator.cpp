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
 * @file debug_validator.cpp
 * @brief Debug tool for testing SBOM validation functionality
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides a simple debug tool for testing the SBOM validation
 * functionality. It includes a hardcoded SPDX test document and validates
 * it using the SBOMValidator framework.
 * 
 * The tool is useful for:
 * - Testing validation logic during development
 * - Debugging validation issues
 * - Verifying SBOM format parsing
 * - Testing error and warning reporting
 */

#include "../common/SBOMValidator.hpp"
#include <iostream>

/**
 * @brief Main function for the debug validator tool
 * 
 * Creates a test SPDX document and validates it using the SBOMValidator
 * framework. Outputs validation results including any errors or warnings.
 * 
 * @return 0 on success, 1 on failure
 */
int main() {
    // Test SPDX content for validation
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

    // Create SPDX validator
    auto validator = heimdall::SBOMValidatorFactory::createValidator("spdx");
    if (!validator) {
        std::cout << "Failed to create validator\n";
        return 1;
    }

    // Validate the test content
    auto result = validator->validateContent(spdx_content);
    std::cout << "Valid: " << (result.isValid ? "true" : "false") << "\n";
    std::cout << "Format: " << result.metadata["format"] << "\n";
    std::cout << "Version: " << result.metadata["version"] << "\n";
    
    // Report any validation errors
    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  " << error << "\n";
        }
    }
    
    // Report any validation warnings
    if (!result.warnings.empty()) {
        std::cout << "Warnings:\n";
        for (const auto& warning : result.warnings) {
            std::cout << "  " << warning << "\n";
        }
    }

    return 0;
} 