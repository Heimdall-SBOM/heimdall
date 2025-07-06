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
#include <sstream>
#include <string>
#include <vector>
#include "../common/ComponentInfo.hpp"
#include "../common/Utils.hpp"

namespace heimdall {

class LLDAdapter::Impl {
public:
    Impl();
    ~Impl() = default;

    void initialize();
    void processInputFile(const std::string& filePath);
    void finalize();

private:
    std::vector<std::string> processedFiles;
    bool initialized{false};

    static void extractBasicInfo(const std::string& filePath);
    static void logProcessing(const std::string& message);
};

// LLDAdapter::Impl implementation
LLDAdapter::Impl::Impl() = default;

void LLDAdapter::Impl::initialize() {
    if (!initialized) {
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
    logProcessing("Processing file: " + filePath);

    // Extract basic information
    extractBasicInfo(filePath);
}

void LLDAdapter::Impl::finalize() {
    if (initialized) {
        logProcessing("LLDAdapter finalized - processed " + std::to_string(processedFiles.size()) +
                      " files");
        initialized = false;
    }
}

void LLDAdapter::Impl::extractBasicInfo(const std::string& filePath) {
    // Basic file information extraction
    if (!Utils::fileExists(filePath)) {
        logProcessing("Warning: File does not exist: " + filePath);
        return;
    }

    std::string fileName = Utils::getFileName(filePath);
    std::string fileExtension = Utils::getFileExtension(filePath);
    uint64_t fileSize = Utils::getFileSize(filePath);

    logProcessing("File: " + fileName + ", Extension: " + fileExtension +
                  ", Size: " + std::to_string(fileSize) + " bytes");
}

void LLDAdapter::Impl::logProcessing(const std::string& message) {
    std::cout << "[Heimdall LLD] " << message << '\n';
}

// LLDAdapter implementation
LLDAdapter::LLDAdapter() : pImpl(std::make_unique<Impl>()) {}

LLDAdapter::~LLDAdapter() = default;

void LLDAdapter::initialize() {
    pImpl->initialize();
}

void LLDAdapter::processInputFile(const std::string& filePath) {
    pImpl->processInputFile(filePath);
}

void LLDAdapter::finalize() {
    pImpl->finalize();
}

}  // namespace heimdall
