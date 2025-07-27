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
 * @file SBOMSigner.cpp
 * @brief Implementation of SBOM signing functionality
 * @author Trevor Bakker
 * @date 2025
 */

#include "SBOMSigner.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>
#include "Utils.hpp"

// OpenSSL includes
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>

namespace heimdall
{

/**
 * @brief Private implementation class for SBOMSigner
 */
class SBOMSigner::Impl
{
   public:
   SignatureAlgorithm algorithm = SignatureAlgorithm::RS256;
   std::string        keyId;
   std::string        lastError;
   
   // OpenSSL objects
   EVP_PKEY*          privateKey = nullptr;
   X509*              certificate = nullptr;
   
   /**
    * @brief Constructor
    */
   Impl() = default;
   
   /**
    * @brief Destructor
    */
   ~Impl()
   {
      if (privateKey) {
         EVP_PKEY_free(privateKey);
      }
      if (certificate) {
         X509_free(certificate);
      }
   }
   
   /**
    * @brief Get current timestamp in ISO 8601 format
    * @return ISO 8601 timestamp string
    */
   std::string getCurrentTimestamp()
   {
      auto now = std::chrono::system_clock::now();
      auto time_t = std::chrono::system_clock::to_time_t(now);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
         now.time_since_epoch()) % 1000;
      
      std::stringstream ss;
      ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
      ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
      return ss.str();
   }
   
   /**
    * @brief Convert signature algorithm to string
    * @param algo The signature algorithm
    * @return Algorithm string
    */
   std::string algorithmToString(SignatureAlgorithm algo)
   {
      switch (algo) {
         case SignatureAlgorithm::RS256: return "RS256";
         case SignatureAlgorithm::RS384: return "RS384";
         case SignatureAlgorithm::RS512: return "RS512";
         case SignatureAlgorithm::ES256: return "ES256";
         case SignatureAlgorithm::ES384: return "ES384";
         case SignatureAlgorithm::ES512: return "ES512";
         case SignatureAlgorithm::Ed25519: return "Ed25519";
         default: return "RS256";
      }
   }
   
   /**
    * @brief Get digest algorithm for signature algorithm
    * @param algo The signature algorithm
    * @return OpenSSL digest algorithm
    */
   const EVP_MD* getDigestAlgorithm(SignatureAlgorithm algo)
   {
      switch (algo) {
         case SignatureAlgorithm::RS256:
         case SignatureAlgorithm::ES256:
            return EVP_sha256();
         case SignatureAlgorithm::RS384:
         case SignatureAlgorithm::ES384:
            return EVP_sha384();
         case SignatureAlgorithm::RS512:
         case SignatureAlgorithm::ES512:
            return EVP_sha512();
         case SignatureAlgorithm::Ed25519:
            return nullptr;  // Ed25519 doesn't use separate digest
         default:
            return EVP_sha256();
      }
   }
   
   /**
    * @brief Base64 encode data
    * @param data Input data
    * @param length Data length
    * @return Base64 encoded string
    */
   std::string base64Encode(const unsigned char* data, size_t length)
   {
      BIO* bio = BIO_new(BIO_s_mem());
      BIO* b64 = BIO_new(BIO_f_base64());
      bio = BIO_push(b64, bio);
      
      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      BIO_write(bio, data, static_cast<int>(length));
      BIO_flush(bio);
      
      BUF_MEM* bufferPtr;
      BIO_get_mem_ptr(bio, &bufferPtr);
      
      std::string result(bufferPtr->data, bufferPtr->length);
      BIO_free_all(bio);
      
      return result;
   }
   
   /**
    * @brief Base64 decode string
    * @param input Base64 encoded string
    * @return Decoded data
    */
   std::vector<unsigned char> base64Decode(const std::string& input)
   {
      BIO* bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
      BIO* b64 = BIO_new(BIO_f_base64());
      bio = BIO_push(b64, bio);
      
      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      
      std::vector<unsigned char> buffer(input.length());
      int decodedLength = BIO_read(bio, buffer.data(), static_cast<int>(buffer.size()));
      BIO_free_all(bio);
      
      if (decodedLength > 0) {
         buffer.resize(decodedLength);
      } else {
         buffer.clear();
      }
      
      return buffer;
   }
   
