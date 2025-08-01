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
 * @file LightweightDWARFParser.cpp
 * @brief Implementation of lightweight DWARF parser for C++11/14 compatibility
 * @author Trevor Bakker
 * @date 2025
 * @version 2.0.0
 *
 * This file implements a lightweight DWARF parser that extracts debug information
 * without using LLVM's problematic template features. It's designed for C++11/14
 * compatibility and provides the same interface as the LLVM-based DWARFExtractor.
 *
 * The parser implements the IBinaryExtractor interface and specializes in
 * extracting debug information from ELF files.
 */

#include "LightweightDWARFParser.hpp"
#include "../compat/compatibility.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#ifdef __linux__
#include <elf.h>
#endif

namespace heimdall
{

// DWARF constants
constexpr uint32_t DW_TAG_compile_unit           = 0x11;
constexpr uint32_t DW_TAG_subprogram             = 0x2e;
constexpr uint32_t DW_TAG_namespace              = 0x39;
constexpr uint32_t DW_TAG_class_type             = 0x02;
constexpr uint32_t DW_TAG_structure_type         = 0x13;
constexpr uint32_t DW_TAG_enumeration_type       = 0x04;
constexpr uint32_t DW_TAG_enumerator             = 0x0b;
constexpr uint32_t DW_TAG_variable               = 0x34;
constexpr uint32_t DW_TAG_formal_parameter       = 0x05;
constexpr uint32_t DW_TAG_unspecified_parameters = 0x06;
constexpr uint32_t DW_TAG_unspecified_type       = 0x3b;
constexpr uint32_t DW_TAG_pointer_type           = 0x0f;
constexpr uint32_t DW_TAG_reference_type         = 0x10;
constexpr uint32_t DW_TAG_const_type             = 0x26;
constexpr uint32_t DW_TAG_volatile_type          = 0x12;
constexpr uint32_t DW_TAG_typedef                = 0x16;
constexpr uint32_t DW_TAG_array_type             = 0x01;
constexpr uint32_t DW_TAG_subrange_type          = 0x21;
constexpr uint32_t DW_TAG_union_type             = 0x17;
constexpr uint32_t DW_TAG_inheritance            = 0x28;
constexpr uint32_t DW_TAG_member                 = 0x0d;
constexpr uint32_t DW_TAG_subroutine_type        = 0x21;
constexpr uint32_t DW_TAG_inlined_subroutine     = 0x1d;
constexpr uint32_t DW_TAG_lexical_block          = 0x0b;
constexpr uint32_t DW_TAG_try_block              = 0x2b;
constexpr uint32_t DW_TAG_catch_block            = 0x03;
constexpr uint32_t DW_TAG_label                  = 0x0a;
constexpr uint32_t DW_TAG_imported_declaration   = 0x08;
constexpr uint32_t DW_TAG_imported_module        = 0x3a;
constexpr uint32_t DW_TAG_imported_unit          = 0x3d;
constexpr uint32_t DW_TAG_condition              = 0x3f;
constexpr uint32_t DW_TAG_shared_type            = 0x40;
constexpr uint32_t DW_TAG_type_unit              = 0x41;
constexpr uint32_t DW_TAG_rvalue_reference_type  = 0x42;
constexpr uint32_t DW_TAG_template_alias         = 0x43;
constexpr uint32_t DW_TAG_coarray_type           = 0x44;
constexpr uint32_t DW_TAG_dynamic_type           = 0x45;
constexpr uint32_t DW_TAG_generic_subrange       = 0x46;
constexpr uint32_t DW_TAG_atomic_type            = 0x47;
constexpr uint32_t DW_TAG_call_site              = 0x48;
constexpr uint32_t DW_TAG_call_site_parameter    = 0x49;
constexpr uint32_t DW_TAG_skeleton_unit          = 0x4a;
constexpr uint32_t DW_TAG_immutable_type         = 0x4b;

// DWARF attributes
constexpr uint32_t DW_AT_sibling              = 0x01;
constexpr uint32_t DW_AT_location             = 0x02;
constexpr uint32_t DW_AT_name                 = 0x03;
constexpr uint32_t DW_AT_ordering             = 0x09;
constexpr uint32_t DW_AT_subscr_data          = 0x0a;
constexpr uint32_t DW_AT_byte_size            = 0x0b;
constexpr uint32_t DW_AT_bit_offset           = 0x0c;
constexpr uint32_t DW_AT_bit_size             = 0x0d;
constexpr uint32_t DW_AT_element_list         = 0x0f;
constexpr uint32_t DW_AT_stmt_list            = 0x10;
constexpr uint32_t DW_AT_low_pc               = 0x11;
constexpr uint32_t DW_AT_high_pc              = 0x12;
constexpr uint32_t DW_AT_language             = 0x13;
constexpr uint32_t DW_AT_member               = 0x0e;
constexpr uint32_t DW_AT_discr                = 0x15;
constexpr uint32_t DW_AT_discr_value          = 0x16;
constexpr uint32_t DW_AT_visibility           = 0x17;
constexpr uint32_t DW_AT_import               = 0x18;
constexpr uint32_t DW_AT_string_length        = 0x19;
constexpr uint32_t DW_AT_common_reference     = 0x1a;
constexpr uint32_t DW_AT_comp_dir             = 0x1b;
constexpr uint32_t DW_AT_const_value          = 0x1c;
constexpr uint32_t DW_AT_containing_type      = 0x1d;
constexpr uint32_t DW_AT_default_value        = 0x1e;
constexpr uint32_t DW_AT_inline               = 0x20;
constexpr uint32_t DW_AT_is_optional          = 0x21;
constexpr uint32_t DW_AT_lower_bound          = 0x22;
constexpr uint32_t DW_AT_producer             = 0x25;
constexpr uint32_t DW_AT_prototyped           = 0x27;
constexpr uint32_t DW_AT_return_addr          = 0x2a;
constexpr uint32_t DW_AT_start_scope          = 0x2c;
constexpr uint32_t DW_AT_stride_size          = 0x2e;
constexpr uint32_t DW_AT_upper_bound          = 0x2f;
constexpr uint32_t DW_AT_abstract_origin      = 0x31;
constexpr uint32_t DW_AT_accessibility        = 0x32;
constexpr uint32_t DW_AT_address_class        = 0x33;
constexpr uint32_t DW_AT_artificial           = 0x34;
constexpr uint32_t DW_AT_base_types           = 0x35;
constexpr uint32_t DW_AT_calling_convention   = 0x36;
constexpr uint32_t DW_AT_count                = 0x37;
constexpr uint32_t DW_AT_data_member_location = 0x38;
constexpr uint32_t DW_AT_decl_column          = 0x39;
constexpr uint32_t DW_AT_decl_file            = 0x3a;
constexpr uint32_t DW_AT_decl_line            = 0x3b;
constexpr uint32_t DW_AT_declaration          = 0x3c;
constexpr uint32_t DW_AT_discr_list           = 0x3d;
constexpr uint32_t DW_AT_encoding             = 0x3e;
constexpr uint32_t DW_AT_external             = 0x3f;
constexpr uint32_t DW_AT_frame_base           = 0x40;
constexpr uint32_t DW_AT_friend               = 0x41;
constexpr uint32_t DW_AT_identifier_case      = 0x42;
constexpr uint32_t DW_AT_macro_info           = 0x43;
constexpr uint32_t DW_AT_namelist_item        = 0x44;
constexpr uint32_t DW_AT_priority             = 0x45;
constexpr uint32_t DW_AT_segment              = 0x46;
constexpr uint32_t DW_AT_specification        = 0x47;
constexpr uint32_t DW_AT_static_link          = 0x48;
constexpr uint32_t DW_AT_type                 = 0x49;
constexpr uint32_t DW_AT_use_location         = 0x4a;
constexpr uint32_t DW_AT_variable_parameter   = 0x4b;
constexpr uint32_t DW_AT_virtuality           = 0x4c;
constexpr uint32_t DW_AT_vtable_elem_location = 0x4d;

// DWARF forms
constexpr uint32_t DW_FORM_addr           = 0x01;
constexpr uint32_t DW_FORM_block2         = 0x03;
constexpr uint32_t DW_FORM_block4         = 0x04;
constexpr uint32_t DW_FORM_data2          = 0x05;
constexpr uint32_t DW_FORM_data4          = 0x06;
constexpr uint32_t DW_FORM_data8          = 0x07;
constexpr uint32_t DW_FORM_string         = 0x08;
constexpr uint32_t DW_FORM_block          = 0x09;
constexpr uint32_t DW_FORM_block1         = 0x0a;
constexpr uint32_t DW_FORM_data1          = 0x0b;
constexpr uint32_t DW_FORM_flag           = 0x0c;
constexpr uint32_t DW_FORM_sdata          = 0x0d;
constexpr uint32_t DW_FORM_strp           = 0x0e;
constexpr uint32_t DW_FORM_udata          = 0x0f;
constexpr uint32_t DW_FORM_ref_addr       = 0x10;
constexpr uint32_t DW_FORM_ref1           = 0x11;
constexpr uint32_t DW_FORM_ref2           = 0x12;
constexpr uint32_t DW_FORM_ref4           = 0x13;
constexpr uint32_t DW_FORM_ref8           = 0x14;
constexpr uint32_t DW_FORM_ref_udata      = 0x15;
constexpr uint32_t DW_FORM_indirect       = 0x16;
constexpr uint32_t DW_FORM_sec_offset     = 0x17;
constexpr uint32_t DW_FORM_exprloc        = 0x18;
constexpr uint32_t DW_FORM_flag_present   = 0x19;
constexpr uint32_t DW_FORM_ref_sig8       = 0x20;
constexpr uint32_t DW_FORM_strx           = 0x1a;
constexpr uint32_t DW_FORM_addrx          = 0x1b;
constexpr uint32_t DW_FORM_ref_sup4       = 0x1c;
constexpr uint32_t DW_FORM_strp_sup       = 0x1d;
constexpr uint32_t DW_FORM_data16         = 0x1e;
constexpr uint32_t DW_FORM_line_strp      = 0x1f;
constexpr uint32_t DW_FORM_implicit_const = 0x21;
constexpr uint32_t DW_FORM_loclistx       = 0x22;
constexpr uint32_t DW_FORM_rnglistx       = 0x23;
constexpr uint32_t DW_FORM_ref_sup8       = 0x24;
constexpr uint32_t DW_FORM_strx1          = 0x25;
constexpr uint32_t DW_FORM_strx2          = 0x26;
constexpr uint32_t DW_FORM_strx3          = 0x27;
constexpr uint32_t DW_FORM_strx4          = 0x28;
constexpr uint32_t DW_FORM_addrx1         = 0x29;
constexpr uint32_t DW_FORM_addrx2         = 0x2a;
constexpr uint32_t DW_FORM_addrx3         = 0x2b;
constexpr uint32_t DW_FORM_addrx4         = 0x2c;

// PIMPL implementation
class LightweightDWARFParser::Impl
{
   public:
   Impl()  = default;
   ~Impl() = default;

