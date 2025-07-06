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
    std::cout << "=== Simple Plugin Test ===" << std::endl;
    
    // Check if plugins exist
    std::vector<std::string> plugins = {"heimdall-lld.so", "heimdall-gold.so"};
    
    for (const auto& pluginName : plugins) {
        std::cout << "\n--- Testing " << pluginName << " ---" << std::endl;
        
        if (!std::filesystem::exists(pluginName)) {
            std::cout << "Plugin not found: " << pluginName << std::endl;
            continue;
        }
        
        std::cout << "Plugin found: " << pluginName << std::endl;
        
        // Load plugin
        void* handle = dlopen(pluginName.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cout << "Failed to load plugin: " << dlerror() << std::endl;
            continue;
        }
        
        std::cout << "Plugin loaded successfully" << std::endl;
        
        // Get function pointers
        init_func_t onload = (init_func_t)dlsym(handle, "onload");
        set_format_func_t set_format = (set_format_func_t)dlsym(handle, "heimdall_set_format");
        set_output_path_func_t set_output_path = (set_output_path_func_t)dlsym(handle, "heimdall_set_output_path");
        process_input_file_func_t process_input_file = (process_input_file_func_t)dlsym(handle, "heimdall_process_input_file");
        finalize_func_t finalize = (finalize_func_t)dlsym(handle, "heimdall_finalize");
        
        if (!onload) std::cout << "onload function not found" << std::endl;
        if (!set_format) std::cout << "set_format function not found" << std::endl;
        if (!set_output_path) std::cout << "set_output_path function not found" << std::endl;
        if (!process_input_file) std::cout << "process_input_file function not found" << std::endl;
        if (!finalize) std::cout << "finalize function not found" << std::endl;
        
        if (!onload || !set_format || !set_output_path || !process_input_file || !finalize) {
            std::cout << "Some functions not found: " << dlerror() << std::endl;
            dlclose(handle);
            continue;
        }
        
        std::cout << "All functions found successfully" << std::endl;
        
        // Initialize plugin
        if (onload(nullptr) != 0) {
            std::cout << "Failed to initialize plugin" << std::endl;
            dlclose(handle);
            continue;
        }
        
        std::cout << "Plugin initialized successfully" << std::endl;
        
        // Set format and output path
        std::string outputPath = "/tmp/test_" + pluginName + ".spdx";
        set_format("spdx");
        set_output_path(outputPath.c_str());
        
        std::cout << "Format and output path set" << std::endl;
        
        // Create a simple test binary
        std::string testBinary = "/tmp/simple_test_binary";
        if (!std::filesystem::exists(testBinary)) {
            std::cout << "Creating simple test binary..." << std::endl;
            
            // Create a simple C source file
            std::ofstream source("/tmp/simple_test_binary.c");
            source << R"(
#include <stdio.h>
int main() {
    printf("Hello, World!\n");
    return 0;
}
)";
            source.close();
            
            // Compile it
            std::string compileCmd = "gcc -o " + testBinary + " /tmp/simple_test_binary.c";
            int result = system(compileCmd.c_str());
            if (result != 0) {
                std::cout << "Failed to compile test binary" << std::endl;
                dlclose(handle);
                continue;
            }
        }
        
        std::cout << "Test binary: " << testBinary << std::endl;
        
        // Process binary
        if (process_input_file(testBinary.c_str()) != 0) {
            std::cout << "Failed to process binary" << std::endl;
            dlclose(handle);
            continue;
        }
        
        std::cout << "Binary processed successfully" << std::endl;
        
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