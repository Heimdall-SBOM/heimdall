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
 * @file heimdall-sbom.cpp
 * @brief Dynamic SBOM generator loader for Heimdall plugins
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides a minimal C++ SBOM generator that dynamically loads
 * Heimdall plugins and generates SBOMs from binary files. It supports:
 *
 * - Dynamic loading of LLD and Gold linker plugins
 * - SBOM generation in SPDX and CycloneDX formats
 * - Configurable output formats and versions
 * - Command-line interface for batch processing
 *
 * The loader uses dynamic linking (dlopen/dlsym) to load plugin functions
 * and provides a simple interface for SBOM generation without requiring
 * the full Heimdall library to be linked.
 *
 * Supported Formats:
 * - SPDX 2.3, 3.0, 3.0.0, 3.0.1
 * - CycloneDX 1.4, 1.5, 1.6
 */

// Minimal C++ SBOM generator for Heimdall plugins (heimdall-sbom)
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "../common/SBOMSigner.hpp"

// LLVM symbols are now provided by llvm_symbols shared library

/**
 * @brief Function pointer types for plugin interface
 */
using init_func_t                  = int (*)(void*);
using set_format_func_t            = int (*)(const char*);
using set_cyclonedx_version_func_t = int (*)(const char*);
using set_spdx_version_func_t      = int (*)(const char*);
using set_output_path_func_t       = int (*)(const char*);
using process_input_file_func_t    = int (*)(const char*);
using finalize_func_t              = void (*)();

/**
 * @brief Generate SBOM from binary file using a dynamic plugin
 *
 * This function dynamically loads a Heimdall plugin and uses it to generate
 * an SBOM from a binary file. It handles plugin initialization, configuration,
 * processing, and cleanup.
 *
 * @param plugin_path Path to the plugin shared library
 * @param binary_path Path to the binary file to analyze
 * @param format SBOM format ("spdx", "cyclonedx", etc.)
 * @param output_path Path for the output SBOM file
 * @param cyclonedx_version CycloneDX specification version
 * @param spdx_version SPDX specification version
 * @return 0 on success, 1 on failure
 */
