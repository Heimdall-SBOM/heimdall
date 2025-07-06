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

namespace heimdall {

GoldAdapter::Impl::Impl() : sbomGenerator(std::make_unique<SBOMGenerator>()) {}
GoldAdapter::Impl::~Impl() = default;

bool GoldAdapter::Impl::initialize() {
    processedFiles.clear();
    processedLibraries.clear();
    verbose = false;
    extractDebugInfo = true;
    includeSystemLibraries = false;
    outputPath = "heimdall-gold-sbom.json";
    format = "spdx";
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
        std::cout << "[GoldAdapter] Processing input file: " << filePath << std::endl;
    ComponentInfo component(Utils::getFileName(filePath), filePath);
    component.setDetectedBy(LinkerType::Gold);
    MetadataExtractor extractor;
    extractor.extractMetadata(component);
    sbomGenerator->processComponent(component);
}

void GoldAdapter::Impl::processLibrary(const std::string& libraryPath) {
    if (std::find(processedLibraries.begin(), processedLibraries.end(), libraryPath) !=
        processedLibraries.end())
        return;
    processedLibraries.push_back(libraryPath);
    if (verbose)
        std::cout << "[GoldAdapter] Processing library: " << libraryPath << std::endl;
    ComponentInfo component(Utils::getFileName(libraryPath), libraryPath);
    component.setDetectedBy(LinkerType::Gold);
    component.fileType = FileType::SharedLibrary;
    MetadataExtractor extractor;
    extractor.extractMetadata(component);
    sbomGenerator->processComponent(component);
}

void GoldAdapter::Impl::processSymbol(const std::string& symbolName, uint64_t address,
                                      uint64_t size) {
    if (verbose)
        std::cout << "[GoldAdapter] Processed symbol: " << symbolName << " at " << address
                  << std::endl;
}

void GoldAdapter::Impl::setOutputPath(const std::string& path) {
    outputPath = path;
    sbomGenerator->setOutputPath(path);
}

void GoldAdapter::Impl::setFormat(const std::string& fmt) {
    format = fmt;
    sbomGenerator->setFormat(fmt);
}

void GoldAdapter::Impl::generateSBOM() {
    sbomGenerator->setOutputPath(outputPath);
    sbomGenerator->setFormat(format);
    sbomGenerator->generateSBOM();
    if (verbose)
        std::cout << "[GoldAdapter] SBOM generated at " << outputPath << std::endl;
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
size_t GoldAdapter::Impl::getComponentCount() const {
    return sbomGenerator->getComponentCount();
}
void GoldAdapter::Impl::printStatistics() const {
    sbomGenerator->printStatistics();
}

GoldAdapter::GoldAdapter() : pImpl(std::make_unique<Impl>()) {}
GoldAdapter::~GoldAdapter() = default;
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
void GoldAdapter::setOutputPath(const std::string& path) {
    pImpl->setOutputPath(path);
}
void GoldAdapter::setFormat(const std::string& format) {
    pImpl->setFormat(format);
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
size_t GoldAdapter::getComponentCount() const {
    return pImpl->getComponentCount();
}
void GoldAdapter::printStatistics() const {
    pImpl->printStatistics();
}

}  // namespace heimdall
