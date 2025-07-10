# HeimdallConfig.cmake
# Configuration options and defaults for Heimdall SBOM generation
#
# This module defines global configuration options and target-specific properties
# for customizing SBOM generation behavior.

# Global configuration options
option(HEIMDALL_ENABLE_SBOM "Enable SBOM generation globally" ON)
option(HEIMDALL_VERBOSE "Enable verbose SBOM output" OFF)
option(HEIMDALL_INCLUDE_SYSTEM_LIBS "Include system libraries in SBOM" OFF)
option(HEIMDALL_FALLBACK_TO_WRAPPER "Fallback to wrapper approach if plugin fails" ON)
option(HEIMDALL_VALIDATE_SBOM "Validate generated SBOMs" OFF)

# Default values for SBOM generation
set(HEIMDALL_SBOM_FORMAT "spdx" CACHE STRING "Default SBOM format")
set(HEIMDALL_SBOM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/sbom" CACHE PATH "SBOM output directory")
set(HEIMDALL_PREFERRED_LINKER "auto" CACHE STRING "Preferred linker (lld, gold, auto)")
set(HEIMDALL_SBOM_NAME_TEMPLATE "${TARGET_NAME}" CACHE STRING "Template for SBOM component names")

# Plugin search paths
set(HEIMDALL_PLUGIN_SEARCH_PATHS "" CACHE STRING "Additional plugin search paths")

# SBOM format validation
set_property(CACHE HEIMDALL_SBOM_FORMAT PROPERTY STRINGS 
    "spdx" "spdx-2.3" "spdx-3.0" "spdx-3.0.0" "spdx-3.0.1"
    "cyclonedx" "cyclonedx-1.4" "cyclonedx-1.5" "cyclonedx-1.6"
)

# Linker preference validation
set_property(CACHE HEIMDALL_PREFERRED_LINKER PROPERTY STRINGS 
    "auto" "lld" "gold"
)

# Target-specific properties
define_property(TARGET PROPERTY HEIMDALL_SBOM_FORMAT
    BRIEF_DOCS "SBOM format for this target"
    FULL_DOCS "Specifies the SBOM format to generate for this target"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_OUTPUT
    BRIEF_DOCS "SBOM output file for this target"
    FULL_DOCS "Specifies the output file path for SBOM generation"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_VERBOSE
    BRIEF_DOCS "Enable verbose SBOM generation for this target"
    FULL_DOCS "Enables verbose output during SBOM generation"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS
    BRIEF_DOCS "Include system libraries in SBOM for this target"
    FULL_DOCS "Includes system libraries in the generated SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_NAME
    BRIEF_DOCS "Component name for this target in SBOM"
    FULL_DOCS "Specifies the component name to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_VERSION
    BRIEF_DOCS "Component version for this target in SBOM"
    FULL_DOCS "Specifies the component version to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_SUPPLIER
    BRIEF_DOCS "Component supplier for this target in SBOM"
    FULL_DOCS "Specifies the component supplier to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_LICENSE
    BRIEF_DOCS "Component license for this target in SBOM"
    FULL_DOCS "Specifies the component license to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_DESCRIPTION
    BRIEF_DOCS "Component description for this target in SBOM"
    FULL_DOCS "Specifies the component description to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_PURL
    BRIEF_DOCS "Component PURL for this target in SBOM"
    FULL_DOCS "Specifies the component PURL to use in the SBOM"
)

define_property(TARGET PROPERTY HEIMDALL_SBOM_PROPERTIES
    BRIEF_DOCS "Additional properties for this target in SBOM"
    FULL_DOCS "Specifies additional properties to include in the SBOM"
)

# Function to set default target properties
function(heimdall_set_default_target_properties target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} does not exist")
    endif()
    
    # Set default format if not already set
    get_target_property(format ${target} HEIMDALL_SBOM_FORMAT)
    if(NOT format)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_FORMAT ${HEIMDALL_SBOM_FORMAT})
    endif()
    
    # Set default verbose setting if not already set
    get_target_property(verbose ${target} HEIMDALL_SBOM_VERBOSE)
    if(NOT verbose)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_VERBOSE ${HEIMDALL_VERBOSE})
    endif()
    
    # Set default system libs setting if not already set
    get_target_property(include_system ${target} HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS)
    if(NOT include_system)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS ${HEIMDALL_INCLUDE_SYSTEM_LIBS})
    endif()
    
    # Set default name if not already set
    get_target_property(name ${target} HEIMDALL_SBOM_NAME)
    if(NOT name)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_NAME ${target})
    endif()
endfunction()

# Function to validate SBOM format
function(heimdall_validate_format format)
    set(valid_formats
        "spdx" "spdx-2.3" "spdx-3.0" "spdx-3.0.0" "spdx-3.0.1"
        "cyclonedx" "cyclonedx-1.4" "cyclonedx-1.5" "cyclonedx-1.6"
    )
    
    list(FIND valid_formats ${format} format_index)
    if(format_index EQUAL -1)
        message(FATAL_ERROR "Invalid SBOM format: ${format}. Valid formats are: ${valid_formats}")
    endif()
