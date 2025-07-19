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
#include "LLDAdapter.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../common/MetadataExtractor.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/Utils.hpp"
#include "../compat/compatibility.hpp"
#include "../common/ParallelProcessor.hpp"

namespace heimdall {

class LLDAdapter::Impl {
public:
    Impl();
    ~Impl() = default;

    void initialize();
    void processInputFile(const std::string& filePath);
    void processLibrary(const std::string& libraryPath);
    void finalize();
    void setOutputPath(const std::string& path);
    void setFormat(const std::string& format);
    void setCycloneDXVersion(const std::string& version);
    void setSPDXVersion(const std::string& version);
    void setVerbose(bool verbose);
    void setExtractDebugInfo(bool extract);
    void setIncludeSystemLibraries(bool include);
    size_t getComponentCount() const;
    void printStatistics() const;

    // Getter methods for accessing private members
    const std::vector<std::string>& getProcessedFiles() const { return processedFiles; }
    const std::vector<std::string>& getProcessedLibraries() const { return processedLibraries; }
    bool isInitialized() const { return initialized; }
    void processFilesParallel(const std::vector<std::string>& filePaths); // Add this line

private:
    std::vector<std::string> processedFiles;
    std::vector<std::string> processedLibraries;
    bool initialized{false};
    bool verbose{false};
    bool extractDebugInfo{true};
    bool includeSystemLibraries{false};
    std::string outputPath{"heimdall-lld-sbom.json"};
    std::string format{"spdx"};
    std::string cycloneDXVersion{"1.4"};
    std::unique_ptr<SBOMGenerator> sbomGenerator;

