#include "llvm_detector.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace heimdall
{
namespace llvm
{

LLVMVersion LLVMDetector::detectVersion()
{
   if (!isLLVMAvailable())
   {
      return LLVMVersion::UNKNOWN;
   }

   // Try to get LLVM version using llvm-config
   const char* llvm_config = std::getenv("LLVM_CONFIG");
   std::string config_cmd  = llvm_config ? llvm_config : "llvm-config";

   // Execute llvm-config --version
   FILE* pipe = popen((config_cmd + " --version").c_str(), "r");
   if (!pipe)
   {
      return LLVMVersion::UNKNOWN;
   }

   char        buffer[128];
   std::string result;
   while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
   {
      result += buffer;
   }
   pclose(pipe);

   // Remove newline
   if (!result.empty() && result[result.length() - 1] == '\n')
   {
      result.erase(result.length() - 1);
   }

   return parseVersionString(result);
}

bool LLVMDetector::supportsDWARF(LLVMVersion version)
{
   switch (version)
   {
      case LLVMVersion::LLVM_7_10:
         return true;  // Basic DWARF support
      case LLVMVersion::LLVM_11_18:
         return true;  // Good DWARF support
      case LLVMVersion::LLVM_19_PLUS:
         return true;  // Full DWARF support
      case LLVMVersion::UNKNOWN:
      default:
         return false;
   }
}

bool LLVMDetector::supportsCXXStandard(LLVMVersion version, int standard)
{
   switch (version)
   {
      case LLVMVersion::LLVM_7_10:
         return standard >= 11 && standard <= 14;  // C++11/14 only
      case LLVMVersion::LLVM_11_18:
         return standard >= 11 && standard <= 17;  // C++11-17
      case LLVMVersion::LLVM_19_PLUS:
         return standard >= 11 && standard <= 23;  // C++11-23
      case LLVMVersion::UNKNOWN:
      default:
         return false;
   }
}

LLVMVersion LLVMDetector::getMinimumLLVMVersion(int standard)
{
   if (standard >= 11 && standard <= 14)
   {
      return LLVMVersion::LLVM_7_10;
   }
   else if (standard >= 15 && standard <= 17)
   {
      return LLVMVersion::LLVM_11_18;
   }
   else if (standard >= 18 && standard <= 23)
   {
      return LLVMVersion::LLVM_19_PLUS;
   }
   else
   {
      return LLVMVersion::UNKNOWN;
   }
}

std::string LLVMDetector::getVersionString(LLVMVersion version)
{
   switch (version)
   {
      case LLVMVersion::LLVM_7_10:
         return "LLVM 7-10 (C++11/14 compatible)";
      case LLVMVersion::LLVM_11_18:
         return "LLVM 11-18 (C++14+ compatible)";
      case LLVMVersion::LLVM_19_PLUS:
         return "LLVM 19+ (C++17+ required, C++20/23 supported)";
      case LLVMVersion::UNKNOWN:
      default:
         return "Unknown LLVM version";
   }
}

std::vector<int> LLVMDetector::getSupportedCXXStandards(LLVMVersion version)
{
   std::vector<int> standards;

   switch (version)
   {
      case LLVMVersion::LLVM_7_10:
         standards = {11, 14};
         break;
      case LLVMVersion::LLVM_11_18:
         standards = {11, 14, 17};
         break;
      case LLVMVersion::LLVM_19_PLUS:
         standards = {11, 14, 17, 20, 23};
         break;
      case LLVMVersion::UNKNOWN:
      default:
         break;
   }

   return standards;
}

LLVMVersion LLVMDetector::parseVersionString(const std::string& versionString)
{
   if (versionString.empty())
   {
      return LLVMVersion::UNKNOWN;
   }

   // Parse major version number
   std::istringstream iss(versionString);
   int                major;
   if (!(iss >> major))
   {
      return LLVMVersion::UNKNOWN;
   }

   // Determine version category
   if (major >= 7 && major <= 10)
   {
      return LLVMVersion::LLVM_7_10;
   }
   else if (major >= 11 && major <= 18)
   {
      return LLVMVersion::LLVM_11_18;
   }
   else if (major >= 19)
   {
      return LLVMVersion::LLVM_19_PLUS;
   }
   else
   {
      return LLVMVersion::UNKNOWN;
   }
}

bool LLVMDetector::isLLVMAvailable()
{
   // Check if llvm-config is available
   int result = system("llvm-config --version > /dev/null 2>&1");
   return result == 0;
}

}  // namespace llvm
}  // namespace heimdall