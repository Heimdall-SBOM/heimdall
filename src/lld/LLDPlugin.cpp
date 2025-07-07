/**
 * @file LLDPlugin.cpp
 * @brief Heimdall plugin for LLVM LLD linker: SBOM generation and metadata extraction
 * @author Trevor Bakker
 * @date 2025
 *
 * This file implements the Heimdall plugin for the LLVM LLD linker, enabling
 * SBOM generation and metadata extraction during the link process. It provides
 * C interface functions for plugin configuration and file processing.
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

/**
 * @brief Plugin initialization function called when the plugin is loaded
 * @param tv Unused (reserved for future use)
 * @return 0 on success
 */
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

/**
 * @brief Plugin cleanup function called when the plugin is unloaded
 */
void onunload() {
    if (globalAdapter) {
        globalAdapter->finalize();
    }

    std::cout << "Heimdall LLD Plugin deactivated\n";
}

/**
 * @brief Get the version string for the Heimdall LLD plugin
 * @return Version string
 */
const char* heimdall_lld_version() {
    return "1.0.0";
}

/**
 * @brief Get the description string for the Heimdall LLD plugin
 * @return Description string
 */
const char* heimdall_lld_description() {
    return "Heimdall SBOM Generator Plugin for LLVM LLD Linker";
}

/**
 * @brief Set the output path for the generated SBOM
 * @param path Output file path (C string)
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Set the output format for the generated SBOM
 * @param fmt Output format (C string, e.g., "spdx" or "cyclonedx")
 * @return 0 on success, -1 on error
 */
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
    if (globalAdapter) {
        globalAdapter->setVerbose(v);
    }
}

/**
 * @brief Set whether to extract debug information
 * @param extract true to extract debug info, false otherwise
 */
void heimdall_set_extract_debug_info(bool extract) {
    extractDebugInfo = extract;
    if (globalAdapter) {
        globalAdapter->setExtractDebugInfo(extract);
    }
}

/**
 * @brief Set whether to include system libraries in the SBOM
 * @param include true to include system libraries, false otherwise
 */
void heimdall_set_include_system_libraries(bool include) {
    includeSystemLibraries = include;
    if (globalAdapter) {
        globalAdapter->setIncludeSystemLibraries(include);
    }
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

    if (verbose) {
        std::cout << "Heimdall: Processing input file: " << path << "\n";
    }

    // Process the file through the enhanced adapter
    globalAdapter->processInputFile(path);

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

    if (verbose) {
        std::cout << "Heimdall: Processing library: " << path << "\n";
    }

    // Process the library through the enhanced adapter
    globalAdapter->processLibrary(path);

    return 0;
}

/**
 * @brief Finalize the plugin and generate the SBOM
 */
void heimdall_finalize() {
    if (globalAdapter) {
        globalAdapter->finalize();
    }
}

/**
 * @brief Get the number of components processed by the plugin
 * @return Number of components
 */
size_t heimdall_get_component_count() {
    if (globalAdapter) {
        return globalAdapter->getComponentCount();
    }
    return 0;
}

/**
 * @brief Print statistics about the plugin's processing
 */
void heimdall_print_statistics() {
    if (globalAdapter) {
        globalAdapter->printStatistics();
    }
}

/**
 * @brief Initialize the Heimdall LLD plugin (legacy entry point)
 */
void heimdall_lld_plugin_init() {
    onload(nullptr);
}

/**
 * @brief Cleanup the Heimdall LLD plugin (legacy entry point)
 */
void heimdall_lld_plugin_cleanup() {
    onunload();
}

/**
 * @brief Process a file using the Heimdall LLD plugin (legacy entry point)
 * @param filePath Path to the file (C string)
 */
void heimdall_lld_process_file(const char* filePath) {
    heimdall_process_input_file(filePath);
}

/**
 * @brief Process a library using the Heimdall LLD plugin (legacy entry point)
 * @param libraryPath Path to the library (C string)
 */
void heimdall_lld_process_library(const char* libraryPath) {
    heimdall_process_library(libraryPath);
}

}  // extern "C"