    static void logProcessing(const std::string& message);
};

// LLDAdapter::Impl implementation
LLDAdapter::Impl::Impl() : sbomGenerator(heimdall::compat::make_unique<SBOMGenerator>()) {}

void LLDAdapter::Impl::initialize() {
    if (!initialized) {
        processedFiles.clear();
        processedLibraries.clear();
        verbose = false;
        extractDebugInfo = true;
        includeSystemLibraries = false;
        outputPath = "heimdall-lld-sbom.json";
        format = "spdx";
        cycloneDXVersion = "1.4";
        logProcessing("LLDAdapter initialized");
        initialized = true;
    }
}

void LLDAdapter::Impl::processInputFile(const std::string& filePath) {
    if (!initialized) {
        logProcessing("Warning: LLDAdapter not initialized");
        return;
    }

    // Check if already processed
    if (std::find(processedFiles.begin(), processedFiles.end(), filePath) != processedFiles.end()) {
        return;
    }

    processedFiles.push_back(filePath);
    if (verbose) {
        logProcessing("Processing input file: " + filePath);
    }

    // Create component and extract metadata using MetadataExtractor
    ComponentInfo component(Utils::getFileName(filePath), filePath);
    component.setDetectedBy(LinkerType::LLD);
    
    // Use MetadataExtractor to get comprehensive metadata including DWARF data
    MetadataExtractor extractor;
    extractor.setExtractDebugInfo(extractDebugInfo);
    extractor.setVerbose(verbose);
    extractor.extractMetadata(component);
    
    // Add to SBOM generator
    sbomGenerator->processComponent(component);

    // Detect and process dependencies (linked libraries)
    std::vector<std::string> deps = heimdall::MetadataHelpers::detectDependencies(filePath);
    for (const auto& dep : deps) {
        // Try to resolve the library path using the improved resolver
        std::string depPath = Utils::resolveLibraryPath(dep);
        if (!depPath.empty() && Utils::fileExists(depPath)) {
            processLibrary(depPath);
        }
    }
}

void LLDAdapter::Impl::processLibrary(const std::string& libraryPath) {
    if (!initialized) {
        logProcessing("Warning: LLDAdapter not initialized");
        return;
    }

    // Check if already processed
    if (std::find(processedLibraries.begin(), processedLibraries.end(), libraryPath) != processedLibraries.end()) {
        return;
    }

    processedLibraries.push_back(libraryPath);
    if (verbose) {
        logProcessing("Processing library: " + libraryPath);
    }

    // Create component and extract metadata using MetadataExtractor
    ComponentInfo component(Utils::getFileName(libraryPath), libraryPath);
    component.setDetectedBy(LinkerType::LLD);
    component.fileType = FileType::SharedLibrary;
    
    // Use MetadataExtractor to get comprehensive metadata including DWARF data
    MetadataExtractor extractor;
    extractor.setExtractDebugInfo(extractDebugInfo);
    extractor.setVerbose(verbose);
    extractor.extractMetadata(component);
    
    // Add to SBOM generator
    sbomGenerator->processComponent(component);
}

void LLDAdapter::Impl::processFilesParallel(const std::vector<std::string>& filePaths) {
    // DWARF/LLVM debug info extraction is disabled for parallel runs (thread-safety).
    auto processFile = [this](const std::string& filePath) -> ComponentInfo {
        ComponentInfo component(Utils::getFileName(filePath), filePath);
        component.setDetectedBy(LinkerType::LLD);
        MetadataExtractor extractor;
        extractor.setSuppressWarnings(false); // Use default for now
        extractor.setExtractDebugInfo(false); // Disable DWARF for parallel
        extractor.extractMetadata(component);
        return component;
    };
    auto results = ParallelProcessor::process(filePaths, processFile);
    for (auto& component : results) {
        sbomGenerator->processComponent(component);
    }
}

void LLDAdapter::Impl::finalize() {
    if (initialized) {
        // Debug: Print component count before generation
        logProcessing("Finalizing with " + std::to_string(sbomGenerator->getComponentCount()) + " components");
        logProcessing("Format: " + format + ", Output: " + outputPath);
        
        // Generate the final SBOM
        sbomGenerator->setOutputPath(outputPath);
        sbomGenerator->setFormat(format);
        sbomGenerator->generateSBOM();
        
        logProcessing("LLDAdapter finalized - processed " + std::to_string(processedFiles.size()) +
                      " files and " + std::to_string(processedLibraries.size()) + " libraries");
        logProcessing("SBOM generated at: " + outputPath);
        initialized = false;
    }
}

void LLDAdapter::Impl::setOutputPath(const std::string& path) {
    outputPath = path;
    sbomGenerator->setOutputPath(path);
}

void LLDAdapter::Impl::setFormat(const std::string& fmt) {
    format = fmt;
    sbomGenerator->setFormat(fmt);
}

void LLDAdapter::Impl::setCycloneDXVersion(const std::string& version) {
    cycloneDXVersion = version;
    sbomGenerator->setCycloneDXVersion(version);
}

void LLDAdapter::Impl::setSPDXVersion(const std::string& version) {
    sbomGenerator->setSPDXVersion(version);
}

void LLDAdapter::Impl::setVerbose(bool v) {
    verbose = v;
}

void LLDAdapter::Impl::setExtractDebugInfo(bool extract) {
    extractDebugInfo = extract;
}

void LLDAdapter::Impl::setIncludeSystemLibraries(bool include) {
    includeSystemLibraries = include;
}

size_t LLDAdapter::Impl::getComponentCount() const {
    return sbomGenerator->getComponentCount();
}

void LLDAdapter::Impl::printStatistics() const {
    sbomGenerator->printStatistics();
}

void LLDAdapter::Impl::logProcessing(const std::string& message) {
    std::cout << "[Heimdall LLD] " << message << '\n';
}

// LLDAdapter implementation
LLDAdapter::LLDAdapter() : pImpl(heimdall::compat::make_unique<Impl>()) {}

LLDAdapter::~LLDAdapter() = default;

bool LLDAdapter::initialize() {
    pImpl->initialize();
    return true;
}

void LLDAdapter::processInputFile(const std::string& filePath) {
    pImpl->processInputFile(filePath);
}

void LLDAdapter::processLibrary(const std::string& libraryPath) {
    pImpl->processLibrary(libraryPath);
}

void LLDAdapter::finalize() {
    pImpl->finalize();
}

void LLDAdapter::setOutputPath(const std::string& path) {
    pImpl->setOutputPath(path);
}

void LLDAdapter::setFormat(const std::string& format) {
    pImpl->setFormat(format);
}

void LLDAdapter::setCycloneDXVersion(const std::string& version) {
    pImpl->setCycloneDXVersion(version);
}

void LLDAdapter::setSPDXVersion(const std::string& version) {
    pImpl->setSPDXVersion(version);
}

void LLDAdapter::setVerbose(bool verbose) {
    pImpl->setVerbose(verbose);
}

void LLDAdapter::setExtractDebugInfo(bool extract) {
    pImpl->setExtractDebugInfo(extract);
}

void LLDAdapter::setIncludeSystemLibraries(bool include) {
    pImpl->setIncludeSystemLibraries(include);
}

void LLDAdapter::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) {
    // Store symbol information for SBOM generation
    // This could be extended to store symbol metadata
}