   // C++11 compatibility: Use std::set instead of std::unordered_set for better C++11 support
   std::set<std::string> uniqueSourceFiles;
   std::set<std::string> uniqueCompileUnits;
   std::set<std::string> uniqueFunctions;
};

LightweightDWARFParser::LightweightDWARFParser() : pImpl(std::make_unique<Impl>())
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser constructor called" << std::endl;
#endif
}

LightweightDWARFParser::~LightweightDWARFParser() = default;

LightweightDWARFParser::LightweightDWARFParser(const LightweightDWARFParser& other)
   : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

LightweightDWARFParser::LightweightDWARFParser(LightweightDWARFParser&& other) noexcept
   : pImpl(std::move(other.pImpl))
{
}

LightweightDWARFParser& LightweightDWARFParser::operator=(const LightweightDWARFParser& other)
{
   if (this != &other)
   {
      pImpl = std::make_unique<Impl>(*other.pImpl);
   }
   return *this;
}

LightweightDWARFParser& LightweightDWARFParser::operator=(LightweightDWARFParser&& other) noexcept
{
   if (this != &other)
   {
      pImpl = std::move(other.pImpl);
   }
   return *this;
}

// IBinaryExtractor interface implementation
bool LightweightDWARFParser::extractSymbols(const std::string&       filePath,
                                            std::vector<SymbolInfo>& symbols)
{
   std::vector<std::string> functions;
   if (extractFunctions(filePath, functions))
   {
      // Convert function names to SymbolInfo
      for (const auto& funcName : functions)
      {
         SymbolInfo symbol;
         symbol.name      = funcName;
         symbol.isDefined = true;
         symbol.isGlobal  = true;
         symbols.push_back(symbol);
      }
      return true;
   }
   return false;
}

