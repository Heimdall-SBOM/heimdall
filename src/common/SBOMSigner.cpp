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
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>
#include "Utils.hpp"

// OpenSSL includes
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
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
   EVP_PKEY* privateKey  = nullptr;
   EVP_PKEY* publicKey   = nullptr;
   X509*     certificate = nullptr;

   /**
    * @brief Constructor
    */
   Impl() = default;

   /**
    * @brief Destructor
    */
   ~Impl()
   {
      if (privateKey)
      {
         EVP_PKEY_free(privateKey);
      }
      if (publicKey)
      {
         EVP_PKEY_free(publicKey);
      }
      if (certificate)
      {
         X509_free(certificate);
      }
   }

   // Disable copy constructor and copy assignment operator for proper resource management
   Impl(const Impl&)            = delete;
   Impl& operator=(const Impl&) = delete;

   // Enable move constructor and move assignment operator
   Impl(Impl&&)            = default;
   Impl& operator=(Impl&&) = default;

   /**
    * @brief Get current timestamp in ISO 8601 format
    * @return ISO 8601 timestamp string
    */
   std::string getCurrentTimestamp() const
   {
      auto now    = std::chrono::system_clock::now();
      auto time_t = std::chrono::system_clock::to_time_t(now);
      auto ms =
         std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

      std::stringstream ss;
      struct tm         utc_tm = {};
#ifdef _WIN32
      gmtime_s(&utc_tm, &time_t);
#else
      gmtime_r(&time_t, &utc_tm);
#endif
      ss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S");
      ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
      return ss.str();
   }

   /**
    * @brief Convert signature algorithm to string
    * @param algo The signature algorithm
    * @return Algorithm string
    */
   std::string algorithmToString(SignatureAlgorithm algo) const
   {
      switch (algo)
      {
         case SignatureAlgorithm::RS256:
            return "RS256";
         case SignatureAlgorithm::RS384:
            return "RS384";
         case SignatureAlgorithm::RS512:
            return "RS512";
         case SignatureAlgorithm::ES256:
            return "ES256";
         case SignatureAlgorithm::ES384:
            return "ES384";
         case SignatureAlgorithm::ES512:
            return "ES512";
         case SignatureAlgorithm::Ed25519:
            return "Ed25519";
         default:
            return "RS256";
      }
   }

   /**
    * @brief Get digest algorithm for signature algorithm
    * @param algo The signature algorithm
    * @return OpenSSL digest algorithm
    */
   const EVP_MD* getDigestAlgorithm(SignatureAlgorithm algo) const
   {
      switch (algo)
      {
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
   std::string base64Encode(const unsigned char* data, size_t length) const
   {
      BIO* bio = BIO_new(BIO_s_mem());
      BIO* b64 = BIO_new(BIO_f_base64());
      bio      = BIO_push(b64, bio);

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
   std::vector<unsigned char> base64Decode(const std::string& input) const
   {
      BIO* bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
      BIO* b64 = BIO_new(BIO_f_base64());
      bio      = BIO_push(b64, bio);

      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

      std::vector<unsigned char> buffer(input.length());
      int decodedLength = BIO_read(bio, buffer.data(), static_cast<int>(buffer.size()));
      BIO_free_all(bio);

      if (decodedLength > 0)
      {
         buffer.resize(decodedLength);
      }
      else
      {
         buffer.clear();
      }

      return buffer;
   }

   /**
    * @brief Base64URL encode data (Base64 without padding and with URL-safe characters)
    * @param data Input data
    * @param length Data length
    * @return Base64URL encoded string
    */
   std::string base64URLEncode(const unsigned char* data, size_t length) const
   {
      std::string base64 = base64Encode(data, length);
      // Remove padding
      while (!base64.empty() && base64.back() == '=')
      {
         base64.pop_back();
      }
      // Replace URL-unsafe characters
      std::replace(base64.begin(), base64.end(), '+', '-');
      std::replace(base64.begin(), base64.end(), '/', '_');
      return base64;
   }

   /**
    * @brief Base64URL decode string (Base64 without padding and with URL-safe characters)
    * @param input Base64URL encoded string
    * @return Decoded data
    */
   std::vector<unsigned char> base64URLDecode(const std::string& input) const
   {
      std::string base64 = input;
      // Replace URL-safe characters back to standard Base64
      std::replace(base64.begin(), base64.end(), '-', '+');
      std::replace(base64.begin(), base64.end(), '_', '/');

      // Add padding if needed
      while (base64.length() % 4 != 0)
      {
         base64 += '=';
      }

      return base64Decode(base64);
   }

   /**
    * @brief Convert BIGNUM to Base64URL string
    * @param bn BIGNUM to convert
    * @return Base64URL encoded string
    */
   std::string bignumToBase64URL(const BIGNUM* bn) const
   {
      if (!bn)
      {
         return "";
      }

      int                        numBytes = BN_num_bytes(bn);
      std::vector<unsigned char> buffer(numBytes);
      BN_bn2bin(bn, buffer.data());

      // Remove leading zeros (JWK requirement)
      size_t start = 0;
      while (start < buffer.size() && buffer[start] == 0)
      {
         start++;
      }
      if (start == buffer.size())
      {
         return "";
      }

      return base64URLEncode(buffer.data() + start, buffer.size() - start);
   }

   /**
    * @brief Verify data signature using loaded public key
    * @param data Data that was signed
    * @param dataLength Length of data
    * @param signature Signature to verify
    * @param algorithm Signature algorithm
    * @return true if signature is valid
    */
   bool verifyData(const unsigned char* data, size_t dataLength, const std::string& signature,
                   SignatureAlgorithm algo)
   {
      if (!publicKey)
      {
         lastError = "No public key loaded for verification";
         return false;
      }

      std::vector<unsigned char> sigData = base64URLDecode(signature);
      if (sigData.empty())
      {
         lastError = "Failed to decode signature";
         return false;
      }

      EVP_MD_CTX* ctx = EVP_MD_CTX_new();
      if (!ctx)
      {
         lastError = "Failed to create verification context";
         return false;
      }

      bool success = false;

      if (algo == SignatureAlgorithm::Ed25519)
      {
         // Ed25519 verification
         if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, publicKey) != 1)
         {
            lastError = "Failed to initialize Ed25519 verification";
         }
         else
         {
            success =
               (EVP_DigestVerify(ctx, sigData.data(), sigData.size(), data, dataLength) == 1);
            if (!success)
            {
               lastError = "Ed25519 signature verification failed";
            }
         }
      }
      else
      {
         // RSA/ECDSA verification
         const EVP_MD* md = getDigestAlgorithm(algo);
         if (!md)
         {
            lastError = "Unsupported signature algorithm for verification";
         }
         else if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, publicKey) != 1)
         {
            lastError = "Failed to initialize verification context";
         }
         else
         {
            success =
               (EVP_DigestVerify(ctx, sigData.data(), sigData.size(), data, dataLength) == 1);
            if (!success)
            {
               lastError = "Signature verification failed";
            }
         }
      }

      EVP_MD_CTX_free(ctx);
      return success;
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
      if (!privateKey)
      {
         lastError = "No private key loaded";
         return false;
      }

      EVP_MD_CTX* ctx = EVP_MD_CTX_new();
      if (!ctx)
      {
         lastError = "Failed to create signature context";
         return false;
      }

      bool success = false;

      if (algorithm == SignatureAlgorithm::Ed25519)
      {
         // Ed25519 signing
         if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, privateKey) != 1)
         {
            lastError = "Failed to initialize Ed25519 signing";
         }
         else
         {
            size_t sigLen = 0;
            if (EVP_DigestSign(ctx, nullptr, &sigLen, data, dataLength) != 1)
            {
               lastError = "Failed to get Ed25519 signature length";
            }
            else
            {
               std::vector<unsigned char> sig(sigLen);
               if (EVP_DigestSign(ctx, sig.data(), &sigLen, data, dataLength) == 1)
               {
                  signature = base64URLEncode(sig.data(), sigLen);
                  success   = true;
               }
               else
               {
                  lastError = "Failed to create Ed25519 signature";
               }
            }
         }
      }
      else
      {
         // RSA/ECDSA signing
         const EVP_MD* md = getDigestAlgorithm(algorithm);
         if (!md)
         {
            lastError = "Unsupported signature algorithm";
         }
         else if (EVP_DigestSignInit(ctx, nullptr, md, nullptr, privateKey) != 1)
         {
            lastError = "Failed to initialize signature context";
         }
         else
         {
            size_t sigLen = 0;
            if (EVP_DigestSign(ctx, nullptr, &sigLen, data, dataLength) != 1)
            {
               lastError = "Failed to get signature length";
            }
            else
            {
               std::vector<unsigned char> sig(sigLen);
               if (EVP_DigestSign(ctx, sig.data(), &sigLen, data, dataLength) == 1)
               {
                  signature = base64URLEncode(sig.data(), sigLen);
                  success   = true;
               }
               else
               {
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
   // Ensure OpenSSL is properly initialized (modern API)
   int init_result =
      OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS |
                             OPENSSL_INIT_ADD_ALL_DIGESTS,
                          nullptr);
   if (init_result != 1)
   {
      pImpl->lastError = "Failed to initialize OpenSSL";
      return false;
   }

   std::ifstream keyFile(keyPath);
   if (!keyFile.is_open())
   {
      pImpl->lastError = "Failed to open private key file: " + keyPath;
      return false;
   }

   std::string keyContent((std::istreambuf_iterator<char>(keyFile)),
                          std::istreambuf_iterator<char>());
   keyFile.close();

   BIO* bio = BIO_new_mem_buf(keyContent.c_str(), static_cast<int>(keyContent.length()));
   if (!bio)
   {
      pImpl->lastError = "Failed to create BIO for private key";
      return false;
   }

   EVP_PKEY* key = nullptr;
   if (!password.empty())
   {
      // OpenSSL requires non-const void* for password, but won't modify it
      // Using C-style cast to avoid clang-tidy const_cast warning while maintaining safety
      std::string password_copy = password;  // Make a copy to be safe
      key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, (void*)password_copy.c_str());
   }
   else
   {
      key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
   }

   BIO_free(bio);

   if (!key)
   {
      pImpl->lastError = "Failed to load private key (check password if encrypted)";
      return false;
   }

   if (pImpl->privateKey)
   {
      EVP_PKEY_free(pImpl->privateKey);
   }
   pImpl->privateKey = key;

   // Extract public key from private key for JSF compliance
   if (pImpl->publicKey)
   {
      EVP_PKEY_free(pImpl->publicKey);
   }
   pImpl->publicKey = EVP_PKEY_dup(key);

   return true;
}

bool SBOMSigner::loadCertificate(const std::string& certPath)
{
   // Ensure OpenSSL is properly initialized (modern API)
   int init_result =
      OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS |
                             OPENSSL_INIT_ADD_ALL_DIGESTS,
                          nullptr);
   if (init_result != 1)
   {
      pImpl->lastError = "Failed to initialize OpenSSL";
      return false;
   }

   std::ifstream certFile(certPath);
   if (!certFile.is_open())
   {
      pImpl->lastError = "Failed to open certificate file: " + certPath;
      return false;
   }

   std::string certContent((std::istreambuf_iterator<char>(certFile)),
                           std::istreambuf_iterator<char>());
   certFile.close();

   BIO* bio = BIO_new_mem_buf(certContent.c_str(), static_cast<int>(certContent.length()));
   if (!bio)
   {
      pImpl->lastError = "Failed to create BIO for certificate";
      return false;
   }

   X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
   BIO_free(bio);

   if (!cert)
   {
      pImpl->lastError = "Failed to load certificate";
      return false;
   }

   if (pImpl->certificate)
   {
      X509_free(pImpl->certificate);
   }
   pImpl->certificate = cert;

   return true;
}

