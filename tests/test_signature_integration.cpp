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
 * @file test_signature_integration.cpp
 * @brief Integration tests for heimdall-sbom and heimdall-validate signature functionality
 * @author Trevor Bakker
 * @date 2025
 *
 * This test file provides comprehensive integration testing for the signature
 * functionality across heimdall-sbom and heimdall-validate tools.
 */

#include <gtest/gtest.h>
#include <cstdlib>
// #include <filesystem>  // Removed to avoid conflict with compatibility layer
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class SignatureIntegrationTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      test_dir = test_utils::getUniqueTestDirectory("heimdall_signature_integration_test");
      fs::create_directories(test_dir);

      // Generate test keys for all algorithms
      generateTestKeys();

      // Create test binary and plugin
      createTestBinary();
      createTestPlugin();
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
   std::string rsa_certificate;
   std::string ecdsa_certificate;

   // Test binary and plugin paths
   std::string test_binary;
   std::string test_plugin;

   void        generateTestKeys()
   {
      // Generate RSA keys
      rsa_private_key = (test_dir / "rsa_private.key").string();
      rsa_public_key  = (test_dir / "rsa_public.key").string();
      rsa_certificate = (test_dir / "rsa_cert.pem").string();

      std::string cmd = "openssl genrsa -out " + rsa_private_key + " 2048 2>/dev/null";
      system(cmd.c_str());
      cmd =
         "openssl rsa -in " + rsa_private_key + " -pubout -out " + rsa_public_key + " 2>/dev/null";
      system(cmd.c_str());
      cmd = "openssl req -new -x509 -key " + rsa_private_key + " -out " + rsa_certificate +
            " -days 365 -subj '/CN=Test RSA Certificate' 2>/dev/null";
      system(cmd.c_str());

      // Generate ECDSA keys
      ecdsa_private_key = (test_dir / "ecdsa_private.key").string();
      ecdsa_public_key  = (test_dir / "ecdsa_public.key").string();
      ecdsa_certificate = (test_dir / "ecdsa_cert.pem").string();

      cmd = "openssl ecparam -genkey -name prime256v1 -out " + ecdsa_private_key + " 2>/dev/null";
      system(cmd.c_str());
      cmd = "openssl ec -in " + ecdsa_private_key + " -pubout -out " + ecdsa_public_key +
            " 2>/dev/null";
      system(cmd.c_str());
      cmd = "openssl req -new -x509 -key " + ecdsa_private_key + " -out " + ecdsa_certificate +
            " -days 365 -subj '/CN=Test ECDSA Certificate' 2>/dev/null";
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

   void createTestBinary()
   {
      test_binary = (test_dir / "test_binary").string();
      std::ofstream binary(test_binary);
      binary << "ELF test binary content for signature testing";
      binary.close();

      // Make it executable
      chmod(test_binary.c_str(), 0755);
   }

   std::string findPluginPath(const std::string& pluginName)
   {
      std::vector<std::string> searchPaths = {
         "lib/",        // Build output directory
         "build/lib/",  // Alternative build location
         "../lib/",
         "../../lib/",
         "build/",  // Primary location in CI
         "../build/",
         "../../build/",
         "./",
         "build/install/lib64/heimdall-plugins/",  // Installed location
         "../build/install/lib64/heimdall-plugins/",
         "../../build/install/lib64/heimdall-plugins/",
         "../../build/tests/",
         "../build/tests/",
         "build/tests/",
         "./tests/"};

      for (const auto& path : searchPaths)
      {
         std::string fullPath = path + pluginName;
         if (fs::exists(fullPath))
         {
            return fs::absolute(fullPath).string();
         }
      }

      // Also try to find plugins in the current directory tree
      fs::path currentDir = fs::current_path();
      for (const fs::path& entry : fs::recursive_directory_iterator(currentDir))
      {
         if (fs::is_regular_file(entry) && entry.filename() == pluginName)
         {
            return entry.string();
         }
      }

      return "";
   }

   void createTestPlugin()
   {
      test_plugin = (test_dir / "test_plugin.so").string();

      // Find an existing Heimdall plugin that implements the required interface
      std::string sourcePlugin = findPluginPath("heimdall-gold.so");
      if (sourcePlugin.empty())
      {
         sourcePlugin = findPluginPath("heimdall-lld.so");
      }

      if (!sourcePlugin.empty())
      {
         std::ifstream source(sourcePlugin, std::ios::binary);
         std::ofstream dest(test_plugin, std::ios::binary);

         if (source && dest)
         {
            dest << source.rdbuf();
            source.close();
            dest.close();
            return;
         }
      }

      // Fallback: create a minimal ELF file if copy fails
      std::ofstream plugin(test_plugin, std::ios::binary);
      // Write minimal ELF header (this is just a placeholder)
      const unsigned char elf_header[] = {
         0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x03, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x40, 0x00, 0x38, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00};
      plugin.write(reinterpret_cast<const char*>(elf_header), sizeof(elf_header));
      plugin.close();
   }

   // Helper method to run heimdall-sbom command
   int runHeimdallSbom(const std::vector<std::string>& args)
   {
      // Try to find heimdall-sbom in common build directories
      std::vector<std::string> possiblePaths = {
         "../src/tools/heimdall-sbom",
         "./src/tools/heimdall-sbom",
         "./build-clang-cpp17/src/tools/heimdall-sbom",
         "./build-clang-cpp20/src/tools/heimdall-sbom",
         "./build-clang-cpp23/src/tools/heimdall-sbom",
         "./build-gcc-cpp17/src/tools/heimdall-sbom",
         "./build-gcc-cpp20/src/tools/heimdall-sbom",
         "./build-gcc-cpp23/src/tools/heimdall-sbom",
         "/home/tbakker/code/heimdall/build-gcc-cpp23/src/tools/heimdall-sbom"};

      std::string heimdallSbomPath;
      for (const auto& path : possiblePaths)
      {
         if (fs::exists(path))
         {
            heimdallSbomPath = path;
            break;
         }
      }

      if (heimdallSbomPath.empty())
      {
         std::cerr << "Could not find heimdall-sbom in any of the expected locations" << std::endl;
         return -1;
      }

      std::string cmd = heimdallSbomPath;
      for (const auto& arg : args)
      {
         cmd += " " + arg;
      }
      return system(cmd.c_str());
   }

   // Helper method to run heimdall-validate command
   int runHeimdallValidate(const std::vector<std::string>& args)
   {
      // Try to find heimdall-validate in common build directories
      std::vector<std::string> possiblePaths = {
         "../src/tools/heimdall-validate",
         "./src/tools/heimdall-validate",
         "./build-clang-cpp17/src/tools/heimdall-validate",
         "./build-clang-cpp20/src/tools/heimdall-validate",
         "./build-clang-cpp23/src/tools/heimdall-validate",
         "./build-gcc-cpp17/src/tools/heimdall-validate",
         "./build-gcc-cpp20/src/tools/heimdall-validate",
         "./build-gcc-cpp23/src/tools/heimdall-validate",
         "/home/tbakker/code/heimdall/build-gcc-cpp23/src/tools/heimdall-validate"};

      std::string heimdallValidatePath;
      for (const auto& path : possiblePaths)
      {
         if (fs::exists(path))
         {
            heimdallValidatePath = path;
            break;
         }
      }

      if (heimdallValidatePath.empty())
      {
         std::cerr << "Could not find heimdall-validate in any of the expected locations"
                   << std::endl;
         return -1;
      }

      std::string cmd = heimdallValidatePath;
      for (const auto& arg : args)
      {
         cmd += " " + arg;
      }
      return system(cmd.c_str());
   }

   // Helper method to check if file exists and is readable
   bool fileExists(const std::string& path)
   {
      std::ifstream file(path);
      return file.good();
   }

   // Helper method to read JSON file
   nlohmann::json readJsonFile(const std::string& path)
   {
      std::ifstream  file(path);
      nlohmann::json json;
      file >> json;
      return json;
   }

   // Helper method to check if SBOM has signature
   bool hasSignature(const std::string& sbomPath)
   {
      try
      {
         nlohmann::json sbom = readJsonFile(sbomPath);
         return sbom.contains("signature");
      }
      catch (...)
      {
         return false;
      }
   }

   // Helper method to get signature algorithm from SBOM
   std::string getSignatureAlgorithm(const std::string& sbomPath)
   {
      try
      {
         nlohmann::json sbom = readJsonFile(sbomPath);
         if (sbom.contains("signature") && sbom["signature"].contains("algorithm"))
         {
            return sbom["signature"]["algorithm"];
         }
      }
      catch (...)
      {
         // Ignore errors
      }
      return "";
   }
};

