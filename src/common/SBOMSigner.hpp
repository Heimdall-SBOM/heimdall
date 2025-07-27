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
 * @file SBOMSigner.hpp
 * @brief SBOM signing functionality using JSON Signature Format (JSF)
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides functionality for signing SBOM documents using the
 * JSON Signature Format (JSF) as specified in CycloneDX 1.6+.
 * 
 * Supported signing methods:
 * - RSA signatures
 * - ECDSA signatures
 * - Ed25519 signatures
 */

#pragma once

#include <string>
#include <memory>
#include "../compat/compatibility.hpp"

namespace heimdall
{

/**
 * @brief Enumeration of supported signature algorithms
 */
enum class SignatureAlgorithm
{
   RS256,    ///< RSA with SHA-256
   RS384,    ///< RSA with SHA-384
   RS512,    ///< RSA with SHA-512
   ES256,    ///< ECDSA with SHA-256
   ES384,    ///< ECDSA with SHA-384
   ES512,    ///< ECDSA with SHA-512
   Ed25519   ///< Ed25519 signature
};

/**
 * @brief Structure representing signature information
 */
struct SignatureInfo
{
   std::string algorithm;     ///< Signature algorithm (e.g., "RS256")
   std::string keyId;         ///< Key identifier
   std::string signature;     ///< Base64-encoded signature
   std::string certificate;   ///< Base64-encoded certificate (optional)
   std::string timestamp;     ///< ISO 8601 timestamp
};

/**
 * @brief Main class for signing SBOM documents
 */
class SBOMSigner
{
   public:
   /**
    * @brief Default constructor
    */
   SBOMSigner();

   /**
    * @brief Destructor
    */
   ~SBOMSigner();

   /**
    * @brief Load private key from file
    * @param keyPath Path to the private key file
    * @param password Optional password for encrypted keys
    * @return true if key was loaded successfully
    */
   bool loadPrivateKey(const std::string& keyPath, const std::string& password = "");

   /**
    * @brief Load certificate from file
    * @param certPath Path to the certificate file
    * @return true if certificate was loaded successfully
    */
   bool loadCertificate(const std::string& certPath);

   /**
    * @brief Set signature algorithm
    * @param algorithm The signature algorithm to use
    */
   void setSignatureAlgorithm(SignatureAlgorithm algorithm);

   /**
    * @brief Set key identifier
    * @param keyId The key identifier
    */
   void setKeyId(const std::string& keyId);

   /**
    * @brief Sign an SBOM document
    * @param sbomContent The SBOM content to sign
    * @param signatureInfo Output signature information
    * @return true if signing was successful
    */
   bool signSBOM(const std::string& sbomContent, SignatureInfo& signatureInfo);

   /**
    * @brief Add signature to CycloneDX SBOM
    * @param sbomContent The SBOM content
    * @param signatureInfo The signature information
    * @return Signed SBOM content
    */
   std::string addSignatureToCycloneDX(const std::string& sbomContent, const SignatureInfo& signatureInfo);

   /**
    * @brief Verify signature in SBOM
    * @param sbomContent The SBOM content with signature
    * @return true if signature is valid
    */
   bool verifySignature(const std::string& sbomContent);

   /**
    * @brief Extract signature from SBOM
    * @param sbomContent The SBOM content
    * @param signatureInfo Output signature information
    * @return true if signature was extracted successfully
    */
   bool extractSignature(const std::string& sbomContent, SignatureInfo& signatureInfo);

   /**
    * @brief Get last error message
    * @return Error message
    */
   std::string getLastError() const;

   private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

}  // namespace heimdall 