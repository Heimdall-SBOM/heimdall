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
 * @file MetadataHelpers.cpp
 * @brief Implementation of helper functions for metadata extraction
 * @author Trevor Bakker
 * @date 2025
 */

#include "MetadataHelpers.hpp"
#include <iostream>
// Conditionally include DWARFExtractor for C++17+
#if __cplusplus >= 201703L
#include "../extractors/DWARFExtractor.hpp"
#else
#include "../extractors/LightweightDWARFParser.hpp"
#endif
#include "../factories/BinaryFormatFactory.hpp"

namespace heimdall
{

namespace MetadataHelpers
{

namespace
{
bool testMode = false;
}

bool isELF(const std::string& filePath)
{
   auto extractor = BinaryFormatFactory::createExtractor("ELF");
   return extractor && extractor->canHandle(filePath);
}

bool isMachO(const std::string& filePath)
{
   auto extractor = BinaryFormatFactory::createExtractor("Mach-O");
   return extractor && extractor->canHandle(filePath);
}

bool isPE(const std::string& filePath)
{
   auto extractor = BinaryFormatFactory::createExtractor("PE");
   return extractor && extractor->canHandle(filePath);
}

bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
#else
   LightweightDWARFParser dwarfExtractor;
#endif
   return dwarfExtractor.extractSourceFiles(filePath, sourceFiles);
}

bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
#else
   LightweightDWARFParser dwarfExtractor;
#endif
   return dwarfExtractor.extractCompileUnits(filePath, compileUnits);
}

bool extractDebugInfo(const std::string& filePath, ComponentInfo& component)
{
   // Use appropriate DWARF extractor based on C++ standard
#if __cplusplus >= 201703L
   DWARFExtractor dwarfExtractor;
   std::vector<std::string> sourceFiles, compileUnits, functions, lineInfo;
   bool                     result =
      dwarfExtractor.extractAllDebugInfo(filePath, sourceFiles, compileUnits, functions, lineInfo);
#else
   LightweightDWARFParser dwarfExtractor;
   std::vector<std::string> sourceFiles, compileUnits, functions;
   bool                     result =
      dwarfExtractor.extractAllDebugInfo(filePath, sourceFiles, compileUnits, functions);
   std::vector<std::string> lineInfo; // LightweightDWARFParser doesn't support lineInfo
#endif

   if (result)
   {
      component.containsDebugInfo = true;
      component.sourceFiles       = sourceFiles;
      component.compileUnits      = compileUnits;
      component.functions         = functions;
   }

   return result;
}

bool extractELFBuildId(const std::string& filePath, std::string& buildId)
{
   // This is a placeholder implementation
   // In a real implementation, this would parse the ELF file and extract the build ID
   // from the .note.gnu.build-id section
   if (testMode)
   {
      buildId = "test_build_id_1234567890abcdef";
      return true;
   }

   // For now, return false as this is not fully implemented
   return false;
}

void setTestMode(bool enabled)
{
   testMode = enabled;
}

}  // namespace MetadataHelpers

}  // namespace heimdall