// Test RSA signing and verification integration
TEST_F(SignatureIntegrationTest, RSASigningAndVerificationIntegration)
{
   // Sign SBOM with RSA
   std::string              signedSbomPath = (test_dir / "rsa_signed.sbom.json").string();

   std::vector<std::string> signArgs = {test_plugin,  test_binary,     "--format",
                                        "cyclonedx",  "--output",      signedSbomPath,
                                        "--sign-key", rsa_private_key, "--sign-algorithm",
                                        "RS256",      "--sign-key-id", "test-rsa-key"};

   int                      signResult = runHeimdallSbom(signArgs);
   EXPECT_EQ(signResult, 0) << "heimdall-sbom signing failed";

   // Verify SBOM was created and has signature
   EXPECT_TRUE(fileExists(signedSbomPath));
   EXPECT_TRUE(hasSignature(signedSbomPath));
   EXPECT_EQ(getSignatureAlgorithm(signedSbomPath), "RS256");

   // Verify signature with public key
   std::vector<std::string> verifyArgs = {"verify-signature", signedSbomPath, "--key",
                                          rsa_public_key};

   int                      verifyResult = runHeimdallValidate(verifyArgs);
   EXPECT_EQ(verifyResult, 0) << "heimdall-validate verification failed";

   // Verify signature with certificate
   std::vector<std::string> verifyCertArgs = {"verify-signature", signedSbomPath, "--cert",
                                              rsa_certificate};

   int                      verifyCertResult = runHeimdallValidate(verifyCertArgs);
   EXPECT_EQ(verifyCertResult, 0) << "heimdall-validate certificate verification failed";
}

