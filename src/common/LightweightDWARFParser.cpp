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
 * @brief Implementation of lightweight DWARF parser for C++14 compatibility
 * @author Trevor Bakker
 * @date 2025
 *
 * This file implements a lightweight DWARF parser that extracts debug information
 * without using LLVM's problematic template features. It's designed for C++14
 * compatibility and provides the same interface as the LLVM-based DWARFExtractor.
 */

#include "LightweightDWARFParser.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#ifdef __linux__
#include <elf.h>
#endif

namespace heimdall {

// DWARF constants
constexpr uint32_t DW_TAG_compile_unit = 0x11;
constexpr uint32_t DW_TAG_subprogram = 0x2e;
constexpr uint32_t DW_TAG_namespace = 0x39;
constexpr uint32_t DW_TAG_class_type = 0x02;
constexpr uint32_t DW_TAG_structure_type = 0x13;
constexpr uint32_t DW_TAG_enumeration_type = 0x04;
constexpr uint32_t DW_TAG_enumerator = 0x0b;
constexpr uint32_t DW_TAG_variable = 0x34;
constexpr uint32_t DW_TAG_formal_parameter = 0x05;
constexpr uint32_t DW_TAG_unspecified_parameters = 0x06;
constexpr uint32_t DW_TAG_unspecified_type = 0x3b;
constexpr uint32_t DW_TAG_pointer_type = 0x0f;
constexpr uint32_t DW_TAG_reference_type = 0x10;
constexpr uint32_t DW_TAG_const_type = 0x26;
constexpr uint32_t DW_TAG_volatile_type = 0x12;
constexpr uint32_t DW_TAG_typedef = 0x16;
constexpr uint32_t DW_TAG_array_type = 0x01;
constexpr uint32_t DW_TAG_subrange_type = 0x21;
constexpr uint32_t DW_TAG_union_type = 0x17;
constexpr uint32_t DW_TAG_inheritance = 0x28;
constexpr uint32_t DW_TAG_member = 0x0d;
constexpr uint32_t DW_TAG_subroutine_type = 0x21;
constexpr uint32_t DW_TAG_inlined_subroutine = 0x1d;
constexpr uint32_t DW_TAG_lexical_block = 0x0b;
constexpr uint32_t DW_TAG_try_block = 0x2b;
constexpr uint32_t DW_TAG_catch_block = 0x03;
constexpr uint32_t DW_TAG_label = 0x0a;
constexpr uint32_t DW_TAG_imported_declaration = 0x08;
constexpr uint32_t DW_TAG_imported_module = 0x3a;
constexpr uint32_t DW_TAG_imported_unit = 0x3d;
constexpr uint32_t DW_TAG_condition = 0x3f;
constexpr uint32_t DW_TAG_shared_type = 0x40;
constexpr uint32_t DW_TAG_type_unit = 0x41;
constexpr uint32_t DW_TAG_rvalue_reference_type = 0x42;
constexpr uint32_t DW_TAG_template_alias = 0x43;
constexpr uint32_t DW_TAG_coarray_type = 0x44;
constexpr uint32_t DW_TAG_dynamic_type = 0x45;
constexpr uint32_t DW_TAG_generic_subrange = 0x46;
constexpr uint32_t DW_TAG_atomic_type = 0x47;
constexpr uint32_t DW_TAG_call_site = 0x48;
constexpr uint32_t DW_TAG_call_site_parameter = 0x49;
constexpr uint32_t DW_TAG_skeleton_unit = 0x4a;
constexpr uint32_t DW_TAG_immutable_type = 0x4b;

// DWARF attribute constants
constexpr uint32_t DW_AT_sibling = 0x01;
constexpr uint32_t DW_AT_location = 0x02;
constexpr uint32_t DW_AT_name = 0x03;
constexpr uint32_t DW_AT_ordering = 0x09;
constexpr uint32_t DW_AT_byte_size = 0x0b;
constexpr uint32_t DW_AT_bit_offset = 0x0c;
constexpr uint32_t DW_AT_bit_size = 0x0d;
constexpr uint32_t DW_AT_stmt_list = 0x10;
constexpr uint32_t DW_AT_low_pc = 0x11;
constexpr uint32_t DW_AT_high_pc = 0x12;
constexpr uint32_t DW_AT_language = 0x13;
constexpr uint32_t DW_AT_discr = 0x15;
constexpr uint32_t DW_AT_discr_value = 0x16;
constexpr uint32_t DW_AT_visibility = 0x17;
constexpr uint32_t DW_AT_import = 0x18;
constexpr uint32_t DW_AT_string_length = 0x19;
constexpr uint32_t DW_AT_common_reference = 0x1a;
constexpr uint32_t DW_AT_comp_dir = 0x1b;
constexpr uint32_t DW_AT_const_value = 0x1c;
constexpr uint32_t DW_AT_containing_type = 0x1d;
constexpr uint32_t DW_AT_default_value = 0x1e;
constexpr uint32_t DW_AT_inline = 0x20;
constexpr uint32_t DW_AT_is_optional = 0x21;
constexpr uint32_t DW_AT_lower_bound = 0x22;
constexpr uint32_t DW_AT_producer = 0x25;
constexpr uint32_t DW_AT_prototyped = 0x27;
constexpr uint32_t DW_AT_return_addr = 0x2a;
constexpr uint32_t DW_AT_start_scope = 0x2c;
constexpr uint32_t DW_AT_bit_stride = 0x2e;
constexpr uint32_t DW_AT_upper_bound = 0x2f;
constexpr uint32_t DW_AT_abstract_origin = 0x31;
constexpr uint32_t DW_AT_accessibility = 0x32;
constexpr uint32_t DW_AT_address_class = 0x33;
constexpr uint32_t DW_AT_artificial = 0x34;
constexpr uint32_t DW_AT_base_types = 0x35;
constexpr uint32_t DW_AT_calling_convention = 0x36;
constexpr uint32_t DW_AT_count = 0x37;
constexpr uint32_t DW_AT_data_member_location = 0x38;
constexpr uint32_t DW_AT_decl_column = 0x39;
constexpr uint32_t DW_AT_decl_file = 0x3a;
constexpr uint32_t DW_AT_decl_line = 0x3b;
constexpr uint32_t DW_AT_declaration = 0x3c;
constexpr uint32_t DW_AT_discr_list = 0x3d;
constexpr uint32_t DW_AT_encoding = 0x3e;
constexpr uint32_t DW_AT_external = 0x3f;
constexpr uint32_t DW_AT_frame_base = 0x40;
constexpr uint32_t DW_AT_friend = 0x41;
constexpr uint32_t DW_AT_identifier_case = 0x42;
constexpr uint32_t DW_AT_macro_info = 0x43;
constexpr uint32_t DW_AT_namelist_item = 0x44;
constexpr uint32_t DW_AT_priority = 0x45;
constexpr uint32_t DW_AT_segment = 0x46;
constexpr uint32_t DW_AT_specification = 0x47;
constexpr uint32_t DW_AT_static_link = 0x48;
constexpr uint32_t DW_AT_type = 0x49;
constexpr uint32_t DW_AT_use_location = 0x4a;
constexpr uint32_t DW_AT_variable_parameter = 0x4b;
constexpr uint32_t DW_AT_virtuality = 0x4c;
constexpr uint32_t DW_AT_vtable_elem_location = 0x4d;
constexpr uint32_t DW_AT_allocated = 0x4e;
constexpr uint32_t DW_AT_associated = 0x4f;
constexpr uint32_t DW_AT_data_location = 0x50;
constexpr uint32_t DW_AT_byte_stride = 0x51;
constexpr uint32_t DW_AT_entry_pc = 0x52;
constexpr uint32_t DW_AT_use_UTF8 = 0x53;
constexpr uint32_t DW_AT_extension = 0x54;
constexpr uint32_t DW_AT_ranges = 0x55;
constexpr uint32_t DW_AT_trampoline = 0x56;
constexpr uint32_t DW_AT_call_column = 0x57;
constexpr uint32_t DW_AT_call_file = 0x58;
constexpr uint32_t DW_AT_call_line = 0x59;
constexpr uint32_t DW_AT_description = 0x5a;
constexpr uint32_t DW_AT_binary_scale = 0x5b;
constexpr uint32_t DW_AT_decimal_scale = 0x5c;
constexpr uint32_t DW_AT_small = 0x5d;
constexpr uint32_t DW_AT_decimal_sign = 0x5e;
constexpr uint32_t DW_AT_digit_count = 0x5f;
constexpr uint32_t DW_AT_picture_string = 0x60;
constexpr uint32_t DW_AT_mutable = 0x61;
constexpr uint32_t DW_AT_threads_scaled = 0x62;
constexpr uint32_t DW_AT_explicit = 0x63;
constexpr uint32_t DW_AT_object_pointer = 0x64;
constexpr uint32_t DW_AT_endianity = 0x65;
constexpr uint32_t DW_AT_elemental = 0x66;
constexpr uint32_t DW_AT_pure = 0x67;
constexpr uint32_t DW_AT_recursive = 0x68;
constexpr uint32_t DW_AT_signature = 0x69;
constexpr uint32_t DW_AT_main_subprogram = 0x6a;
constexpr uint32_t DW_AT_data_bit_offset = 0x6b;
constexpr uint32_t DW_AT_const_expr = 0x6c;
constexpr uint32_t DW_AT_enum_class = 0x6d;
constexpr uint32_t DW_AT_linkage_name = 0x6e;

// DWARF form constants
constexpr uint32_t DW_FORM_addr = 0x01;
constexpr uint32_t DW_FORM_block2 = 0x03;
constexpr uint32_t DW_FORM_block4 = 0x04;
constexpr uint32_t DW_FORM_data2 = 0x05;
constexpr uint32_t DW_FORM_data4 = 0x06;
constexpr uint32_t DW_FORM_data8 = 0x07;
constexpr uint32_t DW_FORM_string = 0x08;
constexpr uint32_t DW_FORM_block = 0x09;
constexpr uint32_t DW_FORM_block1 = 0x0a;
constexpr uint32_t DW_FORM_data1 = 0x0b;
constexpr uint32_t DW_FORM_flag = 0x0c;
constexpr uint32_t DW_FORM_sdata = 0x0d;
constexpr uint32_t DW_FORM_strp = 0x0e;
constexpr uint32_t DW_FORM_udata = 0x0f;
constexpr uint32_t DW_FORM_ref_addr = 0x10;
constexpr uint32_t DW_FORM_ref1 = 0x11;
constexpr uint32_t DW_FORM_ref2 = 0x12;
constexpr uint32_t DW_FORM_ref4 = 0x13;
constexpr uint32_t DW_FORM_ref8 = 0x14;
constexpr uint32_t DW_FORM_ref_udata = 0x15;
constexpr uint32_t DW_FORM_indirect = 0x16;
constexpr uint32_t DW_FORM_sec_offset = 0x17;
constexpr uint32_t DW_FORM_exprloc = 0x18;
constexpr uint32_t DW_FORM_flag_present = 0x19;
constexpr uint32_t DW_FORM_strx = 0x1a;
constexpr uint32_t DW_FORM_addrx = 0x1b;
constexpr uint32_t DW_FORM_ref_sup4 = 0x1c;
constexpr uint32_t DW_FORM_strp_sup = 0x1d;
constexpr uint32_t DW_FORM_data16 = 0x1e;
constexpr uint32_t DW_FORM_line_strp = 0x1f;
constexpr uint32_t DW_FORM_ref_sig8 = 0x20;
constexpr uint32_t DW_FORM_implicit_const = 0x21;
constexpr uint32_t DW_FORM_loclistx = 0x22;
constexpr uint32_t DW_FORM_rnglistx = 0x23;
constexpr uint32_t DW_FORM_ref_sup8 = 0x24;
constexpr uint32_t DW_FORM_strx1 = 0x25;
constexpr uint32_t DW_FORM_strx2 = 0x26;
constexpr uint32_t DW_FORM_strx3 = 0x27;
constexpr uint32_t DW_FORM_strx4 = 0x28;
constexpr uint32_t DW_FORM_addrx1 = 0x29;
constexpr uint32_t DW_FORM_addrx2 = 0x2a;
constexpr uint32_t DW_FORM_addrx3 = 0x2b;
constexpr uint32_t DW_FORM_addrx4 = 0x2c;

LightweightDWARFParser::LightweightDWARFParser() {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser constructor called");
#endif
}

LightweightDWARFParser::~LightweightDWARFParser() {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser destructor called");
#endif
}

bool LightweightDWARFParser::extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractSourceFiles called for " + filePath);
#endif

