/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file test_plugin_sbom_consistency.cpp
 * @brief Test for plugin SBOM consistency and completeness
 * @author Trevor Bakker
 * @date 2025
 */

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <regex>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "nlohmann_json.hpp"

// Plugin function typedefs
typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

namespace {

class PluginSBOMConsistencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir = std::filesystem::temp_directory_path() / "heimdall_plugin_test";
        std::filesystem::create_directories(testDir);
        
        // Find plugin paths
        lldPluginPath = findPluginPath("heimdall-lld.so");
        goldPluginPath = findPluginPath("heimdall-gold.so");
        
        // Create a simple test binary
        createTestBinary();
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::string findPluginPath(const std::string& pluginName) {
        std::vector<std::string> searchPaths = {
            "../../build/",
            "../build/",
            "build/",
            "./"
        };
        
        for (const auto& path : searchPaths) {
            std::string fullPath = path + pluginName;
            if (std::filesystem::exists(fullPath)) {
                return std::filesystem::absolute(fullPath).string();
            }
        }
        
        return "";
    }
    
    void createTestBinary() {
        // Create a simple C source file that links against common libraries
        std::string sourceFile = testDir / "test_binary.c";
        std::ofstream source(sourceFile);
        source << R"(
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <pthread.h>

void* thread_func(void* arg) {
    printf("Thread running\n");
    return NULL;
}

int main() {
    // Use OpenSSL
    SSL_library_init();
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    
    // Use pthreads
    pthread_t thread;
    pthread_create(&thread, NULL, thread_func, NULL);
    pthread_join(thread, NULL);
    
    printf("Test binary completed successfully\n");
    return 0;
}
)";
        source.close();
        
        // Compile the test binary
        std::filesystem::path binaryPath = testDir / "test_binary";
        std::filesystem::path sourcePath = sourceFile;
        std::string compileCmd = "gcc -o " + binaryPath.string() + " " + sourcePath.string() +
                                " -lssl -lcrypto -lpthread";
        
        int result = system(compileCmd.c_str());
        if (result != 0) {
            GTEST_SKIP() << "Failed to compile test binary. Skipping test.";
        }
        
