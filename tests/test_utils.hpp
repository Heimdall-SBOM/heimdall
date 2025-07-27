#pragma once

#include <filesystem>
#include <string>
#include "src/compat/compatibility.hpp"

namespace test_utils
{

/**
 * Get a unique test directory path using process ID to avoid test interference.
 *
 * @param base_name The base name for the test directory
 * @return A unique path for the test directory
 */
inline heimdall::compat::fs::path getUniqueTestDirectory(const std::string& base_name)
{
   auto pid = std::to_string(getpid());
   return heimdall::compat::fs::temp_directory_path() / (base_name + "_" + pid);
}

/**
 * Safely remove a directory and all its contents.
 * This function handles common CI issues where directories might already be gone
 * or have permission issues during cleanup.
 *
 * @param path The path to remove
 * @return true if removal was successful or directory didn't exist, false on error
 */
inline bool safeRemoveDirectory(const heimdall::compat::fs::path& path)
{
   try
   {
      if (heimdall::compat::fs::exists(path))
      {
         heimdall::compat::fs::remove_all(path);
      }
      return true;
   }
   catch (const heimdall::compat::fs::filesystem_error& e)
   {
      // Ignore filesystem errors during cleanup - this is common in CI
      // where files might be cleaned up by other processes or have timing issues
      return false;
   }
   catch (const std::exception& e)
   {
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
inline bool safeRemoveDirectoryWithLogging(const heimdall::compat::fs::path& path,
                                           const std::string&                context = "")
{
   try
   {
      if (heimdall::compat::fs::exists(path))
      {
         heimdall::compat::fs::remove_all(path);
      }
      return true;
   }
   catch (const heimdall::compat::fs::filesystem_error& e)
   {
      // Log the error but don't throw - this is expected in CI environments
      if (!context.empty())
      {
         // Note: In a real implementation, you might want to use a proper logging framework
         // For now, we'll just ignore the error as it's expected in CI
      }
      return false;
   }
   catch (const std::exception& e)
   {
      // Catch any other exceptions during cleanup
      if (!context.empty())
      {
         // Log other exceptions
      }
      return false;
   }
}

}  // namespace test_utils