endfunction()

# Function to get SBOM format version
function(heimdall_get_format_version format format_type version)
    if(format MATCHES "^spdx")
        set(${format_type} "spdx" PARENT_SCOPE)
        if(format STREQUAL "spdx")
            set(${version} "2.3" PARENT_SCOPE)
        elseif(format MATCHES "spdx-([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
            set(${version} ${CMAKE_MATCH_1} PARENT_SCOPE)
        else()
            set(${version} "2.3" PARENT_SCOPE)
        endif()
    elseif(format MATCHES "^cyclonedx")
        set(${format_type} "cyclonedx" PARENT_SCOPE)
        if(format STREQUAL "cyclonedx")
            set(${version} "1.6" PARENT_SCOPE)
        elseif(format MATCHES "cyclonedx-([0-9]+\\.[0-9]+)")
            set(${version} ${CMAKE_MATCH_1} PARENT_SCOPE)
        else()
            set(${version} "1.6" PARENT_SCOPE)
        endif()
    else()
        message(FATAL_ERROR "Unknown SBOM format: ${format}")
    endif()
endfunction()

# Function to get target SBOM properties
function(heimdall_get_target_sbom_properties target format output verbose include_system_libs name)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} does not exist")
    endif()
    
    # Get format
    get_target_property(_format ${target} HEIMDALL_SBOM_FORMAT)
    if(_format)
        set(_heimdall_format_value ${_format})
    else()
        set(_heimdall_format_value ${HEIMDALL_SBOM_FORMAT})
    endif()
    set(${format} ${_heimdall_format_value} PARENT_SCOPE)
    
    # Get output
    get_target_property(_output ${target} HEIMDALL_SBOM_OUTPUT)
    if(_output)
        set(_heimdall_output_value ${_output})
    else()
        # Generate default output path
        heimdall_get_format_version(${_heimdall_format_value} _format_type _format_version)
        if(_format_type STREQUAL "spdx")
            set(_heimdall_output_value "${HEIMDALL_SBOM_OUTPUT_DIR}/${target}.spdx")
        else()
            set(_heimdall_output_value "${HEIMDALL_SBOM_OUTPUT_DIR}/${target}.cyclonedx.json")
        endif()
    endif()
    set(${output} ${_heimdall_output_value} PARENT_SCOPE)
    
    # Get verbose setting
    get_target_property(_verbose ${target} HEIMDALL_SBOM_VERBOSE)
    if(_verbose)
        set(_heimdall_verbose_value ${_verbose})
    else()
        set(_heimdall_verbose_value ${HEIMDALL_VERBOSE})
    endif()
    set(${verbose} ${_heimdall_verbose_value} PARENT_SCOPE)
    
    # Get system libs setting
    get_target_property(_include_system ${target} HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS)
    if(_include_system)
        set(_heimdall_include_system_value ${_include_system})
    else()
        set(_heimdall_include_system_value ${HEIMDALL_INCLUDE_SYSTEM_LIBS})
    endif()
    set(${include_system_libs} ${_heimdall_include_system_value} PARENT_SCOPE)
    
    # Get name
    get_target_property(_name ${target} HEIMDALL_SBOM_NAME)
    if(_name)
        set(_heimdall_name_value ${_name})
    else()
        set(_heimdall_name_value ${target})
    endif()
    set(${name} ${_heimdall_name_value} PARENT_SCOPE)
endfunction()

# Function to create SBOM output directory
function(heimdall_create_sbom_output_dir)
    if(NOT EXISTS ${HEIMDALL_SBOM_OUTPUT_DIR})
        file(MAKE_DIRECTORY ${HEIMDALL_SBOM_OUTPUT_DIR})
        message(STATUS "Created SBOM output directory: ${HEIMDALL_SBOM_OUTPUT_DIR}")
    endif()
endfunction()

# Print configuration summary
if(HEIMDALL_ENABLE_SBOM)
    message(STATUS "Heimdall SBOM Configuration:")
    message(STATUS "  Default Format: ${HEIMDALL_SBOM_FORMAT}")
    message(STATUS "  Output Directory: ${HEIMDALL_SBOM_OUTPUT_DIR}")
    message(STATUS "  Preferred Linker: ${HEIMDALL_PREFERRED_LINKER}")
    message(STATUS "  Verbose Output: ${HEIMDALL_VERBOSE}")
    message(STATUS "  Include System Libraries: ${HEIMDALL_INCLUDE_SYSTEM_LIBS}")
    message(STATUS "  Fallback to Wrapper: ${HEIMDALL_FALLBACK_TO_WRAPPER}")
    message(STATUS "  Validate SBOMs: ${HEIMDALL_VALIDATE_SBOM}")
endif() 