        testBinaryPath = binaryPath.string();
    }
    
    struct SBOMData {
        std::set<std::string> components;
        std::map<std::string, std::string> componentTypes;
        std::map<std::string, std::string> componentVersions;
        std::map<std::string, std::string> componentSuppliers;
        std::set<std::string> dependencies;
    };
    
    SBOMData parseSPDX(const std::string& filePath) {
        SBOMData data;
        std::ifstream file(filePath);
        std::string line;
        std::string currentComponent;
        
        while (std::getline(file, line)) {
            if (line.find("FileName: ") == 0) {
                currentComponent = line.substr(10);
                data.components.insert(currentComponent);
            } else if (line.find("FileType: ") == 0 && !currentComponent.empty()) {
                data.componentTypes[currentComponent] = line.substr(10);
            } else if (line.find("Version: ") == 0 && !currentComponent.empty()) {
                data.componentVersions[currentComponent] = line.substr(9);
            } else if (line.find("Supplier: ") == 0 && !currentComponent.empty()) {
                data.componentSuppliers[currentComponent] = line.substr(10);
            }
        }
        
        // Debug output
        std::cerr << "DEBUG: SPDX parsing found " << data.components.size() << " components" << std::endl;
        for (const auto& name : data.components) {
            std::cerr << "DEBUG: Component: " << name << std::endl;
        }
        
        return data;
    }
    
    SBOMData parseCycloneDX(const std::string& filePath) {
        SBOMData data;
        std::ifstream file(filePath);
        if (!file.is_open()) return data;
        nlohmann::json j;
        file >> j;
        if (!j.contains("components") || !j["components"].is_array()) return data;
        for (const auto& comp : j["components"]) {
            if (comp.contains("name") && comp["name"].is_string()) {
                data.components.insert(comp["name"].get<std::string>());
            }
        }
        // Debug output
        std::cerr << "DEBUG: CycloneDX parsing found " << data.components.size() << " components" << std::endl;
        for (const auto& name : data.components) {
            std::cerr << "DEBUG: Component: " << name << std::endl;
        }
        return data;
    }
    
    bool generateSBOM(const std::string& pluginPath, const std::string& format, 
                     const std::string& outputPath, const std::string& binaryPath) {
        void* handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
            return false;
        }
        
        // Get function pointers
        init_func_t onload = (init_func_t)dlsym(handle, "onload");
        set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
        set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
        process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
        finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");
        
        if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
            std::cerr << "Failed to get function symbols: " << dlerror() << std::endl;
            dlclose(handle);
            return false;
        }
        
        // Initialize plugin
        if (onload(nullptr) != 0) {
            std::cerr << "Failed to initialize plugin" << std::endl;
            dlclose(handle);
            return false;
        }
        
        // Set format
        if (set_format(format.c_str()) != 0) {
            std::cerr << "Failed to set format" << std::endl;
            dlclose(handle);
            return false;
        }
        
        // Set output path
        if (set_output_path(outputPath.c_str()) != 0) {
            std::cerr << "Failed to set output path" << std::endl;
            dlclose(handle);
            return false;
        }
        
        // Process binary
        if (process_input_file(binaryPath.c_str()) != 0) {
            std::cerr << "Failed to process binary" << std::endl;
            dlclose(handle);
            return false;
        }
        
        // Generate SBOM
        finalize();
        dlclose(handle);
        
        return std::filesystem::exists(outputPath);
    }
    
    std::filesystem::path testDir;
    std::string lldPluginPath;
    std::string goldPluginPath;
    std::string testBinaryPath;
};

TEST_F(PluginSBOMConsistencyTest, PluginPathsExist) {
    EXPECT_FALSE(lldPluginPath.empty()) << "LLD plugin not found";
    EXPECT_FALSE(goldPluginPath.empty()) << "Gold plugin not found";
}

TEST_F(PluginSBOMConsistencyTest, TestBinaryExists) {
    EXPECT_TRUE(std::filesystem::exists(testBinaryPath)) << "Test binary not created";
}

TEST_F(PluginSBOMConsistencyTest, LLDPluginSPDXGeneration) {
    if (lldPluginPath.empty()) {
        GTEST_SKIP() << "LLD plugin not available";
    }
    
    std::string outputPath = (testDir / "lld_test.spdx").string();
    bool success = generateSBOM(lldPluginPath, "spdx", outputPath, testBinaryPath);
    
    EXPECT_TRUE(success) << "Failed to generate LLD SPDX SBOM";
    EXPECT_TRUE(std::filesystem::exists(outputPath)) << "LLD SPDX file not created";
    
    // Parse and validate SPDX
    SBOMData spdxData = parseSPDX(outputPath);
    
    // Should contain main binary
    EXPECT_TRUE(spdxData.components.find("test_binary") != spdxData.components.end()) 
        << "Main binary not found in LLD SPDX";
    
    // Should contain OpenSSL libraries
    EXPECT_TRUE(spdxData.components.find("libssl.so") != spdxData.components.end() ||
                spdxData.components.find("libssl.so.3") != spdxData.components.end())
        << "OpenSSL SSL library not found in LLD SPDX";
    
    EXPECT_TRUE(spdxData.components.find("libcrypto.so") != spdxData.components.end() ||
                spdxData.components.find("libcrypto.so.3") != spdxData.components.end())
        << "OpenSSL crypto library not found in LLD SPDX";
    
    // Should contain system libraries
    EXPECT_TRUE(spdxData.components.find("libc.so") != spdxData.components.end() ||
                spdxData.components.find("libc.so.6") != spdxData.components.end())
        << "System C library not found in LLD SPDX";
    
    // Should contain pthread library (optional, warn if missing)
    if (!(spdxData.components.find("libpthread.so") != spdxData.components.end() ||
          spdxData.components.find("libpthread.so.0") != spdxData.components.end())) {
        std::cerr << "[WARN] Pthread library not found in SPDX (may be merged with libc on this system)" << std::endl;
    }
    // Should have at least 4 components (main binary + 3+ libraries)
    EXPECT_GE(spdxData.components.size(), 4) << "LLD SPDX has insufficient components";
}

