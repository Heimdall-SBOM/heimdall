#include <iostream>
#include <dlfcn.h>
#include <string>
#include <filesystem>

// Plugin function typedefs
typedef int (*init_func_t)(void*);
typedef int (*set_format_func_t)(const char*);
typedef int (*set_output_path_func_t)(const char*);
typedef int (*process_input_file_func_t)(const char*);
typedef void (*finalize_func_t)(void);

int main() {
    std::cout << "=== Direct Plugin Test ===" << std::endl;
    
    // Test with a simple binary that uses OpenSSL
    std::string testBinary = "/tmp/test_binary";
    
    // Create a simple test binary if it doesn't exist
    if (!std::filesystem::exists(testBinary)) {
        std::cout << "Creating test binary..." << std::endl;
        
        // Create a simple C source file
        std::ofstream source("/tmp/test_binary.c");
        source << R"(
#include <stdio.h>
#include <openssl/ssl.h>
#include <pthread.h>

int main() {
    SSL_library_init();
    printf("Test binary completed\n");
    return 0;
}
)";
        source.close();
        
        // Compile it
        std::string compileCmd = "gcc -o " + testBinary + " /tmp/test_binary.c -lssl -lcrypto -lpthread";
        int result = system(compileCmd.c_str());
        if (result != 0) {
            std::cout << "Failed to compile test binary" << std::endl;
            return 1;
        }
    }
    
    std::cout << "Test binary: " << testBinary << std::endl;
    
    // Test both plugins
    std::vector<std::string> plugins = {"heimdall-lld.so", "heimdall-gold.so"};
    
    for (const auto& pluginName : plugins) {
        std::cout << "\n--- Testing " << pluginName << " ---" << std::endl;
        
        // Load plugin
        void* handle = dlopen(pluginName.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cout << "Failed to load plugin: " << dlerror() << std::endl;
            continue;
        }
        
        // Get function pointers
        init_func_t onload = (init_func_t)dlsym(handle, "onload");
        set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
        set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
        process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
        finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");
        
        if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
            std::cout << "Failed to get function symbols: " << dlerror() << std::endl;
            dlclose(handle);
            continue;
        }
        
        // Initialize plugin
        if (onload(nullptr) != 0) {
            std::cout << "Failed to initialize plugin" << std::endl;
            dlclose(handle);
            continue;
        }
        
        // Set format and output path
        std::string outputPath = "/tmp/test_" + pluginName + ".spdx";
        set_format("spdx");
        set_output_path(outputPath.c_str());
        
        // Process binary
        if (process_input_file(testBinary.c_str()) != 0) {
            std::cout << "Failed to process binary" << std::endl;
            dlclose(handle);
            continue;
        }
        
        // Generate SBOM
        finalize();
        
        // Check if SBOM was generated
        if (std::filesystem::exists(outputPath)) {
            std::cout << "✓ SBOM generated: " << outputPath << std::endl;
            
            // Check content
            std::ifstream sbom(outputPath);
            std::string line;
            int componentCount = 0;
            while (std::getline(sbom, line)) {
                if (line.find("FileName: ") == 0) {
                    componentCount++;
                    std::cout << "  Component: " << line.substr(10) << std::endl;
                }
            }
            std::cout << "  Total components: " << componentCount << std::endl;
        } else {
            std::cout << "✗ SBOM not generated" << std::endl;
        }
        
        dlclose(handle);
    }
    
    return 0;
} 