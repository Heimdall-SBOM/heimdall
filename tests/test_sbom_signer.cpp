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
 * @file test_sbom_signer.cpp
 * @brief Unit tests for SBOMSigner class based on JSF test vectors
 * @author Trevor Bakker
 * @date 2025
 *
 * This test file implements comprehensive tests for the SBOMSigner class
 * based on the JSON Signature Format (JSF) test vectors from Appendix A
 * of the JSF specification: https://cyberphone.github.io/doc/security/jsf.html#Test_Vectors
 */

#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include "common/SBOMSigner.hpp"
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class SBOMSignerTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      test_dir = test_utils::getUniqueTestDirectory("heimdall_signer_test");
      fs::create_directories(test_dir);

      // Generate test keys for all algorithms
      generateTestKeys();
   }

   void TearDown() override
   {
      test_utils::safeRemoveDirectory(test_dir);
   }

   fs::path test_dir;

   // Test key paths
   std::string rsa_private_key;
   std::string rsa_public_key;
   std::string ecdsa_private_key;
   std::string ecdsa_public_key;
   std::string ed25519_private_key;
   std::string ed25519_public_key;

   void        generateTestKeys()
   {
      // Generate RSA keys
      rsa_private_key = (test_dir / "rsa_private.key").string();
      rsa_public_key  = (test_dir / "rsa_public.key").string();

      std::string cmd = "openssl genrsa -out " + rsa_private_key + " 2048 2>/dev/null";
      system(cmd.c_str());
      cmd =
         "openssl rsa -in " + rsa_private_key + " -pubout -out " + rsa_public_key + " 2>/dev/null";
      system(cmd.c_str());

      // Generate ECDSA keys (P-256)
      ecdsa_private_key = (test_dir / "ecdsa_private.key").string();
      ecdsa_public_key  = (test_dir / "ecdsa_public.key").string();

      cmd = "openssl ecparam -genkey -name prime256v1 -out " + ecdsa_private_key + " 2>/dev/null";
      system(cmd.c_str());
      cmd = "openssl ec -in " + ecdsa_private_key + " -pubout -out " + ecdsa_public_key +
            " 2>/dev/null";
      system(cmd.c_str());

      // Generate Ed25519 keys
      ed25519_private_key = (test_dir / "ed25519_private.key").string();
      ed25519_public_key  = (test_dir / "ed25519_public.key").string();

      cmd = "openssl genpkey -algorithm ED25519 -out " + ed25519_private_key + " 2>/dev/null";
      system(cmd.c_str());
      cmd = "openssl pkey -in " + ed25519_private_key + " -pubout -out " + ed25519_public_key +
            " 2>/dev/null";
      system(cmd.c_str());
   }

   // Helper method to create a simple test SBOM
   std::string createTestSBOM()
   {
      nlohmann::json sbom;
      sbom["bomFormat"]   = "CycloneDX";
      sbom["specVersion"] = "1.6";
      sbom["version"]     = 1;
      sbom["metadata"]    = {
         {"timestamp", "2025-01-01T00:00:00Z"},
         {"tools",
             nlohmann::json::array(
             {{{"vendor", "Heimdall"}, {"name", "SBOM Generator"}, {"version", "1.0.0"}}})}};
      sbom["components"] = nlohmann::json::array({{{"bom-ref", "component-1"},
                                                   {"name", "test-component"},
                                                   {"version", "1.0.0"},
                                                   {"type", "library"}}});

      return sbom.dump(2);
   }

   // Helper method to create JSF test vector SBOM (based on JSF sample)
   std::string createJSFTestSBOM()
   {
      nlohmann::json sbom;
      sbom["now"]  = "2019-02-10T11:23:06Z";
      sbom["name"] = "Joe";
      sbom["id"]   = 2200063;

      return sbom.dump(2);
   }
};

