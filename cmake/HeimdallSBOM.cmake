# HeimdallSBOM.cmake
# Main SBOM generation functions for Heimdall
#
# This module provides the main interface for enabling SBOM generation
# on CMake targets using Heimdall's wrapper and plugin approaches.

include(CMakeParseArguments)

# Only call find_package(Heimdall REQUIRED) once per project
if(NOT TARGET Heimdall::SBOMTool)
    find_package(Heimdall REQUIRED)
endif()

# Main function to enable SBOM generation for a target
function(heimdall_enable_sbom target)
    cmake_parse_arguments(HEIMDALL_SBOM
        "VERBOSE;INCLUDE_SYSTEM_LIBS;VALIDATE"
        "FORMAT;OUTPUT;LINKER;NAME;VERSION;SUPPLIER;LICENSE;DESCRIPTION;PURL"
        "PROPERTIES"
        ${ARGN}
    )
    
    # Validate target exists
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} does not exist")
    endif()
    
    # Set target properties from arguments
    if(HEIMDALL_SBOM_FORMAT)
        heimdall_validate_format(${HEIMDALL_SBOM_FORMAT})
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_FORMAT ${HEIMDALL_SBOM_FORMAT})
    endif()
    
    if(HEIMDALL_SBOM_OUTPUT)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_OUTPUT ${HEIMDALL_SBOM_OUTPUT})
    endif()
    
    if(HEIMDALL_SBOM_VERBOSE)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_VERBOSE ON)
    endif()
    
    if(HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_INCLUDE_SYSTEM_LIBS ON)
    endif()
    
    if(HEIMDALL_SBOM_NAME)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_NAME ${HEIMDALL_SBOM_NAME})
    endif()
    
    if(HEIMDALL_SBOM_VERSION)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_VERSION ${HEIMDALL_SBOM_VERSION})
    endif()
    
    if(HEIMDALL_SBOM_SUPPLIER)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_SUPPLIER ${HEIMDALL_SBOM_SUPPLIER})
    endif()
    
    if(HEIMDALL_SBOM_LICENSE)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_LICENSE ${HEIMDALL_SBOM_LICENSE})
    endif()
    
    if(HEIMDALL_SBOM_DESCRIPTION)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_DESCRIPTION ${HEIMDALL_SBOM_DESCRIPTION})
    endif()
    
    if(HEIMDALL_SBOM_PURL)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_PURL ${HEIMDALL_SBOM_PURL})
    endif()
    
    if(HEIMDALL_SBOM_PROPERTIES)
        set_target_properties(${target} PROPERTIES HEIMDALL_SBOM_PROPERTIES "${HEIMDALL_SBOM_PROPERTIES}")
    endif()
    
    # Set default properties
    heimdall_set_default_target_properties(${target})
    
    # Detect available linkers
    heimdall_detect_linkers()
    
    # Configure SBOM generation based on available linkers
    heimdall_configure_sbom_generation(${target})
    
    # Create output directory
    heimdall_create_sbom_output_dir()
    
    message(STATUS "Enabled SBOM generation for target: ${target}")
endfunction()

# Function to detect available linkers
function(heimdall_detect_linkers)
    # Check for LLD
    find_program(LLD_LINKER ld.lld)
    if(LLD_LINKER)
        set(HEIMDALL_LLD_AVAILABLE TRUE PARENT_SCOPE)
        message(STATUS "Found LLD linker: ${LLD_LINKER}")
    else()
        set(HEIMDALL_LLD_AVAILABLE FALSE PARENT_SCOPE)
        message(STATUS "LLD linker not found")
    endif()
    
    # Check for Gold (Linux only)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_program(GOLD_LINKER ld.gold)
        if(GOLD_LINKER)
            set(HEIMDALL_GOLD_AVAILABLE TRUE PARENT_SCOPE)
            message(STATUS "Found Gold linker: ${GOLD_LINKER}")
        else()
            set(HEIMDALL_GOLD_AVAILABLE FALSE PARENT_SCOPE)
            message(STATUS "Gold linker not found")
        endif()
    else()
        set(HEIMDALL_GOLD_AVAILABLE FALSE PARENT_SCOPE)
        message(STATUS "Gold linker not available on ${CMAKE_SYSTEM_NAME}")
    endif()