bool SBOMSigner::loadPublicKey(const std::string& keyPath)
{
   // Ensure OpenSSL is properly initialized (modern API)
   int init_result =
      OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS |
                             OPENSSL_INIT_ADD_ALL_DIGESTS,
                          nullptr);
   if (init_result != 1)
   {
      pImpl->lastError = "Failed to initialize OpenSSL";
      return false;
   }

   std::ifstream keyFile(keyPath);
   if (!keyFile.is_open())
   {
      pImpl->lastError = "Failed to open public key file: " + keyPath;
      return false;
   }

   std::string keyContent((std::istreambuf_iterator<char>(keyFile)),
                          std::istreambuf_iterator<char>());
   keyFile.close();

   BIO* bio = BIO_new_mem_buf(keyContent.c_str(), static_cast<int>(keyContent.length()));
   if (!bio)
   {
      pImpl->lastError = "Failed to create BIO for public key";
      return false;
   }

   EVP_PKEY* key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
   BIO_free(bio);

   if (!key)
   {
      pImpl->lastError = "Failed to load public key";
      return false;
   }

   if (pImpl->publicKey)
   {
      EVP_PKEY_free(pImpl->publicKey);
   }
   pImpl->publicKey = key;

   return true;
}

bool SBOMSigner::loadPublicKeyFromCertificate(const std::string& certPath)
{
   std::ifstream certFile(certPath);
   if (!certFile.is_open())
   {
      pImpl->lastError = "Failed to open certificate file: " + certPath;
      return false;
   }

   std::string certContent((std::istreambuf_iterator<char>(certFile)),
                           std::istreambuf_iterator<char>());
   certFile.close();

   BIO* bio = BIO_new_mem_buf(certContent.c_str(), static_cast<int>(certContent.length()));
   if (!bio)
   {
      pImpl->lastError = "Failed to create BIO for certificate";
      return false;
   }

   X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
   BIO_free(bio);

   if (!cert)
   {
      pImpl->lastError = "Failed to load certificate";
      return false;
   }

   EVP_PKEY* key = X509_get_pubkey(cert);
   X509_free(cert);

   if (!key)
   {
      pImpl->lastError = "Failed to extract public key from certificate";
      return false;
   }

   if (pImpl->publicKey)
   {
      EVP_PKEY_free(pImpl->publicKey);
   }
   pImpl->publicKey = key;

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

nlohmann::json SBOMSigner::getPublicKeyAsJWK() const
{
   if (!pImpl->publicKey)
   {
      return nlohmann::json();
   }

   nlohmann::json jwk;
   int            keyType = EVP_PKEY_id(pImpl->publicKey);

   if (keyType == EVP_PKEY_RSA)
   {
      // RSA key
      const RSA* rsa = EVP_PKEY_get0_RSA(pImpl->publicKey);
      if (!rsa)
      {
         return nlohmann::json();
      }

      const BIGNUM* n = nullptr;
      const BIGNUM* e = nullptr;
      RSA_get0_key(rsa, &n, &e, nullptr);

      if (n && e)
      {
         jwk["kty"] = "RSA";
         jwk["n"]   = pImpl->bignumToBase64URL(n);
         jwk["e"]   = pImpl->bignumToBase64URL(e);
      }
   }
   else if (keyType == EVP_PKEY_EC)
   {
      // EC key
      const EC_KEY* ec = EVP_PKEY_get0_EC_KEY(pImpl->publicKey);
      if (!ec)
      {
         return nlohmann::json();
      }

      const EC_POINT* point = EC_KEY_get0_public_key(ec);
      const EC_GROUP* group = EC_KEY_get0_group(ec);
      if (!point || !group)
      {
         return nlohmann::json();
      }

      // Get curve name
      int nid = EC_GROUP_get_curve_name(group);
      if (nid == NID_X9_62_prime256v1)
      {
         jwk["kty"] = "EC";
         jwk["crv"] = "P-256";
      }
      else if (nid == NID_secp384r1)
      {
         jwk["kty"] = "EC";
         jwk["crv"] = "P-384";
      }
      else if (nid == NID_secp521r1)
      {
         jwk["kty"] = "EC";
         jwk["crv"] = "P-521";
      }
      else
      {
         return nlohmann::json();
      }

      // Get coordinates
      BIGNUM* x = BN_new();
      BIGNUM* y = BN_new();
      if (EC_POINT_get_affine_coordinates(group, point, x, y, nullptr))
      {
         jwk["x"] = pImpl->bignumToBase64URL(x);
         jwk["y"] = pImpl->bignumToBase64URL(y);
      }
      BN_free(x);
      BN_free(y);
   }
   else if (keyType == EVP_PKEY_ED25519)
   {
      // Ed25519 key
      size_t keyLen = EVP_PKEY_get_raw_public_key(pImpl->publicKey, nullptr, 0);
      if (keyLen == 32)
      {
         std::vector<unsigned char> keyData(keyLen);
         if (EVP_PKEY_get_raw_public_key(pImpl->publicKey, keyData.data(), &keyLen))
         {
            jwk["kty"] = "OKP";
            jwk["crv"] = "Ed25519";
            jwk["x"]   = pImpl->base64URLEncode(keyData.data(), keyLen);
         }
      }
   }

   return jwk;
}

bool SBOMSigner::signSBOM(const std::string& sbomContent, SignatureInfo& signatureInfo)
{
   // Ensure OpenSSL is properly initialized (modern API)
   int init_result =
      OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS |
                             OPENSSL_INIT_ADD_ALL_DIGESTS,
                          nullptr);
   if (init_result != 1)
   {
      pImpl->lastError = "Failed to initialize OpenSSL";
      return false;
   }

   // Create canonical JSON for signing (remove signature field if present)
   nlohmann::json sbomJson;
   try
   {
      sbomJson = nlohmann::json::parse(sbomContent);
   }
   catch (const std::exception& e)
   {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return false;
   }

   // Create canonical JSON for signing according to JSF standards
   std::vector<std::string> excludes;
   std::string              canonicalJsonString = createCanonicalJSON(sbomJson, excludes);

   // Sign the canonical JSON
   std::string signature;
   if (!pImpl->signData(reinterpret_cast<const unsigned char*>(canonicalJsonString.c_str()),
                        canonicalJsonString.length(), signature))
   {
      return false;
   }

   // Fill signature info
   signatureInfo.algorithm = pImpl->algorithmToString(pImpl->algorithm);
   signatureInfo.keyId     = pImpl->keyId;
   signatureInfo.signature = signature;
   signatureInfo.timestamp = pImpl->getCurrentTimestamp();
   signatureInfo.excludes  = excludes;
   signatureInfo.publicKey = getPublicKeyAsJWK();

   // Add certificate if available
   if (pImpl->certificate)
   {
      BIO* bio = BIO_new(BIO_s_mem());
      if (PEM_write_bio_X509(bio, pImpl->certificate))
      {
         BUF_MEM* bufferPtr;
         BIO_get_mem_ptr(bio, &bufferPtr);
         signatureInfo.certificate = std::string(bufferPtr->data, bufferPtr->length);
      }
      BIO_free(bio);
   }

   return true;
}