// Test vector 1: Basic RSA signing and verification
TEST_F(SBOMSignerTest, RS256SigningAndVerification)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
   signer.setKeyId("test-rsa-key");

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify signature details
   EXPECT_EQ(signatureInfo.algorithm, "RS256");
   EXPECT_EQ(signatureInfo.keyId, "test-rsa-key");
   EXPECT_FALSE(signatureInfo.signature.empty());
   EXPECT_FALSE(signatureInfo.timestamp.empty());

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Load public key for verification
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));

   // Verify the signature
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 2: ECDSA signing and verification
TEST_F(SBOMSignerTest, ES256SigningAndVerification)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(ecdsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::ES256);
   signer.setKeyId("test-ecdsa-key");

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify signature details
   EXPECT_EQ(signatureInfo.algorithm, "ES256");
   EXPECT_EQ(signatureInfo.keyId, "test-ecdsa-key");
   EXPECT_FALSE(signatureInfo.signature.empty());
   EXPECT_FALSE(signatureInfo.timestamp.empty());

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Load public key for verification
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(ecdsa_public_key));

   // Verify the signature
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 3: Ed25519 signing and verification
TEST_F(SBOMSignerTest, Ed25519SigningAndVerification)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(ed25519_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::Ed25519);
   signer.setKeyId("test-ed25519-key");

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify signature details
   EXPECT_EQ(signatureInfo.algorithm, "Ed25519");
   EXPECT_EQ(signatureInfo.keyId, "test-ed25519-key");
   EXPECT_FALSE(signatureInfo.signature.empty());
   EXPECT_FALSE(signatureInfo.timestamp.empty());

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Load public key for verification
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(ed25519_public_key));

   // Verify the signature
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 4: JSF sample object recreation (based on JSF specification)
TEST_F(SBOMSignerTest, JSFSampleObjectRecreation)
{
   SBOMSigner signer;

   // Load ECDSA private key for ES256 (as used in JSF sample)
   ASSERT_TRUE(signer.loadPrivateKey(ecdsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::ES256);

   // Create JSF sample object (without signature)
   std::string jsfContent = createJSFTestSBOM();

   // Sign the content
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(jsfContent, signatureInfo));

   // Verify signature details match JSF format
   EXPECT_EQ(signatureInfo.algorithm, "ES256");
   EXPECT_FALSE(signatureInfo.signature.empty());

   // Add signature to create complete JSF object
   std::string signedJsf = signer.addSignatureToCycloneDX(jsfContent, signatureInfo);

   // Parse and verify structure matches JSF specification
   nlohmann::json signedJson = nlohmann::json::parse(signedJsf);

   // Check required fields from JSF sample
   EXPECT_TRUE(signedJson.contains("now"));
   EXPECT_TRUE(signedJson.contains("name"));
   EXPECT_TRUE(signedJson.contains("id"));
   EXPECT_TRUE(signedJson.contains("signature"));

   // Check signature structure
   nlohmann::json signature = signedJson["signature"];
   EXPECT_TRUE(signature.contains("algorithm"));
   EXPECT_TRUE(signature.contains("value"));
   EXPECT_EQ(signature["algorithm"], "ES256");

   // Verify the signature
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(ecdsa_public_key));
   EXPECT_TRUE(verifier.verifySignature(signedJsf));
}