int generate_sbom(const char* plugin_path, const char* binary_path, const char* format,
                  const char* output_path, const char* cyclonedx_version, const char* spdx_version)
{
   // Load the plugin shared library
   void* handle = dlopen(plugin_path, RTLD_LAZY);
   if (!handle)
   {
      std::cerr << "Failed to load plugin " << plugin_path << ": " << dlerror() << std::endl;
      return 1;
   }

   // Get function pointers from the plugin
   auto onload     = (init_func_t)dlsym(handle, "onload");
   auto set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
   auto set_cyclonedx_version =
      (set_cyclonedx_version_func_t)dlsym(handle, "heimdall_set_cyclonedx_version");
   auto set_spdx_version = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
   auto set_output_path  = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
   auto process_input_file =
      (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
   auto finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");

   // Check that all required functions are available
   if (!onload || !set_format || !set_output_path || !process_input_file || !finalize)
   {
      std::cerr << "Failed to get function symbols: " << dlerror() << std::endl;
      dlclose(handle);
      return 1;
   }

   // Initialize the plugin
   if (onload(nullptr) != 0)
   {
      std::cerr << "Failed to initialize plugin" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Set the output format
   if (set_format(format) != 0)
   {
      std::cerr << "Failed to set format" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Handle CycloneDX version configuration
   if (strncmp(format, "cyclonedx", 9) == 0 && set_cyclonedx_version &&
       set_cyclonedx_version(cyclonedx_version) != 0)
   {
      std::cerr << "Failed to set CycloneDX version" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Handle SPDX version configuration
   if (strncmp(format, "spdx", 4) == 0 && set_spdx_version && set_spdx_version(spdx_version) != 0)
   {
      std::cerr << "Failed to set SPDX version" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Set output path
   if (set_output_path(output_path) != 0)
   {
      std::cerr << "Failed to set output path" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Process the binary file
   if (process_input_file(binary_path) != 0)
   {
      std::cerr << "Failed to process binary" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Finalize and generate the SBOM
   finalize();
   dlclose(handle);
   return 0;
}

/**
 * @brief Configuration structure for SBOM generation options
 */
struct SBOMConfig
{
   const char* plugin_path             = nullptr;
   const char* binary_path             = nullptr;
   const char* format                  = "spdx";
   const char* output_path             = "sbom.json";
   const char* cyclonedx_version       = "1.6";
   const char* spdx_version            = "2.3";
   bool        transitive_dependencies = true;

   // Signing options
   const char* sign_key_path  = nullptr;
   const char* sign_cert_path = nullptr;
   const char* sign_algorithm = "RS256";
   const char* sign_key_id    = nullptr;

   // Ada detection options
   const char* ali_file_path = nullptr;
};

/**
 * @brief Print help information
 */
void print_help()
{
   std::cout << "Heimdall SBOM Generator Tool\n\n";
   std::cout << "Usage: heimdall-sbom <plugin_path> <binary_path> --format <format> --output "
                "<output_path> [options]\n\n";
   std::cout << "Required Arguments:\n";
   std::cout << "  <plugin_path>           Path to the Heimdall plugin (.so file)\n";
   std::cout << "  <binary_path>           Path to the binary file to analyze\n";
   std::cout << "  --format <format>       SBOM format to generate\n";
   std::cout << "  --output <output_path>  Output file path for the generated SBOM\n\n";
   std::cout << "Format Options:\n";
   std::cout << "  --format spdx           Generate SPDX 2.3 format (default)\n";
   std::cout << "  --format spdx-2.3       Generate SPDX 2.3 format\n";
   std::cout << "  --format spdx-3.0       Generate SPDX 3.0 format\n";
   std::cout << "  --format spdx-3.0.0     Generate SPDX 3.0.0 format\n";
   std::cout << "  --format spdx-3.0.1     Generate SPDX 3.0.1 format\n";
   std::cout << "  --format cyclonedx      Generate CycloneDX 1.6 format\n";
   std::cout << "  --format cyclonedx-1.4  Generate CycloneDX 1.4 format\n";
   std::cout << "  --format cyclonedx-1.6  Generate CycloneDX 1.6 format\n\n";
   std::cout << "Version Options:\n";
   std::cout << "  --cyclonedx-version <version>  Specify CycloneDX version (1.4, 1.6)\n";
   std::cout
      << "  --spdx-version <version>       Specify SPDX version (2.3, 3.0, 3.0.0, 3.0.1)\n\n";
   std::cout << "Dependency Options:\n";
   std::cout << "  --no-transitive-dependencies   Include only direct dependencies\n";
   std::cout
      << "                                  (default: include all transitive dependencies)\n\n";
   std::cout << "Signing Options:\n";
   std::cout << "  --sign-key <key_path>          Path to private key file for signing\n";
   std::cout << "  --sign-cert <cert_path>        Path to certificate file (optional)\n";
   std::cout << "  --sign-algorithm <algorithm>   Signature algorithm\n";
   std::cout << "  --sign-key-id <key_id>         Key identifier for the signature\n\n";
   std::cout << "Supported Signature Algorithms:\n";
   std::cout << "  RS256, RS384, RS512            RSA with SHA-256/384/512\n";
   std::cout << "  ES256, ES384, ES512            ECDSA with SHA-256/384/512\n";
   std::cout << "  Ed25519                        Ed25519 digital signature\n\n";
   std::cout << "Ada Language Support:\n";
   std::cout
      << "  --ali-file-path <path>         Enable Ada detection and search for .ali files\n\n";
   std::cout << "Examples:\n";
   std::cout << "  # Generate unsigned SPDX SBOM\n";
   std::cout
      << "  heimdall-sbom ./lib/heimdall-lld.so ./myapp --format spdx --output sbom.spdx\n\n";
   std::cout << "  # Generate signed CycloneDX SBOM with RSA\n";
   std::cout << "  heimdall-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output "
                "sbom.cdx.json \\\n";
   std::cout << "    --sign-key private.key --sign-algorithm RS256 --sign-key-id my-key-2025\n\n";
   std::cout << "  # Generate signed SBOM with certificate\n";
   std::cout << "  heimdall-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output "
                "sbom.cdx.json \\\n";
   std::cout << "    --sign-key private.key --sign-cert certificate.pem --sign-algorithm ES256\n\n";
   std::cout << "  # Generate SBOM with Ada support\n";
   std::cout << "  heimdall-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output "
                "sbom.cdx.json \\\n";
   std::cout << "    --ali-file-path /path/to/ada/source\n\n";
   std::cout << "  # Generate SBOM with only direct dependencies\n";
   std::cout << "  heimdall-sbom ./lib/heimdall-lld.so ./myapp --format cyclonedx --output "
                "sbom.cdx.json \\\n";
   std::cout << "    --no-transitive-dependencies\n\n";
   std::cout << "Notes:\n";
   std::cout << "  - Signing requires a valid private key file\n";
   std::cout << "  - Certificate files are optional but recommended for verification\n";
   std::cout << "  - Key ID is used to identify the signing key in the signature\n";
   std::cout << "  - Ada detection requires .ali files to be present in the specified path\n";
   std::cout << "  - Generated SBOMs are compliant with NTIA minimum requirements\n";
}

/**
 * @brief Print usage information for errors
 */
void print_usage()
{
   std::cerr << "Usage: heimdall-sbom <plugin_path> <binary_path> --format <format> --output "
                "<output_path> [--cyclonedx-version <version>] [--spdx-version <version>] "
                "[--no-transitive-dependencies] [--sign-key <key_path>] [--sign-cert <cert_path>] "
                "[--sign-algorithm <algorithm>] [--sign-key-id <key_id>] [--ali-file-path <path>]"
             << std::endl;
   std::cerr << "  Supported formats: spdx, spdx-2.3, spdx-3.0, spdx-3.0.0, spdx-3.0.1, "
                "cyclonedx, cyclonedx-1.4, cyclonedx-1.6"
             << std::endl;
   std::cerr << "  Default versions: cyclonedx-1.6, spdx-2.3" << std::endl;
   std::cerr << "  --no-transitive-dependencies: Include only direct dependencies (default: "
                "include all transitive dependencies)"
             << std::endl;
   std::cerr << "  --sign-key <key_path>: Path to private key file for signing" << std::endl;
   std::cerr << "  --sign-cert <cert_path>: Path to certificate file (optional)" << std::endl;
   std::cerr << "  --sign-algorithm <algorithm>: Signature algorithm (RS256, RS384, RS512, ES256, "
                "ES384, ES512, Ed25519)"
             << std::endl;
   std::cerr << "  --sign-key-id <key_id>: Key identifier for the signature" << std::endl;
   std::cerr << "  --ali-file-path <path>: Enable Ada detection and search for .ali files in the "
                "specified path"
             << std::endl;
}

/**
 * @brief Parse command line arguments and populate configuration
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @param config Configuration structure to populate
 * @return true if parsing successful, false if help was requested or error occurred
 */
bool parse_arguments(int argc, char* argv[], SBOMConfig& config)
{
   // Check for help option first
   for (int i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
      {
         print_help();
         return false;
      }
   }

   if (argc < 5)
   {
      print_usage();
      return false;
   }

   config.plugin_path = argv[1];
   config.binary_path = argv[2];

   // Parse command line arguments
   for (int i = 3; i < argc; i++)
   {
      if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
      {
         print_help();
         return false;
      }
      else if (strcmp(argv[i], "--no-transitive-dependencies") == 0)
      {
         config.transitive_dependencies = false;
      }
      else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc)
      {
         i++;
         config.format = argv[i];
         // Extract version from format string for SPDX
         if (strncmp(config.format, "spdx-", 5) == 0)
         {
            config.spdx_version = config.format + 5;  // Skip "spdx-" prefix
         }
         // Extract version from format string for CycloneDX
         if (strncmp(config.format, "cyclonedx-", 10) == 0)
         {
            config.cyclonedx_version = config.format + 10;  // Skip "cyclonedx-" prefix
         }
      }
      else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
      {
         i++;
         config.output_path = argv[i];
      }
      else if (strcmp(argv[i], "--cyclonedx-version") == 0 && i + 1 < argc)
      {
         i++;
         config.cyclonedx_version = argv[i];
      }
      else if (strcmp(argv[i], "--spdx-version") == 0 && i + 1 < argc)
      {
         i++;
         config.spdx_version = argv[i];
      }
      else if (strcmp(argv[i], "--sign-key") == 0 && i + 1 < argc)
      {
         i++;
         config.sign_key_path = argv[i];
      }
      else if (strcmp(argv[i], "--sign-cert") == 0 && i + 1 < argc)
      {
         i++;
         config.sign_cert_path = argv[i];
      }
      else if (strcmp(argv[i], "--sign-algorithm") == 0 && i + 1 < argc)
      {
         i++;
         config.sign_algorithm = argv[i];
      }
      else if (strcmp(argv[i], "--sign-key-id") == 0 && i + 1 < argc)
      {
         i++;
         config.sign_key_id = argv[i];
      }
      else if (strcmp(argv[i], "--ali-file-path") == 0 && i + 1 < argc)
      {
         i++;
         config.ali_file_path = argv[i];
      }
   }

   return true;
}

/**
 * @brief Load plugin and get function pointers
 * @param plugin_path Path to the plugin shared library
 * @param handle Output plugin handle
 * @param onload Output onload function pointer
 * @param set_format Output set_format function pointer
 * @param set_cyclonedx_version Output set_cyclonedx_version function pointer
 * @param set_spdx_version Output set_spdx_version function pointer
 * @param set_output_path Output set_output_path function pointer
 * @param process_input_file Output process_input_file function pointer
 * @param finalize Output finalize function pointer
 * @param set_transitive Output set_transitive function pointer
 * @param set_ali_file_path Output set_ali_file_path function pointer
 * @return 0 on success, 1 on failure
 */
int load_plugin_functions(const char* plugin_path, void*& handle, init_func_t& onload,
                          set_format_func_t&            set_format,
                          set_cyclonedx_version_func_t& set_cyclonedx_version,
                          set_spdx_version_func_t&      set_spdx_version,
                          set_output_path_func_t&       set_output_path,
                          process_input_file_func_t& process_input_file, finalize_func_t& finalize,
                          int (*&set_transitive)(int), int (*&set_ali_file_path)(const char*))
{
   // Load the plugin shared library
   handle = dlopen(plugin_path, RTLD_LAZY);
   if (!handle)
   {
      std::cerr << "Failed to load plugin " << plugin_path << ": " << dlerror() << std::endl;
      return 1;
   }

   // Get function pointers from the plugin
   onload     = (init_func_t)dlsym(handle, "onload");
   set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
   set_cyclonedx_version =
      (set_cyclonedx_version_func_t)dlsym(handle, "heimdall_set_cyclonedx_version");
   set_spdx_version   = (set_spdx_version_func_t)dlsym(handle, "heimdall_set_spdx_version");
   set_output_path    = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
   process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
   finalize           = (finalize_func_t)dlsym(handle, "heimdall_finalize");

   set_transitive    = (int (*)(int))dlsym(handle, "heimdall_set_transitive_dependencies");
   set_ali_file_path = (int (*)(const char*))dlsym(handle, "heimdall_set_ali_file_path");

   // Check that all required functions are available
   if (!onload || !set_format || !set_output_path || !process_input_file || !finalize)
   {
      std::cerr << "Failed to get function symbols: " << dlerror() << std::endl;
      dlclose(handle);
      return 1;
   }

   return 0;
}

/**
 * @brief Configure and run the plugin with the given configuration
 * @param config Configuration structure
 * @param handle Plugin handle
 * @param onload onload function pointer
 * @param set_format set_format function pointer
 * @param set_cyclonedx_version set_cyclonedx_version function pointer
 * @param set_spdx_version set_spdx_version function pointer
 * @param set_output_path set_output_path function pointer
 * @param process_input_file process_input_file function pointer
 * @param finalize finalize function pointer
 * @param set_transitive set_transitive function pointer
 * @param set_ali_file_path set_ali_file_path function pointer
 * @return 0 on success, 1 on failure
 */
int configure_and_run_plugin(const SBOMConfig& config, void* handle, init_func_t onload,
                             set_format_func_t            set_format,
                             set_cyclonedx_version_func_t set_cyclonedx_version,
                             set_spdx_version_func_t      set_spdx_version,
                             set_output_path_func_t       set_output_path,
                             process_input_file_func_t process_input_file, finalize_func_t finalize,
                             int (*set_transitive)(int), int (*set_ali_file_path)(const char*))
{
   // Initialize the plugin
   if (onload(nullptr) != 0)
   {
      std::cerr << "Failed to initialize plugin" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Set transitive dependencies flag after plugin is initialized
   if (set_transitive)
   {
      set_transitive(config.transitive_dependencies ? 1 : 0);
   }

   // Set Ada file path if specified
   if (config.ali_file_path && set_ali_file_path && set_ali_file_path(config.ali_file_path) != 0)
   {
      std::cerr << "Failed to set Ada file path" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Set the output format
   if (set_format(config.format) != 0)
   {
      std::cerr << "Failed to set format" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Handle CycloneDX version configuration
   if (strncmp(config.format, "cyclonedx", 9) == 0 && set_cyclonedx_version &&
       set_cyclonedx_version(config.cyclonedx_version) != 0)
   {
      std::cerr << "Failed to set CycloneDX version" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Handle SPDX version configuration
   if (strncmp(config.format, "spdx", 4) == 0 && set_spdx_version &&
       set_spdx_version(config.spdx_version) != 0)
   {
      std::cerr << "Failed to set SPDX version" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Set output path
   if (set_output_path(config.output_path) != 0)
   {
      std::cerr << "Failed to set output path" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Process the binary file
   if (process_input_file(config.binary_path) != 0)
   {
      std::cerr << "Failed to process binary" << std::endl;
      dlclose(handle);
      return 1;
   }

   // Finalize and generate the SBOM
   finalize();

   return 0;
}

/**
 * @brief Sign the generated SBOM if signing options are provided
 * @param config Configuration structure
 * @return 0 on success, 1 on failure
 */
int sign_sbom_if_requested(const SBOMConfig& config)
{
   if (!config.sign_key_path || strncmp(config.format, "cyclonedx", 9) != 0)
   {
      return 0;  // No signing requested or format doesn't support signing
   }

   // Only sign CycloneDX format for now
   std::cout << "Signing SBOM with key: " << config.sign_key_path << std::endl;

   // Read the generated SBOM file
   std::ifstream sbomFile(config.output_path);
   if (!sbomFile.is_open())
   {
      std::cerr << "Failed to open generated SBOM file for signing: " << config.output_path
                << std::endl;
      return 1;
   }

   std::string sbomContent((std::istreambuf_iterator<char>(sbomFile)),
                           std::istreambuf_iterator<char>());
   sbomFile.close();

   // Create signer and configure it
   heimdall::SBOMSigner signer;

   // Load private key
   if (!signer.loadPrivateKey(config.sign_key_path))
   {
      std::cerr << "Failed to load private key: " << signer.getLastError() << std::endl;
      return 1;
   }

   // Load certificate if provided
   if (config.sign_cert_path && !signer.loadCertificate(config.sign_cert_path))
   {
      std::cerr << "Failed to load certificate: " << signer.getLastError() << std::endl;
      return 1;
   }

   // Set key ID if provided
   if (config.sign_key_id)
   {
      signer.setKeyId(config.sign_key_id);
   }

   // Set signature algorithm
   heimdall::SignatureAlgorithm algorithm = heimdall::SignatureAlgorithm::RS256;
   if (strcmp(config.sign_algorithm, "RS384") == 0)
      algorithm = heimdall::SignatureAlgorithm::RS384;
   else if (strcmp(config.sign_algorithm, "RS512") == 0)
      algorithm = heimdall::SignatureAlgorithm::RS512;
   else if (strcmp(config.sign_algorithm, "ES256") == 0)
      algorithm = heimdall::SignatureAlgorithm::ES256;
   else if (strcmp(config.sign_algorithm, "ES384") == 0)
      algorithm = heimdall::SignatureAlgorithm::ES384;
   else if (strcmp(config.sign_algorithm, "ES512") == 0)
      algorithm = heimdall::SignatureAlgorithm::ES512;
   else if (strcmp(config.sign_algorithm, "Ed25519") == 0)
      algorithm = heimdall::SignatureAlgorithm::Ed25519;

   signer.setSignatureAlgorithm(algorithm);

   // Sign the SBOM
   heimdall::SignatureInfo signatureInfo;
   if (!signer.signSBOM(sbomContent, signatureInfo))
   {
      std::cerr << "Failed to sign SBOM: " << signer.getLastError() << std::endl;
      return 1;
   }

   // Add signature to SBOM and write back
   std::string   signedSbom = signer.addSignatureToCycloneDX(sbomContent, signatureInfo);

   std::ofstream signedFile(config.output_path);
   if (!signedFile.is_open())
   {
      std::cerr << "Failed to write signed SBOM to: " << config.output_path << std::endl;
      return 1;
   }

   signedFile << signedSbom;
   signedFile.close();

   std::cout << "SBOM signed successfully with algorithm: " << signatureInfo.algorithm << std::endl;

   return 0;
}

/**
 * @brief Main function for the heimdall-sbom tool
 *
 * Parses command-line arguments and calls generate_sbom() to create
 * an SBOM from a binary file using a dynamic plugin.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* argv[])
{
   SBOMConfig config;

   // Parse command-line arguments
   if (!parse_arguments(argc, argv, config))
   {
      return 1;
   }

   // Load plugin and get function pointers
   void*                        handle;
   init_func_t                  onload;
   set_format_func_t            set_format;
   set_cyclonedx_version_func_t set_cyclonedx_version;
   set_spdx_version_func_t      set_spdx_version;
   set_output_path_func_t       set_output_path;
   process_input_file_func_t    process_input_file;
   finalize_func_t              finalize;
   int (*set_transitive)(int);
   int (*set_ali_file_path)(const char*);

   if (load_plugin_functions(config.plugin_path, handle, onload, set_format, set_cyclonedx_version,
                             set_spdx_version, set_output_path, process_input_file, finalize,
                             set_transitive, set_ali_file_path) != 0)
   {
      return 1;
   }

   // Configure and run the plugin
   if (configure_and_run_plugin(config, handle, onload, set_format, set_cyclonedx_version,
                                set_spdx_version, set_output_path, process_input_file, finalize,
                                set_transitive, set_ali_file_path) != 0)
   {
      dlclose(handle);
      return 1;
   }

   // Sign the SBOM if requested
   int sign_result = sign_sbom_if_requested(config);

   dlclose(handle);
   return sign_result;
}