endfunction()

# Function to configure SBOM generation for a target
function(heimdall_configure_sbom_generation target)
    set(format "")
    set(output "")
    set(verbose "")
    set(include_system_libs "")
    set(name "")
    # Get target SBOM properties
    heimdall_get_target_sbom_properties(${target} format output verbose include_system_libs name)
    
    # Determine which linker to use
    set(linker_to_use "auto")
    if(HEIMDALL_PREFERRED_LINKER AND NOT HEIMDALL_PREFERRED_LINKER STREQUAL "auto")
        set(linker_to_use ${HEIMDALL_PREFERRED_LINKER})
    elseif(HEIMDALL_GOLD_AVAILABLE)
        set(linker_to_use "gold")
    elseif(HEIMDALL_LLD_AVAILABLE)
        set(linker_to_use "lld")
    endif()
    
    message(STATUS "Configuring SBOM generation for ${target} with ${linker_to_use} linker")
    
    # Configure based on selected linker
    if(linker_to_use STREQUAL "gold" AND HEIMDALL_GOLD_AVAILABLE)
        heimdall_configure_gold_plugin(${target} ${format} ${output} ${verbose} ${include_system_libs} ${name})
    elseif(linker_to_use STREQUAL "lld" AND HEIMDALL_LLD_AVAILABLE)
        heimdall_configure_lld_wrapper(${target} ${format} ${output} ${verbose} ${include_system_libs} ${name})
    elseif(linker_to_use STREQUAL "auto")
        # Try Gold first, then LLD
        if(HEIMDALL_GOLD_AVAILABLE)
            heimdall_configure_gold_plugin(${target} ${format} ${output} ${verbose} ${include_system_libs} ${name})
        elseif(HEIMDALL_LLD_AVAILABLE)
            heimdall_configure_lld_wrapper(${target} ${format} ${output} ${verbose} ${include_system_libs} ${name})
        else()
            message(WARNING "No suitable linker found for SBOM generation on target ${target}")
        endif()
    else()
        message(WARNING "Requested linker ${linker_to_use} not available for target ${target}")
    endif()
endfunction()

# Function to configure Gold plugin approach
function(heimdall_configure_gold_plugin target format output verbose include_system_libs name)
    if(NOT HEIMDALL_GOLD_PLUGIN)
        message(WARNING "Gold plugin not found, skipping SBOM generation for target ${target}.")
        return()
    endif()
    
    message(STATUS "Configuring Gold plugin for ${target}")
    
    # Set linker to Gold
    set_target_properties(${target} PROPERTIES LINKER ld.gold)
    
    # Add plugin options
    target_link_options(${target} PRIVATE
        "LINKER:--plugin=${HEIMDALL_GOLD_PLUGIN}"
        "LINKER:--plugin-opt=sbom-output=${output}"
        "LINKER:--plugin-opt=format=${format}"
    )
    
    # Add verbose option if requested
    if(verbose)
        target_link_options(${target} PRIVATE "LINKER:--plugin-opt=verbose")
    endif()
    
    # Add system libs option if requested
    if(include_system_libs)
        target_link_options(${target} PRIVATE "LINKER:--plugin-opt=include-system-libs")
    endif()
    
    # Add fallback wrapper if enabled
    if(HEIMDALL_FALLBACK_TO_WRAPPER)
        heimdall_add_gold_fallback_wrapper(${target} ${format} ${output} ${verbose} ${include_system_libs} ${name})
    endif()
endfunction()