std::string SBOMSigner::addSignatureToCycloneDX(const std::string&   sbomContent,
                                                const SignatureInfo& signatureInfo)
{
   nlohmann::json sbomJson;
   try
   {
      sbomJson = nlohmann::json::parse(sbomContent);
   }
   catch (const std::exception& e)
   {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return sbomContent;
   }

   // Create signature object according to JSON Signature Format (JSF) specification
   // JSF format: https://cyberphone.github.io/doc/security/jsf.html
   nlohmann::json signatureObj;

   // Required fields for JSF signaturecore
   signatureObj["algorithm"] = signatureInfo.algorithm;
   signatureObj["value"]     = signatureInfo.signature;  // Base64URL-encoded signature

   // Required public key in JWK format for JSF compliance
   if (!signatureInfo.publicKey.empty())
   {
      signatureObj["publicKey"] = signatureInfo.publicKey;
   }

   // Add signature to SBOM
   sbomJson["signature"] = signatureObj;

   return sbomJson.dump(2);
}

bool SBOMSigner::verifySignature(const std::string& sbomContent)
{
   // Ensure OpenSSL is properly initialized (modern API)
   int init_result =
      OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_ADD_ALL_CIPHERS |
                             OPENSSL_INIT_ADD_ALL_DIGESTS,
                          nullptr);
   if (init_result != 1)
   {
      pImpl->lastError = "Failed to initialize OpenSSL";
      return false;
   }

   SignatureInfo signatureInfo;
   if (!extractSignature(sbomContent, signatureInfo))
   {
      return false;
   }

   // Check if we have a public key loaded
   if (!pImpl->publicKey)
   {
      pImpl->lastError = "No public key loaded for verification";
      return false;
   }

   // Parse the SBOM content
   nlohmann::json sbomJson;
   try
   {
      sbomJson = nlohmann::json::parse(sbomContent);
   }
   catch (const std::exception& e)
   {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return false;
   }

   // Create canonical JSON (same as during signing)
   std::vector<std::string> excludes;
   std::string              canonicalJsonString = createCanonicalJSON(sbomJson, excludes);

   // Convert algorithm string to enum
   SignatureAlgorithm algorithm = SignatureAlgorithm::RS256;  // default
   if (signatureInfo.algorithm == "RS256")
      algorithm = SignatureAlgorithm::RS256;
   else if (signatureInfo.algorithm == "RS384")
      algorithm = SignatureAlgorithm::RS384;
   else if (signatureInfo.algorithm == "RS512")
      algorithm = SignatureAlgorithm::RS512;
   else if (signatureInfo.algorithm == "ES256")
      algorithm = SignatureAlgorithm::ES256;
   else if (signatureInfo.algorithm == "ES384")
      algorithm = SignatureAlgorithm::ES384;
   else if (signatureInfo.algorithm == "ES512")
      algorithm = SignatureAlgorithm::ES512;
   else if (signatureInfo.algorithm == "Ed25519")
      algorithm = SignatureAlgorithm::Ed25519;

   // Verify the signature
   return pImpl->verifyData(reinterpret_cast<const unsigned char*>(canonicalJsonString.c_str()),
                            canonicalJsonString.length(), signatureInfo.signature, algorithm);
}