// Test vector 5: Canonicalization with excludes
TEST_F(SBOMSignerTest, CanonicalizationWithExcludes)
{
   SBOMSigner signer;

   // Load private key
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create SBOM with existing signature field
   nlohmann::json sbom;
   sbom["bomFormat"]   = "CycloneDX";
   sbom["specVersion"] = "1.6";
   sbom["version"]     = 1;
   sbom["signature"]   = {{"algorithm", "RS256"}, {"value", "existing-signature"}};
   sbom["components"]  = nlohmann::json::array(
      {{{"bom-ref", "component-1"},
         {"name", "test-component"},
         {"signature", {{"algorithm", "RS256"}, {"value", "component-signature"}}}}});

   std::string sbomContent = sbom.dump(2);

   // Sign the SBOM (should exclude existing signature fields)
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify excludes were tracked
   EXPECT_FALSE(signatureInfo.excludes.empty());
   EXPECT_TRUE(std::find(signatureInfo.excludes.begin(), signatureInfo.excludes.end(),
                         "signature") != signatureInfo.excludes.end());
   EXPECT_TRUE(std::find(signatureInfo.excludes.begin(), signatureInfo.excludes.end(),
                         "components[0].signature") != signatureInfo.excludes.end());

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Verify the signature
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 6: Multiple algorithm support
TEST_F(SBOMSignerTest, MultipleAlgorithmSupport)
{
   std::vector<std::pair<SignatureAlgorithm, std::string>> algorithms = {
      {SignatureAlgorithm::RS256, rsa_private_key},
      {SignatureAlgorithm::RS384, rsa_private_key},
      {SignatureAlgorithm::RS512, rsa_private_key},
      {SignatureAlgorithm::ES256, ecdsa_private_key},
      {SignatureAlgorithm::ES384, ecdsa_private_key},
      {SignatureAlgorithm::ES512, ecdsa_private_key},
      {SignatureAlgorithm::Ed25519, ed25519_private_key}};

   std::string sbomContent = createTestSBOM();

   for (const auto& [algorithm, privateKey] : algorithms)
   {
      SBOMSigner signer;
      ASSERT_TRUE(signer.loadPrivateKey(privateKey));
      signer.setSignatureAlgorithm(algorithm);

      // Sign the SBOM
      SignatureInfo signatureInfo;
      ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

      // Verify algorithm string conversion
      std::string expectedAlgorithm;
      switch (algorithm)
      {
         case SignatureAlgorithm::RS256:
            expectedAlgorithm = "RS256";
            break;
         case SignatureAlgorithm::RS384:
            expectedAlgorithm = "RS384";
            break;
         case SignatureAlgorithm::RS512:
            expectedAlgorithm = "RS512";
            break;
         case SignatureAlgorithm::ES256:
            expectedAlgorithm = "ES256";
            break;
         case SignatureAlgorithm::ES384:
            expectedAlgorithm = "ES384";
            break;
         case SignatureAlgorithm::ES512:
            expectedAlgorithm = "ES512";
            break;
         case SignatureAlgorithm::Ed25519:
            expectedAlgorithm = "Ed25519";
            break;
      }
      EXPECT_EQ(signatureInfo.algorithm, expectedAlgorithm);
   }
}

// Test vector 7: Certificate-based verification
TEST_F(SBOMSignerTest, CertificateBasedVerification)
{
   // Generate a self-signed certificate
   std::string certPath = (test_dir / "test_cert.pem").string();
   std::string cmd      = "openssl req -new -x509 -key " + rsa_private_key + " -out " + certPath +
                     " -days 365 -subj '/CN=Test Certificate' 2>/dev/null";
   system(cmd.c_str());

   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Verify using certificate
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKeyFromCertificate(certPath));
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 8: Invalid signature detection
TEST_F(SBOMSignerTest, InvalidSignatureDetection)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Generate different keys for verification
   std::string wrongPrivateKey = (test_dir / "wrong_private.key").string();
   std::string wrongPublicKey  = (test_dir / "wrong_public.key").string();

   std::string cmd = "openssl genrsa -out " + wrongPrivateKey + " 2048 2>/dev/null";
   system(cmd.c_str());
   cmd = "openssl rsa -in " + wrongPrivateKey + " -pubout -out " + wrongPublicKey + " 2>/dev/null";
   system(cmd.c_str());

   // Verify with wrong key (should fail)
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(wrongPublicKey));
   EXPECT_FALSE(verifier.verifySignature(signedSbom));
}

// Test vector 9: Missing signature detection
TEST_F(SBOMSignerTest, MissingSignatureDetection)
{
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));

   // Try to verify unsigned SBOM
   std::string unsignedSbom = createTestSBOM();
   EXPECT_FALSE(verifier.verifySignature(unsignedSbom));

   // Check error message
   EXPECT_FALSE(verifier.getLastError().empty());
}

// Test vector 10: Signature extraction and parsing
TEST_F(SBOMSignerTest, SignatureExtractionAndParsing)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
   signer.setKeyId("test-key-id");

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Extract signature information
   SignatureInfo extractedInfo;
   ASSERT_TRUE(signer.extractSignature(signedSbom, extractedInfo));

   // Verify extracted information matches original
   EXPECT_EQ(extractedInfo.algorithm, signatureInfo.algorithm);
   EXPECT_EQ(extractedInfo.signature, signatureInfo.signature);
   // Note: keyId, timestamp, and excludes are not part of JSF-compliant signatures
   // so they won't be extracted from the signature object
}

// Test vector 11: Canonicalization verification
TEST_F(SBOMSignerTest, CanonicalizationVerification)
{
   SBOMSigner signer;

   // Create SBOM with signature fields
   nlohmann::json sbom;
   sbom["bomFormat"]   = "CycloneDX";
   sbom["specVersion"] = "1.6";
   sbom["version"]     = 1;
   sbom["signature"]   = {{"algorithm", "RS256"}, {"value", "test"}};
   sbom["components"] =
      nlohmann::json::array({{{"bom-ref", "component-1"},
                              {"name", "test-component"},
                              {"signature", {{"algorithm", "RS256"}, {"value", "test"}}}}});

   // Test canonicalization
   std::vector<std::string> excludes;
   std::string              canonical = signer.createCanonicalJSON(sbom, excludes);

   // Verify excludes were tracked
   EXPECT_FALSE(excludes.empty());
   EXPECT_TRUE(std::find(excludes.begin(), excludes.end(), "signature") != excludes.end());
   EXPECT_TRUE(std::find(excludes.begin(), excludes.end(), "components[0].signature") !=
               excludes.end());

   // Verify canonicalization removed signature fields
   nlohmann::json canonicalJson = nlohmann::json::parse(canonical);
   EXPECT_FALSE(canonicalJson.contains("signature"));
   EXPECT_FALSE(canonicalJson["components"][0].contains("signature"));

   // Verify canonicalization is correct
   EXPECT_TRUE(signer.verifyCanonicalization(sbom, canonical));
}

