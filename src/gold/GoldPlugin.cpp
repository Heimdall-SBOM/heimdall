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
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "../common/MetadataExtractor.hpp"
#include "../common/Utils.hpp"
#include "GoldAdapter.hpp"
#include "GoldPlugin.hpp"
#include "../compat/compatibility.hpp"

namespace {
std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
std::string outputPath = "heimdall-gold-sbom.json";
std::string format = "spdx";
bool verbose = false;
std::vector<std::string> processedFiles;
std::vector<std::string> processedLibraries;
std::string cyclonedxVersion = "1.6"; // NEW: store requested CycloneDX version

// Simple utility functions to avoid heimdall-core dependencies
std::string getFileName(const std::string& path) {
    return heimdall::Utils::getFileName(path);
}

bool fileExists(const std::string& path) {
    return heimdall::Utils::fileExists(path);
}

std::string calculateSimpleHash(const std::string& path) {
    // Simple hash calculation without OpenSSL dependency
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return "NOASSERTION";

    // Read file content and calculate hash
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (content.empty()) {
        return "NOASSERTION";
    }

    // Calculate a simple hash of the content
    std::hash<std::string> hasher;
    std::stringstream ss;
    ss << std::hex << hasher(content);
    return ss.str();
}

std::string getFileSize(const std::string& path) {
    if (heimdall::Utils::fileExists(path)) {
        return std::to_string(heimdall::Utils::getFileSize(path));
    }
    return "0";
}

std::string getFileType(const std::string& fileName) {
    std::string extension = heimdall::Utils::getFileExtension(fileName);

    if (extension == ".o" || extension == ".obj") {
        return "OBJECT";
    }
    if (extension == ".a") {
        return "ARCHIVE";
    }
    if (extension == ".so" || extension == ".dylib" || extension == ".dll") {
        return "SHARED_LIBRARY";
    }
    if (extension == ".exe") {
        return "EXECUTABLE";
    }
    return "OTHER";
}
}  // namespace

// GoldPlugin class implementation
namespace heimdall {

class GoldPlugin::Impl {
public:
    Impl() : adapter(heimdall::compat::make_unique<GoldAdapter>()), verbose(false) {}
    
    std::unique_ptr<GoldAdapter> adapter;
    std::vector<std::string> processedFiles;
    std::vector<std::string> processedLibraries;
    std::vector<std::string> processedSymbols;
    std::string outputPath = "heimdall-gold-sbom.json";
    std::string format = "spdx";
    std::string cyclonedxVersion = "1.6";
    std::string spdxVersion = "2.3";
    bool verbose = false;
    bool extractDebugInfo = true;
    bool includeSystemLibraries = false;
};

GoldPlugin::GoldPlugin() : pImpl(heimdall::compat::make_unique<Impl>()) {}

GoldPlugin::~GoldPlugin() = default;

bool GoldPlugin::initialize() {
    try {
        return pImpl->adapter->initialize();
    } catch (...) {
        return false;
    }
}

void GoldPlugin::cleanup() {
    try {
        pImpl->adapter->cleanup();
    } catch (...) {
        // Ignore cleanup errors
    }
}

void GoldPlugin::processInputFile(const std::string& filePath) {
    try {
        pImpl->adapter->processInputFile(filePath);
        pImpl->processedFiles.push_back(filePath);
    } catch (...) {
        // Ignore processing errors
    }
}

void GoldPlugin::processLibrary(const std::string& libraryPath) {
    try {
        pImpl->adapter->processLibrary(libraryPath);
        pImpl->processedLibraries.push_back(libraryPath);
    } catch (...) {
        // Ignore processing errors
    }
}

void GoldPlugin::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) {
    try {
        pImpl->adapter->processSymbol(symbolName, address, size);
        pImpl->processedSymbols.push_back(symbolName);
    } catch (...) {
        // Ignore processing errors
    }
}

void GoldPlugin::setOutputPath(const std::string& path) {
    pImpl->outputPath = path;
    pImpl->adapter->setOutputPath(path);
}

void GoldPlugin::setFormat(const std::string& format) {
    pImpl->format = format;
    pImpl->adapter->setFormat(format);
}

void GoldPlugin::setCycloneDXVersion(const std::string& version) {
    pImpl->cyclonedxVersion = version;
    pImpl->adapter->setCycloneDXVersion(version);
}

