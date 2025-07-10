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
#include "../compat/compatibility.hpp"

namespace {
std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
std::string outputPath = "heimdall-gold-sbom.json";
std::string format = "spdx";
bool verbose = false;
std::vector<std::string> processedFiles;
std::vector<std::string> processedLibraries;
const std::string cyclonedxVersion = "1.6"; // NEW: store requested CycloneDX version

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

extern "C" {
int onload(void* handle) {
    std::cout << "Heimdall Gold Plugin activated\n";

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

// Plugin cleanup and finalization
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->generateSBOM();
        globalAdapter->cleanup();
    }

    std::cout << "Heimdall Gold Plugin finalized\n";
}
}