// Test vector 12: Error handling for invalid keys
TEST_F(SBOMSignerTest, ErrorHandlingInvalidKeys)
{
   SBOMSigner signer;

   // Try to load non-existent private key
   EXPECT_FALSE(signer.loadPrivateKey("non_existent.key"));
   EXPECT_FALSE(signer.getLastError().empty());

   // Try to load non-existent public key
   EXPECT_FALSE(signer.loadPublicKey("non_existent.key"));
   EXPECT_FALSE(signer.getLastError().empty());

   // Try to load non-existent certificate
   EXPECT_FALSE(signer.loadPublicKeyFromCertificate("non_existent.pem"));
   EXPECT_FALSE(signer.getLastError().empty());
}

// Test vector 13: Error handling for invalid algorithms
TEST_F(SBOMSignerTest, ErrorHandlingInvalidAlgorithms)
{
   SBOMSigner signer;

   // Try to sign without loading a key
   std::string   sbomContent = createTestSBOM();
   SignatureInfo signatureInfo;
   EXPECT_FALSE(signer.signSBOM(sbomContent, signatureInfo));
   EXPECT_FALSE(signer.getLastError().empty());

   // Try to verify without loading a key
   EXPECT_FALSE(signer.verifySignature(sbomContent));
   EXPECT_FALSE(signer.getLastError().empty());
}

// Test vector 14: JSF compliance - signature structure validation
TEST_F(SBOMSignerTest, JSFComplianceSignatureStructure)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);
   signer.setKeyId("test-key-id");

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Parse and verify JSF structure compliance
   nlohmann::json signedJson = nlohmann::json::parse(signedSbom);

   // Verify signature object exists and has required fields
   EXPECT_TRUE(signedJson.contains("signature"));
   nlohmann::json signature = signedJson["signature"];

   // Required fields according to JSF specification
   EXPECT_TRUE(signature.contains("algorithm"));
   EXPECT_TRUE(signature.contains("value"));

   // Optional fields
   if (signature.contains("keyId"))
   {
      EXPECT_TRUE(signature["keyId"].is_string());
   }
   if (signature.contains("excludes"))
   {
      EXPECT_TRUE(signature["excludes"].is_array());
   }

   // Verify field types
   EXPECT_TRUE(signature["algorithm"].is_string());
   EXPECT_TRUE(signature["value"].is_string());
   EXPECT_EQ(signature["algorithm"], "RS256");
}

// Test vector 15: Base64 encoding/decoding consistency
TEST_F(SBOMSignerTest, Base64EncodingDecodingConsistency)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify signature is valid base64
   EXPECT_FALSE(signatureInfo.signature.empty());

   // The signature should be base64url-encoded and should not contain invalid characters
   std::string validBase64URLChars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
   for (char c : signatureInfo.signature)
   {
      EXPECT_NE(validBase64URLChars.find(c), std::string::npos)
         << "Invalid base64url character found: " << c;
   }
}

// Test vector 16: Timestamp format validation
TEST_F(SBOMSignerTest, TimestampFormatValidation)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create test SBOM
   std::string sbomContent = createTestSBOM();

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Verify timestamp format (ISO 8601)
   EXPECT_FALSE(signatureInfo.timestamp.empty());

   // Check ISO 8601 format: YYYY-MM-DDTHH:MM:SS.sssZ
   std::regex iso8601_regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z)");
   EXPECT_TRUE(std::regex_match(signatureInfo.timestamp, iso8601_regex));
}

