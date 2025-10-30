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
#include "GoldPlugin.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
// Use the real plugin-api.h header
#include <plugin-api.h>
// MetadataExtractor.hpp no longer needed - GoldAdapter uses MetadataExtractor
#include "../common/Utils.hpp"
#include "../compat/compatibility.hpp"
#include "GoldAdapter.hpp"

namespace
{
std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
std::string                            outputPath = "heimdall-gold-sbom.json";
std::string                            format     = "spdx";
bool                                   verbose    = false;
std::vector<std::string>               processedFiles;
std::vector<std::string>               processedLibraries;
std::string cyclonedxVersion = "1.6";  // NEW: store requested CycloneDX version

// Plugin API interface variables
static ld_plugin_register_cleanup register_cleanup = nullptr;
static ld_plugin_register_claim_file register_claim_file = nullptr;
static ld_plugin_register_all_symbols_read register_all_symbols_read = nullptr;
static bool cleanup_registered = false;
static bool cleanup_completed = false;

// Simple utility functions to avoid heimdall-core dependencies
std::string getFileName(const std::string& path)
{
   return heimdall::Utils::getFileName(path);
}

bool fileExists(const std::string& path)
{
   return heimdall::Utils::fileExists(path);
}

std::string calculateSimpleHash(const std::string& path)
{
   // Simple hash calculation without OpenSSL dependency
   std::ifstream file(path, std::ios::binary);
   if (!file)
      return "NOASSERTION";

   // Read file content and calculate hash
   std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

   if (content.empty())
   {
      return "NOASSERTION";
   }

   // Calculate a simple hash of the content
   std::hash<std::string> hasher;
   std::stringstream      ss;
   ss << std::hex << hasher(content);
   return ss.str();
}

std::string getFileSize(const std::string& path)
{
   if (heimdall::Utils::fileExists(path))
   {
      return std::to_string(heimdall::Utils::getFileSize(path));
   }
   return "0";
}

std::string getFileType(const std::string& fileName)
{
   std::string extension = heimdall::Utils::getFileExtension(fileName);

   if (extension == ".o" || extension == ".obj")
   {
      return "OBJECT";
   }
   if (extension == ".a")
   {
      return "ARCHIVE";
   }
   if (extension == ".so" || extension == ".dylib" || extension == ".dll")
   {
      return "SHARED_LIBRARY";
   }
   if (extension == ".exe")
   {
      return "EXECUTABLE";
   }
   return "OTHER";
}
}  // namespace

// GoldPlugin class implementation
namespace heimdall
{

class GoldPlugin::Impl
{
   public:
   Impl() : adapter(heimdall::compat::make_unique<GoldAdapter>()), verbose(false) {}

   std::unique_ptr<GoldAdapter> adapter;
   std::vector<std::string>     processedFiles;
   std::vector<std::string>     processedLibraries;
   std::vector<std::string>     processedSymbols;
   std::string                  outputPath             = "heimdall-gold-sbom.json";
   std::string                  format                 = "spdx";
   std::string                  cyclonedxVersion       = "1.6";
   std::string                  spdxVersion            = "2.3";
   bool                         verbose                = false;
   bool                         extractDebugInfo       = true;
   bool                         includeSystemLibraries = false;
};

GoldPlugin::GoldPlugin() : pImpl(heimdall::compat::make_unique<Impl>()) {}

GoldPlugin::~GoldPlugin() = default;

bool GoldPlugin::initialize()
{
   try
   {
      return pImpl->adapter->initialize();
   }
   catch (...)
   {
      return false;
   }
}

void GoldPlugin::cleanup()
{
   try
   {
      pImpl->adapter->cleanup();
   }
   catch (...)
   {
      // Ignore cleanup errors
   }
}

void GoldPlugin::processInputFile(const std::string& filePath)
{
   try
   {
      pImpl->adapter->processInputFile(filePath);
      pImpl->processedFiles.push_back(filePath);
   }
   catch (...)
   {
      // Ignore processing errors
   }
}

void GoldPlugin::processLibrary(const std::string& libraryPath)
{
   try
   {
      pImpl->adapter->processLibrary(libraryPath);
      pImpl->processedLibraries.push_back(libraryPath);
   }
   catch (...)
   {
      // Ignore processing errors
   }
}

void GoldPlugin::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size)
{
   try
   {
      pImpl->adapter->processSymbol(symbolName, address, size);
      pImpl->processedSymbols.push_back(symbolName);
   }
   catch (...)
   {
      // Ignore processing errors
   }
}

