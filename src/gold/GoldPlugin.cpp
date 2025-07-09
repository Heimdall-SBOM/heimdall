/**
 * @file GoldPlugin.cpp
 * @brief Heimdall plugin for GNU Gold linker: SBOM generation and metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This file implements the Heimdall plugin for the GNU Gold linker, enabling
 * SBOM generation and metadata extraction during the link process. It provides
 * C interface functions for plugin configuration and file processing.
 */
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "../common/MetadataExtractor.hpp"
#include "GoldAdapter.hpp"

namespace {
std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
// Non-const global variables - these are modified at runtime by plugin configuration functions
std::string outputPath = "heimdall-gold-sbom.json";  // Modified by heimdall_set_output_path()
std::string format = "spdx";                         // Modified by heimdall_set_format()
std::string spdxVersion = "3.0";                     // Modified by heimdall_set_spdx_version()
bool verbose = false;                                // Modified by heimdall_set_verbose()
std::vector<std::string> processedFiles;             // Modified during file processing
std::vector<std::string> processedLibraries;         // Modified during library processing

// Simple utility functions to avoid heimdall-core dependencies
std::string getFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
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
/**
 * @brief Plugin initialization function called when the plugin is loaded
 * @param tv Unused (reserved for future use)
 * @return 0 on success
 */
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

/**
 * @brief Get the version string for the Heimdall Gold plugin
 * @return Version string
 */
const char* heimdall_gold_version() {
    return "1.0.0";
}

/**
 * @brief Get the description string for the Heimdall Gold plugin
 * @return Description string
 */
const char* heimdall_gold_description() {
    return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
}

/**
 * @brief Set the output path for the generated SBOM
 * @param path Output file path (C string)
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Set the output format for the generated SBOM
 * @param fmt Output format (C string, e.g., "spdx" or "cyclonedx")
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Set the CycloneDX version for SBOM output
 * @param version CycloneDX version string
 * @return 0 on success, -1 on error
 */
int heimdall_set_cyclonedx_version(const char* version) {
    if (version) {
        if (globalAdapter) {
            globalAdapter->setCycloneDXVersion(version);
        }
        if (verbose) {
            std::cout << "Heimdall: CycloneDX version set to " << version << "\n";
        }
        return 0;
    }
    return -1;
}

/**
 * @brief Set the SPDX version for SBOM output
 * @param version SPDX version string
 * @return 0 on success, -1 on error
 */
int heimdall_set_spdx_version(const char* version) {
    if (version) {
        spdxVersion = std::string(version);
        if (globalAdapter) {
            globalAdapter->setSPDXVersion(version);
        }
        if (verbose) {
            std::cout << "Heimdall: SPDX version set to " << version << "\n";
        }
        return 0;
    }
    return -1;
}

/**
 * @brief Set verbose output mode
 * @param v true to enable verbose output, false to disable
 */
void heimdall_set_verbose(bool v) {
    verbose = v;
    if (globalAdapter)
        globalAdapter->setVerbose(v);
}

/**
 * @brief Process an input file for SBOM generation
 * @param filePath Path to the input file (C string)
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Process a library file for SBOM generation
 * @param libraryPath Path to the library file (C string)
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Finalize the plugin and generate the SBOM
 */
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->generateSBOM();
        globalAdapter->cleanup();
    }
}
}