void GoldPlugin::setSPDXVersion(const std::string& version) {
    pImpl->spdxVersion = version;
    pImpl->adapter->setSPDXVersion(version);
}

void GoldPlugin::generateSBOM() {
    try {
        pImpl->adapter->generateSBOM();
    } catch (...) {
        // Ignore generation errors
    }
}

void GoldPlugin::setVerbose(bool verbose) {
    pImpl->verbose = verbose;
    pImpl->adapter->setVerbose(verbose);
}

void GoldPlugin::setExtractDebugInfo(bool extract) {
    pImpl->extractDebugInfo = extract;
    pImpl->adapter->setExtractDebugInfo(extract);
}

void GoldPlugin::setIncludeSystemLibraries(bool include) {
    pImpl->includeSystemLibraries = include;
    pImpl->adapter->setIncludeSystemLibraries(include);
}

size_t GoldPlugin::getComponentCount() const {
    return pImpl->adapter->getComponentCount();
}

std::vector<std::string> GoldPlugin::getProcessedFiles() const {
    return pImpl->processedFiles;
}

std::vector<std::string> GoldPlugin::getProcessedLibraries() const {
    return pImpl->processedLibraries;
}

std::vector<std::string> GoldPlugin::getProcessedSymbols() const {
    return pImpl->processedSymbols;
}

void GoldPlugin::printStatistics() const {
    pImpl->adapter->printStatistics();
}

std::string GoldPlugin::getVersion() const {
    return "1.0.0";
}

std::string GoldPlugin::getDescription() const {
    return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
}

} // namespace heimdall

extern "C" {
int onload(void* handle) {
    std::cout << "Heimdall Gold Plugin activated\n";

    // Reset all global state
    processedFiles.clear();
    processedLibraries.clear();
    format = "spdx";
    outputPath = "heimdall-gold-sbom.json";
    verbose = false;

    // Initialize the adapter
    globalAdapter = heimdall::compat::make_unique<heimdall::GoldAdapter>();
    globalAdapter->initialize();

    if (verbose) {
        std::cout << "Heimdall Gold Plugin initialized with output: " << outputPath << "\n";
    }

    return 0;
}

const char* heimdall_gold_version() {
    return "1.0.0";
}

const char* heimdall_gold_description() {
    return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
}

// Configuration functions
int heimdall_set_output_path(const char* path) {
    if (path) {
        outputPath = std::string(path);
        if (globalAdapter)
            globalAdapter->setOutputPath(outputPath);
        if (verbose) {
            std::cout << "Heimdall: Output path set to " << outputPath << "\n";
        }
        return 0;
    }
    return -1;
}

int heimdall_set_format(const char* fmt) {
    if (fmt) {
        format = std::string(fmt);
        if (globalAdapter)
            globalAdapter->setFormat(format);
        if (verbose) {
            std::cout << "Heimdall: Format set to " << format << "\n";
        }
        return 0;
    }
    return -1;
}

void heimdall_set_verbose(bool v) {
    verbose = v;
    if (globalAdapter)
        globalAdapter->setVerbose(v);
}

// File processing functions
int heimdall_process_input_file(const char* filePath) {
    if (!globalAdapter || !filePath)
        return -1;

    std::string path(filePath);

    // Check if already processed
    if (std::find(processedFiles.begin(), processedFiles.end(), path) != processedFiles.end()) {
        return 0;  // Already processed, not an error
    }

    processedFiles.push_back(path);

    if (verbose) {
        std::cout << "Heimdall: Processing input file: " << path << "\n";
    }

    // Process the file through the adapter
    globalAdapter->processInputFile(path);

    // --- NEW: Detect and process dependencies ---
    std::vector<std::string> deps = heimdall::MetadataHelpers::detectDependencies(path);
    for (const auto& dep : deps) {
        std::string depPath;
        // Absolute path? Use as is
        if (!dep.empty() && dep[0] == '/') {
            depPath = dep;
        } else {
            // Search standard library paths
            std::vector<std::string> libPaths = {
                "/usr/lib", "/usr/local/lib", "/opt/local/lib", "/opt/homebrew/lib",
                "/lib",     "/lib64",         "/usr/lib64"};
            for (const auto& libDir : libPaths) {
                std::string candidate = libDir;
                candidate += "/";
                candidate += dep;
                if (heimdall::Utils::fileExists(candidate)) {
                    depPath = candidate;
                    break;
                }
            }
        }
        if (!depPath.empty() && heimdall::Utils::fileExists(depPath)) {
            // Avoid duplicate processing
            if (std::find(processedLibraries.begin(), processedLibraries.end(), depPath) ==
                processedLibraries.end()) {
                processedLibraries.push_back(depPath);
                if (verbose) {
                    std::cout << "Heimdall: Auto-processing dependency library: " << depPath
                              << "\n";
                }
                globalAdapter->processLibrary(depPath);
            }
        } else if (verbose) {
            std::cout << "Heimdall: Could not resolve dependency: " << dep << "\n";
        }
    }
    // --- END NEW ---

    // Generate a simple SBOM entry
    std::string fileName = getFileName(path);
    std::string checksum = calculateSimpleHash(path);
    std::string fileSize = getFileSize(path);

    if (verbose) {
        std::cout << "Heimdall: Processed file: " << fileName << " (checksum: " << checksum
                  << ", size: " << fileSize << ")\n";
    }

    return 0;
}

int heimdall_process_library(const char* libraryPath) {
    if (!globalAdapter || !libraryPath)
        return -1;

    std::string path(libraryPath);

    // Check if already processed
    if (std::find(processedLibraries.begin(), processedLibraries.end(), path) !=
        processedLibraries.end()) {
        return 0;  // Already processed, not an error
    }

    processedLibraries.push_back(path);

    if (verbose) {
        std::cout << "Heimdall: Processing library: " << path << "\n";
    }

    // Process the library through the adapter
    globalAdapter->processLibrary(path);

    // Generate a simple SBOM entry
    std::string fileName = getFileName(path);
    std::string checksum = calculateSimpleHash(path);
    std::string fileSize = getFileSize(path);

    if (verbose) {
        std::cout << "Heimdall: Processed library: " << fileName << " (checksum: " << checksum
                  << ", size: " << fileSize << ")\n";
    }

    return 0;
}

int heimdall_set_cyclonedx_version(const char* version) {
    if (version) {
        cyclonedxVersion = version;
        return 0;
    }
    return -1;
}

int heimdall_set_spdx_version(const char* version) {
    if (globalAdapter && version) {
        globalAdapter->setSPDXVersion(std::string(version));
        return 0;
    }
    return -1;
}

// Plugin cleanup and finalization
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->generateSBOM();
        globalAdapter->cleanup();
    }

    std::cout << "Heimdall Gold Plugin finalized\n";
}

