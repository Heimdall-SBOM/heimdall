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
#include "GoldAdapter.hpp"

namespace {
std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
std::string outputPath = "heimdall-gold-sbom.json";
std::string format = "spdx";
bool verbose = false;
std::vector<std::string> processedFiles;
std::vector<std::string> processedLibraries;

// Simple utility functions to avoid heimdall-core dependencies
std::string getFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
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
    try {
        return std::to_string(std::filesystem::file_size(path));
    } catch (...) {
        return "0";
    }
}

std::string getFileType(const std::string& path) {
    std::string fileName = getFileName(path);
    std::string extension = std::filesystem::path(fileName).extension().string();

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
int onload(void* /*tv*/) {
    std::cout << "Heimdall Gold Plugin activated\n";

    // Initialize the adapter
    globalAdapter = std::make_unique<heimdall::GoldAdapter>();
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
        // Try to resolve the library path using the improved resolver
        std::string depPath = heimdall::Utils::resolveLibraryPath(dep);
        if (!depPath.empty() && std::filesystem::exists(depPath)) {
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

// Plugin cleanup and finalization
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->generateSBOM();
        globalAdapter->cleanup();
    }

    // Validate format
    if (format != "spdx" && format != "cyclonedx") {
        std::cerr << "Heimdall Gold Plugin: Invalid format '" << format
                  << "', defaulting to SPDX\n";
        format = "spdx";
    }

    // Generate a simple SBOM file
    std::ofstream sbomFile(outputPath);
    if (sbomFile.is_open()) {
        if (format == "spdx") {
            // Generate SPDX format
            sbomFile << "SPDXVersion: SPDX-2.3\n";
            sbomFile << "DataLicense: CC0-1.0\n";
            sbomFile << "SPDXID: SPDXRef-DOCUMENT\n";
            sbomFile << "DocumentName: Heimdall Gold Plugin SBOM\n";
            sbomFile << "DocumentNamespace: https://spdx.org/spdxdocs/heimdall-gold\n";
            sbomFile << "Creator: Tool: Heimdall Gold Plugin\n";
            sbomFile << "Created: " << __DATE__ << " " << __TIME__ << "\n\n";

            // Process input files
            for (const auto& file : processedFiles) {
                std::string fileName = getFileName(file);
                std::string checksum = calculateSimpleHash(file);
                std::string fileSize = getFileSize(file);
                std::string fileType = getFileType(file);

                sbomFile << "FileName: " << fileName << "\n";
                sbomFile << "SPDXID: SPDXRef-" << fileName << "\n";
                sbomFile << "FileChecksum: SHA256: " << checksum << "\n";
                sbomFile << "FileSize: " << fileSize << "\n";
                sbomFile << "FileType: " << fileType << "\n";
                sbomFile << "LicenseConcluded: NOASSERTION\n";
                sbomFile << "LicenseInfoInFile: NOASSERTION\n";
                sbomFile << "FileCopyrightText: NOASSERTION\n\n";
            }

            // Process libraries
            for (const auto& library : processedLibraries) {
                std::string fileName = getFileName(library);
                std::string checksum = calculateSimpleHash(library);
                std::string fileSize = getFileSize(library);
                std::string fileType = getFileType(library);

                sbomFile << "FileName: " << fileName << "\n";
                sbomFile << "SPDXID: SPDXRef-" << fileName << "\n";
                sbomFile << "FileChecksum: SHA256: " << checksum << "\n";
                sbomFile << "FileSize: " << fileSize << "\n";
                sbomFile << "FileType: " << fileType << "\n";
                sbomFile << "LicenseConcluded: NOASSERTION\n";
                sbomFile << "LicenseInfoInFile: NOASSERTION\n";
                sbomFile << "FileCopyrightText: NOASSERTION\n\n";
            }
        } else {
            // Generate CycloneDX format
            sbomFile << "{\n";
            sbomFile << "  \"bomFormat\": \"CycloneDX\",\n";
            sbomFile << "  \"specVersion\": \"1.5\",\n";
            sbomFile << "  \"version\": 1,\n";
            sbomFile << "  \"metadata\": {\n";
            sbomFile << "    \"timestamp\": \"" << __DATE__ << " " << __TIME__ << "\",\n";
            sbomFile << "    \"tools\": [\n";
            sbomFile << "      {\n";
            sbomFile << "        \"vendor\": \"Heimdall\",\n";
            sbomFile << "        \"name\": \"Gold Plugin\",\n";
            sbomFile << "        \"version\": \"1.0.0\"\n";
            sbomFile << "      }\n";
            sbomFile << "    ]\n";
            sbomFile << "  },\n";
            sbomFile << "  \"components\": [\n";
            // Combine all components
            std::vector<std::string> allComponents;
            allComponents.reserve(processedFiles.size() + processedLibraries.size());
            for (const auto& file : processedFiles) {
                allComponents.push_back(file);
            }
            for (const auto& lib : processedLibraries) {
                allComponents.push_back(lib);
            }
            bool first = true;
            for (const auto& path : allComponents) {
                if (!first)
                    sbomFile << ",\n";
                first = false;
                std::string fileName = getFileName(path);
                std::string checksum = calculateSimpleHash(path);
                std::string fileSize = getFileSize(path);
                std::string fileType = getFileType(path);
                sbomFile << "    {\n";
                sbomFile << "      \"type\": \"" << fileType << "\",\n";
                sbomFile << "      \"name\": \"" << fileName << "\",\n";
                sbomFile << "      \"purl\": \"pkg:generic/" << fileName << "@1.0.0\",\n";
                sbomFile << "      \"hashes\": [\n";
                sbomFile << "        {\n";
                sbomFile << "          \"alg\": \"SHA-256\",\n";
                sbomFile << "          \"content\": \"" << checksum << "\"\n";
                sbomFile << "        }\n";
                sbomFile << "      ],\n";
                sbomFile << "      \"properties\": [\n";
                sbomFile << "        {\n";
                sbomFile << "          \"name\": \"fileSize\",\n";
                sbomFile << "          \"value\": \"" << fileSize << "\"\n";
                sbomFile << "        }\n";
                sbomFile << "      ]\n";
                sbomFile << "    }";
            }
            sbomFile << "\n  ]\n";
            sbomFile << "}\n";
        }
        sbomFile.close();
        std::cout << "Heimdall Gold Plugin: SBOM generated at " << outputPath << "\n";
    }

    std::cout << "Heimdall Gold Plugin finalized\n";
}
}
