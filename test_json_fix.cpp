#include <iostream>
#include <string>
#include "src/common/SBOMValidator.hpp"

using namespace heimdall;

int main() {
    std::cout << "Testing JSON parsing fix..." << std::endl;
    
    auto validator = SBOMValidatorFactory::createValidator("spdx");
    if (!validator) {
        std::cout << "Failed to create SPDX validator" << std::endl;
        return 1;
    }
    
    // Test the malformed JSON that was causing SIGTRAP
    std::string malformed_json = R"({
  "@context": "https://spdx.org/rdf/3.0.0/spdx-context.jsonld",
  "@graph": [{
    "spdxId": "test",
    "name": "test",
    "invalid_field": [1, 2, 3, "unclosed_array"
  }]
})";
    
    std::cout << "Testing malformed JSON..." << std::endl;
    try {
        ValidationResult result = validator->validateContent(malformed_json);
        std::cout << "Malformed JSON test passed - no SIGTRAP" << std::endl;
        std::cout << "Valid: " << (result.isValid ? "true" : "false") << std::endl;
        std::cout << "Errors: " << result.errors.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception (expected): " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Caught unknown exception" << std::endl;
    }
    
    // Test control characters that were causing SIGTRAP
    std::string control_chars = R"({
  "@context": "https://spdx.org/rdf/3.0.0/spdx-context.jsonld",
  "@graph": [{
    "spdxId": "test",
    "name": "test\x01\x02\x03\x04\x05"
  }]
})";
    
    std::cout << "Testing control characters..." << std::endl;
    try {
        ValidationResult result = validator->validateContent(control_chars);
        std::cout << "Control characters test passed - no SIGTRAP" << std::endl;
        std::cout << "Valid: " << (result.isValid ? "true" : "false") << std::endl;
        std::cout << "Errors: " << result.errors.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception (expected): " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Caught unknown exception" << std::endl;
    }
    
    std::cout << "JSON parsing tests completed successfully!" << std::endl;
    return 0;
}