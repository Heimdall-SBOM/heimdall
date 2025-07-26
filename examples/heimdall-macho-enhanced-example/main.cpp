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
#include <string>
#include <vector>
#include "common/MetadataExtractor.hpp"
#include "common/ComponentInfo.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <macho_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /usr/bin/ls" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    
    std::cout << "=== Enhanced Mach-O Analysis ===" << std::endl;
    std::cout << "File: " << filePath << std::endl;
    std::cout << std::endl;

    MetadataExtractor extractor;
    ComponentInfo component;
    component.filePath = filePath;

    // Check if it's a Mach-O file
    if (!extractor.isMachO(filePath)) {
        std::cerr << "Error: File is not a Mach-O binary" << std::endl;
        return 1;
    }

    std::cout << "✓ File is a Mach-O binary" << std::endl;

    // Extract basic metadata
    if (extractor.extractMetadata(component)) {
        std::cout << "✓ Basic metadata extracted successfully" << std::endl;
    } else {
        std::cout << "⚠ Basic metadata extraction had issues" << std::endl;
    }

    // Extract enhanced Mach-O metadata
    if (extractor.extractEnhancedMachOMetadata(component)) {
        std::cout << "✓ Enhanced Mach-O metadata extracted successfully" << std::endl;
    } else {
        std::cout << "⚠ Enhanced Mach-O metadata extraction had issues" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "=== Analysis Results ===" << std::endl;

    // Display basic info
    std::cout << "File Type: " << component.getFileTypeString() << std::endl;
    std::cout << "File Size: " << component.fileSize << " bytes" << std::endl;
    std::cout << "SHA256: " << component.checksum << std::endl;
    
    if (!component.version.empty()) {
        std::cout << "Version: " << component.version << std::endl;
    }
    
    if (!component.uuid.empty()) {
        std::cout << "UUID: " << component.uuid << std::endl;
    }

    // Display platform info
    std::cout << std::endl << "--- Platform Information ---" << std::endl;
    if (!component.platformInfo.architecture.empty()) {
        std::cout << "Architecture: " << component.platformInfo.architecture << std::endl;
    }
    if (!component.platformInfo.platform.empty()) {
        std::cout << "Platform: " << component.platformInfo.platform << std::endl;
    }
    if (component.platformInfo.isSimulator) {
        std::cout << "Simulator: Yes" << std::endl;
    }

    // Display build configuration
    std::cout << std::endl << "--- Build Configuration ---" << std::endl;
    if (!component.buildConfig.targetPlatform.empty()) {
        std::cout << "Target Platform: " << component.buildConfig.targetPlatform << std::endl;
    }
    if (!component.buildConfig.minOSVersion.empty()) {
        std::cout << "Minimum OS Version: " << component.buildConfig.minOSVersion << std::endl;
    }
    if (!component.buildConfig.sdkVersion.empty()) {
        std::cout << "SDK Version: " << component.buildConfig.sdkVersion << std::endl;
    }
    if (component.buildConfig.isSimulator) {
        std::cout << "Simulator Build: Yes" << std::endl;
    }

    // Display code signing info
    std::cout << std::endl << "--- Code Signing Information ---" << std::endl;
    if (!component.codeSignInfo.signer.empty()) {
        std::cout << "Signer: " << component.codeSignInfo.signer << std::endl;
    }
    if (!component.codeSignInfo.teamId.empty()) {
        std::cout << "Team ID: " << component.codeSignInfo.teamId << std::endl;
    }
    if (!component.codeSignInfo.certificateHash.empty()) {
        std::cout << "Certificate Hash: " << component.codeSignInfo.certificateHash << std::endl;
    }
    std::cout << "Ad-hoc Signed: " << (component.codeSignInfo.isAdHocSigned ? "Yes" : "No") << std::endl;
    std::cout << "Hardened Runtime: " << (component.codeSignInfo.isHardenedRuntime ? "Yes" : "No") << std::endl;

    // Display architectures
    std::cout << std::endl << "--- Architectures ---" << std::endl;
    for (const auto& arch : component.architectures) {
        std::cout << "Architecture: " << arch.name << std::endl;
        std::cout << "  CPU Type: 0x" << std::hex << arch.cpuType << std::dec << std::endl;
        std::cout << "  CPU Subtype: 0x" << std::hex << arch.cpuSubtype << std::dec << std::endl;
        std::cout << "  Offset: " << arch.offset << std::endl;
        std::cout << "  Size: " << arch.size << " bytes" << std::endl;
        std::cout << "  Alignment: " << arch.align << std::endl;
    }

    // Display dependencies
    std::cout << std::endl << "--- Dependencies ---" << std::endl;
    for (const auto& dep : component.dependencies) {
        std::cout << "Dependency: " << dep << std::endl;
    }

    // Display frameworks
    std::cout << std::endl << "--- Framework Dependencies ---" << std::endl;
    for (const auto& framework : component.frameworks) {
        std::cout << "Framework: " << framework << std::endl;
    }

    // Display entitlements
    std::cout << std::endl << "--- Entitlements ---" << std::endl;
    for (const auto& entitlement : component.entitlements) {
        std::cout << "Entitlement: " << entitlement << std::endl;
    }

    // Display symbols (first 10)
    std::cout << std::endl << "--- Symbols (first 10) ---" << std::endl;
    int symbolCount = 0;
    for (const auto& symbol : component.symbols) {
        if (symbolCount++ >= 10) break;
        std::cout << "Symbol: " << symbol.name;
        if (symbol.isGlobal) std::cout << " (global)";
        if (symbol.isDefined) std::cout << " (defined)";
        std::cout << std::endl;
    }
    if (component.symbols.size() > 10) {
        std::cout << "... and " << (component.symbols.size() - 10) << " more symbols" << std::endl;
    }

    // Display sections (first 10)
    std::cout << std::endl << "--- Sections (first 10) ---" << std::endl;
    int sectionCount = 0;
    for (const auto& section : component.sections) {
        if (sectionCount++ >= 10) break;
        std::cout << "Section: " << section.name << " (" << section.size << " bytes)" << std::endl;
    }
    if (component.sections.size() > 10) {
        std::cout << "... and " << (component.sections.size() - 10) << " more sections" << std::endl;
    }

    std::cout << std::endl << "=== Analysis Complete ===" << std::endl;
    return 0;
} 