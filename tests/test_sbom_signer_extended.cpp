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
 * @file test_sbom_signer_extended.cpp
 * @brief Extended unit tests for SBOMSigner class
 * @author Trevor Bakker
 * @date 2025
 * 
 * This test file provides additional unit tests for the SBOMSigner class,
 * complementing the existing test_sbom_signer.cpp with more edge cases,
 * performance tests, and advanced scenarios.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <regex>
#include <nlohmann/json.hpp>
#include "common/SBOMSigner.hpp"
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class SBOMSignerExtendedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir = test_utils::getUniqueTestDirectory("heimdall_sbom_signer_extended_test");
        heimdall::compat::fs::create_directories(test_dir);
        
        // Generate test keys
        generateTestKeys();
    }
    
    void TearDown() override
    {
        test_utils::safeRemoveDirectory(test_dir);
    }
    
    heimdall::compat::fs::path test_dir;
    
    // Test key paths
    std::string rsa_private_key;
    std::string rsa_public_key;
    std::string ecdsa_private_key;
    std::string ecdsa_public_key;
    std::string ed25519_private_key;
    std::string ed25519_public_key;
    std::string rsa_certificate;
    std::string ecdsa_certificate;
    
    void generateTestKeys()
    {
        // Generate RSA keys
        rsa_private_key = (test_dir / "rsa_private.key").string();
        rsa_public_key = (test_dir / "rsa_public.key").string();
        rsa_certificate = (test_dir / "rsa_cert.pem").string();
        
        std::string cmd = "openssl genrsa -out " + rsa_private_key + " 2048 2>/dev/null";
        system(cmd.c_str());
        cmd = "openssl rsa -in " + rsa_private_key + " -pubout -out " + rsa_public_key + " 2>/dev/null";
        system(cmd.c_str());
        cmd = "openssl req -new -x509 -key " + rsa_private_key + 
              " -out " + rsa_certificate + " -days 365 -subj '/CN=Test RSA Certificate' 2>/dev/null";
        system(cmd.c_str());
        
        // Generate ECDSA keys
        ecdsa_private_key = (test_dir / "ecdsa_private.key").string();
        ecdsa_public_key = (test_dir / "ecdsa_public.key").string();
        ecdsa_certificate = (test_dir / "ecdsa_cert.pem").string();
        
        cmd = "openssl ecparam -genkey -name prime256v1 -out " + ecdsa_private_key + " 2>/dev/null";
        system(cmd.c_str());
        cmd = "openssl ec -in " + ecdsa_private_key + " -pubout -out " + ecdsa_public_key + " 2>/dev/null";
        system(cmd.c_str());
        cmd = "openssl req -new -x509 -key " + ecdsa_private_key + 
              " -out " + ecdsa_certificate + " -days 365 -subj '/CN=Test ECDSA Certificate' 2>/dev/null";
        system(cmd.c_str());
        
        // Generate Ed25519 keys
        ed25519_private_key = (test_dir / "ed25519_private.key").string();
        ed25519_public_key = (test_dir / "ed25519_public.key").string();
        
        cmd = "openssl genpkey -algorithm ED25519 -out " + ed25519_private_key + " 2>/dev/null";
        system(cmd.c_str());
        cmd = "openssl pkey -in " + ed25519_private_key + " -pubout -out " + ed25519_public_key + " 2>/dev/null";
        system(cmd.c_str());
    }
    
    // Helper method to create a test SBOM
    nlohmann::json createTestSBOM()
    {
        nlohmann::json sbom;
        sbom["bomFormat"] = "CycloneDX";
        sbom["specVersion"] = "1.6";
        sbom["version"] = 1;
        sbom["metadata"] = {
            {"timestamp", "2025-07-28T10:00:00Z"},
            {"tools", nlohmann::json::array()}
        };
        sbom["components"] = nlohmann::json::array();
        
        // Add a component
        nlohmann::json component;
        component["bom-ref"] = "test-component-1.0.0";
        component["name"] = "test-component";
        component["version"] = "1.0.0";
        component["type"] = "library";
        component["purl"] = "pkg:generic/test-component@1.0.0";
        component["licenses"] = nlohmann::json::array();
        component["licenses"].push_back({{"license", {{"id", "MIT"}}}});
        
        sbom["components"].push_back(component);
        
        return sbom;
    }
    
    // Helper method to create a large test SBOM
    nlohmann::json createLargeTestSBOM(int componentCount = 1000)
    {
        nlohmann::json sbom = createTestSBOM();
        
        for (int i = 0; i < componentCount; ++i) {
            nlohmann::json component;
            component["bom-ref"] = "component-" + std::to_string(i) + "-1.0.0";
            component["name"] = "component-" + std::to_string(i);
            component["version"] = "1.0.0";
            component["type"] = "library";
            component["purl"] = "pkg:generic/component-" + std::to_string(i) + "@1.0.0";
            component["licenses"] = nlohmann::json::array();
            component["licenses"].push_back({{"license", {{"id", "MIT"}}}});
            
            // Add some metadata to make it more realistic
            component["description"] = "Component " + std::to_string(i) + " description";
            component["scope"] = "required";
            component["group"] = "com.example";
            
            sbom["components"].push_back(component);
        }
        
        return sbom;
    }
    
    // Helper method to validate signature structure
    bool validateSignatureStructure(const nlohmann::json& sbom)
    {
        if (!sbom.contains("signature")) {
            return false;
        }
        
        nlohmann::json signature = sbom["signature"];
        
        // Required fields
        if (!signature.contains("algorithm") || !signature.contains("value")) {
            return false;
        }
        
        // Optional fields should be present but can be empty
        if (!signature.contains("timestamp") || !signature.contains("excludes")) {
            return false;
        }
        
        // Validate timestamp format (ISO 8601)
        std::string timestamp = signature["timestamp"];
        std::regex timestampRegex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z)");
        if (!std::regex_match(timestamp, timestampRegex)) {
            return false;
        }
        
        // Validate excludes is an array
        if (!signature["excludes"].is_array()) {
            return false;
        }
        
        return true;
    }
};