# Function to configure LLD wrapper approach
function(heimdall_configure_lld_wrapper target format output verbose include_system_libs name)
    if(NOT HEIMDALL_SBOM_TOOL)
        message(WARNING "heimdall-sbom tool not found, skipping SBOM generation for target ${target}.")
        return()
    endif()
    if(NOT HEIMDALL_LLD_PLUGIN)
        message(WARNING "LLD plugin not found, skipping SBOM generation for target ${target}.")
        return()
    endif()
    
    message(STATUS "Configuring LLD wrapper for ${target}")
    
    # Set linker to LLD
    set_target_properties(${target} PROPERTIES LINKER ld.lld)
    
    # Build command for SBOM generation
    set(sbom_command ${HEIMDALL_SBOM_TOOL} ${HEIMDALL_LLD_PLUGIN} $<TARGET_FILE:${target}>)
    
    # Add format option
    list(APPEND sbom_command --format ${format})
    
    # Add output option
    list(APPEND sbom_command --output ${output})
    
    # Add verbose option if requested
    if(verbose)
        list(APPEND sbom_command --verbose)
    endif()
    
    # Add system libs option if requested
    if(include_system_libs)
        list(APPEND sbom_command --include-system-libs)
    endif()
    
    # Add name option if specified
    if(name)
        list(APPEND sbom_command --name ${name})
    endif()
    
    # Add post-build command
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${sbom_command}
        COMMENT "Generating SBOM for ${target} using LLD wrapper"
        VERBATIM
    )
    
    # Add validation if enabled
    if(HEIMDALL_VALIDATE_SBOM AND HEIMDALL_VALIDATE_TOOL)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${HEIMDALL_VALIDATE_TOOL} --validate ${output}
            COMMENT "Validating SBOM for ${target}"
            VERBATIM
        )
    endif()
endfunction()

# Function to add Gold fallback wrapper
function(heimdall_add_gold_fallback_wrapper target format output verbose include_system_libs name)
    if(NOT HEIMDALL_SBOM_TOOL)
        return()
    endif()
    
    # Create fallback output name
    string(REPLACE ".spdx" "_fallback.spdx" fallback_output ${output})
    string(REPLACE ".cyclonedx.json" "_fallback.cyclonedx.json" fallback_output ${fallback_output})
    
    # Build fallback command
    set(fallback_command ${HEIMDALL_SBOM_TOOL} ${HEIMDALL_GOLD_PLUGIN} $<TARGET_FILE:${target}>)
    list(APPEND fallback_command --format ${format})
    list(APPEND fallback_command --output ${fallback_output})
    
    if(verbose)
        list(APPEND fallback_command --verbose)
    endif()
    
    if(include_system_libs)
        list(APPEND fallback_command --include-system-libs)
    endif()
    
    if(name)
        list(APPEND fallback_command --name ${name})
    endif()
    
    # Add fallback command
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Plugin may have failed, generating fallback SBOM"
        COMMAND ${fallback_command}
        COMMENT "Generating fallback SBOM for ${target}"
        VERBATIM
    )
endfunction()

