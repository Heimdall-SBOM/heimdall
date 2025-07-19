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

// Only include LLVM headers if we have C++17+ (LLVM 20 requires C++17+)
#if __cplusplus >= 201703L
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

// LLD Plugin Interface - Only include if C++17+
#include "lld/Common/Driver.h"
#endif

namespace {
std::unique_ptr<heimdall::LLDAdapter> globalAdapter;
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

    // Reset all global state
    processedFiles.clear();
    processedLibraries.clear();
    // Do NOT reset format/outputPath/verbose here, keep current values

    // Initialize the adapter
    globalAdapter = heimdall::compat::make_unique<heimdall::LLDAdapter>();
    globalAdapter->initialize();

    // Always apply current config to the adapter
    globalAdapter->setOutputPath(outputPath);
    globalAdapter->setFormat(format);
    globalAdapter->setVerbose(verbose);

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
        // Always update global outputPath
        outputPath = std::string(path);
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
        // Always update global format
        format = std::string(fmt);
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

    // Process the file through the adapter (adapter's state determines format/outputPath)
    globalAdapter->processInputFile(path);

    return 0;  // Success
}

void heimdall_process_library(const char* libraryPath) {
    if (!globalAdapter || !libraryPath)
        return;

    std::string path(libraryPath);

    // Check if already processed
    if (std::find(processedLibraries.begin(), processedLibraries.end(), path) !=
        processedLibraries.end()) {
        return;  // Already processed
    }

    processedLibraries.push_back(path);

    if (verbose) {
        std::cout << "Heimdall: Processing library: " << path << "\n";
    }

    // Process the library through the adapter
    globalAdapter->processLibrary(path);
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

// Plugin cleanup and finalization
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->finalize();
    }

    std::cout << "Heimdall LLD Plugin finalized\n";
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
    if (globalAdapter && version) {
        globalAdapter->setSPDXVersion(std::string(version));
        return 0;
    }
    return -1;
}

// LLD plugin option handler
int heimdall_lld_set_plugin_option(const char* option) {
    if (!option) {
        return -1;
    }

    std::string opt(option);
    
    if (verbose) {
        std::cout << "Heimdall: LLD plugin option: " << opt << "\n";
    }

    // Parse plugin options
    if (opt.find("--plugin-opt=output=") == 0) {
        std::string outputPath = opt.substr(19); // Length of "--plugin-opt=output="
        if (globalAdapter) {
            globalAdapter->setOutputPath(outputPath);
        }
        return 0;
    } else if (opt.find("--plugin-opt=format=") == 0) {
        std::string format = opt.substr(18); // Length of "--plugin-opt=format="
        if (globalAdapter) {
            globalAdapter->setFormat(format);
        }
        return 0;
    } else if (opt.find("--plugin-opt=verbose") == 0) {
        verbose = true;
        if (globalAdapter) {
            globalAdapter->setVerbose(true);
        }
        return 0;
    } else if (opt.find("--plugin-opt=cyclonedx-version=") == 0) {
        std::string version = opt.substr(28); // Length of "--plugin-opt=cyclonedx-version="
        return heimdall_set_cyclonedx_version(version.c_str());
    } else if (opt.find("--plugin-opt=spdx-version=") == 0) {
        std::string version = opt.substr(23); // Length of "--plugin-opt=spdx-version="
        if (globalAdapter) {
            globalAdapter->setSPDXVersion(version);
        }
        return 0;
    } else if (opt.find("--plugin-opt=include-system-libraries") == 0) {
        if (globalAdapter) {
            globalAdapter->setIncludeSystemLibraries(true);
        }
        return 0;
    } else if (opt.find("--plugin-opt=extract-debug-info") == 0) {
        if (globalAdapter) {
            globalAdapter->setExtractDebugInfo(true);
        }
        return 0;
    }

    // Unknown option
    if (verbose) {
        std::cout << "Heimdall: Unknown LLD plugin option: " << opt << "\n";
    }
    return -1;
}

}  // extern "C"

// LLVM Pass Plugin Interface (Legacy Pass Manager) - Only compile if C++17+
#if __cplusplus >= 201703L
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