TEST_F(PluginSBOMConsistencyTest, LLDPluginCycloneDXGeneration) {
    if (lldPluginPath.empty()) {
        GTEST_SKIP() << "LLD plugin not available";
    }
    
    std::string outputPath = (testDir / "lld_test.cyclonedx.json").string();
    bool success = generateSBOM(lldPluginPath, "cyclonedx", outputPath, testBinaryPath);
    
    EXPECT_TRUE(success) << "Failed to generate LLD CycloneDX SBOM";
    EXPECT_TRUE(std::filesystem::exists(outputPath)) << "LLD CycloneDX file not created";
    
    // Parse and validate CycloneDX
    SBOMData cyclonedxData = parseCycloneDX(outputPath);
    
    // Debug: write to stderr to ensure visibility
    std::cerr << "DEBUG: Parsed " << cyclonedxData.components.size() << " components from CycloneDX" << std::endl;
    
    // Debug: print all parsed component names
    std::ofstream debugOut("/tmp/gold_cyclonedx_components.txt");
    if (!debugOut.is_open()) {
        std::cerr << "DEBUG: Failed to open debug file for writing" << std::endl;
    } else {
        std::cerr << "DEBUG: Successfully opened debug file" << std::endl;
    }
    debugOut << "Parsed CycloneDX components:\n";
    for (const auto& name : cyclonedxData.components) {
        debugOut << "  - " << name << "\n";
    }
    debugOut.close();
    std::cerr << "DEBUG: Closed debug file" << std::endl;
    
    // Should contain main binary
    EXPECT_TRUE(cyclonedxData.components.find("test_binary") != cyclonedxData.components.end()) 
        << "Main binary not found in LLD CycloneDX";
    
    // Should contain OpenSSL libraries
    EXPECT_TRUE(cyclonedxData.components.find("libssl.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libssl.so.3") != cyclonedxData.components.end())
        << "OpenSSL SSL library not found in LLD CycloneDX";
    
    EXPECT_TRUE(cyclonedxData.components.find("libcrypto.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libcrypto.so.3") != cyclonedxData.components.end())
        << "OpenSSL crypto library not found in LLD CycloneDX";
    
    // Should contain system libraries
    EXPECT_TRUE(cyclonedxData.components.find("libc.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libc.so.6") != cyclonedxData.components.end())
        << "System C library not found in LLD CycloneDX";
    
    // Should have at least 4 components (main binary + 3+ libraries)
    EXPECT_GE(cyclonedxData.components.size(), 4) << "LLD CycloneDX has insufficient components";
}