    // Try DWARF debug line section first
    if (parseDWARFDebugLine(filePath, sourceFiles)) {
        return true;
    }

    // Fallback to heuristic extraction
    return extractSourceFilesHeuristic(filePath, sourceFiles);
}

bool LightweightDWARFParser::extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractCompileUnits called for " + filePath);
#endif

    // Try DWARF debug info section
    std::vector<std::string> dummySourceFiles, dummyFunctions;
    if (parseDWARFDebugInfo(filePath, dummySourceFiles, compileUnits, dummyFunctions)) {
        return true;
    }

    // Fallback: return empty (no compile units found)
    return false;
}

bool LightweightDWARFParser::extractFunctions(const std::string& filePath, std::vector<std::string>& functions) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractFunctions called for " + filePath);
#endif

    // Try DWARF debug info section first
    std::vector<std::string> dummySourceFiles, dummyCompileUnits;
    if (parseDWARFDebugInfo(filePath, dummySourceFiles, dummyCompileUnits, functions)) {
        return true;
    }

    // Fallback to symbol table extraction
    return extractFunctionsFromSymbolTable(filePath, functions);
}

bool LightweightDWARFParser::extractAllDebugInfo(const std::string& filePath,
                                                 std::vector<std::string>& sourceFiles,
                                                 std::vector<std::string>& compileUnits,
                                                 std::vector<std::string>& functions) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractAllDebugInfo called for " + filePath);
