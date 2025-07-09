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
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../common/MetadataExtractor.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/Utils.hpp"
#include "LLDAdapter.hpp"
#include "LLDPlugin.hpp"
#include "../compat/compatibility.hpp"

#ifdef LLVM_AVAILABLE
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#endif

// LLD Plugin Interface - Simplified
#include "lld/Common/Driver.h"

namespace {
std::unique_ptr<heimdall::LLDAdapter> globalAdapter;
std::unique_ptr<heimdall::SBOMGenerator> globalSBOMGenerator;
std::string outputPath = "heimdall-sbom.json";
std::string format = "spdx";
bool verbose = false;
std::vector<std::string> processedFiles;
std::vector<std::string> processedLibraries;
}  // namespace

extern "C" {

// Plugin initialization
int onload(void* /*tv*/) {
    std::cout << "Heimdall LLD Plugin activated\n";

    // Initialize the adapter
    globalAdapter = heimdall::compat::make_unique<heimdall::LLDAdapter>();
    globalAdapter->initialize();

    // Initialize the SBOM generator
    globalSBOMGenerator = heimdall::compat::make_unique<heimdall::SBOMGenerator>();
    // globalSBOMGenerator->initialize(); // Removed: SBOMGenerator does not have initialize()

    if (verbose) {
        std::cout << "Heimdall LLD Plugin initialized with output: " << outputPath << "\n";
    }

    return 0;
}

// Plugin cleanup
void onunload() {
    if (globalAdapter) {
        globalAdapter->finalize();
    }

    if (globalSBOMGenerator) {
        globalSBOMGenerator->generateSBOM();
    }

    std::cout << "Heimdall LLD Plugin deactivated\n";
}

// Plugin metadata
const char* heimdall_lld_version() {
    return "1.0.0";
}

const char* heimdall_lld_description() {
    return "Heimdall SBOM Generator Plugin for LLVM LLD Linker";
}

// Configuration functions
int heimdall_set_output_path(const char* path) {
    if (path) {
        outputPath = std::string(path);
        if (globalSBOMGenerator) {
            globalSBOMGenerator->setOutputPath(outputPath);
        }
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
        if (globalSBOMGenerator) {
            globalSBOMGenerator->setFormat(format);
        }
        if (verbose) {
            std::cout << "Heimdall: Format set to " << format << "\n";
        }
        return 0;
    }
    return -1;
}

void heimdall_set_verbose(bool v) {
    verbose = v;
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

    // Extract component info and add to SBOM
    heimdall::ComponentInfo component(heimdall::Utils::getFileName(path), path);
    component.setDetectedBy(heimdall::LinkerType::LLD);

    // Determine file type
    if (heimdall::Utils::isObjectFile(path)) {
        component.fileType = heimdall::FileType::Object;
    } else if (heimdall::Utils::isStaticLibrary(path)) {
        component.fileType = heimdall::FileType::StaticLibrary;
    } else if (heimdall::Utils::isSharedLibrary(path)) {
        component.fileType = heimdall::FileType::SharedLibrary;
    } else if (heimdall::Utils::isExecutable(path)) {
        component.fileType = heimdall::FileType::Executable;
    }

    // Calculate checksum
    component.checksum = heimdall::Utils::calculateSHA256(path);
    component.fileSize = heimdall::Utils::getFileSize(path);

    // Add to SBOM generator
    if (globalSBOMGenerator) {
        // Set minimum SBOM fields for main binary
        if (component.supplier.empty())
            component.supplier = "Organization: UNKNOWN";
        if (component.downloadLocation.empty())
            component.downloadLocation = "NOASSERTION";
        if (component.homepage.empty())
            component.homepage = "N/A";
        if (component.version.empty())
            component.version = "UNKNOWN";
        if (component.license.empty()) {
            // Try to detect license from name and path
            std::string detectedLicense = heimdall::Utils::detectLicenseFromName(component.name);
            if (detectedLicense == "NOASSERTION") {
                detectedLicense = heimdall::Utils::detectLicenseFromPath(component.filePath);
            }
            component.license = detectedLicense;
        }
        // SPDX requires copyright
        // (We don't extract, so use NOASSERTION)
        // component.copyright = "NOASSERTION"; // Not in struct, handled in SBOMGenerator
        globalSBOMGenerator->processComponent(component);
    }

    // Also process linked libraries as components
    std::vector<std::string> deps = heimdall::MetadataHelpers::detectDependencies(path);
    for (const auto& dep : deps) {
        // Try to resolve the library path
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
            heimdall::ComponentInfo libComponent(heimdall::Utils::getFileName(depPath), depPath);
            libComponent.setDetectedBy(heimdall::LinkerType::LLD);
            libComponent.fileType = heimdall::FileType::SharedLibrary;
            libComponent.checksum = heimdall::Utils::calculateSHA256(depPath);
            libComponent.fileSize = heimdall::Utils::getFileSize(depPath);
            // Set minimum SBOM fields for library
            libComponent.supplier = "Organization: UNKNOWN";
            libComponent.downloadLocation = "NOASSERTION";
            libComponent.homepage = "N/A";
            libComponent.version = "UNKNOWN";
            // Detect license for library
            std::string detectedLicense = heimdall::Utils::detectLicenseFromName(libComponent.name);
            if (detectedLicense == "NOASSERTION") {
                detectedLicense = heimdall::Utils::detectLicenseFromPath(depPath);
            }
            libComponent.license = detectedLicense;
            // libComponent.copyright = "NOASSERTION"; // Not in struct
            if (globalSBOMGenerator) {
                globalSBOMGenerator->processComponent(libComponent);
            }
        }
    }

    return 0;  // Success
}