bool LightweightDWARFParser::extractSections(const std::string&        filePath,
                                             std::vector<SectionInfo>& sections)
{
   // This extractor specializes in debug information, not section extraction
   // Delegate to a more appropriate extractor if available
   return false;
}

bool LightweightDWARFParser::extractVersion(const std::string& filePath, std::string& version)
{
   // This extractor specializes in debug information, not version extraction
   // Delegate to a more appropriate extractor if available
   return false;
}

std::vector<std::string> LightweightDWARFParser::extractDependencies(const std::string& filePath)
{
   // This extractor specializes in debug information, not dependency extraction
   // Delegate to a more appropriate extractor if available
   return {};
}

bool LightweightDWARFParser::canHandle(const std::string& filePath) const
{
   // Can handle ELF files with DWARF debug information
   return hasDWARFInfo(filePath);
}

std::string LightweightDWARFParser::getFormatName() const
{
   return "Lightweight DWARF Parser";
}

int LightweightDWARFParser::getPriority() const
{
   return 20;  // Lower priority than specific format extractors, higher than generic ones
}

// Legacy interface methods for backward compatibility
bool LightweightDWARFParser::extractSourceFiles(const std::string&        filePath,
                                                std::vector<std::string>& sourceFiles)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractSourceFiles called for " << filePath << std::endl;
