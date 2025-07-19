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
#include "GoldAdapter.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../common/MetadataExtractor.hpp"
#include "../common/PluginInterface.hpp"
#include "../common/SBOMGenerator.hpp"
#include "../common/Utils.hpp"
#include "../compat/compatibility.hpp"
#include "../common/ParallelProcessor.hpp"

namespace heimdall {

GoldAdapter::Impl::Impl() : sbomGenerator(heimdall::compat::make_unique<SBOMGenerator>()) {}
GoldAdapter::Impl::~Impl() = default;

bool GoldAdapter::Impl::initialize() {
    processedFiles.clear();
    processedLibraries.clear();
    verbose = false;
    extractDebugInfo = true;
    includeSystemLibraries = false;
    outputPath = "heimdall-gold-sbom.json";
    format = "spdx";
    cyclonedxVersion = "1.6";
    return true;
}

void GoldAdapter::Impl::cleanup() {
    processedFiles.clear();
    processedLibraries.clear();
}

void GoldAdapter::Impl::processInputFile(const std::string& filePath) {
    if (std::find(processedFiles.begin(), processedFiles.end(), filePath) != processedFiles.end())
        return;
    processedFiles.push_back(filePath);
    if (verbose)
        std::cout << "[GoldAdapter] Processing input file: " << filePath << '\n';
    ComponentInfo component(Utils::getFileName(filePath), filePath);
    component.setDetectedBy(LinkerType::Gold);
    MetadataExtractor extractor;
    extractor.setSuppressWarnings(suppressWarnings);
    extractor.extractMetadata(component);
    sbomGenerator->processComponent(component);
}

void GoldAdapter::Impl::processLibrary(const std::string& libraryPath) {
    if (std::find(processedLibraries.begin(), processedLibraries.end(), libraryPath) !=
        processedLibraries.end())
        return;
    processedLibraries.push_back(libraryPath);
    if (verbose)
        std::cout << "[GoldAdapter] Processing library: " << libraryPath << '\n';
    ComponentInfo component(Utils::getFileName(libraryPath), libraryPath);
    component.setDetectedBy(LinkerType::Gold);
    component.fileType = FileType::SharedLibrary;
    MetadataExtractor extractor;
    extractor.setSuppressWarnings(suppressWarnings);
    extractor.extractMetadata(component);
    sbomGenerator->processComponent(component);
}

void GoldAdapter::Impl::processSymbol(const std::string& symbolName, uint64_t address,
                                      uint64_t /*size*/) {
    if (verbose)
        std::cout << "[GoldAdapter] Processed symbol: " << symbolName << " at " << address << '\n';
}

void GoldAdapter::Impl::processFilesParallel(const std::vector<std::string>& filePaths) {
    // DWARF/LLVM debug info extraction is disabled for parallel runs (thread-safety).
    auto processFile = [this](const std::string& filePath) -> ComponentInfo {
        ComponentInfo component(Utils::getFileName(filePath), filePath);
        component.setDetectedBy(LinkerType::Gold);
        MetadataExtractor extractor;
        extractor.setSuppressWarnings(suppressWarnings);
        extractor.setExtractDebugInfo(false); // Disable DWARF for parallel
        extractor.extractMetadata(component);
        return component;
    };
    auto results = ParallelProcessor::process(filePaths, processFile);
    for (auto& component : results) {
        sbomGenerator->processComponent(component);
    }
}

void GoldAdapter::Impl::setOutputPath(const std::string& path) {
    outputPath = path;
    sbomGenerator->setOutputPath(path);
}

void GoldAdapter::Impl::setFormat(const std::string& fmt) {
    format = fmt;
    sbomGenerator->setFormat(fmt);
}

void GoldAdapter::Impl::setCycloneDXVersion(const std::string& version) {
    cyclonedxVersion = version;
    sbomGenerator->setCycloneDXVersion(version);
}

void GoldAdapter::Impl::setSPDXVersion(const std::string& version) {
    sbomGenerator->setSPDXVersion(version);
}

void GoldAdapter::Impl::generateSBOM() {
    sbomGenerator->setOutputPath(outputPath);
    sbomGenerator->setFormat(format);
    sbomGenerator->generateSBOM();
    if (verbose)
        std::cout << "[GoldAdapter] SBOM generated at " << outputPath << '\n';
}

void GoldAdapter::Impl::setVerbose(bool v) {
    verbose = v;
}
void GoldAdapter::Impl::setExtractDebugInfo(bool extract) {
    extractDebugInfo = extract;
}
void GoldAdapter::Impl::setIncludeSystemLibraries(bool include) {
    includeSystemLibraries = include;
}
void GoldAdapter::Impl::setSuppressWarnings(bool suppress) {
    suppressWarnings = suppress;
    if (sbomGenerator) {
        sbomGenerator->setSuppressWarnings(suppress);
    }
}
size_t GoldAdapter::Impl::getComponentCount() const {
    return sbomGenerator->getComponentCount();
}
void GoldAdapter::Impl::printStatistics() const {
    sbomGenerator->printStatistics();
}

GoldAdapter::GoldAdapter() : pImpl(heimdall::compat::make_unique<Impl>()) {}
GoldAdapter::~GoldAdapter() {
    if (pImpl) {
        pImpl->cleanup();
    }
}
bool GoldAdapter::initialize() {
    return pImpl->initialize();
}
void GoldAdapter::cleanup() {
    pImpl->cleanup();
}
void GoldAdapter::processInputFile(const std::string& filePath) {
    pImpl->processInputFile(filePath);
}
void GoldAdapter::processLibrary(const std::string& libraryPath) {
    pImpl->processLibrary(libraryPath);
}
void GoldAdapter::processSymbol(const std::string& symbolName, uint64_t address, uint64_t size) {
    pImpl->processSymbol(symbolName, address, size);
}
void GoldAdapter::processFilesParallel(const std::vector<std::string>& filePaths) {
    pImpl->processFilesParallel(filePaths);
}
void GoldAdapter::setOutputPath(const std::string& path) {
    pImpl->setOutputPath(path);
}
void GoldAdapter::setFormat(const std::string& format) {
    pImpl->setFormat(format);
}
void GoldAdapter::setCycloneDXVersion(const std::string& version) {
    pImpl->setCycloneDXVersion(version);
}
void GoldAdapter::setSPDXVersion(const std::string& version) {
    pImpl->setSPDXVersion(version);
}
void GoldAdapter::generateSBOM() {
    pImpl->generateSBOM();
}
void GoldAdapter::setVerbose(bool verbose) {
    pImpl->setVerbose(verbose);
}
void GoldAdapter::setExtractDebugInfo(bool extract) {
    pImpl->setExtractDebugInfo(extract);
}
void GoldAdapter::setIncludeSystemLibraries(bool include) {
    pImpl->setIncludeSystemLibraries(include);
}
void GoldAdapter::setSuppressWarnings(bool suppress) {
    pImpl->setSuppressWarnings(suppress);
}
size_t GoldAdapter::getComponentCount() const {
    return pImpl->getComponentCount();
}
void GoldAdapter::printStatistics() const {
    pImpl->printStatistics();
}

void GoldAdapter::finalize() {
    pImpl->generateSBOM();
}

std::vector<std::string> GoldAdapter::getProcessedFiles() const {
    return pImpl->getProcessedFiles();
}

std::vector<std::string> GoldAdapter::getProcessedLibraries() const {
    return pImpl->getProcessedLibraries();
}

std::vector<std::string> GoldAdapter::getProcessedSymbols() const {
    // Return empty vector for now - symbol processing not fully implemented
    return std::vector<std::string>();
}

bool GoldAdapter::shouldProcessFile(const std::string& filePath) const {
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

std::string GoldAdapter::extractComponentName(const std::string& filePath) const {
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

}  // namespace heimdall
