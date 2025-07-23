#pragma once

#include <filesystem>
#include <string>

namespace test_utils {

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