void heimdall_process_library(const char* libraryPath) {
    if (!globalAdapter || !libraryPath)
        return;

    std::string path(libraryPath);

    // Check if already processed
    if (std::find(processedLibraries.begin(), processedLibraries.end(), path) !=
        processedLibraries.end()) {
        return;
    }

    processedLibraries.push_back(path);

    if (verbose) {
        std::cout << "Heimdall: Processing library: " << path << "\n";
    }

    // Process the library through the adapter
    globalAdapter->processInputFile(path);

    // Extract component info and add to SBOM
    heimdall::ComponentInfo component(heimdall::Utils::getFileName(path), path);
    component.setDetectedBy(heimdall::LinkerType::LLD);

    // Determine file type
    if (heimdall::Utils::isStaticLibrary(path)) {
        component.fileType = heimdall::FileType::StaticLibrary;
    } else if (heimdall::Utils::isSharedLibrary(path)) {
        component.fileType = heimdall::FileType::SharedLibrary;
    }

    // Calculate checksum
    component.checksum = heimdall::Utils::calculateSHA256(path);
    component.fileSize = heimdall::Utils::getFileSize(path);

    // Add to SBOM generator
    if (globalSBOMGenerator) {
        globalSBOMGenerator->processComponent(component);
    }
}

// Finalization
void heimdall_finalize() {
    if (verbose) {
        std::cout << "Heimdall: Finalizing SBOM generation\n";
    }

    if (globalSBOMGenerator) {
        globalSBOMGenerator->generateSBOM();

        if (verbose) {
            std::cout << "Heimdall: SBOM generated with "
                      << globalSBOMGenerator->getComponentCount() << " components\n";
            globalSBOMGenerator->printStatistics();
        }
    }
}

int heimdall_set_cyclonedx_version(const char* version) {
    if (version && globalSBOMGenerator) {
        globalSBOMGenerator->setCycloneDXVersion(version);
        return 0;
    }
    return -1;
}

}  // extern "C"

// LLVM Pass Plugin Interface (Legacy Pass Manager)
#ifdef LLVM_AVAILABLE
namespace {
class HeimdallPass : public llvm::ModulePass {
public:
    static char ID;
    HeimdallPass() : ModulePass(ID) {}

    bool runOnModule(llvm::Module& M) override {
        if (verbose) {
            llvm::errs() << "Heimdall: Processing module: " << M.getModuleIdentifier() << "\n";
        }

        // Process the module file
        std::string moduleName = M.getModuleIdentifier();
        if (!moduleName.empty()) {
            heimdall_process_input_file(moduleName.c_str());
        }

        // Process functions as symbols
        for (auto& F : M) {
            if (!F.isDeclaration()) {
                std::string funcName = F.getName().str();
                if (verbose) {
                    llvm::errs() << "Heimdall: Processing function: " << funcName << "\n";
                }
            }
        }

        // Process global variables as symbols
        for (auto& GV : M.globals()) {
            if (!GV.isDeclaration()) {
                std::string varName = GV.getName().str();
                if (verbose) {
                    llvm::errs() << "Heimdall: Processing global: " << varName << "\n";
                }
            }
        }

        return false;  // Don't modify the module
    }

    virtual llvm::StringRef getPassName() const override {
        return "Heimdall SBOM Generator";
    }
};
}  // namespace

char HeimdallPass::ID = 0;

// Simple pass registration
extern "C" {
void heimdall_register_pass() {
    // This will be called when the plugin is loaded
    std::cout << "Heimdall: LLVM Pass registered\n";
}
}
#endif

// LLD Plugin Interface - Working Implementation
extern "C" {
// LLD Plugin entry point - called when plugin is loaded
void heimdall_lld_plugin_init() {
    std::cout << "Heimdall: LLD Plugin loaded and initialized\n";

    // Initialize our plugin
    onload(nullptr);
}

// LLD Plugin cleanup - called when plugin is unloaded
void heimdall_lld_plugin_cleanup() {
    std::cout << "Heimdall: LLD Plugin cleanup\n";

    // Finalize SBOM generation
    heimdall_finalize();

    // Cleanup plugin
    onunload();
}

// LLD Plugin hook for file processing
void heimdall_lld_process_file(const char* filePath) {
    if (filePath) {
        heimdall_process_input_file(filePath);
    }
}

// LLD Plugin hook for library processing
void heimdall_lld_process_library(const char* libraryPath) {
    if (libraryPath) {
        heimdall_process_library(libraryPath);
    }
}
}
