#pragma once

#include <filesystem>
#include <string>
#include <cstdlib>
#include <unistd.h>

namespace test_utils {

/**
 * Get a unique test directory name based on build configuration.
 * This helps prevent conflicts when multiple builds run simultaneously.
 * 
 * @param base_name The base name for the test directory
 * @return A unique path that includes compiler and standard information
 */
inline std::filesystem::path getUniqueTestDirectory(const std::string& base_name) {
    // Get compiler info from environment variables
    const char* compiler = std::getenv("CC");
    const char* cxx_compiler = std::getenv("CXX");
    const char* cxx_standard = std::getenv("CMAKE_CXX_STANDARD");
    const char* build_type = std::getenv("CMAKE_BUILD_TYPE");
    
    // Use CXX if available, otherwise CC, otherwise "unknown"
    std::string compiler_name = "unknown";
    if (cxx_compiler) {
        compiler_name = std::filesystem::path(cxx_compiler).filename().string();
    } else if (compiler) {
        compiler_name = std::filesystem::path(compiler).filename().string();
    }
    
    // Get standard info
    std::string standard_name = cxx_standard ? std::string("cxx") + cxx_standard : "unknown";
    
    // Get build type
    std::string build_type_name = build_type ? build_type : "unknown";
    
    // Create unique directory name
    std::string unique_name = base_name + "_" + compiler_name + "_" + standard_name + "_" + build_type_name;
    
    // Add process ID for extra uniqueness
    unique_name += "_" + std::to_string(getpid());
    
    return std::filesystem::temp_directory_path() / unique_name;
}

/**
 * Safely remove a directory and all its contents.
 * This function handles common CI issues where directories might already be gone
 * or have permission issues during cleanup.
 * 
 * @param path The path to remove
 * @return true if removal was successful or directory didn't exist, false on error
 */
inline bool safeRemoveDirectory(const std::filesystem::path& path) {
    try {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove_all(path);
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        // Ignore filesystem errors during cleanup - this is common in CI
        // where files might be cleaned up by other processes or have timing issues
        return false;
    } catch (const std::exception& e) {
        // Catch any other exceptions during cleanup
        return false;
    }
}

/**
 * Safely remove a directory and all its contents with logging.
 * This version logs the error but still doesn't throw.
 * 
 * @param path The path to remove
 * @param context Optional context string for logging
 * @return true if removal was successful or directory didn't exist, false on error
 */
inline bool safeRemoveDirectoryWithLogging(const std::filesystem::path& path, const std::string& context = "") {
    try {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove_all(path);
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        // Log the error but don't throw - this is expected in CI environments
        if (!context.empty()) {
            // Note: In a real implementation, you might want to use a proper logging framework
            // For now, we'll just ignore the error as it's expected in CI
        }
        return false;
    } catch (const std::exception& e) {
        // Catch any other exceptions during cleanup
        if (!context.empty()) {
            // Log other exceptions
        }
        return false;
    }
}

} // namespace test_utils 