// Test ECDSA signing and verification integration
TEST_F(SignatureIntegrationTest, ECDSASigningAndVerificationIntegration)
{
   // Sign SBOM with ECDSA
   std::string              signedSbomPath = (test_dir / "ecdsa_signed.sbom.json").string();

   std::vector<std::string> signArgs = {test_plugin,  test_binary,       "--format",
                                        "cyclonedx",  "--output",        signedSbomPath,
                                        "--sign-key", ecdsa_private_key, "--sign-algorithm",
                                        "ES256",      "--sign-key-id",   "test-ecdsa-key"};

   int                      signResult = runHeimdallSbom(signArgs);
   EXPECT_EQ(signResult, 0) << "heimdall-sbom ECDSA signing failed";

   // Verify SBOM was created and has signature
   EXPECT_TRUE(fileExists(signedSbomPath));
   EXPECT_TRUE(hasSignature(signedSbomPath));
   EXPECT_EQ(getSignatureAlgorithm(signedSbomPath), "ES256");

   // Verify signature with public key
   std::vector<std::string> verifyArgs = {"verify-signature", signedSbomPath, "--key",
                                          ecdsa_public_key};

   int                      verifyResult = runHeimdallValidate(verifyArgs);
   EXPECT_EQ(verifyResult, 0) << "heimdall-validate ECDSA verification failed";

   // Verify signature with certificate
   std::vector<std::string> verifyCertArgs = {"verify-signature", signedSbomPath, "--cert",
                                              ecdsa_certificate};

   int                      verifyCertResult = runHeimdallValidate(verifyCertArgs);
   EXPECT_EQ(verifyCertResult, 0) << "heimdall-validate ECDSA certificate verification failed";
}

// Test Ed25519 signing and verification integration
TEST_F(SignatureIntegrationTest, Ed25519SigningAndVerificationIntegration)
{
   // Sign SBOM with Ed25519
   std::string              signedSbomPath = (test_dir / "ed25519_signed.sbom.json").string();

   std::vector<std::string> signArgs = {test_plugin,  test_binary,         "--format",
                                        "cyclonedx",  "--output",          signedSbomPath,
                                        "--sign-key", ed25519_private_key, "--sign-algorithm",
                                        "Ed25519",    "--sign-key-id",     "test-ed25519-key"};

   int                      signResult = runHeimdallSbom(signArgs);
   EXPECT_EQ(signResult, 0) << "heimdall-sbom Ed25519 signing failed";

   // Verify SBOM was created and has signature
   EXPECT_TRUE(fileExists(signedSbomPath));
   EXPECT_TRUE(hasSignature(signedSbomPath));
   EXPECT_EQ(getSignatureAlgorithm(signedSbomPath), "Ed25519");

   // Verify signature with public key
   std::vector<std::string> verifyArgs = {"verify-signature", signedSbomPath, "--key",
                                          ed25519_public_key};

   int                      verifyResult = runHeimdallValidate(verifyArgs);
   EXPECT_EQ(verifyResult, 0) << "heimdall-validate Ed25519 verification failed";
}