#endif

   // Clear previous results
   pImpl->uniqueSourceFiles.clear();
   sourceFiles.clear();

   // Try DWARF debug line section first
   if (parseDWARFDebugLine(filePath, sourceFiles))
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LightweightDWARFParser: DWARF debug line parsing succeeded" << std::endl;
#endif
      return true;
   }

#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout
      << "LightweightDWARFParser: DWARF debug line parsing failed, trying heuristic extraction"
      << std::endl;
#endif

   // Fallback to heuristic extraction
   std::cout << "DEBUG: Calling heuristic extraction for " << filePath << std::endl;
   return extractSourceFilesHeuristic(filePath, sourceFiles);
}

bool LightweightDWARFParser::extractCompileUnits(const std::string&        filePath,
                                                 std::vector<std::string>& compileUnits)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractCompileUnits called for " << filePath << std::endl;
#endif

   // Clear previous results
   pImpl->uniqueCompileUnits.clear();
   compileUnits.clear();

   // Try DWARF debug info section
   std::vector<std::string> dummySourceFiles, dummyFunctions;
   if (parseDWARFDebugInfo(filePath, dummySourceFiles, compileUnits, dummyFunctions))
   {
      return true;
   }

   // Fallback: return empty (no compile units found)
   return false;
}

bool LightweightDWARFParser::extractFunctions(const std::string&        filePath,
                                              std::vector<std::string>& functions)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractFunctions called for " << filePath << std::endl;
#endif

   // Clear previous results
   pImpl->uniqueFunctions.clear();
   functions.clear();

   // Try DWARF debug info section first
   std::vector<std::string> dummySourceFiles, dummyCompileUnits;
   if (parseDWARFDebugInfo(filePath, dummySourceFiles, dummyCompileUnits, functions))
   {
      return true;
   }

   // Fallback to symbol table extraction
   return extractFunctionsFromSymbolTable(filePath, functions);
}

bool LightweightDWARFParser::extractAllDebugInfo(const std::string&        filePath,
                                                 std::vector<std::string>& sourceFiles,
                                                 std::vector<std::string>& compileUnits,
                                                 std::vector<std::string>& functions)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractAllDebugInfo called for " << filePath << std::endl;
#endif

   // Clear previous results
   pImpl->uniqueSourceFiles.clear();
   pImpl->uniqueCompileUnits.clear();
   pImpl->uniqueFunctions.clear();
   sourceFiles.clear();
   compileUnits.clear();
   functions.clear();

   // Try DWARF debug info section first
   if (parseDWARFDebugInfo(filePath, sourceFiles, compileUnits, functions))
   {
      return true;
   }

   // Fallback: try individual extractions
   bool success = false;

   if (extractSourceFiles(filePath, sourceFiles))
   {
      success = true;
   }

   if (extractCompileUnits(filePath, compileUnits))
   {
      success = true;
   }

   if (extractFunctions(filePath, functions))
   {
      success = true;
   }

   return success;
}

bool LightweightDWARFParser::hasDWARFInfo(const std::string& filePath) const
{
   uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
   return findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset);
}

// Private methods
bool LightweightDWARFParser::parseDWARFDebugInfo(const std::string&        filePath,
                                                 std::vector<std::string>& sourceFiles,
                                                 std::vector<std::string>& compileUnits,
                                                 std::vector<std::string>& functions)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: parseDWARFDebugInfo called for " << filePath << std::endl;