# Function to generate SBOM for existing binary
function(heimdall_generate_sbom target binary_path)
    cmake_parse_arguments(HEIMDALL_GEN
        "VERBOSE;INCLUDE_SYSTEM_LIBS"
        "FORMAT;OUTPUT;PLUGIN;NAME"
        ""
        ${ARGN}
    )
    
    # Find Heimdall
    find_package(Heimdall REQUIRED)
    
    # Validate binary exists
    if(NOT EXISTS ${binary_path})
        message(FATAL_ERROR "Binary ${binary_path} does not exist")
    endif()
    
    # Set defaults
    if(NOT HEIMDALL_GEN_FORMAT)
        set(HEIMDALL_GEN_FORMAT "spdx")
    endif()
    
    if(NOT HEIMDALL_GEN_OUTPUT)
        heimdall_get_format_version(${HEIMDALL_GEN_FORMAT} format_type format_version)
        if(format_type STREQUAL "spdx")
            set(HEIMDALL_GEN_OUTPUT "${HEIMDALL_SBOM_OUTPUT_DIR}/${target}.spdx")
        else()
            set(HEIMDALL_GEN_OUTPUT "${HEIMDALL_SBOM_OUTPUT_DIR}/${target}.cyclonedx.json")
        endif()
    endif()
    
    if(NOT HEIMDALL_GEN_PLUGIN)
        # Try to auto-detect plugin
        if(HEIMDALL_LLD_PLUGIN)
            set(HEIMDALL_GEN_PLUGIN ${HEIMDALL_LLD_PLUGIN})
        elseif(HEIMDALL_GOLD_PLUGIN)
            set(HEIMDALL_GEN_PLUGIN ${HEIMDALL_GOLD_PLUGIN})
        else()
            message(FATAL_ERROR "No Heimdall plugin found")
        endif()
    endif()
    
    # Build command
    set(sbom_command ${HEIMDALL_SBOM_TOOL} ${HEIMDALL_GEN_PLUGIN} ${binary_path})
    list(APPEND sbom_command --format ${HEIMDALL_GEN_FORMAT})
    list(APPEND sbom_command --output ${HEIMDALL_GEN_OUTPUT})
    
    if(HEIMDALL_GEN_VERBOSE)
        list(APPEND sbom_command --verbose)
    endif()
    
    if(HEIMDALL_GEN_INCLUDE_SYSTEM_LIBS)
        list(APPEND sbom_command --include-system-libs)
    endif()
    
    if(HEIMDALL_GEN_NAME)
        list(APPEND sbom_command --name ${HEIMDALL_GEN_NAME})
    endif()
    
    # Create output directory
    heimdall_create_sbom_output_dir()
    
    # Add custom target for SBOM generation
    add_custom_target(${target}-sbom
        COMMAND ${sbom_command}
        COMMENT "Generating SBOM for ${binary_path}"
        VERBATIM
    )
    
    message(STATUS "Created SBOM generation target: ${target}-sbom")
endfunction()

# Function to validate SBOM
function(heimdall_validate_sbom sbom_file)
    if(NOT HEIMDALL_VALIDATE_TOOL)
        message(WARNING "heimdall-validate tool not found, skipping validation")
        return()
    endif()
    
    if(NOT EXISTS ${sbom_file})
        message(FATAL_ERROR "SBOM file ${sbom_file} does not exist")
    endif()
    
    add_custom_target(validate-${sbom_file}
        COMMAND ${HEIMDALL_VALIDATE_TOOL} --validate ${sbom_file}
        COMMENT "Validating SBOM: ${sbom_file}"
        VERBATIM
    )
    
    message(STATUS "Created SBOM validation target: validate-${sbom_file}")
endfunction()

# Function to compare SBOMs
function(heimdall_compare_sboms old_sbom new_sbom output_file)
    if(NOT HEIMDALL_VALIDATE_TOOL)
        message(WARNING "heimdall-validate tool not found, skipping comparison")
        return()
    endif()
    
    if(NOT EXISTS ${old_sbom})
        message(FATAL_ERROR "Old SBOM file ${old_sbom} does not exist")
    endif()
    
    if(NOT EXISTS ${new_sbom})
        message(FATAL_ERROR "New SBOM file ${new_sbom} does not exist")
    endif()
    
    add_custom_target(compare-${old_sbom}-${new_sbom}
        COMMAND ${HEIMDALL_VALIDATE_TOOL} --compare ${old_sbom} ${new_sbom} --output ${output_file}
        COMMENT "Comparing SBOMs: ${old_sbom} vs ${new_sbom}"
        VERBATIM
    )
    
    message(STATUS "Created SBOM comparison target: compare-${old_sbom}-${new_sbom}")
endfunction() 