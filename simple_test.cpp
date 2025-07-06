#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

// Simple library path resolution test
std::string resolveLibraryPath(const std::string& libraryName) {
    // If it's already an absolute path, check if it exists
    if (!libraryName.empty() && libraryName[0] == '/') {
        if (std::filesystem::exists(libraryName)) {
            return libraryName;
        }
        return "";
    }

    // Search in standard library paths
    std::vector<std::string> libPaths = {
        "/usr/lib", "/usr/local/lib", "/opt/local/lib", "/opt/homebrew/lib",
        "/lib", "/lib64", "/usr/lib64", "/usr/lib/x86_64-linux-gnu"
    };

    // First try the exact name
    for (const auto& libDir : libPaths) {
        std::string candidate = libDir + "/" + libraryName;
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    // If not found and it's a versioned library, try without version
    if (libraryName.find(".so.") != std::string::npos) {
        std::string baseName = libraryName.substr(0, libraryName.find(".so.")) + ".so";
        for (const auto& libDir : libPaths) {
            std::string candidate = libDir + "/" + baseName;
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }

    // If still not found, try with .so extension
    if (libraryName.find(".so") == std::string::npos) {
        std::string withExt = libraryName + ".so";
        for (const auto& libDir : libPaths) {
            std::string candidate = libDir + "/" + withExt;
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }

    return "";
}

int main() {
    std::cout << "=== Library Resolution Test ===" << std::endl;
    
    // Test library names that should be found on Ubuntu
    std::vector<std::string> testLibs = {
        "libssl.so.3",
        "libcrypto.so.3", 
        "libc.so.6",
        "libpthread.so.0",
        "libssl.so",
        "libcrypto.so",
        "libc.so",
        "libpthread.so"
    };
    
    for (const auto& lib : testLibs) {
        std::string resolved = resolveLibraryPath(lib);
        if (!resolved.empty()) {
            std::cout << "✓ " << lib << " -> " << resolved << std::endl;
        } else {
            std::cout << "✗ " << lib << " -> NOT FOUND" << std::endl;
        }
    }
    
    // Check what's actually in the Ubuntu library directory
    std::cout << "\n=== Ubuntu Library Directory Contents ===" << std::endl;
    std::string ubuntuLibDir = "/usr/lib/x86_64-linux-gnu";
    if (std::filesystem::exists(ubuntuLibDir)) {
        std::cout << "Directory exists: " << ubuntuLibDir << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator(ubuntuLibDir)) {
            std::string filename = entry.path().filename().string();
            if (filename.find("libssl") != std::string::npos || 
                filename.find("libcrypto") != std::string::npos ||
                filename.find("libc.so") != std::string::npos ||
                filename.find("libpthread") != std::string::npos) {
                std::cout << "  " << filename << std::endl;
            }
        }
    } else {
        std::cout << "Directory does not exist: " << ubuntuLibDir << std::endl;
    }
    
    return 0;
} 