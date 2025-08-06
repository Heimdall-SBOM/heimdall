# Copyright 2025 The Heimdall Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Heimdall Compiler Plugin CMake Integration Functions

# Function to enable Heimdall compiler plugins for a target
function(enable_heimdall_compiler_plugin TARGET)
    cmake_parse_arguments(
        PLUGIN
        "VERBOSE;INCLUDE_SYSTEM_HEADERS"
        "OUTPUT_DIR;FORMAT;COMPILER"
        "EXCLUDE_PATTERNS"
        ${ARGN}
    )
    
    # Validate target exists
    if(NOT TARGET ${TARGET})
        message(FATAL_ERROR "Target ${TARGET} does not exist")
    endif()
    
    # Set defaults
    if(NOT PLUGIN_OUTPUT_DIR)
        set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/heimdall-metadata")
    endif()
    
    if(NOT PLUGIN_FORMAT)
        set(PLUGIN_FORMAT "json")
    endif()
    
    if(NOT PLUGIN_COMPILER)
        set(PLUGIN_COMPILER "auto")
    endif()
    
    # Create metadata output directory
    file(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})
    
    # Determine which compiler to use
    set(USE_GCC_PLUGIN FALSE)
    set(USE_CLANG_PLUGIN FALSE)
    
    if(PLUGIN_COMPILER STREQUAL "auto")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(USE_GCC_PLUGIN TRUE)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(USE_CLANG_PLUGIN TRUE)
        endif()
    elseif(PLUGIN_COMPILER STREQUAL "gcc")
        set(USE_GCC_PLUGIN TRUE)
    elseif(PLUGIN_COMPILER STREQUAL "clang")
        set(USE_CLANG_PLUGIN TRUE)
    endif()
    
    # Configure GCC plugin
    if(USE_GCC_PLUGIN)
        find_library(HEIMDALL_GCC_PLUGIN heimdall-gcc-plugin
                     PATHS ${CMAKE_INSTALL_PREFIX}/lib/heimdall/compiler
                     NO_DEFAULT_PATH)
        
        if(HEIMDALL_GCC_PLUGIN)
            message(STATUS "Enabling Heimdall GCC plugin for target ${TARGET}")
            
            target_compile_options(${TARGET} PRIVATE
                -fplugin=${HEIMDALL_GCC_PLUGIN}
                -fplugin-arg-heimdall-gcc-plugin-output-dir=${PLUGIN_OUTPUT_DIR}
                -fplugin-arg-heimdall-gcc-plugin-format=${PLUGIN_FORMAT}
            )
            
            if(PLUGIN_VERBOSE)
                target_compile_options(${TARGET} PRIVATE
                    -fplugin-arg-heimdall-gcc-plugin-verbose
                )
            endif()
            
            if(PLUGIN_INCLUDE_SYSTEM_HEADERS)
                target_compile_options(${TARGET} PRIVATE
                    -fplugin-arg-heimdall-gcc-plugin-include-system-headers
                )
            endif()
        else()
            message(WARNING "Heimdall GCC plugin not found, skipping plugin configuration")
        endif()
    endif()
    
    # Configure Clang plugin
    if(USE_CLANG_PLUGIN)
        find_library(HEIMDALL_CLANG_PLUGIN heimdall-clang-plugin
                     PATHS ${CMAKE_INSTALL_PREFIX}/lib/heimdall/compiler
                     NO_DEFAULT_PATH)
        
        if(HEIMDALL_CLANG_PLUGIN)
            message(STATUS "Enabling Heimdall Clang plugin for target ${TARGET}")
            
            target_compile_options(${TARGET} PRIVATE
                -Xclang -load
                -Xclang ${HEIMDALL_CLANG_PLUGIN}
                -Xclang -plugin
                -Xclang heimdall-sbom
                -Xclang -plugin-arg-heimdall-sbom
                -Xclang output-dir=${PLUGIN_OUTPUT_DIR}
                -Xclang -plugin-arg-heimdall-sbom
                -Xclang format=${PLUGIN_FORMAT}
            )
            
            if(PLUGIN_VERBOSE)
                target_compile_options(${TARGET} PRIVATE
                    -Xclang -plugin-arg-heimdall-sbom
                    -Xclang verbose
                )
            endif()
            
            if(PLUGIN_INCLUDE_SYSTEM_HEADERS)
                target_compile_options(${TARGET} PRIVATE
                    -Xclang -plugin-arg-heimdall-sbom
                    -Xclang include-system-headers
                )
            endif()
        else()
            message(WARNING "Heimdall Clang plugin not found, skipping plugin configuration")
        endif()
    endif()
    
    # Add custom target for enhanced SBOM generation
    add_custom_target(${TARGET}-enhanced-sbom
        COMMAND ${CMAKE_COMMAND} -E echo "Generating enhanced SBOM for ${TARGET}..."
        COMMAND heimdall-sbom 
                $<TARGET_FILE:${TARGET}> 
                --metadata-dir ${PLUGIN_OUTPUT_DIR}
                --format ${PLUGIN_FORMAT}
                --output ${CMAKE_BINARY_DIR}/${TARGET}.${PLUGIN_FORMAT}
                $<IF:$<BOOL:${PLUGIN_VERBOSE}>,--verbose,>
        DEPENDS ${TARGET}
        COMMENT "Generating enhanced SBOM for ${TARGET}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # Set target properties for metadata tracking
    set_target_properties(${TARGET} PROPERTIES
        HEIMDALL_METADATA_DIR ${PLUGIN_OUTPUT_DIR}
        HEIMDALL_ENHANCED_SBOM TRUE
    )
    
    message(STATUS "Heimdall compiler plugin configured for ${TARGET}")
    message(STATUS "  Metadata directory: ${PLUGIN_OUTPUT_DIR}")
    message(STATUS "  Format: ${PLUGIN_FORMAT}")
    message(STATUS "  Verbose: ${PLUGIN_VERBOSE}")
    message(STATUS "  Include system headers: ${PLUGIN_INCLUDE_SYSTEM_HEADERS}")
endfunction()

# Function to add enhanced SBOM generation to all targets in a directory
function(enable_heimdall_for_directory)
    cmake_parse_arguments(
        PLUGIN
        "VERBOSE;INCLUDE_SYSTEM_HEADERS"
        "OUTPUT_DIR;FORMAT;COMPILER"
        "EXCLUDE_TARGETS"
        ${ARGN}
    )
    
    # Get all targets in current directory
    get_property(TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    
    foreach(TARGET ${TARGETS})
        # Skip excluded targets
        list(FIND PLUGIN_EXCLUDE_TARGETS ${TARGET} EXCLUDE_INDEX)
        if(EXCLUDE_INDEX EQUAL -1)
            # Check if target is executable or library
            get_target_property(TARGET_TYPE ${TARGET} TYPE)
            if(TARGET_TYPE STREQUAL "EXECUTABLE" OR 
               TARGET_TYPE STREQUAL "SHARED_LIBRARY" OR 
               TARGET_TYPE STREQUAL "STATIC_LIBRARY")
               
                enable_heimdall_compiler_plugin(${TARGET}
                    OUTPUT_DIR ${PLUGIN_OUTPUT_DIR}
                    FORMAT ${PLUGIN_FORMAT}
                    COMPILER ${PLUGIN_COMPILER}
                    $<IF:$<BOOL:${PLUGIN_VERBOSE}>,VERBOSE,>
                    $<IF:$<BOOL:${PLUGIN_INCLUDE_SYSTEM_HEADERS}>,INCLUDE_SYSTEM_HEADERS,>
                )
            endif()
        endif()
    endforeach()
endfunction()

# Function to create a unified SBOM from multiple targets
function(create_unified_sbom OUTPUT_NAME)
    cmake_parse_arguments(
        UNIFIED
        "VERBOSE"
        "FORMAT;OUTPUT_DIR"
        "TARGETS;METADATA_DIRS"
        ${ARGN}
    )
    
    if(NOT UNIFIED_FORMAT)
        set(UNIFIED_FORMAT "cyclonedx")
    endif()
    
    if(NOT UNIFIED_OUTPUT_DIR)
        set(UNIFIED_OUTPUT_DIR ${CMAKE_BINARY_DIR})
    endif()
    
    # Create custom target for unified SBOM
    add_custom_target(${OUTPUT_NAME}
        COMMAND ${CMAKE_COMMAND} -E echo "Creating unified SBOM: ${OUTPUT_NAME}"
        COMMENT "Creating unified SBOM from multiple targets"
    )
    
    # Add dependencies on individual target SBOMs
    foreach(TARGET ${UNIFIED_TARGETS})
        if(TARGET ${TARGET}-enhanced-sbom)
            add_dependencies(${OUTPUT_NAME} ${TARGET}-enhanced-sbom)
        endif()
    endforeach()
    
    message(STATUS "Unified SBOM target created: ${OUTPUT_NAME}")
endfunction()

# Function to check if Heimdall compiler plugins are available
function(check_heimdall_compiler_support)
    set(HEIMDALL_GCC_AVAILABLE FALSE PARENT_SCOPE)
    set(HEIMDALL_CLANG_AVAILABLE FALSE PARENT_SCOPE)
    
    # Check for GCC plugin
    find_library(HEIMDALL_GCC_PLUGIN heimdall-gcc-plugin
                 PATHS ${CMAKE_INSTALL_PREFIX}/lib/heimdall/compiler)
    if(HEIMDALL_GCC_PLUGIN)
        set(HEIMDALL_GCC_AVAILABLE TRUE PARENT_SCOPE)
    endif()
    
    # Check for Clang plugin
    find_library(HEIMDALL_CLANG_PLUGIN heimdall-clang-plugin
                 PATHS ${CMAKE_INSTALL_PREFIX}/lib/heimdall/compiler)
    if(HEIMDALL_CLANG_PLUGIN)
        set(HEIMDALL_CLANG_AVAILABLE TRUE PARENT_SCOPE)
    endif()
endfunction()

# Print availability information
message(STATUS "Heimdall compiler plugin integration functions loaded")
check_heimdall_compiler_support()
if(HEIMDALL_GCC_AVAILABLE)
    message(STATUS "GCC plugin support: Available")
else()
    message(STATUS "GCC plugin support: Not available")
endif()
if(HEIMDALL_CLANG_AVAILABLE)
    message(STATUS "Clang plugin support: Available")
else()
    message(STATUS "Clang plugin support: Not available")
endif()