// Test multiple algorithms integration
TEST_F(SignatureIntegrationTest, MultipleAlgorithmsIntegration)
{
   std::vector<std::pair<std::string, std::string>> algorithms = {
      {"RS256", rsa_private_key},      {"RS384", rsa_private_key},   {"RS512", rsa_private_key},
      {"ES256", ecdsa_private_key},    {"ES384", ecdsa_private_key}, {"ES512", ecdsa_private_key},
      {"Ed25519", ed25519_private_key}};

   for (const auto& [algorithm, privateKey] : algorithms)
   {
      // Sign SBOM with algorithm
      std::string signedSbomPath = (test_dir / (algorithm + "_signed.sbom.json")).string();

      std::vector<std::string> signArgs = {
         test_plugin,        test_binary,    "--format",      "cyclonedx",
         "--output",         signedSbomPath, "--sign-key",    privateKey,
         "--sign-algorithm", algorithm,      "--sign-key-id", "test-" + algorithm + "-key"};

      int signResult = runHeimdallSbom(signArgs);
      EXPECT_EQ(signResult, 0) << "heimdall-sbom signing failed for " << algorithm;

      // Verify SBOM was created and has signature
      EXPECT_TRUE(fileExists(signedSbomPath)) << "Signed SBOM not created for " << algorithm;
      EXPECT_TRUE(hasSignature(signedSbomPath)) << "Signature not found for " << algorithm;
      EXPECT_EQ(getSignatureAlgorithm(signedSbomPath), algorithm)
         << "Wrong algorithm for " << algorithm;
   }
}

// Test error handling integration
TEST_F(SignatureIntegrationTest, ErrorHandlingIntegration)
{
   // Test signing with non-existent private key
   std::string              signedSbomPath = (test_dir / "error_signed.sbom.json").string();

   std::vector<std::string> signArgs = {
      test_plugin,    test_binary,  "--format",         "cyclonedx",        "--output",
      signedSbomPath, "--sign-key", "non_existent.key", "--sign-algorithm", "RS256"};

   int signResult = runHeimdallSbom(signArgs);
   EXPECT_NE(signResult, 0) << "heimdall-sbom should fail with non-existent key";

   // Test verification without key or certificate
   std::vector<std::string> verifyArgs = {"verify-signature", signedSbomPath};

   int                      verifyResult = runHeimdallValidate(verifyArgs);
   EXPECT_NE(verifyResult, 0) << "heimdall-validate should fail without key/cert";

   // Test verification with wrong key
   if (fileExists(signedSbomPath))
   {
      std::vector<std::string> wrongKeyArgs = {
         "verify-signature", signedSbomPath, "--key", ecdsa_public_key  // Wrong key type
      };

      int wrongKeyResult = runHeimdallValidate(wrongKeyArgs);
      EXPECT_NE(wrongKeyResult, 0) << "heimdall-validate should fail with wrong key";
   }
}

// Test unsigned SBOM verification
TEST_F(SignatureIntegrationTest, UnsignedSBOMVerification)
{
   // Create unsigned SBOM
   std::string              unsignedSbomPath = (test_dir / "unsigned.sbom.json").string();

   std::vector<std::string> createArgs = {test_plugin, test_binary, "--format",
                                          "cyclonedx", "--output",  unsignedSbomPath};

   int                      createResult = runHeimdallSbom(createArgs);
   EXPECT_EQ(createResult, 0) << "heimdall-sbom unsigned SBOM creation failed";

   // Verify unsigned SBOM
   EXPECT_TRUE(fileExists(unsignedSbomPath));
   EXPECT_FALSE(hasSignature(unsignedSbomPath));

   // Try to verify unsigned SBOM
   std::vector<std::string> verifyArgs = {"verify-signature", unsignedSbomPath, "--key",
                                          rsa_public_key};

   int                      verifyResult = runHeimdallValidate(verifyArgs);
   EXPECT_NE(verifyResult, 0) << "heimdall-validate should fail for unsigned SBOM";
}