   /**
    * @brief Sign data using loaded private key
    * @param data Data to sign
    * @param dataLength Length of data
    * @param signature Output signature
    * @return true if signing was successful
    */
   bool signData(const unsigned char* data, size_t dataLength, std::string& signature)
   {
      if (!privateKey) {
         lastError = "No private key loaded";
         return false;
      }
      
      EVP_MD_CTX* ctx = EVP_MD_CTX_new();
      if (!ctx) {
         lastError = "Failed to create signature context";
         return false;
      }
      
      bool success = false;
      
      if (algorithm == SignatureAlgorithm::Ed25519) {
         // Ed25519 signing
         if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, privateKey) != 1) {
            lastError = "Failed to initialize Ed25519 signing";
         } else {
            size_t sigLen = 0;
            if (EVP_DigestSign(ctx, nullptr, &sigLen, data, dataLength) != 1) {
               lastError = "Failed to get Ed25519 signature length";
            } else {
               std::vector<unsigned char> sig(sigLen);
               if (EVP_DigestSign(ctx, sig.data(), &sigLen, data, dataLength) == 1) {
                  signature = base64Encode(sig.data(), sigLen);
                  success = true;
               } else {
                  lastError = "Failed to create Ed25519 signature";
               }
            }
         }
      } else {
         // RSA/ECDSA signing
         const EVP_MD* md = getDigestAlgorithm(algorithm);
         if (!md) {
            lastError = "Unsupported signature algorithm";
         } else if (EVP_DigestSignInit(ctx, nullptr, md, nullptr, privateKey) != 1) {
            lastError = "Failed to initialize signature context";
         } else {
            size_t sigLen = 0;
            if (EVP_DigestSign(ctx, nullptr, &sigLen, data, dataLength) != 1) {
               lastError = "Failed to get signature length";
            } else {
               std::vector<unsigned char> sig(sigLen);
               if (EVP_DigestSign(ctx, sig.data(), &sigLen, data, dataLength) == 1) {
                  signature = base64Encode(sig.data(), sigLen);
                  success = true;
               } else {
                  lastError = "Failed to create signature";
               }
            }
         }
      }
      
      EVP_MD_CTX_free(ctx);
      return success;
   }
};

// Public interface implementation

SBOMSigner::SBOMSigner() : pImpl(heimdall::compat::make_unique<Impl>()) {}

SBOMSigner::~SBOMSigner() = default;

bool SBOMSigner::loadPrivateKey(const std::string& keyPath, const std::string& password)
{
   std::ifstream keyFile(keyPath);
   if (!keyFile.is_open()) {
      pImpl->lastError = "Failed to open private key file: " + keyPath;
      return false;
   }
   
   std::string keyContent((std::istreambuf_iterator<char>(keyFile)),
                         std::istreambuf_iterator<char>());
   keyFile.close();
   
   BIO* bio = BIO_new_mem_buf(keyContent.c_str(), static_cast<int>(keyContent.length()));
   if (!bio) {
      pImpl->lastError = "Failed to create BIO for private key";
      return false;
   }
   
   EVP_PKEY* key = nullptr;
   if (!password.empty()) {
      key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, 
                                   const_cast<void*>(static_cast<const void*>(password.c_str())));
   } else {
      key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
   }
   
   BIO_free(bio);
   
   if (!key) {
      pImpl->lastError = "Failed to load private key (check password if encrypted)";
      return false;
   }
   
   if (pImpl->privateKey) {
      EVP_PKEY_free(pImpl->privateKey);
   }
   pImpl->privateKey = key;
   
   return true;
}

bool SBOMSigner::loadCertificate(const std::string& certPath)
{
   std::ifstream certFile(certPath);
   if (!certFile.is_open()) {
      pImpl->lastError = "Failed to open certificate file: " + certPath;
      return false;
   }
   
   std::string certContent((std::istreambuf_iterator<char>(certFile)),
                          std::istreambuf_iterator<char>());
   certFile.close();
   
   BIO* bio = BIO_new_mem_buf(certContent.c_str(), static_cast<int>(certContent.length()));
   if (!bio) {
      pImpl->lastError = "Failed to create BIO for certificate";
      return false;
   }
   
   X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
   BIO_free(bio);
   
   if (!cert) {
      pImpl->lastError = "Failed to load certificate";
      return false;
   }
   
   if (pImpl->certificate) {
      X509_free(pImpl->certificate);
   }
   pImpl->certificate = cert;
   
   return true;
}