// Test vector 17: Multiple signatures on same content
TEST_F(SBOMSignerTest, MultipleSignaturesSameContent)
{
   std::string sbomContent = createTestSBOM();

   // Sign with RSA
   SBOMSigner rsaSigner;
   ASSERT_TRUE(rsaSigner.loadPrivateKey(rsa_private_key));
   rsaSigner.setSignatureAlgorithm(SignatureAlgorithm::RS256);
   rsaSigner.setKeyId("rsa-key");

   SignatureInfo rsaSignature;
   ASSERT_TRUE(rsaSigner.signSBOM(sbomContent, rsaSignature));

   // Sign with ECDSA
   SBOMSigner ecdsaSigner;
   ASSERT_TRUE(ecdsaSigner.loadPrivateKey(ecdsa_private_key));
   ecdsaSigner.setSignatureAlgorithm(SignatureAlgorithm::ES256);
   ecdsaSigner.setKeyId("ecdsa-key");

   SignatureInfo ecdsaSignature;
   ASSERT_TRUE(ecdsaSigner.signSBOM(sbomContent, ecdsaSignature));

   // Verify both signatures are different
   EXPECT_NE(rsaSignature.signature, ecdsaSignature.signature);
   EXPECT_NE(rsaSignature.algorithm, ecdsaSignature.algorithm);

   // Verify both signatures are valid
   SBOMSigner rsaVerifier;
   ASSERT_TRUE(rsaVerifier.loadPublicKey(rsa_public_key));

   SBOMSigner ecdsaVerifier;
   ASSERT_TRUE(ecdsaVerifier.loadPublicKey(ecdsa_public_key));

   std::string rsaSigned   = rsaSigner.addSignatureToCycloneDX(sbomContent, rsaSignature);
   std::string ecdsaSigned = ecdsaSigner.addSignatureToCycloneDX(sbomContent, ecdsaSignature);

   EXPECT_TRUE(rsaVerifier.verifySignature(rsaSigned));
   EXPECT_TRUE(ecdsaVerifier.verifySignature(ecdsaSigned));
}

// Test vector 18: Large SBOM signing and verification
TEST_F(SBOMSignerTest, LargeSBOMSigningAndVerification)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create large SBOM with many components
   nlohmann::json sbom;
   sbom["bomFormat"]   = "CycloneDX";
   sbom["specVersion"] = "1.6";
   sbom["version"]     = 1;
   sbom["metadata"]    = {{"timestamp", "2025-01-01T00:00:00Z"}};

   // Add many components
   sbom["components"] = nlohmann::json::array();
   for (int i = 0; i < 100; ++i)
   {
      nlohmann::json component;
      component["bom-ref"]     = "component-" + std::to_string(i);
      component["name"]        = "test-component-" + std::to_string(i);
      component["version"]     = "1.0." + std::to_string(i);
      component["type"]        = "library";
      component["description"] = "Test component " + std::to_string(i) + " for large SBOM testing";
      sbom["components"].push_back(component);
   }

   std::string sbomContent = sbom.dump(2);

   // Sign the large SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Verify the signature
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 19: Edge cases - empty SBOM
TEST_F(SBOMSignerTest, EdgeCaseEmptySBOM)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create minimal SBOM
   nlohmann::json sbom;
   sbom["bomFormat"]   = "CycloneDX";
   sbom["specVersion"] = "1.6";
   sbom["version"]     = 1;

   std::string sbomContent = sbom.dump(2);

   // Sign the minimal SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Verify the signature
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}

// Test vector 20: Edge cases - special characters in SBOM
TEST_F(SBOMSignerTest, EdgeCaseSpecialCharacters)
{
   SBOMSigner signer;

   // Load private key for signing
   ASSERT_TRUE(signer.loadPrivateKey(rsa_private_key));
   signer.setSignatureAlgorithm(SignatureAlgorithm::RS256);

   // Create SBOM with special characters
   nlohmann::json sbom;
   sbom["bomFormat"]   = "CycloneDX";
   sbom["specVersion"] = "1.6";
   sbom["version"]     = 1;
   sbom["metadata"]    = {{"timestamp", "2025-01-01T00:00:00Z"},
                          {"description", "SBOM with special chars: éñüß日本語한국어العربية"}};
   sbom["components"]  = nlohmann::json::array(
      {{{"bom-ref", "component-1"},
         {"name", "test-component-特殊文字"},
         {"version", "1.0.0"},
         {"type", "library"},
         {"description", "Component with special characters: éñüß日本語한국어العربية"}}});

   std::string sbomContent = sbom.dump(2);

   // Sign the SBOM
   SignatureInfo signatureInfo;
   ASSERT_TRUE(signer.signSBOM(sbomContent, signatureInfo));

   // Add signature to SBOM
   std::string signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   // Verify the signature
   SBOMSigner verifier;
   ASSERT_TRUE(verifier.loadPublicKey(rsa_public_key));
   EXPECT_TRUE(verifier.verifySignature(signedSbom));
}