TEST_F(PluginSBOMConsistencyTest, GoldPluginSPDXGeneration) {
    if (goldPluginPath.empty()) {
        GTEST_SKIP() << "Gold plugin not available";
    }
    
    std::string outputPath = (testDir / "gold_test.spdx").string();
    bool success = generateSBOM(goldPluginPath, "spdx", outputPath, testBinaryPath);
    
    EXPECT_TRUE(success) << "Failed to generate Gold SPDX SBOM";
    EXPECT_TRUE(std::filesystem::exists(outputPath)) << "Gold SPDX file not created";
    
    // Parse and validate SPDX
    SBOMData spdxData = parseSPDX(outputPath);
    
    // Should contain main binary
    EXPECT_TRUE(spdxData.components.find("test_binary") != spdxData.components.end()) 
        << "Main binary not found in Gold SPDX";
    
    // Should contain OpenSSL libraries
    EXPECT_TRUE(spdxData.components.find("libssl.so") != spdxData.components.end() ||
                spdxData.components.find("libssl.so.3") != spdxData.components.end())
        << "OpenSSL SSL library not found in Gold SPDX";
    
    EXPECT_TRUE(spdxData.components.find("libcrypto.so") != spdxData.components.end() ||
                spdxData.components.find("libcrypto.so.3") != spdxData.components.end())
        << "OpenSSL crypto library not found in Gold SPDX";
    
    // Should contain system libraries
    EXPECT_TRUE(spdxData.components.find("libc.so") != spdxData.components.end() ||
                spdxData.components.find("libc.so.6") != spdxData.components.end())
        << "System C library not found in Gold SPDX";
    
    // Should contain pthread library (optional, warn if missing)
    if (!(spdxData.components.find("libpthread.so") != spdxData.components.end() ||
          spdxData.components.find("libpthread.so.0") != spdxData.components.end())) {
        std::cerr << "[WARN] Pthread library not found in SPDX (may be merged with libc on this system)" << std::endl;
    }
    // Should have at least 4 components (main binary + 3+ libraries)
    EXPECT_GE(spdxData.components.size(), 4) << "Gold SPDX has insufficient components";
}

TEST_F(PluginSBOMConsistencyTest, GoldPluginCycloneDXGeneration) {
    std::cerr << "DEBUG: Running GoldPluginCycloneDXGeneration" << std::endl;
    if (goldPluginPath.empty()) {
        GTEST_SKIP() << "Gold plugin not available";
    }
    
    std::string outputPath = (testDir / "gold_test.cyclonedx.json").string();
    std::cerr << "DEBUG: About to generate SBOM at " << outputPath << std::endl;
    bool success = generateSBOM(goldPluginPath, "cyclonedx", outputPath, testBinaryPath);
    std::cerr << "DEBUG: SBOM generation returned " << success << std::endl;
    
    EXPECT_TRUE(success) << "Failed to generate Gold CycloneDX SBOM";
    EXPECT_TRUE(std::filesystem::exists(outputPath)) << "Gold CycloneDX file not created";
    
    // Copy SBOM to /tmp for inspection
    std::string tmpCopy = "/tmp/gold_test.cyclonedx.json";
    std::cerr << "DEBUG: Copying SBOM to " << tmpCopy << std::endl;
    std::filesystem::copy_file(outputPath, tmpCopy, std::filesystem::copy_options::overwrite_existing);
    
    // Parse and validate CycloneDX
    std::cerr << "DEBUG: Parsing CycloneDX SBOM" << std::endl;
    SBOMData cyclonedxData = parseCycloneDX(outputPath);
    std::cerr << "DEBUG: Parsed " << cyclonedxData.components.size() << " components from CycloneDX" << std::endl;
    
    // Debug: print all parsed component names
    std::ofstream debugOut("/tmp/gold_cyclonedx_components.txt");
    if (!debugOut.is_open()) {
        std::cerr << "DEBUG: Failed to open debug file for writing" << std::endl;
    } else {
        std::cerr << "DEBUG: Successfully opened debug file" << std::endl;
    }
    debugOut << "Parsed CycloneDX components:\n";
    for (const auto& name : cyclonedxData.components) {
        debugOut << "  - " << name << "\n";
    }
    debugOut.close();
    std::cerr << "DEBUG: Closed debug file" << std::endl;
    
    // Should contain main binary
    EXPECT_TRUE(cyclonedxData.components.find("test_binary") != cyclonedxData.components.end()) 
        << "Main binary not found in Gold CycloneDX";
    
    // Should contain OpenSSL libraries
    EXPECT_TRUE(cyclonedxData.components.find("libssl.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libssl.so.3") != cyclonedxData.components.end())
        << "OpenSSL SSL library not found in Gold CycloneDX";
    
    EXPECT_TRUE(cyclonedxData.components.find("libcrypto.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libcrypto.so.3") != cyclonedxData.components.end())
        << "OpenSSL crypto library not found in Gold CycloneDX";
    
    // Should contain system libraries
    EXPECT_TRUE(cyclonedxData.components.find("libc.so") != cyclonedxData.components.end() ||
                cyclonedxData.components.find("libc.so.6") != cyclonedxData.components.end())
        << "System C library not found in Gold CycloneDX";
    
    // Should have at least 4 components (main binary + 3+ libraries)
    EXPECT_GE(cyclonedxData.components.size(), 4) << "Gold CycloneDX has insufficient components";
}

