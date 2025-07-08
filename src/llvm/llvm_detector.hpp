#pragma once

#include <string>
#include <vector>

namespace heimdall {
namespace llvm {

enum class LLVMVersion {
    UNKNOWN = 0,
    LLVM_7_10 = 1,    // C++11/14 compatible
    LLVM_11_18 = 2,   // C++14+ compatible
    LLVM_19_PLUS = 3  // C++17+ required
};

class LLVMDetector {
public:
    /**
     * @brief Detect the installed LLVM version
     * @return Detected LLVM version
     */
    static LLVMVersion detectVersion();
    
    /**
     * @brief Check if LLVM version supports DWARF functionality
     * @param version LLVM version to check
     * @return true if DWARF is supported
     */
    static bool supportsDWARF(LLVMVersion version);
    
    /**
     * @brief Check if LLVM version supports a specific C++ standard
     * @param version LLVM version to check
     * @param standard C++ standard (11, 14, 17, 20, 23)
     * @return true if the standard is supported
     */
    static bool supportsCXXStandard(LLVMVersion version, int standard);
    
    /**
     * @brief Get the minimum LLVM version required for a C++ standard
     * @param standard C++ standard (11, 14, 17, 20, 23)
     * @return Minimum required LLVM version
     */
    static LLVMVersion getMinimumLLVMVersion(int standard);
    
    /**
     * @brief Get version string for display
     * @param version LLVM version
     * @return Human-readable version string
     */
    static std::string getVersionString(LLVMVersion version);
    
    /**
     * @brief Get supported C++ standards for a given LLVM version
     * @param version LLVM version
     * @return Vector of supported C++ standards
     */
    static std::vector<int> getSupportedCXXStandards(LLVMVersion version);

private:
    /**
     * @brief Parse LLVM version from string
     * @param versionString Version string (e.g., "19.1.0")
     * @return Parsed LLVM version
     */
    static LLVMVersion parseVersionString(const std::string& versionString);
    
    /**
     * @brief Check if LLVM is available on the system
     * @return true if LLVM is found
     */
    static bool isLLVMAvailable();
};

} // namespace llvm
} // namespace heimdall