// Plugin unload function
void onunload() {
    heimdall_finalize();
    globalAdapter.reset();
    std::cout << "Heimdall Gold Plugin unloaded\n";
}

// Symbol processing function
int heimdall_process_symbol(const char* symbolName, uint64_t address, uint64_t size) {
    if (!globalAdapter || !symbolName)
        return -1;

    if (verbose) {
        std::cout << "Heimdall: Processing symbol: " << symbolName 
                  << " (address: 0x" << std::hex << address 
                  << ", size: " << std::dec << size << ")\n";
    }

    // Process the symbol through the adapter
    globalAdapter->processSymbol(std::string(symbolName), address, size);

    return 0;
}

// Plugin option handling function
int heimdall_gold_set_plugin_option(const char* option) {
    if (!option)
        return -1;

    std::string opt(option);
    
    if (verbose) {
        std::cout << "Heimdall: Setting plugin option: " << opt << "\n";
    }

    // Parse common plugin options
    if (opt.find("--plugin-opt=output=") == 0) {
        std::string path = opt.substr(20); // Remove "--plugin-opt=output="
        return heimdall_set_output_path(path.c_str());
    } else if (opt.find("--plugin-opt=format=") == 0) {
        std::string fmt = opt.substr(19); // Remove "--plugin-opt=format="
        return heimdall_set_format(fmt.c_str());
    } else if (opt.find("--plugin-opt=verbose") == 0) {
        heimdall_set_verbose(true);
        return 0;
    } else if (opt.find("--plugin-opt=cyclonedx-version=") == 0) {
        std::string version = opt.substr(29); // Remove "--plugin-opt=cyclonedx-version="
        return heimdall_set_cyclonedx_version(version.c_str());
    } else if (opt.find("--plugin-opt=spdx-version=") == 0) {
        std::string version = opt.substr(24); // Remove "--plugin-opt=spdx-version="
        if (globalAdapter) {
            globalAdapter->setSPDXVersion(version);
        }
        return 0;
    }

    return 0; // Unknown option, not an error
}
}