TEST_F(PluginSBOMConsistencyTest, PluginConsistency) {
    if (lldPluginPath.empty() || goldPluginPath.empty()) {
        GTEST_SKIP() << "One or both plugins not available";
    }
    
    // Generate SBOMs with both plugins in both formats
    std::string lldSpdxPath = (testDir / "lld_consistency.spdx").string();
    std::string lldCycloneDXPath = (testDir / "lld_consistency.cyclonedx.json").string();
    std::string goldSpdxPath = (testDir / "gold_consistency.spdx").string();
    std::string goldCycloneDXPath = (testDir / "gold_consistency.cyclonedx.json").string();
    
    bool lldSpdxSuccess = generateSBOM(lldPluginPath, "spdx", lldSpdxPath, testBinaryPath);
    bool lldCycloneDXSuccess = generateSBOM(lldPluginPath, "cyclonedx", lldCycloneDXPath, testBinaryPath);
    bool goldSpdxSuccess = generateSBOM(goldPluginPath, "spdx", goldSpdxPath, testBinaryPath);
    bool goldCycloneDXSuccess = generateSBOM(goldPluginPath, "cyclonedx", goldCycloneDXPath, testBinaryPath);
    
    EXPECT_TRUE(lldSpdxSuccess) << "LLD SPDX generation failed";
    EXPECT_TRUE(lldCycloneDXSuccess) << "LLD CycloneDX generation failed";
    EXPECT_TRUE(goldSpdxSuccess) << "Gold SPDX generation failed";
    EXPECT_TRUE(goldCycloneDXSuccess) << "Gold CycloneDX generation failed";
    
    // Parse all SBOMs
    SBOMData lldSpdxData = parseSPDX(lldSpdxPath);
    SBOMData lldCycloneDXData = parseCycloneDX(lldCycloneDXPath);
    SBOMData goldSpdxData = parseSPDX(goldSpdxPath);
    SBOMData goldCycloneDXData = parseCycloneDX(goldCycloneDXPath);
    
    // Test 1: Both plugins should generate the same number of components in SPDX
    EXPECT_EQ(lldSpdxData.components.size(), goldSpdxData.components.size())
        << "LLD and Gold plugins generate different numbers of components in SPDX";
    
    // Test 2: Both plugins should generate the same number of components in CycloneDX
    EXPECT_EQ(lldCycloneDXData.components.size(), goldCycloneDXData.components.size())
        << "LLD and Gold plugins generate different numbers of components in CycloneDX";
    
    // Test 3: Each plugin should generate the same number of components in both formats
    EXPECT_EQ(lldSpdxData.components.size(), lldCycloneDXData.components.size())
        << "LLD plugin generates different numbers of components in SPDX vs CycloneDX";
    
    EXPECT_EQ(goldSpdxData.components.size(), goldCycloneDXData.components.size())
        << "Gold plugin generates different numbers of components in SPDX vs CycloneDX";
    
    // Test 4: Both plugins should include the same core components
    std::set<std::string> expectedComponents = {"test_binary"};
    
    for (const auto& component : expectedComponents) {
        EXPECT_TRUE(lldSpdxData.components.find(component) != lldSpdxData.components.end())
            << "LLD SPDX missing expected component: " << component;
        EXPECT_TRUE(goldSpdxData.components.find(component) != goldSpdxData.components.end())
            << "Gold SPDX missing expected component: " << component;
        EXPECT_TRUE(lldCycloneDXData.components.find(component) != lldCycloneDXData.components.end())
            << "LLD CycloneDX missing expected component: " << component;
        EXPECT_TRUE(goldCycloneDXData.components.find(component) != goldCycloneDXData.components.end())
            << "Gold CycloneDX missing expected component: " << component;
    }
    
    // Test 5: Both plugins should include OpenSSL libraries
    bool lldHasOpenSSL = (lldSpdxData.components.find("libssl.so") != lldSpdxData.components.end() ||
                         lldSpdxData.components.find("libssl.so.3") != lldSpdxData.components.end()) &&
                        (lldSpdxData.components.find("libcrypto.so") != lldSpdxData.components.end() ||
                         lldSpdxData.components.find("libcrypto.so.3") != lldSpdxData.components.end());
    
    bool goldHasOpenSSL = (goldSpdxData.components.find("libssl.so") != goldSpdxData.components.end() ||
                         goldSpdxData.components.find("libssl.so.3") != goldSpdxData.components.end()) &&
                        (goldSpdxData.components.find("libcrypto.so") != goldSpdxData.components.end() ||
                         goldSpdxData.components.find("libcrypto.so.3") != goldSpdxData.components.end());
    
    EXPECT_TRUE(lldHasOpenSSL) << "LLD plugin missing OpenSSL libraries";
    EXPECT_TRUE(goldHasOpenSSL) << "Gold plugin missing OpenSSL libraries";
    
    // Test 6: Both plugins should include system libraries
    bool lldHasSystemLibs = (lldSpdxData.components.find("libc.so") != lldSpdxData.components.end() ||
                            lldSpdxData.components.find("libc.so.6") != lldSpdxData.components.end());
    
    bool goldHasSystemLibs = (goldSpdxData.components.find("libc.so") != goldSpdxData.components.end() ||
                             goldSpdxData.components.find("libc.so.6") != goldSpdxData.components.end());
    
    EXPECT_TRUE(lldHasSystemLibs) << "LLD plugin missing system libraries";
    EXPECT_TRUE(goldHasSystemLibs) << "Gold plugin missing system libraries";
}