// Test performance with large SBOMs
TEST_F(SBOMSignerExtendedTest, LargeSBOMPerformance)
{
    SBOMSigner signer;
    
    // Load private key
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create large SBOM
    nlohmann::json largeSbom = createLargeTestSBOM(1000);
    std::string sbomContent = largeSbom.dump(2);
    
    // Measure signing time
    auto start = std::chrono::high_resolution_clock::now();
    
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(signResult) << "Failed to sign large SBOM: " << signer.getLastError();
    EXPECT_LT(duration.count(), 5000) << "Signing large SBOM took too long: " << duration.count() << "ms";
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify large SBOM signature: " << signer.getLastError();
}

// Test memory usage with very large SBOMs
TEST_F(SBOMSignerExtendedTest, VeryLargeSBOMMemoryUsage)
{
    SBOMSigner signer;
    
    // Load private key
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create very large SBOM (10,000 components)
    nlohmann::json veryLargeSbom = createLargeTestSBOM(10000);
    std::string sbomContent = veryLargeSbom.dump(2);
    
    // Measure memory usage before signing
    size_t contentSize = sbomContent.size();
    EXPECT_GT(contentSize, 1000000) << "SBOM should be larger than 1MB";
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign very large SBOM: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
}

// Test concurrent signing operations
TEST_F(SBOMSignerExtendedTest, ConcurrentSigningOperations)
{
    const int numThreads = 4;
    const int operationsPerThread = 10;
    
    std::vector<std::thread> threads;
    std::vector<bool> results(numThreads * operationsPerThread, false);
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, operationsPerThread, &results]() {
            SBOMSigner signer;
            
            // Load private key
            if (!signer.loadPrivateKey(rsa_private_key)) {
                return;
            }
            
            for (int j = 0; j < operationsPerThread; ++j) {
                nlohmann::json sbom = createTestSBOM();
                sbom["metadata"]["timestamp"] = "2025-07-28T" + std::to_string(i) + ":" + std::to_string(j) + ":00Z";
                std::string sbomContent = sbom.dump(2);
                
                SignatureInfo signatureInfo;
                signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
                bool result = signer.signSBOM(sbomContent, signatureInfo);
                results[i * operationsPerThread + j] = result;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Check all operations succeeded
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i]) << "Concurrent signing operation " << i << " failed";
    }
}