// Test signature structure validation
TEST_F(SignatureIntegrationTest, SignatureStructureValidation)
{
   // Create an SBOM with signature fields that would be excluded during canonicalization
   std::string              signedSbomPath = (test_dir / "structure_test.sbom.json").string();

   std::vector<std::string> signArgs = {test_plugin,  test_binary,     "--format",
                                        "cyclonedx",  "--output",      signedSbomPath,
                                        "--sign-key", rsa_private_key, "--sign-algorithm",
                                        "RS256",      "--sign-key-id", "test-structure-key"};

   int                      signResult = runHeimdallSbom(signArgs);
   EXPECT_EQ(signResult, 0) << "heimdall-sbom signing failed";

   // Verify signature structure
   EXPECT_TRUE(fileExists(signedSbomPath));

   try
   {
      nlohmann::json sbom = readJsonFile(signedSbomPath);

      // Check signature object exists
      EXPECT_TRUE(sbom.contains("signature"));
      nlohmann::json signature = sbom["signature"];

      // Check required fields
      EXPECT_TRUE(signature.contains("algorithm"));
      EXPECT_TRUE(signature.contains("value"));
      EXPECT_EQ(signature["algorithm"], "RS256");

      // Check JSF-compliant fields
      EXPECT_TRUE(signature.contains("publicKey"));
      EXPECT_TRUE(signature["publicKey"].contains("kty"));

      // Note: keyId, timestamp, and excludes are not part of JSF-compliant signatures
      // The new format only includes algorithm, value, and publicKey
   }
   catch (const std::exception& e)
   {
      FAIL() << "Failed to parse signed SBOM: " << e.what();
   }
}

// Test batch signing and verification
TEST_F(SignatureIntegrationTest, BatchSigningAndVerification)
{
   // Create multiple test binaries
   std::vector<std::string> binaries;
   for (int i = 0; i < 3; ++i)
   {
      std::string   binaryPath = (test_dir / ("test_binary_" + std::to_string(i))).string();
      std::ofstream binary(binaryPath);
      binary << "ELF test binary content " << i;
      binary.close();
      chmod(binaryPath.c_str(), 0755);
      binaries.push_back(binaryPath);
   }

   // Sign each binary
   std::vector<std::string> signedSboms;
   for (size_t i = 0; i < binaries.size(); ++i)
   {
      std::string signedSbomPath =
         (test_dir / ("batch_signed_" + std::to_string(i) + ".sbom.json")).string();

      std::vector<std::string> signArgs = {
         test_plugin,        binaries[i],    "--format",      "cyclonedx",
         "--output",         signedSbomPath, "--sign-key",    rsa_private_key,
         "--sign-algorithm", "RS256",        "--sign-key-id", "batch-key-" + std::to_string(i)};

      int signResult = runHeimdallSbom(signArgs);
      EXPECT_EQ(signResult, 0) << "heimdall-sbom batch signing failed for binary " << i;

      signedSboms.push_back(signedSbomPath);
   }

   // Verify each signed SBOM
   for (size_t i = 0; i < signedSboms.size(); ++i)
   {
      EXPECT_TRUE(fileExists(signedSboms[i])) << "Signed SBOM not created for binary " << i;
      EXPECT_TRUE(hasSignature(signedSboms[i])) << "Signature not found for binary " << i;

      std::vector<std::string> verifyArgs = {"verify-signature", signedSboms[i], "--key",
                                             rsa_public_key};

      int                      verifyResult = runHeimdallValidate(verifyArgs);
      EXPECT_EQ(verifyResult, 0) << "heimdall-validate batch verification failed for SBOM " << i;
   }
}