TEST_F(PluginSBOMConsistencyTest, FormatConsistency) {
    if (lldPluginPath.empty()) {
        GTEST_SKIP() << "LLD plugin not available";
    }
    
    // Generate both formats with LLD plugin
    std::string spdxPath = (testDir / "format_test.spdx").string();
    std::string cyclonedxPath = (testDir / "format_test.cyclonedx.json").string();
    
    bool spdxSuccess = generateSBOM(lldPluginPath, "spdx", spdxPath, testBinaryPath);
    bool cyclonedxSuccess = generateSBOM(lldPluginPath, "cyclonedx", cyclonedxPath, testBinaryPath);
    
    EXPECT_TRUE(spdxSuccess) << "SPDX generation failed";
    EXPECT_TRUE(cyclonedxSuccess) << "CycloneDX generation failed";
    
    // Parse both formats
    SBOMData spdxData = parseSPDX(spdxPath);
    SBOMData cyclonedxData = parseCycloneDX(cyclonedxPath);
    
    // Both formats should contain the same components
    EXPECT_EQ(spdxData.components.size(), cyclonedxData.components.size())
        << "SPDX and CycloneDX formats have different numbers of components";
    
    // Both formats should contain the main binary
    EXPECT_TRUE(spdxData.components.find("test_binary") != spdxData.components.end())
        << "Main binary not found in SPDX format";
    EXPECT_TRUE(cyclonedxData.components.find("test_binary") != cyclonedxData.components.end())
        << "Main binary not found in CycloneDX format";
}

} // namespace 