std::vector<std::string> LLDAdapter::getProcessedFiles() const {
    return pImpl->getProcessedFiles();
}

std::vector<std::string> LLDAdapter::getProcessedLibraries() const {
    return pImpl->getProcessedLibraries();
}

std::vector<std::string> LLDAdapter::getProcessedSymbols() const {
    // Return empty vector for now - symbol processing not fully implemented
    return std::vector<std::string>();
}

bool LLDAdapter::shouldProcessFile(const std::string& filePath) const {
    if (filePath.empty()) {
        return false;
    }
    
    // Check if file exists
    if (!Utils::fileExists(filePath)) {
        return false;
    }
    
    // Check file extension
    std::string extension = Utils::getFileExtension(filePath);
    return extension == ".o" || extension == ".obj" || 
           extension == ".a" || extension == ".so" || 
           extension == ".dylib" || extension == ".dll" ||
           extension == ".exe" || extension.empty(); // Accept files without extension (Linux executables)
}

std::string LLDAdapter::extractComponentName(const std::string& filePath) const {
    std::string fileName = Utils::getFileName(filePath);
    
    // Remove common prefixes and extensions
    if (fileName.substr(0, 3) == "lib") {
        fileName = fileName.substr(3);
    }
    
    std::string extension = Utils::getFileExtension(fileName);
    if (!extension.empty()) {
        fileName = fileName.substr(0, fileName.length() - extension.length());
    }
    
    // Remove version numbers and suffixes (e.g., -1.2.3, _debug)
    size_t dashPos = fileName.find('-');
    if (dashPos != std::string::npos) {
        // Check if what follows is a version number (contains digits and dots)
        std::string suffix = fileName.substr(dashPos + 1);
        bool isVersion = false;
        bool hasDigit = false;
        for (char c : suffix) {
            if (std::isdigit(c)) {
                hasDigit = true;
            } else if (c != '.' && c != '-') {
                isVersion = false;
                break;
            }
        }
        if (hasDigit && suffix.find('.') != std::string::npos) {
            fileName = fileName.substr(0, dashPos);
        }
    }
    
    // Remove _debug, _release, etc. suffixes
    size_t underscorePos = fileName.find('_');
    if (underscorePos != std::string::npos) {
        std::string suffix = fileName.substr(underscorePos + 1);
        if (suffix == "debug" || suffix == "release" || suffix == "static" || 
            suffix == "shared" || suffix == "dll" || suffix == "so") {
            fileName = fileName.substr(0, underscorePos);
        }
    }
    
    return fileName;
}

void LLDAdapter::cleanup() {
    pImpl->finalize();
}

void LLDAdapter::generateSBOM() {
    pImpl->finalize();
}

size_t LLDAdapter::getComponentCount() const {
    return pImpl->getComponentCount();
}

void LLDAdapter::printStatistics() const {
    pImpl->printStatistics();
}

}  // namespace heimdall
