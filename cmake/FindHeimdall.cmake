# FindHeimdall.cmake
# Locates Heimdall installation and plugins
#
# This module defines the following variables:
#   HEIMDALL_FOUND - True if Heimdall was found
#   HEIMDALL_LLD_PLUGIN - Path to LLD plugin library
#   HEIMDALL_GOLD_PLUGIN - Path to Gold plugin library  
#   HEIMDALL_SBOM_TOOL - Path to heimdall-sbom tool
#   HEIMDALL_LLD_AVAILABLE - True if LLD plugin is available
#   HEIMDALL_GOLD_AVAILABLE - True if Gold plugin is available
#   HEIMDALL_VERSION - Version of Heimdall found
#
# This module defines the following targets:
#   Heimdall::LLD - LLD plugin target
#   Heimdall::Gold - Gold plugin target
#   Heimdall::SBOMTool - SBOM generation tool target

include(FindPackageHandleStandardArgs)

# Search paths for Heimdall installation
set(HEIMDALL_SEARCH_PATHS
    ${HEIMDALL_ROOT}
    $ENV{HEIMDALL_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}/build
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp23
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp20
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp17
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp14
    ${CMAKE_CURRENT_SOURCE_DIR}/build-cpp11
    /usr/local/lib/heimdall
    /usr/local/heimdall
    /opt/heimdall
    /usr/lib/heimdall
    /usr/heimdall
)

# Find LLD plugin
find_library(HEIMDALL_LLD_PLUGIN
    NAMES heimdall-lld
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH
)

# Find Gold plugin
find_library(HEIMDALL_GOLD_PLUGIN
    NAMES heimdall-gold
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH
)

# Find heimdall-sbom tool
find_program(HEIMDALL_SBOM_TOOL
    NAMES heimdall-sbom
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES bin src/tools
    NO_DEFAULT_PATH
)

# Find heimdall-validate tool
find_program(HEIMDALL_VALIDATE_TOOL
    NAMES heimdall-validate
    PATHS ${HEIMDALL_SEARCH_PATHS}
    PATH_SUFFIXES bin src/tools
    NO_DEFAULT_PATH
)

# Check availability of linkers
find_program(LLD_LINKER ld.lld)
find_program(GOLD_LINKER ld.gold)

# Set availability flags
set(HEIMDALL_LLD_AVAILABLE FALSE)
set(HEIMDALL_GOLD_AVAILABLE FALSE)

if(HEIMDALL_LLD_PLUGIN AND LLD_LINKER)
    set(HEIMDALL_LLD_AVAILABLE TRUE)
endif()

if(HEIMDALL_GOLD_PLUGIN AND GOLD_LINKER AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(HEIMDALL_GOLD_AVAILABLE TRUE)
endif()

# Try to extract version information
if(HEIMDALL_SBOM_TOOL)
    execute_process(
        COMMAND ${HEIMDALL_SBOM_TOOL} --version
        OUTPUT_VARIABLE HEIMDALL_VERSION_OUTPUT
        ERROR_VARIABLE HEIMDALL_VERSION_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    
    if(HEIMDALL_VERSION_OUTPUT)
        string(REGEX MATCH "version ([0-9]+\\.[0-9]+\\.[0-9]+)" HEIMDALL_VERSION_MATCH ${HEIMDALL_VERSION_OUTPUT})
        if(HEIMDALL_VERSION_MATCH)
            set(HEIMDALL_VERSION ${CMAKE_MATCH_1})
        endif()
    endif()
endif()

# Set default version if not found
if(NOT HEIMDALL_VERSION)
    set(HEIMDALL_VERSION "1.0.0")
endif()

# Validate that at least the SBOM tool is found
find_package_handle_standard_args(Heimdall
    REQUIRED_VARS HEIMDALL_SBOM_TOOL
    VERSION_VAR HEIMDALL_VERSION
    FOUND_VAR HEIMDALL_FOUND
)

# Create imported targets if found
if(HEIMDALL_FOUND)
    # Create LLD plugin target
    if(HEIMDALL_LLD_PLUGIN)
        add_library(Heimdall::LLD UNKNOWN IMPORTED)
        set_target_properties(Heimdall::LLD PROPERTIES
            IMPORTED_LOCATION ${HEIMDALL_LLD_PLUGIN}
        )
    endif()
    
    # Create Gold plugin target
    if(HEIMDALL_GOLD_PLUGIN)
        add_library(Heimdall::Gold UNKNOWN IMPORTED)
        set_target_properties(Heimdall::Gold PROPERTIES
            IMPORTED_LOCATION ${HEIMDALL_GOLD_PLUGIN}
        )
    endif()
    
    # Create SBOM tool target
    if(HEIMDALL_SBOM_TOOL)
        add_executable(Heimdall::SBOMTool IMPORTED)
        set_target_properties(Heimdall::SBOMTool PROPERTIES
            IMPORTED_LOCATION ${HEIMDALL_SBOM_TOOL}
        )
    endif()
    
    # Create validate tool target
    if(HEIMDALL_VALIDATE_TOOL)
        add_executable(Heimdall::ValidateTool IMPORTED)
        set_target_properties(Heimdall::ValidateTool PROPERTIES
            IMPORTED_LOCATION ${HEIMDALL_VALIDATE_TOOL}
        )
    endif()
endif()

# Print status information
if(HEIMDALL_FOUND)
    message(STATUS "Found Heimdall ${HEIMDALL_VERSION}")
    message(STATUS "  SBOM Tool: ${HEIMDALL_SBOM_TOOL}")
    
    if(HEIMDALL_LLD_AVAILABLE)
        message(STATUS "  LLD Plugin: ${HEIMDALL_LLD_PLUGIN}")
    else()
        message(STATUS "  LLD Plugin: Not available")
    endif()
    
    if(HEIMDALL_GOLD_AVAILABLE)
        message(STATUS "  Gold Plugin: ${HEIMDALL_GOLD_PLUGIN}")
    else()
        message(STATUS "  Gold Plugin: Not available")
    endif()
    
    if(HEIMDALL_VALIDATE_TOOL)
        message(STATUS "  Validate Tool: ${HEIMDALL_VALIDATE_TOOL}")
    endif()
else()
    message(STATUS "Heimdall not found")
    message(STATUS "  Searched in: ${HEIMDALL_SEARCH_PATHS}")
    message(STATUS "  Set HEIMDALL_ROOT to specify custom installation path")
endif()

# Mark variables as advanced
mark_as_advanced(
    HEIMDALL_LLD_PLUGIN
    HEIMDALL_GOLD_PLUGIN
    HEIMDALL_SBOM_TOOL
    HEIMDALL_VALIDATE_TOOL
    HEIMDALL_LLD_AVAILABLE
    HEIMDALL_GOLD_AVAILABLE
    HEIMDALL_VERSION
) 