bool SBOMSigner::extractSignature(const std::string& sbomContent, SignatureInfo& signatureInfo)
{
   nlohmann::json sbomJson;
   try
   {
      sbomJson = nlohmann::json::parse(sbomContent);
   }
   catch (const std::exception& e)
   {
      pImpl->lastError = "Failed to parse SBOM JSON: " + std::string(e.what());
      return false;
   }

   if (!sbomJson.contains("signature"))
   {
      pImpl->lastError = "No signature found in SBOM";
      return false;
   }

   nlohmann::json sigJson = sbomJson["signature"];

   // Handle signature as object with JSF signaturecore format
   if (sigJson.is_object())
   {
      // Extract required JSF fields
      signatureInfo.algorithm = sigJson.value("algorithm", "");
      signatureInfo.signature = sigJson.value("value", "");

      // Extract optional fields
      signatureInfo.keyId       = sigJson.value("keyId", "");
      signatureInfo.certificate = sigJson.value("certificate", "");
      signatureInfo.timestamp   = sigJson.value("timestamp", "");

      // Extract excludes array if present
      if (sigJson.contains("excludes") && sigJson["excludes"].is_array())
      {
         signatureInfo.excludes = sigJson["excludes"].get<std::vector<std::string>>();
      }

      // Extract public key if present (JWK format)
      if (sigJson.contains("publicKey") && sigJson["publicKey"].is_object())
      {
         // TODO: Convert JWK to certificate format if needed
      }
   }
   else if (sigJson.is_string())
   {
      // Fallback for simple string format
      signatureInfo.signature = sigJson.get<std::string>();
   }

   return true;
}