void SBOMSigner::setSignatureAlgorithm(SignatureAlgorithm algorithm)
{
   pImpl->algorithm = algorithm;
}

void SBOMSigner::setKeyId(const std::string& keyId)
{
   pImpl->keyId = keyId;
}

bool SBOMSigner::signSBOM(const std::string& sbomContent, SignatureInfo& signatureInfo)
{
   // Create canonical JSON for signing (remove signature field if present)
   nlohmann::json sbomJson;
   try {
      sbomJson = nlohmann::json::parse(sbomContent);
   } catch (const std::exception& e) {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return false;
   }
   
   // Remove existing signature if present
   if (sbomJson.contains("signature")) {
      sbomJson.erase("signature");
   }
   
   // Create canonical JSON string
   std::string canonicalJson = sbomJson.dump();
   
   // Sign the canonical JSON
   std::string signature;
   if (!pImpl->signData(reinterpret_cast<const unsigned char*>(canonicalJson.c_str()),
                       canonicalJson.length(), signature)) {
      return false;
   }
   
   // Fill signature info
   signatureInfo.algorithm = pImpl->algorithmToString(pImpl->algorithm);
   signatureInfo.keyId = pImpl->keyId;
   signatureInfo.signature = signature;
   signatureInfo.timestamp = pImpl->getCurrentTimestamp();
   
   // Add certificate if available
   if (pImpl->certificate) {
      BIO* bio = BIO_new(BIO_s_mem());
      if (PEM_write_bio_X509(bio, pImpl->certificate)) {
         BUF_MEM* bufferPtr;
         BIO_get_mem_ptr(bio, &bufferPtr);
         signatureInfo.certificate = std::string(bufferPtr->data, bufferPtr->length);
      }
      BIO_free(bio);
   }
   
   return true;
}

std::string SBOMSigner::addSignatureToCycloneDX(const std::string& sbomContent, const SignatureInfo& signatureInfo)
{
   nlohmann::json sbomJson;
   try {
      sbomJson = nlohmann::json::parse(sbomContent);
   } catch (const std::exception& e) {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return sbomContent;
   }
   
   // Create signature object
   nlohmann::json signatureJson;
   signatureJson["algorithm"] = signatureInfo.algorithm;
   signatureJson["keyId"] = signatureInfo.keyId;
   signatureJson["signature"] = signatureInfo.signature;
   signatureJson["timestamp"] = signatureInfo.timestamp;
   
   if (!signatureInfo.certificate.empty()) {
      signatureJson["certificate"] = signatureInfo.certificate;
   }
   
   // Add signature to SBOM
   sbomJson["signature"] = signatureJson;
   
   return sbomJson.dump(2);
}

bool SBOMSigner::verifySignature(const std::string& sbomContent)
{
   SignatureInfo signatureInfo;
   if (!extractSignature(sbomContent, signatureInfo)) {
      return false;
   }
   
   // TODO: Implement signature verification
   // This would require loading the public key and verifying the signature
   pImpl->lastError = "Signature verification not yet implemented";
   return false;
}

bool SBOMSigner::extractSignature(const std::string& sbomContent, SignatureInfo& signatureInfo)
{
   nlohmann::json sbomJson;
   try {
      sbomJson = nlohmann::json::parse(sbomContent);
   } catch (const std::exception& e) {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return false;
   }
   
   if (!sbomJson.contains("signature")) {
      pImpl->lastError = "No signature found in SBOM";
      return false;
   }
   
   nlohmann::json sigJson = sbomJson["signature"];
   
   signatureInfo.algorithm = sigJson.value("algorithm", "");
   signatureInfo.keyId = sigJson.value("keyId", "");
   signatureInfo.signature = sigJson.value("signature", "");
   signatureInfo.timestamp = sigJson.value("timestamp", "");
   signatureInfo.certificate = sigJson.value("certificate", "");
   
   return true;
}

std::string SBOMSigner::getLastError() const
{
   return pImpl->lastError;
}

}  // namespace heimdall 