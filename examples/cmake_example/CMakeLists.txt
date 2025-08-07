# Heimdall Compiler Plugin CMake Example
# Demonstrates compiler plugin integration for SBOM generation

cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Project Configuration
# =============================================================================

project(NetworkSimulator 
    VERSION 1.0.0
    DESCRIPTION "Heimdall Compiler Plugin CMake Example"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable debug symbols and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_BUILD_TYPE Debug)

# =============================================================================
# Heimdall Configuration
# =============================================================================

# Heimdall installation path (can be overridden with -DHEIMDALL_ROOT=...)
# Use HEIMDALL_BUILD_DIR if provided, otherwise default to build-gcc-cpp17 for backwards compatibility
if(DEFINED HEIMDALL_BUILD_DIR)
    set(HEIMDALL_ROOT "${HEIMDALL_BUILD_DIR}" CACHE PATH "Path to Heimdall installation")
else()
    set(HEIMDALL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../build-gcc-cpp17" CACHE PATH "Path to Heimdall installation")
endif()

# Verify Heimdall installation
if(NOT EXISTS "${HEIMDALL_ROOT}")
    message(FATAL_ERROR "Heimdall not found at ${HEIMDALL_ROOT}. Please set HEIMDALL_ROOT correctly.")
endif()

# Project configuration
set(TARGET_NAME "network_simulator")
set(METADATA_DIR "${CMAKE_BINARY_DIR}/metadata")
set(SBOM_DIR "${CMAKE_BINARY_DIR}/sbom")

# Create required directories
file(MAKE_DIRECTORY ${METADATA_DIR})
file(MAKE_DIRECTORY ${SBOM_DIR})

# =============================================================================
# Compiler Plugin Detection and Configuration
# =============================================================================

# Detect compiler type for demonstration
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_TYPE "gcc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_TYPE "clang")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. Supported: GNU (GCC), Clang")
endif()

# Demonstration flags (since compiler plugins need additional setup)
set(PLUGIN_FLAGS -g -O2 -DHEIMDALL_DEMO=1)

# Display configuration
message(STATUS "=== Heimdall Configuration ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} (${COMPILER_TYPE})")
message(STATUS "Metadata Directory: ${METADATA_DIR}")
message(STATUS "SBOM Directory: ${SBOM_DIR}")

# =============================================================================
# Source Files and Target Definition
# =============================================================================

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Define the executable
add_executable(${TARGET_NAME} ${SOURCES})

# Set include directories
target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add compiler plugin flags to the target
target_compile_options(${TARGET_NAME} PRIVATE ${PLUGIN_FLAGS})

# Add threading support (required for network simulation)
find_package(Threads REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)

# =============================================================================
# SBOM Generation
# =============================================================================

# Linker plugin and SBOM generator paths
set(LINKER_PLUGIN "${HEIMDALL_ROOT}/lib/heimdall-lld.so")
set(SBOM_GENERATOR "${HEIMDALL_ROOT}/src/tools/heimdall-sbom")

# Custom target for SBOM generation
add_custom_target(sbom
    COMMAND ${CMAKE_COMMAND} -E echo "=== Generating Enhanced SBOM ==="
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SBOM_DIR}
    COMMAND ${SBOM_GENERATOR} ${LINKER_PLUGIN} $<TARGET_FILE:${TARGET_NAME}>
            --format cyclonedx 
            --output ${SBOM_DIR}/${TARGET_NAME}.cdx.json
            --verbose
    COMMAND ${CMAKE_COMMAND} -E echo "Enhanced SBOM generated: ${SBOM_DIR}/${TARGET_NAME}.cdx.json"
    DEPENDS ${TARGET_NAME}
    COMMENT "Generating enhanced SBOM with compiler metadata"
)

# Automatic SBOM generation after build (optional - uncomment to enable)
# add_dependencies(${TARGET_NAME} sbom)

# =============================================================================
# Custom Targets and Helpers
# =============================================================================

# Clean metadata and SBOM files
add_custom_target(clean-metadata
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${METADATA_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${SBOM_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${METADATA_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SBOM_DIR}
    COMMENT "Cleaning metadata and SBOM directories"
)

# Display build information
add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "=== Build Information ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Project: ${PROJECT_NAME} ${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Target: ${TARGET_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Plugin Type: ${COMPILER_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build Type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "Metadata Dir: ${METADATA_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "SBOM Dir: ${SBOM_DIR}"
    COMMENT "Displaying build information"
)

# =============================================================================
# Installation (Optional)
# =============================================================================

# Install the executable
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Install generated SBOM (if it exists)
install(FILES "${SBOM_DIR}/${TARGET_NAME}.cdx.json"
    DESTINATION share/sbom
    COMPONENT Documentation
    OPTIONAL
)

# =============================================================================
# Summary
# =============================================================================

message(STATUS "=== Build Targets ===")
message(STATUS "make                - Build ${TARGET_NAME} with metadata collection")
message(STATUS "make sbom          - Generate enhanced SBOM")
message(STATUS "make clean-metadata - Clean metadata and SBOM files")
message(STATUS "make info          - Display build information")
message(STATUS "make install       - Install the application")
message(STATUS "")
message(STATUS "After building, run: ./${TARGET_NAME}")
message(STATUS "Check metadata: ls ${METADATA_DIR}/")
message(STATUS "Check SBOM: cat ${SBOM_DIR}/${TARGET_NAME}.cdx.json")