#endif

   uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
   if (!findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset))
   {
      return false;
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check if file is empty
   file.seekg(0, std::ios::end);
   std::streampos fileSize = file.tellg();
   if (fileSize <= 0)
   {
      return false;
   }

   // Read debug info section
   file.seekg(debugInfoOffset);
   std::vector<char> debugInfoData;
   debugInfoData.resize(1024 * 1024);  // 1MB buffer
   file.read(debugInfoData.data(), debugInfoData.size());
   size_t bytesRead = file.gcount();
   debugInfoData.resize(bytesRead);

   // Check if we have any data to parse
   if (debugInfoData.empty())
   {
      return false;
   }

   // Parse DWARF debug info
   uint32_t offset         = 0;
   uint32_t maxIterations  = 1000;  // Prevent infinite loops
   uint32_t iterationCount = 0;

   // Parse compilation units
   while (offset < debugInfoData.size() && iterationCount < maxIterations)
   {
      iterationCount++;

      // Check if we have enough data for a ULEB128
      if (offset >= debugInfoData.size())
      {
         break;
      }

      uint32_t abbrevCode = parseULEB128(debugInfoData.data(), offset);

      if (abbrevCode == 0)
      {
         // End of DIEs
         break;
      }

      // Parse attributes based on abbrev code
      // This is a simplified parser - in a full implementation,
      // we would parse the abbreviation table to know which attributes
      // are present for each DIE

      // For now, we'll extract what we can from common patterns
      if (abbrevCode == DW_TAG_compile_unit)
      {
         // Extract compile unit name
         if (offset < debugInfoData.size())
         {
            std::string name = readDWARFString(debugInfoData.data(), offset);
            if (!name.empty())
            {
               pImpl->uniqueCompileUnits.insert(name);
            }
         }
      }
      else if (abbrevCode == DW_TAG_subprogram)
      {
         // Extract function name
         if (offset < debugInfoData.size())
         {
            std::string name = readDWARFString(debugInfoData.data(), offset);
            if (!name.empty())
            {
               pImpl->uniqueFunctions.insert(name);
            }
         }
      }

      // Skip remaining attributes for this DIE
      // In a full implementation, we would parse the abbreviation table
      // to know exactly which attributes to expect
      if (offset + 4 <= debugInfoData.size())
      {
         offset += 4;  // Simplified skip
      }
      else
      {
         break;  // Not enough data left
      }
   }

   // Convert sets to vectors
   sourceFiles.assign(pImpl->uniqueSourceFiles.begin(), pImpl->uniqueSourceFiles.end());
   compileUnits.assign(pImpl->uniqueCompileUnits.begin(), pImpl->uniqueCompileUnits.end());
   functions.assign(pImpl->uniqueFunctions.begin(), pImpl->uniqueFunctions.end());

   return !sourceFiles.empty() || !compileUnits.empty() || !functions.empty();
}

bool LightweightDWARFParser::parseDWARFDebugLine(const std::string&        filePath,
                                                 std::vector<std::string>& sourceFiles)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: parseDWARFDebugLine called for " << filePath << std::endl;
#endif

   uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
   if (!findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset))
   {
      return false;
   }

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check if file is empty
   file.seekg(0, std::ios::end);
   std::streampos fileSize = file.tellg();
   if (fileSize <= 0)
   {
      return false;
   }

   // Read debug line section
   file.seekg(debugLineOffset);
   std::vector<char> debugLineData;
   debugLineData.resize(1024 * 1024);  // 1MB buffer
   file.read(debugLineData.data(), debugLineData.size());
   size_t bytesRead = file.gcount();
   debugLineData.resize(bytesRead);

   // Check if we have any data to parse
   if (debugLineData.empty())
   {
      return false;
   }

   // Parse DWARF debug line section
   uint32_t offset = 0;

   // Parse line program header
   if (offset + 12 > debugLineData.size())
   {
      return false;
   }

   // Read unit length
   uint32_t unitLength = *reinterpret_cast<const uint32_t*>(&debugLineData[offset]);
   offset += 4;

   // Validate unit length to prevent buffer overruns
   if (unitLength == 0 || unitLength > debugLineData.size() ||
       offset + unitLength > debugLineData.size())
   {
      return false;
   }

   // Read version
   uint16_t version = *reinterpret_cast<const uint16_t*>(&debugLineData[offset]);
   offset += 2;

   // Read header length
   uint32_t headerLength = *reinterpret_cast<const uint32_t*>(&debugLineData[offset]);
   offset += 4;

   // Validate header length
   if (headerLength == 0 || offset + headerLength > debugLineData.size())
   {
      return false;
   }

   // Read minimum instruction length
   uint8_t minInstLength = debugLineData[offset];
   offset += 1;

   // Read default is statement
   uint8_t defaultIsStmt = debugLineData[offset];
   offset += 1;

   // Read line base
   int8_t lineBase = static_cast<int8_t>(debugLineData[offset]);
   offset += 1;

   // Read line range
   uint8_t lineRange = debugLineData[offset];
   offset += 1;

   // Read opcode base
   uint8_t opcodeBase = debugLineData[offset];
   offset += 1;

   // Validate opcode base
   if (opcodeBase == 0 || offset + opcodeBase - 1 > debugLineData.size())
   {
      return false;
   }

   // Skip standard opcode lengths
   offset += opcodeBase - 1;

   // Parse include directories with bounds checking
   while (offset < debugLineData.size() && debugLineData[offset] != 0)
   {
      std::string dirName;
      uint32_t    startOffset = offset;
      while (offset < debugLineData.size() && debugLineData[offset] != 0)
      {
         dirName += debugLineData[offset];
         offset += 1;

         // Prevent infinite loops by checking if we're making progress
         if (offset - startOffset > 1024)
         {  // Max directory name length
            return false;
         }
      }
      if (offset < debugLineData.size())
      {
         offset += 1;  // Skip null terminator
      }
   }
   if (offset < debugLineData.size())
   {
      offset += 1;  // Skip final null terminator
   }

   // Parse file names with bounds checking
   while (offset < debugLineData.size() && debugLineData[offset] != 0)
   {
      std::string fileName;
      uint32_t    startOffset = offset;
      while (offset < debugLineData.size() && debugLineData[offset] != 0)
      {
         fileName += debugLineData[offset];
         offset += 1;

         // Prevent infinite loops by checking if we're making progress
         if (offset - startOffset > 1024)
         {  // Max filename length
            return false;
         }
      }
      if (offset < debugLineData.size())
      {
         offset += 1;  // Skip null terminator
      }

      // Skip directory index and modification time
      if (offset + 4 <= debugLineData.size())
      {
         offset += 4;
      }
      else
      {
         return false;
      }

      if (!fileName.empty())
      {
         pImpl->uniqueSourceFiles.insert(fileName);
      }
   }
   if (offset < debugLineData.size())
   {
      offset += 1;  // Skip final null terminator
   }

   // Skip line program instructions
   // In a full implementation, we would parse these to get line numbers
   if (offset + 4 <= debugLineData.size())
   {
      offset += 4;  // Simplified skip
   }

   sourceFiles.assign(pImpl->uniqueSourceFiles.begin(), pImpl->uniqueSourceFiles.end());
   return !sourceFiles.empty();
}