// Test signature with special characters in SBOM content
TEST_F(SBOMSignerExtendedTest, SpecialCharactersInSBOM)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with special characters
    nlohmann::json sbom = createTestSBOM();
    sbom["metadata"]["description"] = "SBOM with special chars: éñüß日本語한국어العربية";
    sbom["components"][0]["description"] = "Component with © symbols & special chars: <>&\"'";
    sbom["components"][0]["copyright"] = "Copyright © 2025 Example Corp. All rights reserved.";
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with special characters: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with special characters: " << signer.getLastError();
}

// Test signature with very long field values
TEST_F(SBOMSignerExtendedTest, LongFieldValues)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with very long field values
    nlohmann::json sbom = createTestSBOM();
    
    // Create very long strings
    std::string longDescription(10000, 'A');
    std::string longCopyright(5000, 'B');
    std::string longPurl(2000, 'C');
    
    sbom["metadata"]["description"] = longDescription;
    sbom["components"][0]["description"] = longDescription;
    sbom["components"][0]["copyright"] = longCopyright;
    sbom["components"][0]["purl"] = longPurl;
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with long fields: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with long fields: " << signer.getLastError();
}

// Test signature with deeply nested structures
TEST_F(SBOMSignerExtendedTest, DeeplyNestedStructures)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with deeply nested structures
    nlohmann::json sbom = createTestSBOM();
    
    // Add deeply nested component
    nlohmann::json nestedComponent = sbom["components"][0];
    nestedComponent["bom-ref"] = "nested-component-1.0.0";
    nestedComponent["name"] = "nested-component";
    
    // Create deep nesting
    nlohmann::json deepNesting;
    nlohmann::json current = deepNesting;
    for (int i = 0; i < 20; ++i) {
        current["level"] = i;
        current["data"] = "nested data at level " + std::to_string(i);
        current["next"] = nlohmann::json::object();
        current = current["next"];
    }
    
    nestedComponent["deepData"] = deepNesting;
    sbom["components"].push_back(nestedComponent);
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with nested structures: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with nested structures: " << signer.getLastError();
}

// Test signature with arrays containing many elements
TEST_F(SBOMSignerExtendedTest, LargeArrays)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with large arrays
    nlohmann::json sbom = createTestSBOM();
    
    // Add large array of components
    for (int i = 0; i < 500; ++i) {
        nlohmann::json component;
        component["bom-ref"] = "array-component-" + std::to_string(i) + "-1.0.0";
        component["name"] = "array-component-" + std::to_string(i);
        component["version"] = "1.0.0";
        component["type"] = "library";
        component["purl"] = "pkg:generic/array-component-" + std::to_string(i) + "@1.0.0";
        component["licenses"] = nlohmann::json::array();
        component["licenses"].push_back({{"license", {{"id", "MIT"}}}});
        
        sbom["components"].push_back(component);
    }
    
    // Add large array of hashes
    nlohmann::json largeHashArray = nlohmann::json::array();
    for (int i = 0; i < 1000; ++i) {
        nlohmann::json hash;
        hash["alg"] = "SHA-256";
        hash["content"] = "hash" + std::to_string(i) + "0000000000000000000000000000000000000000000000000000000000000000";
        largeHashArray.push_back(hash);
    }
    sbom["components"][0]["hashes"] = largeHashArray;
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with large arrays: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with large arrays: " << signer.getLastError();
}