std::string SBOMSigner::getLastError() const
{
   return pImpl->lastError;
}

std::string SBOMSigner::createCanonicalJSON(const nlohmann::json&     sbomJson,
                                            std::vector<std::string>& excludes) const
{
   // Create a deep copy to avoid modifying the original
   nlohmann::json canonicalJson = sbomJson;

   // Clear the excludes vector
   excludes.clear();

   // Remove signature field from root level
   if (canonicalJson.contains("signature"))
   {
      canonicalJson.erase("signature");
      excludes.push_back("signature");
   }

   // Remove signature fields from all components
   if (canonicalJson.contains("components"))
   {
      for (size_t i = 0; i < canonicalJson["components"].size(); ++i)
      {
         auto& component = canonicalJson["components"][i];
         if (component.contains("signature"))
         {
            component.erase("signature");
            excludes.push_back("components[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from all services
   if (canonicalJson.contains("services"))
   {
      for (size_t i = 0; i < canonicalJson["services"].size(); ++i)
      {
         auto& service = canonicalJson["services"][i];
         if (service.contains("signature"))
         {
            service.erase("signature");
            excludes.push_back("services[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from all vulnerabilities
   if (canonicalJson.contains("vulnerabilities"))
   {
      for (size_t i = 0; i < canonicalJson["vulnerabilities"].size(); ++i)
      {
         auto& vulnerability = canonicalJson["vulnerabilities"][i];
         if (vulnerability.contains("signature"))
         {
            vulnerability.erase("signature");
            excludes.push_back("vulnerabilities[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from all annotations
   if (canonicalJson.contains("annotations"))
   {
      for (size_t i = 0; i < canonicalJson["annotations"].size(); ++i)
      {
         auto& annotation = canonicalJson["annotations"][i];
         if (annotation.contains("signature"))
         {
            annotation.erase("signature");
            excludes.push_back("annotations[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from compositions
   if (canonicalJson.contains("compositions"))
   {
      for (size_t i = 0; i < canonicalJson["compositions"].size(); ++i)
      {
         auto& composition = canonicalJson["compositions"][i];
         if (composition.contains("signature"))
         {
            composition.erase("signature");
            excludes.push_back("compositions[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from formulations
   if (canonicalJson.contains("formulation"))
   {
      for (size_t i = 0; i < canonicalJson["formulation"].size(); ++i)
      {
         auto& formula = canonicalJson["formulation"][i];
         if (formula.contains("signature"))
         {
            formula.erase("signature");
            excludes.push_back("formulation[" + std::to_string(i) + "].signature");
         }
      }
   }

   // Remove signature fields from metadata
   if (canonicalJson.contains("metadata"))
   {
      auto& metadata = canonicalJson["metadata"];
      if (metadata.contains("signature"))
      {
         metadata.erase("signature");
         excludes.push_back("metadata.signature");
      }

      // Remove signature fields from metadata tools
      if (metadata.contains("tools"))
      {
         for (size_t i = 0; i < metadata["tools"].size(); ++i)
         {
            auto& tool = metadata["tools"][i];
            if (tool.contains("signature"))
            {
               tool.erase("signature");
               excludes.push_back("metadata.tools[" + std::to_string(i) + "].signature");
            }
         }
      }

      // Remove signature fields from metadata authors
      if (metadata.contains("authors"))
      {
         for (size_t i = 0; i < metadata["authors"].size(); ++i)
         {
            auto& author = metadata["authors"][i];
            if (author.contains("signature"))
            {
               author.erase("signature");
               excludes.push_back("metadata.authors[" + std::to_string(i) + "].signature");
            }
         }
      }
   }

   // Create canonical JSON string with consistent formatting
   // Use compact JSON to ensure consistent canonicalization
   return canonicalJson.dump();
}

bool SBOMSigner::verifyCanonicalization(const nlohmann::json& originalJson,
                                        const std::string&    canonicalJson)
{
   try
   {
      nlohmann::json canonicalJsonObj = nlohmann::json::parse(canonicalJson);

      // Check that root signature is excluded
      if (canonicalJsonObj.contains("signature"))
      {
         pImpl->lastError = "Root signature field not excluded from canonical JSON";
         return false;
      }

      // Check that component signatures are excluded
      if (canonicalJsonObj.contains("components"))
      {
         for (const auto& component : canonicalJsonObj["components"])
         {
            if (component.contains("signature"))
            {
               pImpl->lastError = "Component signature field not excluded from canonical JSON";
               return false;
            }
         }
      }

      // Check that service signatures are excluded
      if (canonicalJsonObj.contains("services"))
      {
         for (const auto& service : canonicalJsonObj["services"])
         {
            if (service.contains("signature"))
            {
               pImpl->lastError = "Service signature field not excluded from canonical JSON";
               return false;
            }
         }
      }

      // Check that vulnerability signatures are excluded
      if (canonicalJsonObj.contains("vulnerabilities"))
      {
         for (const auto& vulnerability : canonicalJsonObj["vulnerabilities"])
         {
            if (vulnerability.contains("signature"))
            {
               pImpl->lastError = "Vulnerability signature field not excluded from canonical JSON";
               return false;
            }
         }
      }

      // Check that annotation signatures are excluded
      if (canonicalJsonObj.contains("annotations"))
      {
         for (const auto& annotation : canonicalJsonObj["annotations"])
         {
            if (annotation.contains("signature"))
            {
               pImpl->lastError = "Annotation signature field not excluded from canonical JSON";
               return false;
            }
         }
      }

      // Check that metadata signatures are excluded
      if (canonicalJsonObj.contains("metadata"))
      {
         const auto& metadata = canonicalJsonObj["metadata"];
         if (metadata.contains("signature"))
         {
            pImpl->lastError = "Metadata signature field not excluded from canonical JSON";
            return false;
         }
      }

      return true;
   }
   catch (const std::exception& e)
   {
      pImpl->lastError =
         "Failed to parse canonical JSON for verification: " + std::string(e.what());
      return false;
   }
}

}  // namespace heimdall