bool LightweightDWARFParser::parseDWARFDebugInfo(const std::string&        filePath,
                                                 std::vector<std::string>& compileUnits,
                                                 std::vector<std::string>& functions)
{
   std::vector<std::string> dummySourceFiles;
   return parseDWARFDebugInfo(filePath, dummySourceFiles, compileUnits, functions);
}

bool LightweightDWARFParser::extractFunctionsFromSymbolTable(const std::string&        filePath,
                                                             std::vector<std::string>& functions)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractFunctionsFromSymbolTable called for " << filePath
             << std::endl;
#endif

   // This is a simplified implementation that extracts function names from the symbol table
   // In a full implementation, we would parse the ELF symbol table to find function symbols

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Read ELF header
   ELFHeader header{};
   file.read(reinterpret_cast<char*>(&header), sizeof(header));

   if (file.gcount() != sizeof(header))
   {
      return false;
   }

   // Check ELF magic
   if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || header.e_ident[2] != 'L' ||
       header.e_ident[3] != 'F')
   {
      return false;
   }

   // Find symbol table section
   file.seekg(header.e_shoff);
   for (uint16_t i = 0; i < header.e_shnum; ++i)
   {
      ELFSectionHeader section{};
      file.read(reinterpret_cast<char*>(&section), sizeof(section));

      if (file.gcount() != sizeof(section))
      {
         break;
      }

      // Check if this is a symbol table section
      if (section.sh_type == 2)
      {  // SHT_SYMTAB
         // Read symbol table
         file.seekg(section.sh_offset);
         std::vector<char> symbolTable(section.sh_size);
         file.read(symbolTable.data(), section.sh_size);

         if (file.gcount() == section.sh_size)
         {
            // Parse symbol table to find function symbols
            // This is a simplified implementation
            // In a full implementation, we would parse each symbol entry

            // For now, return empty (no functions found)
            return false;
         }
      }
   }

   return false;
}

bool LightweightDWARFParser::extractSourceFilesHeuristic(const std::string&        filePath,
                                                         std::vector<std::string>& sourceFiles)
{
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: extractSourceFilesHeuristic called for " << filePath
             << std::endl;
#endif

   // This is a heuristic approach that tries to extract source file information
   // from the binary without relying on DWARF information

   // For now, we'll use a simple approach: look for common source file extensions
   // in the binary's string table or other sections

   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LightweightDWARFParser: Failed to open file for heuristic extraction"
                << std::endl;
#endif
      return false;
   }

   // Check if file is empty
   file.seekg(0, std::ios::end);
   size_t fileSize = file.tellg();
   if (fileSize <= 0)
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LightweightDWARFParser: File is empty for heuristic extraction" << std::endl;
#endif
      return false;
   }

   file.seekg(0, std::ios::beg);

   std::vector<char> fileData(fileSize);
   file.read(fileData.data(), fileSize);

   if (file.gcount() != fileSize)
   {
#ifdef HEIMDALL_DEBUG_ENABLED
      std::cout << "LightweightDWARFParser: Failed to read file data for heuristic extraction"
                << std::endl;
#endif
      return false;
   }

#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: File size for heuristic extraction: " << fileSize
             << std::endl;
   std::string fileContent(fileData.begin(), fileData.end());
   std::cout << "LightweightDWARFParser: File content: " << fileContent << std::endl;
