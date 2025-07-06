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
// Non-const global variables - these are modified at runtime by plugin configuration functions
std::string outputPath = "heimdall-sbom.json";        // Modified by heimdall_set_output_path()
std::string format = "spdx";                          // Modified by heimdall_set_format()
std::string spdxVersion = "3.0";                      // Modified by heimdall_set_spdx_version()
bool verbose = false;                                 // Modified by heimdall_set_verbose()
bool extractDebugInfo = true;                         // Modified by heimdall_set_extract_debug_info()
bool includeSystemLibraries = false;                  // Modified by heimdall_set_include_system_libraries()
}  // namespace

// Forward declarations for C functions
extern "C" {
int heimdall_process_input_file(const char* filePath);
int heimdall_process_library(const char* libraryPath);
}

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

char HeimdallPass::ID = 0;
}  // namespace

// Simple pass registration
extern "C" {
void heimdall_register_pass() {
    // This will be called when the plugin is loaded
    std::cout << "Heimdall: LLVM Pass registered\n";
}
}
#endif

extern "C" {

// Plugin initialization
int onload(void* /*tv*/) {
    std::cout << "Heimdall LLD Plugin activated\n";

    // Initialize the enhanced adapter
    globalAdapter = std::make_unique<heimdall::LLDAdapter>();
    globalAdapter->initialize();
    globalAdapter->setOutputPath(outputPath);
    globalAdapter->setFormat(format);
    globalAdapter->setVerbose(verbose);
    globalAdapter->setExtractDebugInfo(extractDebugInfo);
    globalAdapter->setIncludeSystemLibraries(includeSystemLibraries);

    if (verbose) {
        std::cout << "Heimdall LLD Plugin initialized with output: " << outputPath << "\n";
        std::cout << "Heimdall LLD Plugin: DWARF extraction " << (extractDebugInfo ? "enabled" : "disabled") << "\n";
    }

    return 0;
}

// Plugin cleanup
void onunload() {
    if (globalAdapter) {
        globalAdapter->finalize();
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
        if (globalAdapter) {
            globalAdapter->setOutputPath(outputPath);
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
        if (globalAdapter) {
            globalAdapter->setFormat(format);
        }
        if (verbose) {
            std::cout << "Heimdall: Format set to " << format << "\n";
        }
        return 0;
    }
    return -1;
}

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

void heimdall_set_verbose(bool v) {
    verbose = v;
    if (globalAdapter) {
        globalAdapter->setVerbose(v);
    }
}

void heimdall_set_extract_debug_info(bool extract) {
    extractDebugInfo = extract;
    if (globalAdapter) {
        globalAdapter->setExtractDebugInfo(extract);
    }
}

void heimdall_set_include_system_libraries(bool include) {
    includeSystemLibraries = include;
    if (globalAdapter) {
        globalAdapter->setIncludeSystemLibraries(include);
    }
}

// File processing functions
int heimdall_process_input_file(const char* filePath) {
    if (!globalAdapter || !filePath)
        return -1;

    std::string path(filePath);

    if (verbose) {
        std::cout << "Heimdall: Processing input file: " << path << "\n";
    }

    // Process the file through the enhanced adapter
    globalAdapter->processInputFile(path);

    return 0;
}

int heimdall_process_library(const char* libraryPath) {
    if (!globalAdapter || !libraryPath)
        return -1;

    std::string path(libraryPath);

    if (verbose) {
        std::cout << "Heimdall: Processing library: " << path << "\n";
    }

    // Process the library through the enhanced adapter
    globalAdapter->processLibrary(path);

    return 0;
}

void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->finalize();
    }
}

size_t heimdall_get_component_count() {
    if (globalAdapter) {
        return globalAdapter->getComponentCount();
    }
    return 0;
}

void heimdall_print_statistics() {
    if (globalAdapter) {
        globalAdapter->printStatistics();
    }
}

// LLD Plugin Interface - Working Implementation
void heimdall_lld_plugin_init() {
    onload(nullptr);
}

void heimdall_lld_plugin_cleanup() {
    onunload();
}

void heimdall_lld_process_file(const char* filePath) {
    heimdall_process_input_file(filePath);
}

void heimdall_lld_process_library(const char* libraryPath) {
    heimdall_process_library(libraryPath);
}

}  // extern "C"