// Test signature with mixed data types
TEST_F(SBOMSignerExtendedTest, MixedDataTypes)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with mixed data types
    nlohmann::json sbom = createTestSBOM();
    
    // Add mixed data types
    sbom["metadata"]["mixedData"] = {
        {"string", "test string"},
        {"number", 42},
        {"boolean", true},
        {"null", nullptr},
        {"array", {1, 2, 3, "string", false}},
        {"object", {
            {"nested", "value"},
            {"numbers", {1.5, 2.7, 3.14}},
            {"booleans", {true, false, true}}
        }}
    };
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with mixed data types: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with mixed data types: " << signer.getLastError();
}

// Test signature with floating point numbers
TEST_F(SBOMSignerExtendedTest, FloatingPointNumbers)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with floating point numbers
    nlohmann::json sbom = createTestSBOM();
    
    // Add floating point data
    sbom["metadata"]["floatingPointData"] = {
        {"pi", 3.14159265359},
        {"e", 2.71828182846},
        {"sqrt2", 1.41421356237},
        {"negative", -1.5},
        {"zero", 0.0},
        {"large", 1.23456789e+10},
        {"small", 1.23456789e-10}
    };
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with floating point numbers: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with floating point numbers: " << signer.getLastError();
}

// Test signature with empty objects and arrays
TEST_F(SBOMSignerExtendedTest, EmptyObjectsAndArrays)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with empty objects and arrays
    nlohmann::json sbom = createTestSBOM();
    
    // Add empty structures
    sbom["metadata"]["emptyObject"] = nlohmann::json::object();
    sbom["metadata"]["emptyArray"] = nlohmann::json::array();
    sbom["components"][0]["emptyObject"] = nlohmann::json::object();
    sbom["components"][0]["emptyArray"] = nlohmann::json::array();
    
    // Add nested empty structures
    sbom["metadata"]["nestedEmpty"] = {
        {"empty1", nlohmann::json::object()},
        {"empty2", nlohmann::json::array()},
        {"mixed", {
            {"empty", nlohmann::json::object()},
            {"data", "value"},
            {"emptyArray", nlohmann::json::array()}
        }}
    };
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with empty structures: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with empty structures: " << signer.getLastError();
}

// Test signature with Unicode normalization
TEST_F(SBOMSignerExtendedTest, UnicodeNormalization)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM with Unicode characters that have multiple representations
    nlohmann::json sbom = createTestSBOM();
    
    // Add Unicode strings with different normalization forms
    sbom["metadata"]["unicodeData"] = {
        {"cafe", "café"},  // é can be represented as e + combining acute accent
        {"umlaut", "naïve"},  // ï can be represented as i + combining diaeresis
        {"cjk", "日本語"},  // CJK characters
        {"arabic", "العربية"},  // Arabic text
        {"emoji", "🚀🔒📦"},  // Emoji characters
        {"mixed", "Hello 世界 🌍"}
    };
    
    std::string sbomContent = sbom.dump(2);
    
    // Sign the SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(sbomContent, signatureInfo);
    
    EXPECT_TRUE(signResult) << "Failed to sign SBOM with Unicode: " << signer.getLastError();
    EXPECT_EQ(signatureInfo.algorithm, "RS256");
    EXPECT_FALSE(signatureInfo.signature.empty());
    
    // Add signature to SBOM content
    sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
    
    // Verify signature
    ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
    bool verifyResult = signer.verifySignature(sbomContent);
    EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with Unicode: " << signer.getLastError();
}