#endif

   // Look for common source file patterns
   std::vector<std::string> extensions = {".c", ".cpp", ".cc", ".cxx", ".h", ".hpp", ".hh", ".hxx"};

   std::cout << "DEBUG: Looking for extensions in file of size " << fileData.size() << std::endl;

   for (size_t i = 0; i < fileData.size() - 1; ++i)
   {
      for (const auto& ext : extensions)
      {
         if (i + ext.length() <= fileData.size())
         {
            bool match = true;
            for (size_t j = 0; j < ext.length(); ++j)
            {
               if (fileData[i + j] != ext[j])
               {
                  match = false;
                  break;
               }
            }

            if (match)
            {
               std::cout << "DEBUG: Found extension match at position " << i << ": " << ext
                         << std::endl;
               // Try to extract the full filename
               size_t       start              = i;
               size_t       startIterations    = 0;
               const size_t maxStartIterations = 2048;  // Increased limit for heuristic extraction
               // Go backwards to find the start of the filename (stop at whitespace or null)
               while (start > 0 && fileData[start - 1] != '\0' && fileData[start - 1] != ' ' &&
                      fileData[start - 1] != '\t' && fileData[start - 1] != '\n' &&
                      fileData[start - 1] != '\r' && startIterations < maxStartIterations)
               {
                  --start;
                  startIterations++;
               }
               size_t       end              = i + ext.length();
               size_t       endIterations    = 0;
               const size_t maxEndIterations = 2048;  // Increased limit for heuristic extraction
               // Go forwards to find the end of the filename (before a slash, space, or null)
               while (end < fileData.size() && fileData[end] != '\0' && fileData[end] != '/' &&
                      fileData[end] != '\\' && fileData[end] != ' ' && fileData[end] != '\t' &&
                      fileData[end] != '\n' && fileData[end] != '\r' &&
                      endIterations < maxEndIterations)
               {
                  ++end;
                  endIterations++;
               }
               if (end - start > 0 && end - start < 512)
               {  // Increased reasonable filename length
                  std::string filename(fileData.begin() + start, fileData.begin() + end);
                  std::cout << "DEBUG: Extracted filename: '" << filename << "'" << std::endl;
                  if (filename.find('.') != std::string::npos)
                  {
                     pImpl->uniqueSourceFiles.insert(filename);
                     std::cout << "DEBUG: Added source file: " << filename << std::endl;
                  }
               }
            }
         }
      }
   }

   sourceFiles.assign(pImpl->uniqueSourceFiles.begin(), pImpl->uniqueSourceFiles.end());
#ifdef HEIMDALL_DEBUG_ENABLED
   std::cout << "LightweightDWARFParser: Heuristic extraction found " << sourceFiles.size()
             << " source files" << std::endl;
   for (const auto& file : sourceFiles)
   {
      std::cout << "LightweightDWARFParser: Source file: " << file << std::endl;
   }
#endif
   return !sourceFiles.empty();
}

bool LightweightDWARFParser::findDWARFSections(const std::string& filePath,
                                               uint64_t& debugInfoOffset, uint64_t& debugLineOffset,
                                               uint64_t& debugAbbrevOffset) const
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      return false;
   }

   // Check if file is empty
   file.seekg(0, std::ios::end);
   std::streampos fileSize = file.tellg();
   if (fileSize <= 0)
   {
      return false;
   }

   // Check if file is too small to be a valid ELF file
   if (fileSize < sizeof(ELFHeader))
   {
      return false;
   }

   // Read ELF header
   file.seekg(0, std::ios::beg);
   ELFHeader header{};
   file.read(reinterpret_cast<char*>(&header), sizeof(header));

   if (file.gcount() != sizeof(header))
   {
      return false;
   }

   // Check ELF magic
   if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || header.e_ident[2] != 'L' ||
       header.e_ident[3] != 'F')
   {
      return false;
   }

   // Validate section header table offset
   if (header.e_shoff == 0 || header.e_shoff >= fileSize)
   {
      return false;
   }

   // Validate section header string table index
   if (header.e_shstrndx >= header.e_shnum)
   {
      return false;
   }

   // Read section header string table
   file.seekg(header.e_shoff + header.e_shstrndx * sizeof(ELFSectionHeader));
   ELFSectionHeader stringTableHeader{};
   file.read(reinterpret_cast<char*>(&stringTableHeader), sizeof(stringTableHeader));

   if (file.gcount() != sizeof(stringTableHeader))
   {
      return false;
   }

   // Validate string table offset and size
   if (stringTableHeader.sh_offset == 0 || stringTableHeader.sh_offset >= fileSize ||
       stringTableHeader.sh_size == 0 ||
       stringTableHeader.sh_offset + stringTableHeader.sh_size > fileSize)
   {
      return false;
   }

   // Read string table
   file.seekg(stringTableHeader.sh_offset);
   std::vector<char> stringTable(stringTableHeader.sh_size);
   file.read(stringTable.data(), stringTableHeader.sh_size);

   if (file.gcount() != stringTableHeader.sh_size)
   {
      return false;
   }

   // Find DWARF sections
   debugInfoOffset   = 0;
   debugLineOffset   = 0;
   debugAbbrevOffset = 0;

   file.seekg(header.e_shoff);
   for (uint16_t i = 0; i < header.e_shnum; ++i)
   {
      ELFSectionHeader section{};
      file.read(reinterpret_cast<char*>(&section), sizeof(section));

      if (file.gcount() != sizeof(section))
      {
         break;
      }

      // Get section name
      std::string sectionName;
      if (section.sh_name < stringTable.size())
      {
         const char* name = &stringTable[section.sh_name];
         sectionName      = name;
      }

      // Check for DWARF sections
      if (sectionName == ".debug_info")
      {
         debugInfoOffset = section.sh_offset;
      }
      else if (sectionName == ".debug_line")
      {
         debugLineOffset = section.sh_offset;
      }
      else if (sectionName == ".debug_abbrev")
      {
         debugAbbrevOffset = section.sh_offset;
      }
   }

   return debugInfoOffset != 0 || debugLineOffset != 0 || debugAbbrevOffset != 0;
}