#endif

    // Try DWARF debug info section first
    if (parseDWARFDebugInfo(filePath, sourceFiles, compileUnits, functions)) {
        return true;
    }

    // Fallback: try individual extractions
    bool success = false;
    
    if (extractSourceFiles(filePath, sourceFiles)) {
        success = true;
    }
    
    if (extractCompileUnits(filePath, compileUnits)) {
        success = true;
    }
    
    if (extractFunctions(filePath, functions)) {
        success = true;
    }

    return success;
}

bool LightweightDWARFParser::hasDWARFInfo(const std::string& filePath) {
    uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
    return findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset);
}

bool LightweightDWARFParser::parseDWARFDebugInfo(const std::string& filePath,
                                                  std::vector<std::string>& sourceFiles,
                                                  std::vector<std::string>& compileUnits,
                                                  std::vector<std::string>& functions) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: parseDWARFDebugInfo called for " + filePath);
#endif

    uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
    if (!findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset)) {
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read debug info section
    file.seekg(debugInfoOffset);
    std::vector<char> debugInfoData;
    debugInfoData.resize(1024 * 1024); // 1MB buffer
    file.read(debugInfoData.data(), debugInfoData.size());
    size_t bytesRead = file.gcount();
    debugInfoData.resize(bytesRead);

    // Parse DWARF debug info
    uint32_t offset = 0;
    std::set<std::string> uniqueSourceFiles;
    std::set<std::string> uniqueCompileUnits;
    std::set<std::string> uniqueFunctions;

    while (offset < debugInfoData.size()) {
        // Read compile unit header
        if (offset + 11 > debugInfoData.size()) {
            break;
        }

        uint32_t unitLength = *reinterpret_cast<uint32_t*>(&debugInfoData[offset]);
        offset += 4;
        
        if (unitLength == 0xffffffff) {
            // 64-bit length
            if (offset + 8 > debugInfoData.size()) {
                break;
            }
            unitLength = *reinterpret_cast<uint64_t*>(&debugInfoData[offset]);
            offset += 8;
        }

        uint16_t version = *reinterpret_cast<uint16_t*>(&debugInfoData[offset]);
        offset += 2;

        uint32_t debugAbbrevOffset = *reinterpret_cast<uint32_t*>(&debugInfoData[offset]);
        offset += 4;

        uint8_t addressSize = debugInfoData[offset];
        offset += 1;

        // Parse DIEs (Debugging Information Entries)
        while (offset < debugInfoData.size()) {
            uint32_t abbrevCode = parseULEB128(debugInfoData.data(), offset);
            
            if (abbrevCode == 0) {
                // End of DIEs
                break;
            }

            // Parse attributes based on abbrev code
            // This is a simplified parser - in a full implementation,
            // we would parse the abbreviation table to know which attributes
            // are present for each DIE

            // For now, we'll extract what we can from common patterns
            if (abbrevCode == DW_TAG_compile_unit) {
                // Extract compile unit name
                std::string name = readDWARFString(debugInfoData.data(), offset);
                if (!name.empty()) {
                    uniqueCompileUnits.insert(name);
                }
            } else if (abbrevCode == DW_TAG_subprogram) {
                // Extract function name
                std::string name = readDWARFString(debugInfoData.data(), offset);
                if (!name.empty()) {
                    uniqueFunctions.insert(name);
                }
            }

            // Skip remaining attributes for this DIE
            // In a full implementation, we would parse the abbreviation table
            // to know exactly which attributes to expect
            offset += 4; // Simplified skip
        }
    }

    // Convert sets to vectors
    sourceFiles.assign(uniqueSourceFiles.begin(), uniqueSourceFiles.end());
    compileUnits.assign(uniqueCompileUnits.begin(), uniqueCompileUnits.end());
    functions.assign(uniqueFunctions.begin(), uniqueFunctions.end());

    return !sourceFiles.empty() || !compileUnits.empty() || !functions.empty();
}