// Test signature with different JSON encodings
TEST_F(SBOMSignerExtendedTest, DifferentJSONEncodings)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create SBOM
    nlohmann::json sbom = createTestSBOM();
    
    // Test different JSON encodings
    std::vector<std::string> jsonEncodings;
    jsonEncodings.push_back(sbom.dump());  // Compact
    jsonEncodings.push_back(sbom.dump(2));  // Pretty printed
    jsonEncodings.push_back(sbom.dump(4));  // More indentation
    jsonEncodings.push_back(sbom.dump(-1)); // Most compact
    
    for (size_t i = 0; i < jsonEncodings.size(); ++i) {
        // Sign the SBOM
        SignatureInfo signatureInfo;
        signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
        bool signResult = signer.signSBOM(jsonEncodings[i], signatureInfo);
        
        EXPECT_TRUE(signResult) << "Failed to sign SBOM with encoding " << i << ": " << signer.getLastError();
        EXPECT_EQ(signatureInfo.algorithm, "RS256");
        EXPECT_FALSE(signatureInfo.signature.empty());
        
        // Add signature to SBOM content
        jsonEncodings[i] = signer.addSignatureToCycloneDX(jsonEncodings[i], signatureInfo);
        
        // Verify signature
        ASSERT_TRUE(signer.loadPublicKey(rsa_public_key));
        bool verifyResult = signer.verifySignature(jsonEncodings[i]);
        EXPECT_TRUE(verifyResult) << "Failed to verify SBOM with encoding " << i << ": " << signer.getLastError();
    }
}

// Test signature with malformed JSON recovery
TEST_F(SBOMSignerExtendedTest, MalformedJSONRecovery)
{
    SBOMSigner signer;
    ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
    
    // Create valid SBOM first
    nlohmann::json sbom = createTestSBOM();
    std::string validContent = sbom.dump(2);
    
    // Sign valid SBOM
    SignatureInfo signatureInfo;
    signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
    bool signResult = signer.signSBOM(validContent, signatureInfo);
    EXPECT_TRUE(signResult) << "Failed to sign valid SBOM: " << signer.getLastError();
    
    // Try to sign malformed JSON
    std::string malformedContent = validContent.substr(0, validContent.length() - 10) + "invalid}";
    
    SignatureInfo malformedSignatureInfo;
    bool malformedSignResult = signer.signSBOM(malformedContent, malformedSignatureInfo);
    
    // Should fail gracefully
    EXPECT_FALSE(malformedSignResult) << "Should fail to sign malformed JSON";
    EXPECT_FALSE(signer.getLastError().empty()) << "Should have error message for malformed JSON";
}

// Test signature with different key sizes
TEST_F(SBOMSignerExtendedTest, DifferentKeySizes)
{
    // Generate different RSA key sizes
    std::vector<int> keySizes = {1024, 2048, 4096};
    
    for (int keySize : keySizes) {
        // Generate key pair
        std::string privateKeyPath = (test_dir / ("rsa_" + std::to_string(keySize) + "_private.key")).string();
        std::string publicKeyPath = (test_dir / ("rsa_" + std::to_string(keySize) + "_public.key")).string();
        
        std::string genCmd = "openssl genrsa -out " + privateKeyPath + " " + std::to_string(keySize) + " 2>/dev/null";
        system(genCmd.c_str());
        std::string pubCmd = "openssl rsa -in " + privateKeyPath + " -pubout -out " + publicKeyPath + " 2>/dev/null";
        system(pubCmd.c_str());
        
        SBOMSigner signer;
        ASSERT_TRUE(signer.loadPrivateKey(privateKeyPath)) << "Failed to load " << keySize << "-bit private key";
        
        // Create and sign SBOM
        nlohmann::json sbom = createTestSBOM();
        std::string sbomContent = sbom.dump(2);
        
        SignatureInfo signatureInfo;
        signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
        bool signResult = signer.signSBOM(sbomContent, signatureInfo);
        
        EXPECT_TRUE(signResult) << "Failed to sign with " << keySize << "-bit key: " << signer.getLastError();
        EXPECT_EQ(signatureInfo.algorithm, "RS256");
        EXPECT_FALSE(signatureInfo.signature.empty());
        
        // Add signature to SBOM content
        sbomContent = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);
        
        // Verify signature
        ASSERT_TRUE(signer.loadPublicKey(publicKeyPath)) << "Failed to load " << keySize << "-bit public key";
        bool verifyResult = signer.verifySignature(sbomContent);
        EXPECT_TRUE(verifyResult) << "Failed to verify with " << keySize << "-bit key: " << signer.getLastError();
    }
} 