std::string LightweightDWARFParser::readDWARFString(const char* data, uint32_t offset)
{
   if (!data)
   {
      return "";
   }

   const char* str = data + offset;
   std::string result;

   // Add bounds checking to prevent infinite loops
   const uint32_t maxLength = 1024;  // Maximum reasonable string length
   uint32_t       length    = 0;

   while (*str != '\0' && length < maxLength)
   {
      result += *str;
      str++;
      length++;
   }

   // If we hit the max length without finding a null terminator, something is wrong
   if (length >= maxLength)
   {
      return "";
   }

   return result;
}

uint64_t LightweightDWARFParser::parseLEB128(const char* data, uint32_t& offset)
{
   uint64_t result         = 0;
   uint32_t shift          = 0;
   uint32_t originalOffset = offset;

   // Add bounds checking to prevent infinite loops
   const uint32_t maxBytes  = 10;  // LEB128 can be at most 10 bytes
   uint32_t       bytesRead = 0;

   while (bytesRead < maxBytes)
   {
      // Check if we've reached the end of the data
      if (offset >= 1024 * 1024)
      {                            // 1MB safety limit
         offset = originalOffset;  // Reset offset to prevent corruption
         return 0;
      }

      uint8_t byte = static_cast<uint8_t>(data[offset]);
      offset++;
      bytesRead++;

      result |= static_cast<uint64_t>(byte & 0x7f) << shift;

      if ((byte & 0x80) == 0)
      {
         break;
      }

      shift += 7;

      // Check for overflow
      if (shift >= 64)
      {
         offset = originalOffset;  // Reset offset to prevent corruption
         return 0;
      }
   }

   // If we read maxBytes without finding the end, something is wrong
   if (bytesRead >= maxBytes)
   {
      offset = originalOffset;  // Reset offset to prevent corruption
      return 0;
   }

   // Sign extend if the last byte had the sign bit set
   if (shift < 64 && (data[offset - 1] & 0x40))
   {
      result |= static_cast<uint64_t>(-1) << shift;
   }

   return result;
}

uint64_t LightweightDWARFParser::parseULEB128(const char* data, uint32_t& offset)
{
   uint64_t result         = 0;
   uint32_t shift          = 0;
   uint32_t originalOffset = offset;

   // Add bounds checking to prevent infinite loops
   const uint32_t maxBytes  = 10;  // ULEB128 can be at most 10 bytes
   uint32_t       bytesRead = 0;

   while (bytesRead < maxBytes)
   {
      // Check if we've reached the end of the data
      if (offset >= 1024 * 1024)
      {                            // 1MB safety limit
         offset = originalOffset;  // Reset offset to prevent corruption
         return 0;
      }

      uint8_t byte = static_cast<uint8_t>(data[offset]);
      offset++;
      bytesRead++;

      result |= static_cast<uint64_t>(byte & 0x7f) << shift;

      if ((byte & 0x80) == 0)
      {
         break;
      }

      shift += 7;

      // Check for overflow
      if (shift >= 64)
      {
         offset = originalOffset;  // Reset offset to prevent corruption
         return 0;
      }
   }

   // If we read maxBytes without finding the end, something is wrong
   if (bytesRead >= maxBytes)
   {
      offset = originalOffset;  // Reset offset to prevent corruption
      return 0;
   }

   return result;
}

}  // namespace heimdall