bool LightweightDWARFParser::parseDWARFDebugLine(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: parseDWARFDebugLine called for " + filePath);
#endif

    uint64_t debugInfoOffset, debugLineOffset, debugAbbrevOffset;
    if (!findDWARFSections(filePath, debugInfoOffset, debugLineOffset, debugAbbrevOffset)) {
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read debug line section
    file.seekg(debugLineOffset);
    std::vector<char> debugLineData;
    debugLineData.resize(1024 * 1024); // 1MB buffer
    file.read(debugLineData.data(), debugLineData.size());
    size_t bytesRead = file.gcount();
    debugLineData.resize(bytesRead);

    // Parse DWARF debug line section
    std::set<std::string> uniqueSourceFiles;
    uint32_t offset = 0;

    while (offset < debugLineData.size()) {
        // Read line program header
        if (offset + 8 > debugLineData.size()) {
            break;
        }

        uint32_t unitLength = *reinterpret_cast<uint32_t*>(&debugLineData[offset]);
        offset += 4;

        uint16_t version = *reinterpret_cast<uint16_t*>(&debugLineData[offset]);
        offset += 2;

        uint32_t headerLength = *reinterpret_cast<uint32_t*>(&debugLineData[offset]);
        offset += 4;

        uint8_t minimumInstructionLength = debugLineData[offset];
        offset += 1;

        uint8_t defaultIsStmt = debugLineData[offset];
        offset += 1;

        int8_t lineBase = debugLineData[offset];
        offset += 1;

        uint8_t lineRange = debugLineData[offset];
        offset += 1;

        uint8_t opcodeBase = debugLineData[offset];
        offset += 1;

        // Read standard opcode lengths
        for (uint8_t i = 1; i < opcodeBase; ++i) {
            if (offset >= debugLineData.size()) {
                break;
            }
            uint8_t length = debugLineData[offset];
            offset += 1;
        }

        // Read include directories
        while (offset < debugLineData.size() && debugLineData[offset] != 0) {
            std::string dir;
            while (offset < debugLineData.size() && debugLineData[offset] != 0) {
                dir += debugLineData[offset];
                offset += 1;
            }
            if (offset < debugLineData.size()) {
                offset += 1; // Skip null terminator
            }
        }
        if (offset < debugLineData.size()) {
            offset += 1; // Skip final null terminator
        }

        // Read file names
        while (offset < debugLineData.size() && debugLineData[offset] != 0) {
            std::string fileName;
            while (offset < debugLineData.size() && debugLineData[offset] != 0) {
                fileName += debugLineData[offset];
                offset += 1;
            }
            if (offset < debugLineData.size()) {
                offset += 1; // Skip null terminator
            }

            // Skip directory index and modification time
            offset += 4;

            if (!fileName.empty()) {
                uniqueSourceFiles.insert(fileName);
            }
        }
        if (offset < debugLineData.size()) {
            offset += 1; // Skip final null terminator
        }

        // Skip line program instructions
        // In a full implementation, we would parse these to get line numbers
        offset += 4; // Simplified skip
    }

    sourceFiles.assign(uniqueSourceFiles.begin(), uniqueSourceFiles.end());
    return !sourceFiles.empty();
}

bool LightweightDWARFParser::parseDWARFDebugInfo(const std::string& filePath,
                                                  std::vector<std::string>& compileUnits,
                                                  std::vector<std::string>& functions) {
    std::vector<std::string> dummySourceFiles;
    return parseDWARFDebugInfo(filePath, dummySourceFiles, compileUnits, functions);
}

bool LightweightDWARFParser::extractFunctionsFromSymbolTable(const std::string& filePath, std::vector<std::string>& functions) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractFunctionsFromSymbolTable called for " + filePath);
#endif

    // This is a simplified implementation that extracts function names from the symbol table
    // In a full implementation, we would parse the ELF symbol table to find function symbols

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read ELF header
    ELFHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (file.gcount() != sizeof(header)) {
        return false;
    }

    // Check ELF magic
    if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || 
        header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
        return false;
    }

    // Find symbol table section
    file.seekg(header.e_shoff);
    for (uint16_t i = 0; i < header.e_shnum; ++i) {
        ELFSectionHeader section;
        file.read(reinterpret_cast<char*>(&section), sizeof(section));
        
        if (file.gcount() != sizeof(section)) {
            break;
        }

        // Check if this is a symbol table section
        if (section.sh_type == 2) { // SHT_SYMTAB
            // Read symbol table
            file.seekg(section.sh_offset);
            std::vector<char> symbolTable(section.sh_size);
            file.read(symbolTable.data(), section.sh_size);
            
            if (file.gcount() == section.sh_size) {
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

bool LightweightDWARFParser::extractSourceFilesHeuristic(const std::string& filePath, std::vector<std::string>& sourceFiles) {
#ifdef HEIMDALL_DEBUG_ENABLED
    Utils::debugPrint("LightweightDWARFParser: extractSourceFilesHeuristic called for " + filePath);
#endif

    // This is a heuristic approach that looks for common patterns in the binary
    // that might indicate source file information

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read a portion of the file to look for source file patterns
    std::vector<char> buffer(1024 * 1024); // 1MB buffer
    file.read(buffer.data(), buffer.size());
    size_t bytesRead = file.gcount();
    buffer.resize(bytesRead);

    // Look for common source file extensions
    std::vector<std::string> extensions = {".c", ".cpp", ".cc", ".cxx", ".h", ".hpp", ".hxx"};
    std::set<std::string> foundFiles;

    std::string data(buffer.begin(), buffer.end());
    
    for (const auto& ext : extensions) {
        size_t pos = 0;
        while ((pos = data.find(ext, pos)) != std::string::npos) {
            // Try to extract a filename around this position
            size_t start = pos;
            while (start > 0 && (isalnum(data[start - 1]) || data[start - 1] == '_' || data[start - 1] == '/')) {
                start--;
            }
            
            size_t end = pos + ext.length();
            while (end < data.length() && (isalnum(data[end]) || data[end] == '_' || data[end] == '.')) {
                end++;
            }
            
            std::string filename = data.substr(start, end - start);
            if (filename.length() > 3 && filename.find('/') != std::string::npos) {
                foundFiles.insert(filename);
            }
            
            pos = end;
        }
    }

    sourceFiles.assign(foundFiles.begin(), foundFiles.end());
    return !sourceFiles.empty();
}

bool LightweightDWARFParser::findDWARFSections(const std::string& filePath,
                                               uint64_t& debugInfoOffset,
                                               uint64_t& debugLineOffset,
                                               uint64_t& debugAbbrevOffset) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read ELF header
    ELFHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (file.gcount() != sizeof(header)) {
        return false;
    }

    // Check ELF magic
    if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || 
        header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
        return false;
    }

    // Read section header string table
    file.seekg(header.e_shoff + header.e_shstrndx * sizeof(ELFSectionHeader));
    ELFSectionHeader stringTableHeader;
    file.read(reinterpret_cast<char*>(&stringTableHeader), sizeof(stringTableHeader));
    
    if (file.gcount() != sizeof(stringTableHeader)) {
        return false;
    }

    // Read string table
    file.seekg(stringTableHeader.sh_offset);
    std::vector<char> stringTable(stringTableHeader.sh_size);
    file.read(stringTable.data(), stringTableHeader.sh_size);
    
    if (file.gcount() != stringTableHeader.sh_size) {
        return false;
    }

    // Find DWARF sections
    debugInfoOffset = 0;
    debugLineOffset = 0;
    debugAbbrevOffset = 0;

    file.seekg(header.e_shoff);
    for (uint16_t i = 0; i < header.e_shnum; ++i) {
        ELFSectionHeader section;
        file.read(reinterpret_cast<char*>(&section), sizeof(section));
        
        if (file.gcount() != sizeof(section)) {
            break;
        }

        // Get section name
        std::string sectionName;
        if (section.sh_name < stringTable.size()) {
            const char* name = &stringTable[section.sh_name];
            sectionName = name;
        }

        // Check for DWARF sections
        if (sectionName == ".debug_info") {
            debugInfoOffset = section.sh_offset;
        } else if (sectionName == ".debug_line") {
            debugLineOffset = section.sh_offset;
        } else if (sectionName == ".debug_abbrev") {
            debugAbbrevOffset = section.sh_offset;
        }
    }

    return debugInfoOffset != 0 || debugLineOffset != 0 || debugAbbrevOffset != 0;
}

std::string LightweightDWARFParser::readDWARFString(const char* data, uint32_t offset) {
    if (!data) {
        return "";
    }

    const char* str = data + offset;
    std::string result;
    
    while (*str != '\0') {
        result += *str;
        str++;
    }
    
    return result;
}

uint64_t LightweightDWARFParser::parseLEB128(const char* data, uint32_t& offset) {
    uint64_t result = 0;
    uint32_t shift = 0;
    
    while (true) {
        uint8_t byte = static_cast<uint8_t>(data[offset]);
        offset++;
        
        result |= static_cast<uint64_t>(byte & 0x7f) << shift;
        
        if ((byte & 0x80) == 0) {
            break;
        }
        
        shift += 7;
    }
    
    // Sign extend if the last byte had the sign bit set
    if (shift < 64 && (data[offset - 1] & 0x40)) {
        result |= static_cast<uint64_t>(-1) << shift;
    }
    
    return result;
}

uint64_t LightweightDWARFParser::parseULEB128(const char* data, uint32_t& offset) {
    uint64_t result = 0;
    uint32_t shift = 0;
    
    while (true) {
        uint8_t byte = static_cast<uint8_t>(data[offset]);
        offset++;
        
        result |= static_cast<uint64_t>(byte & 0x7f) << shift;
        
        if ((byte & 0x80) == 0) {
            break;
        }
        
        shift += 7;
    }
    
    return result;
}

}  // namespace heimdall 