void GoldPlugin::setOutputPath(const std::string& path)
{
   pImpl->outputPath = path;
   pImpl->adapter->setOutputPath(path);
}

void GoldPlugin::setFormat(const std::string& format)
{
   pImpl->format = format;
   pImpl->adapter->setFormat(format);
}

void GoldPlugin::setCycloneDXVersion(const std::string& version)
{
   pImpl->cyclonedxVersion = version;
   pImpl->adapter->setCycloneDXVersion(version);
}

void GoldPlugin::setSPDXVersion(const std::string& version)
{
   pImpl->spdxVersion = version;
   pImpl->adapter->setSPDXVersion(version);
}

void GoldPlugin::generateSBOM()
{
   try
   {
      pImpl->adapter->generateSBOM();
   }
   catch (...)
   {
      // Ignore generation errors
   }
}

void GoldPlugin::setVerbose(bool verbose)
{
   pImpl->verbose = verbose;
   pImpl->adapter->setVerbose(verbose);
}

void GoldPlugin::setExtractDebugInfo(bool extract)
{
   pImpl->extractDebugInfo = extract;
   pImpl->adapter->setExtractDebugInfo(extract);
}

void GoldPlugin::setIncludeSystemLibraries(bool include)
{
   pImpl->includeSystemLibraries = include;
   pImpl->adapter->setIncludeSystemLibraries(include);
}

size_t GoldPlugin::getComponentCount() const
{
   return pImpl->adapter->getComponentCount();
}

std::vector<std::string> GoldPlugin::getProcessedFiles() const
{
   return pImpl->processedFiles;
}

std::vector<std::string> GoldPlugin::getProcessedLibraries() const
{
   return pImpl->processedLibraries;
}

std::vector<std::string> GoldPlugin::getProcessedSymbols() const
{
   return pImpl->processedSymbols;
}

void GoldPlugin::printStatistics() const
{
   pImpl->adapter->printStatistics();
}

std::string GoldPlugin::getVersion() const
{
   return "1.0.0";
}

std::string GoldPlugin::getDescription() const
{
   return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
}

}  // namespace heimdall

// Forward declaration for plugin option handling
extern "C" int heimdall_gold_set_plugin_option(const char* option);

// Gold plugin API hook handlers
static enum ld_plugin_status claim_file_handler(const struct ld_plugin_input_file *file, int *claimed)
{
   if (!file || !file->name) {
      return LDPS_ERR;
   }

   if (verbose) {
      std::cout << "Heimdall: Gold plugin claim file: " << file->name << "\n";
   }

   // Process the input file
   std::string filePath(file->name);
   if (globalAdapter) {
      // Check file extension to determine if we should process it
      std::string ext = heimdall::Utils::getFileExtension(filePath);
      if (ext == ".o" || ext == ".obj" || ext == ".a" || ext == ".so" || ext == ".dylib") {
         globalAdapter->processInputFile(filePath);
         *claimed = 1; // We claim the file for processing
         if (verbose) {
            std::cout << "Heimdall: Claimed and processed file: " << filePath << "\n";
         }
      } else {
         *claimed = 0; // We don't handle this file type
      }
   }

   return LDPS_OK;
}

static enum ld_plugin_status all_symbols_read_handler(void)
{
   if (verbose) {
      std::cout << "Heimdall: All symbols read hook called\n";
   }

   // This is called after all input files have been processed
   // We can do any final processing here before cleanup
   // (No additional processing needed for now)

   return LDPS_OK;
}

