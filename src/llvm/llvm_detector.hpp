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

/**
 * @file llvm_detector.hpp
 * @brief LLVM version detection and compatibility checking utilities
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides utilities for detecting the installed LLVM version
 * and checking compatibility with different C++ standards. It includes:
 * - LLVM version enumeration
 * - Version detection functionality
 * - C++ standard compatibility checking
 * - DWARF support verification
 *
 * The detector helps ensure that the appropriate LLVM features are
 * available for the target C++ standard and platform.
 */

#pragma once

#include <string>
#include <vector>

namespace heimdall
{
namespace llvm
{

/**
 * @brief Enumeration of LLVM versions with compatibility information
 *
 * This enum represents different LLVM versions and their compatibility
 * with various C++ standards and features.
 */
enum class LLVMVersion
{
  UNKNOWN      = 0,  ///< Unknown or undetectable LLVM version
  LLVM_7_10    = 1,  ///< LLVM 7-10: C++11/14 compatible
  LLVM_11_18   = 2,  ///< LLVM 11-18: C++14+ compatible
  LLVM_19_PLUS = 3   ///< LLVM 19+: C++17+ required
};

/**
 * @brief LLVM version detection and compatibility checking class
 *
 * This class provides static methods for detecting the installed LLVM version
 * and checking compatibility with different C++ standards and features.
 * It helps ensure that the appropriate LLVM functionality is available
 * for the target compilation environment.
 */
class LLVMDetector
{
  public:
  /**
   * @brief Detect the installed LLVM version
   *
   * Attempts to detect the LLVM version installed on the system
   * by checking various common installation paths and version
   * indicators.
   *
   * @return Detected LLVM version, or UNKNOWN if detection fails
   */
  static LLVMVersion detectVersion();

  /**
   * @brief Check if LLVM version supports DWARF functionality
   *
   * Determines whether the specified LLVM version has reliable
   * DWARF debug information support, which is required for
   * extracting function and source information.
   *
   * @param version LLVM version to check
   * @return true if DWARF is supported, false otherwise
   */
  static bool supportsDWARF(LLVMVersion version);

  /**
   * @brief Check if LLVM version supports a specific C++ standard
   *
   * Verifies whether the specified LLVM version can properly
   * handle the given C++ standard for compilation and linking.
   *
   * @param version LLVM version to check
   * @param standard C++ standard (11, 14, 17, 20, 23)
   * @return true if the standard is supported, false otherwise
   */
  static bool supportsCXXStandard(LLVMVersion version, int standard);

  /**
   * @brief Get the minimum LLVM version required for a C++ standard
   *
   * Determines the minimum LLVM version needed to properly
   * support the specified C++ standard.
   *
   * @param standard C++ standard (11, 14, 17, 20, 23)
   * @return Minimum required LLVM version
   */
  static LLVMVersion getMinimumLLVMVersion(int standard);

  /**
   * @brief Get version string for display
   *
   * Converts the LLVM version enum to a human-readable string
   * for display purposes.
   *
   * @param version LLVM version
   * @return Human-readable version string
   */
  static std::string getVersionString(LLVMVersion version);

  /**
   * @brief Get supported C++ standards for a given LLVM version
   *
   * Returns a list of C++ standards that are supported by
   * the specified LLVM version.
   *
   * @param version LLVM version
   * @return Vector of supported C++ standards
   */
  static std::vector<int> getSupportedCXXStandards(LLVMVersion version);

  private:
  /**
   * @brief Parse LLVM version from string
   *
   * Converts a version string (e.g., "19.1.0") to the
   * corresponding LLVMVersion enum value.
   *
   * @param versionString Version string (e.g., "19.1.0")
   * @return Parsed LLVM version, or UNKNOWN if parsing fails
   */
  static LLVMVersion parseVersionString(const std::string& versionString);

  /**
   * @brief Check if LLVM is available on the system
   *
   * Attempts to detect whether LLVM is installed and accessible
   * on the current system.
   *
   * @return true if LLVM is found, false otherwise
   */
  static bool isLLVMAvailable();
};

}  // namespace llvm
}  // namespace heimdall