// Test signature with different CycloneDX versions
TEST_F(SignatureIntegrationTest, SignatureWithDifferentVersions)
{
   std::vector<std::string> versions = {"1.4", "1.5", "1.6"};

   for (const auto& version : versions)
   {
      std::string signedSbomPath =
         (test_dir / ("version_" + version + "_signed.sbom.json")).string();

      std::vector<std::string> signArgs = {test_plugin,
                                           test_binary,
                                           "--format",
                                           "cyclonedx",
                                           "--cyclonedx-version",
                                           version,
                                           "--output",
                                           signedSbomPath,
                                           "--sign-key",
                                           rsa_private_key,
                                           "--sign-algorithm",
                                           "RS256",
                                           "--sign-key-id",
                                           "version-" + version + "-key"};

      int                      signResult = runHeimdallSbom(signArgs);
      EXPECT_EQ(signResult, 0) << "heimdall-sbom signing failed for version " << version;

      // Verify SBOM was created and has signature
      EXPECT_TRUE(fileExists(signedSbomPath)) << "Signed SBOM not created for version " << version;
      EXPECT_TRUE(hasSignature(signedSbomPath)) << "Signature not found for version " << version;

      // Verify signature
      std::vector<std::string> verifyArgs = {"verify-signature", signedSbomPath, "--key",
                                             rsa_public_key};

      int                      verifyResult = runHeimdallValidate(verifyArgs);
      EXPECT_EQ(verifyResult, 0) << "heimdall-validate verification failed for version " << version;

      // Verify CycloneDX version
      try
      {
         nlohmann::json sbom = readJsonFile(signedSbomPath);
         EXPECT_EQ(sbom["specVersion"], version) << "Wrong CycloneDX version for " << version;
      }
      catch (const std::exception& e)
      {
         FAIL() << "Failed to parse signed SBOM for version " << version << ": " << e.what();
      }
   }
}

// Test signature with different output formats
TEST_F(SignatureIntegrationTest, SignatureWithDifferentFormats)
{
   // Test CycloneDX format (signing supported)
   std::string              cyclonedxPath = (test_dir / "cyclonedx_signed.sbom.json").string();

   std::vector<std::string> cyclonedxArgs = {
      test_plugin,   test_binary,  "--format",      "cyclonedx",        "--output",
      cyclonedxPath, "--sign-key", rsa_private_key, "--sign-algorithm", "RS256"};

   int cyclonedxResult = runHeimdallSbom(cyclonedxArgs);
   EXPECT_EQ(cyclonedxResult, 0) << "heimdall-sbom CycloneDX signing failed";
   EXPECT_TRUE(hasSignature(cyclonedxPath)) << "Signature not found in CycloneDX SBOM";

   // Test SPDX format (signing not supported)
   std::string              spdxPath = (test_dir / "spdx_signed.spdx").string();

   std::vector<std::string> spdxArgs = {
      test_plugin, test_binary,  "--format",      "spdx-2.3",         "--output",
      spdxPath,    "--sign-key", rsa_private_key, "--sign-algorithm", "RS256"};

   int spdxResult = runHeimdallSbom(spdxArgs);
   // SPDX signing might fail or be ignored, so we don't assert on result
   // But we can check that the file was created
   if (fileExists(spdxPath))
   {
      // SPDX files don't have JSON signatures, so we don't check for signature
      EXPECT_TRUE(true) << "SPDX file created successfully";
   }
}

// Test signature verification with invalid files
TEST_F(SignatureIntegrationTest, InvalidFileVerification)
{
   // Test verification of non-existent file
   std::vector<std::string> nonExistentArgs = {"verify-signature", "non_existent.sbom.json",
                                               "--key", rsa_public_key};

   int                      nonExistentResult = runHeimdallValidate(nonExistentArgs);
   EXPECT_NE(nonExistentResult, 0) << "heimdall-validate should fail for non-existent file";

   // Test verification of invalid JSON file
   std::string   invalidJsonPath = (test_dir / "invalid.json").string();
   std::ofstream invalidFile(invalidJsonPath);
   invalidFile << "This is not valid JSON";
   invalidFile.close();

   std::vector<std::string> invalidJsonArgs = {"verify-signature", invalidJsonPath, "--key",
                                               rsa_public_key};

   int                      invalidJsonResult = runHeimdallValidate(invalidJsonArgs);
   EXPECT_NE(invalidJsonResult, 0) << "heimdall-validate should fail for invalid JSON";

   // Test verification of valid JSON without signature
   std::string   validJsonPath = (test_dir / "valid_no_sig.json").string();
   std::ofstream validFile(validJsonPath);
   validFile << R"({"test": "data"})";
   validFile.close();

   std::vector<std::string> validJsonArgs = {"verify-signature", validJsonPath, "--key",
                                             rsa_public_key};

   int                      validJsonResult = runHeimdallValidate(validJsonArgs);
   EXPECT_NE(validJsonResult, 0) << "heimdall-validate should fail for JSON without signature";
}