// Cleanup handler function for Gold plugin API
static enum ld_plugin_status cleanup_handler(void)
{
   if (cleanup_completed) {
      if (verbose) {
         std::cout << "Heimdall: Cleanup handler called but already completed\n";
      }
      return LDPS_OK;
   }

   if (verbose) {
      std::cout << "Heimdall: Gold plugin cleanup handler called\n";
   }
   
   // Call our existing finalization code
   if (globalAdapter)
   {
      globalAdapter->generateSBOM();
      globalAdapter->cleanup();
   }
   
   cleanup_completed = true;
   std::cout << "Heimdall Gold Plugin finalized via cleanup handler\n";
   return LDPS_OK;
}

extern "C"
{
   // Main Gold plugin onload function following the plugin API
   int onload(struct ld_plugin_tv* tv)
   {
      std::cout << "Heimdall Gold Plugin activated\n";

      // Reset all global state
      processedFiles.clear();
      processedLibraries.clear();
      format     = "spdx";
      outputPath = "heimdall-gold-sbom.json";
      verbose    = false;
      cleanup_registered = false;
      cleanup_completed = false;

      // Process the transfer vector to register plugin hooks and handle options
      if (tv) {
         for (struct ld_plugin_tv* entry = tv; entry->tv_tag != LDPT_NULL; ++entry) {
            switch (entry->tv_tag) {
               case LDPT_OPTION:
                  // Handle plugin options passed via --plugin-opt
                  if (entry->tv_u.tv_string) {
                     std::cout << "Heimdall: Processing option: " << entry->tv_u.tv_string << "\n";
                     heimdall_gold_set_plugin_option(entry->tv_u.tv_string);
                  }
                  break;
               case LDPT_REGISTER_CLEANUP_HOOK:
                  register_cleanup = entry->tv_u.tv_register_cleanup;
                  if (register_cleanup && register_cleanup(cleanup_handler) == LDPS_OK) {
                     cleanup_registered = true;
                     if (verbose) {
                        std::cout << "Heimdall: Cleanup handler registered successfully\n";
                     }
                  }
                  break;
               case LDPT_REGISTER_CLAIM_FILE_HOOK:
                  register_claim_file = entry->tv_u.tv_register_claim_file;
                  if (register_claim_file && register_claim_file(claim_file_handler) == LDPS_OK) {
                     if (verbose) {
                        std::cout << "Heimdall: Claim file handler registered successfully\n";
                     }
                  }
                  break;
               case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
                  register_all_symbols_read = entry->tv_u.tv_register_all_symbols_read;
                  if (register_all_symbols_read && register_all_symbols_read(all_symbols_read_handler) == LDPS_OK) {
                     if (verbose) {
                        std::cout << "Heimdall: All symbols read handler registered successfully\n";
                     }
                  }
                  break;
               default:
                  // Ignore unknown hooks
                  break;
            }
         }
      }

      // Initialize the adapter
      globalAdapter = heimdall::compat::make_unique<heimdall::GoldAdapter>();
      globalAdapter->initialize();

      if (verbose)
      {
         std::cout << "Heimdall Gold Plugin initialized with output: " << outputPath 
                   << ", cleanup registered: " << (cleanup_registered ? "yes" : "no") << "\n";
      }

      return 0;
   }

   // Compatibility wrapper for old-style onload(void* handle) signature for backwards compatibility
   int onload_legacy(void* handle)
   {
      std::cout << "Heimdall Gold Plugin activated (legacy mode)\n";

      // Reset all global state
      processedFiles.clear();
      processedLibraries.clear();
      format     = "spdx";
      outputPath = "heimdall-gold-sbom.json";
      verbose    = false;
      cleanup_completed = false;

      // Initialize the adapter
      globalAdapter = heimdall::compat::make_unique<heimdall::GoldAdapter>();
      globalAdapter->initialize();

      if (verbose)
      {
         std::cout << "Heimdall Gold Plugin initialized with output: " << outputPath << "\n";
      }

      return 0;
   }

   const char* heimdall_gold_version()
   {
      return "1.0.0";
   }

   const char* heimdall_gold_description()
   {
      return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
   }

   // Configuration functions
   int heimdall_set_output_path(const char* path)
   {
      if (path)
      {
         outputPath = std::string(path);
         if (globalAdapter)
            globalAdapter->setOutputPath(outputPath);
         if (verbose)
         {
            std::cout << "Heimdall: Output path set to " << outputPath << "\n";
         }
         return 0;
      }
      return -1;
   }

   int heimdall_set_format(const char* fmt)
   {
      if (fmt)
      {
         format = std::string(fmt);
         if (globalAdapter)
            globalAdapter->setFormat(format);
         if (verbose)
         {
            std::cout << "Heimdall: Format set to " << format << "\n";
         }
         return 0;
      }
      return -1;
   }

   void heimdall_set_verbose(bool v)
   {
      verbose = v;
      if (globalAdapter)
         globalAdapter->setVerbose(v);
   }

   // File processing functions
   int heimdall_process_input_file(const char* filePath)
   {
      if (!globalAdapter || !filePath)
         return -1;

      std::string path(filePath);

      // Check if already processed
      if (std::find(processedFiles.begin(), processedFiles.end(), path) != processedFiles.end())
      {
         return 0;  // Already processed, not an error
      }

      processedFiles.push_back(path);

      if (verbose)
      {
         std::cout << "Heimdall: Processing input file: " << path << "\n";
      }

      // Process the file through the adapter (includes dependency detection)
      globalAdapter->processInputFile(path);

      // Generate a simple SBOM entry
      std::string fileName = getFileName(path);
      std::string checksum = calculateSimpleHash(path);
      std::string fileSize = getFileSize(path);

      if (verbose)
      {
         std::cout << "Heimdall: Processed file: " << fileName << " (checksum: " << checksum
                   << ", size: " << fileSize << ")\n";
      }

      return 0;
   }

   int heimdall_process_library(const char* libraryPath)
   {
      if (!globalAdapter || !libraryPath)
         return -1;

      std::string path(libraryPath);

      // Check if already processed
      if (std::find(processedLibraries.begin(), processedLibraries.end(), path) !=
          processedLibraries.end())
      {
         return 0;  // Already processed, not an error
      }

      processedLibraries.push_back(path);

      if (verbose)
      {
         std::cout << "Heimdall: Processing library: " << path << "\n";
      }

      // Process the library through the adapter
      globalAdapter->processLibrary(path);

      // Generate a simple SBOM entry
      std::string fileName = getFileName(path);
      std::string checksum = calculateSimpleHash(path);
      std::string fileSize = getFileSize(path);

      if (verbose)
      {
         std::cout << "Heimdall: Processed library: " << fileName << " (checksum: " << checksum
                   << ", size: " << fileSize << ")\n";
      }

      return 0;
   }

   int heimdall_set_cyclonedx_version(const char* version)
   {
      if (version)
      {
         cyclonedxVersion = version;
         if (globalAdapter)
         {
            globalAdapter->setCycloneDXVersion(version);
         }
         if (verbose)
         {
            std::cout << "Heimdall: CycloneDX version set to " << version << "\n";
         }
         return 0;
      }
      return -1;
   }

   int heimdall_set_spdx_version(const char* version)
   {
      if (globalAdapter && version)
      {
         globalAdapter->setSPDXVersion(std::string(version));
         return 0;
      }
      return -1;
   }

   int heimdall_set_transitive_dependencies(int transitive)
   {
      if (globalAdapter)
      {
         globalAdapter->setTransitiveDependencies(transitive != 0);
         if (verbose)
         {
            std::cout << "Heimdall: Transitive dependencies "
                      << (transitive ? "enabled" : "disabled") << "\n";
         }
         return 0;
      }
      return -1;
   }

   int heimdall_set_include_system_libraries(int include)
   {
      if (globalAdapter)
      {
         globalAdapter->setIncludeSystemLibraries(include != 0);
         if (verbose)
         {
            std::cout << "Heimdall: System libraries " << (include ? "enabled" : "disabled")
                      << "\n";
         }
         return 0;
      }
      return -1;
   }

   // Plugin cleanup and finalization
   void heimdall_finalize()
   {
      if (cleanup_completed) {
         if (verbose) {
            std::cout << "Heimdall: heimdall_finalize() called but cleanup already completed\n";
         }
         return;
      }

      if (globalAdapter)
      {
         globalAdapter->generateSBOM();
         globalAdapter->cleanup();
      }

      cleanup_completed = true;
      std::cout << "Heimdall Gold Plugin finalized\n";
   }

   // Plugin unload function
   void onunload()
   {
      if (!cleanup_completed) {
         if (verbose) {
            std::cout << "Heimdall: onunload() called, cleanup not yet completed - calling heimdall_finalize()\n";
         }
         heimdall_finalize();
      } else {
         if (verbose) {
            std::cout << "Heimdall: onunload() called, cleanup already completed via cleanup handler\n";
         }
      }
      
      globalAdapter.reset();
      std::cout << "Heimdall Gold Plugin unloaded\n";
   }

   // Symbol processing function
   int heimdall_process_symbol(const char* symbolName, uint64_t address, uint64_t size)
   {
      if (!globalAdapter || !symbolName)
         return -1;

      if (verbose)
      {
         std::cout << "Heimdall: Processing symbol: " << symbolName << " (address: 0x" << std::hex
                   << address << ", size: " << std::dec << size << ")\n";
      }

      // Process the symbol through the adapter
      globalAdapter->processSymbol(std::string(symbolName), address, size);

      return 0;
   }

   // Plugin option handling function
   int heimdall_gold_set_plugin_option(const char* option)
   {
      if (!option)
         return -1;

      std::string opt(option);

      if (verbose)
      {
         std::cout << "Heimdall: Setting plugin option: " << opt << "\n";
      }

      // Parse common plugin options (handle both full --plugin-opt= format and raw option values)
      if (opt.find("--plugin-opt=output=") == 0)
      {
         std::string path = opt.substr(20);  // Remove "--plugin-opt=output="
         return heimdall_set_output_path(path.c_str());
      }
      else if (opt.find("output=") == 0)
      {
         std::string path = opt.substr(7);  // Remove "output="
         return heimdall_set_output_path(path.c_str());
      }
      else if (opt.find("--plugin-opt=format=") == 0)
      {
         std::string fmt = opt.substr(19);  // Remove "--plugin-opt=format="
         return heimdall_set_format(fmt.c_str());
      }
      else if (opt.find("format=") == 0)
      {
         std::string fmt = opt.substr(7);  // Remove "format="
         return heimdall_set_format(fmt.c_str());
      }
      else if (opt.find("--plugin-opt=verbose") == 0 || opt == "verbose")
      {
         heimdall_set_verbose(true);
         return 0;
      }
      else if (opt.find("--plugin-opt=cyclonedx-version=") == 0)
      {
         std::string version = opt.substr(29);  // Remove "--plugin-opt=cyclonedx-version="
         return heimdall_set_cyclonedx_version(version.c_str());
      }
      else if (opt.find("cyclonedx-version=") == 0)
      {
         std::string version = opt.substr(18);  // Remove "cyclonedx-version="
         return heimdall_set_cyclonedx_version(version.c_str());
      }
      else if (opt.find("--plugin-opt=spdx-version=") == 0)
      {
         std::string version = opt.substr(24);  // Remove "--plugin-opt=spdx-version="
         if (globalAdapter)
         {
            globalAdapter->setSPDXVersion(version);
         }
         return 0;
      }
      else if (opt.find("spdx-version=") == 0)
      {
         std::string version = opt.substr(13);  // Remove "spdx-version="
         if (globalAdapter)
         {
            globalAdapter->setSPDXVersion(version);
         }
         return 0;
      }

      return 0;  // Unknown option, not an error
   }
}
