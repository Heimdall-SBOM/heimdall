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

# GCC Plugin build configuration for Heimdall

# Find GCC executable
find_program(GCC_EXECUTABLE gcc)
if(NOT GCC_EXECUTABLE)
    message(WARNING "GCC not found, skipping GCC plugin build")
    return()
endif()

message(STATUS "Found GCC: ${GCC_EXECUTABLE}")

# Get GCC version
execute_process(
    COMMAND ${GCC_EXECUTABLE} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "GCC version: ${GCC_VERSION}")

# Get GCC plugin directory
execute_process(
    COMMAND ${GCC_EXECUTABLE} -print-file-name=plugin
    OUTPUT_VARIABLE GCC_PLUGIN_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "GCC plugin directory: ${GCC_PLUGIN_DIR}")

# Check if GCC plugin headers exist
if(NOT EXISTS "${GCC_PLUGIN_DIR}/include")
    message(WARNING "GCC plugin headers not found at ${GCC_PLUGIN_DIR}/include")
    message(WARNING "Please install GCC plugin development headers (gcc-plugin-dev)")
    message(WARNING "Skipping GCC plugin build")
    return()
endif()

# Get GCC plugin include directories
set(GCC_PLUGIN_INCLUDES
    ${GCC_PLUGIN_DIR}/include
    ${GCC_PLUGIN_DIR}/include/c-family
)

# Check for required headers
set(REQUIRED_HEADERS
    "gcc-plugin.h"
    "plugin-version.h"
    "tree.h"
    "diagnostic.h"
)

foreach(HEADER ${REQUIRED_HEADERS})
    if(NOT EXISTS "${GCC_PLUGIN_DIR}/include/${HEADER}")
        message(WARNING "Required GCC header ${HEADER} not found")
        message(WARNING "Skipping GCC plugin build")
        return()
    endif()
endforeach()

# Find required libraries
find_package(OpenSSL REQUIRED)

# Build GCC plugin
add_library(heimdall-gcc-plugin SHARED
    GCCPluginWrapper.cpp
    ../common/CompilerMetadata.cpp
    ../../detectors/LicenseDetector.cpp
    ../../common/Utils.cpp
    ../../utils/FileUtils.cpp
)

# Set target properties
set_target_properties(heimdall-gcc-plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(heimdall-gcc-plugin PRIVATE
    ${GCC_PLUGIN_INCLUDES}
    ../common
    ../../common
    ../../detectors
    ../../utils
    ../../compat
    ${CMAKE_SOURCE_DIR}/external/json/include
)

# Link libraries
target_link_libraries(heimdall-gcc-plugin PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Compiler definitions
target_compile_definitions(heimdall-gcc-plugin PRIVATE
    BUILDING_GCC_PLUGIN=1
    _GNU_SOURCE=1
)

# Compiler flags specific to GCC plugins - build in isolation
target_compile_options(heimdall-gcc-plugin PRIVATE
    -fPIC
    -fno-rtti
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

# Add dependency on heimdall core components
if(TARGET heimdall-core)
    target_link_libraries(heimdall-gcc-plugin PRIVATE heimdall-core)
endif()

# Installation
install(TARGETS heimdall-gcc-plugin
    LIBRARY DESTINATION lib/heimdall/compiler
    COMPONENT gcc-plugin
)

# Create a GCC plugin wrapper script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/heimdall-gcc-wrapper.sh.in
    ${CMAKE_BINARY_DIR}/bin/heimdall-gcc
    @ONLY
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/heimdall-gcc
    DESTINATION bin
    COMPONENT gcc-plugin
)

# Print build information
message(STATUS "GCC plugin will be built as: heimdall-gcc-plugin.so")
message(STATUS "GCC plugin includes: ${GCC_PLUGIN_INCLUDES}")

# Add custom target for easy building
add_custom_target(gcc-plugin DEPENDS heimdall-gcc-plugin)

# Test the plugin compilation
add_custom_target(test-gcc-plugin
    COMMAND ${CMAKE_COMMAND} -E echo "Testing GCC plugin compilation..."
    COMMAND ${GCC_EXECUTABLE} -fplugin=$<TARGET_FILE:heimdall-gcc-plugin> 
            -fplugin-arg-heimdall-gcc-plugin-verbose
            -c -o /dev/null -x c /dev/null
    DEPENDS heimdall-gcc-plugin
    COMMENT "Testing GCC plugin functionality"
)

# Add to main build if enabled
if(HEIMDALL_BUILD_COMPILER_PLUGINS)
    message(STATUS "GCC plugin build enabled")
else()
    message(STATUS "GCC plugin build available but not enabled")
    message(STATUS "Use -DHEIMDALL_BUILD_COMPILER_PLUGINS=ON to enable")
endif()