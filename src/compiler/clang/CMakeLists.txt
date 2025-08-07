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

# Clang Plugin build configuration for Heimdall

# Wrap the entire Clang plugin build in error handling to prevent CI failures
if(CMAKE_CXX_STANDARD LESS 14)
    message(WARNING "Clang plugins require C++14 or later, skipping Clang plugin build")
    return()
endif()

# Find LLVM first
find_package(LLVM CONFIG QUIET)
if(NOT LLVM_FOUND)
    message(WARNING "LLVM not found, skipping Clang plugin build")
    message(WARNING "Install LLVM development libraries to enable Clang plugin support")
    return()
endif()

# Pre-check for essential Clang library files before attempting find_package
# This prevents CMake errors when ClangTargets.cmake tries to import missing files
set(CLANG_ESSENTIAL_LIBS
    "${LLVM_LIBRARY_DIRS}/libclangBasic.a"
    "${LLVM_LIBRARY_DIRS}/libclang-cpp.so"
    "${LLVM_LIBRARY_DIRS}/libclang-cpp.so.19"
)

set(CLANG_LIB_FOUND FALSE)
foreach(CLANG_LIB ${CLANG_ESSENTIAL_LIBS})
    if(EXISTS "${CLANG_LIB}")
        set(CLANG_LIB_FOUND TRUE)
        message(STATUS "Found Clang library: ${CLANG_LIB}")
        break()
    endif()
endforeach()

if(NOT CLANG_LIB_FOUND)
    message(WARNING "Clang development libraries not found in ${LLVM_LIBRARY_DIRS}")
    message(WARNING "Looked for: libclangBasic.a, libclang-cpp.so*")
    message(WARNING "Install Clang development packages (libclang-*-dev) to enable Clang plugin support")
    message(WARNING "Skipping Clang plugin build")
    return()
endif()

# Now try to find Clang package - should be safe since we verified files exist
find_package(Clang CONFIG QUIET)
if(NOT Clang_FOUND)
    message(WARNING "Clang package configuration not found, skipping Clang plugin build") 
    message(WARNING "Install Clang development libraries (libclang-dev) to enable Clang plugin support")
    return()
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found Clang")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

# Check LLVM version compatibility
if(LLVM_VERSION_MAJOR LESS 11)
    message(WARNING "LLVM version ${LLVM_VERSION_MAJOR} is too old (minimum: 11)")
    message(WARNING "Skipping Clang plugin build")
    return()
endif()

# Add LLVM definitions and include directories
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Check for required Clang headers
set(REQUIRED_CLANG_HEADERS
    "clang/Frontend/FrontendPluginRegistry.h"
    "clang/Frontend/CompilerInstance.h"
    "clang/AST/ASTConsumer.h"
    "clang/AST/RecursiveASTVisitor.h"
    "clang/Lex/PPCallbacks.h"
)

foreach(HEADER ${REQUIRED_CLANG_HEADERS})
    string(REPLACE "/" "_" HEADER_VAR ${HEADER})
    string(REPLACE "." "_" HEADER_VAR ${HEADER_VAR})
    string(TOUPPER ${HEADER_VAR} HEADER_VAR)
    
    find_path(${HEADER_VAR}_PATH ${HEADER}
        PATHS ${CLANG_INCLUDE_DIRS}
        NO_DEFAULT_PATH
    )
    
    if(NOT ${HEADER_VAR}_PATH)
        message(WARNING "Required Clang header ${HEADER} not found")
        message(WARNING "Skipping Clang plugin build")
        return()
    endif()
endforeach()

# Find required libraries
find_package(OpenSSL REQUIRED)

# Build Clang plugin
add_library(heimdall-clang-plugin SHARED
    HeimdallClangPlugin.cpp
    ../common/CompilerMetadata.cpp
)

# Set target properties
set_target_properties(heimdall-clang-plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
    MACOSX_RPATH ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH ON
    INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib;${CMAKE_INSTALL_PREFIX}/lib"
)

# Include directories
target_include_directories(heimdall-clang-plugin PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ../common
    ../../common
    ../../detectors
    ../../utils
    ../../compat
    ${CMAKE_SOURCE_DIR}/external/json/include
)

# Get LLVM and Clang libraries
llvm_map_components_to_libnames(LLVM_LIBS support core irreader)

# Check if required Clang libraries exist before trying to link
find_library(CLANG_BASIC_LIB clangBasic PATHS ${LLVM_LIBRARY_DIRS} /usr/lib64 /usr/lib)
find_library(CLANG_CPP_LIB clang-cpp PATHS ${LLVM_LIBRARY_DIRS} /usr/lib64 /usr/lib)

if(NOT CLANG_BASIC_LIB AND NOT CLANG_CPP_LIB)
    message(WARNING "Clang development libraries not found (libclangBasic.a or libclang-cpp.so)")
    message(WARNING "Install clang development packages (libclang-*-dev) to enable Clang plugin support")
    message(WARNING "Skipping Clang plugin build")
    return()
endif()

# Link Clang libraries
# Try to use the consolidated libclang-cpp if available (LLVM 19+)
if(CLANG_CPP_LIB)
    message(STATUS "Using consolidated libclang-cpp.so")
    target_link_libraries(heimdall-clang-plugin PRIVATE
        clang-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
    )
elseif(CLANG_BASIC_LIB)
    message(STATUS "Using individual Clang libraries")
    # Fall back to individual libraries for older LLVM versions
    target_link_libraries(heimdall-clang-plugin PRIVATE
        clangFrontend
        clangAST
        clangLex
        clangBasic
        clangSerialization
        clangTooling
        OpenSSL::SSL
        OpenSSL::Crypto
    )
else()
    message(WARNING "No suitable Clang libraries found, skipping Clang plugin build")
    return()
endif()

# Compiler definitions
target_compile_definitions(heimdall-clang-plugin PRIVATE
    ${LLVM_DEFINITIONS}
    ${CLANG_DEFINITIONS}
    BUILDING_CLANG_PLUGIN=1
)

# Compiler flags specific to Clang plugins
target_compile_options(heimdall-clang-plugin PRIVATE
    -fPIC
    -fno-rtti
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# Add dependency on heimdall core components
add_dependencies(heimdall-clang-plugin heimdall-core)

# Link libraries - order matters for shared library dependencies
target_link_libraries(heimdall-clang-plugin PRIVATE
    heimdall-core
    ${LLVM_LIBS}
)

# Installation
install(TARGETS heimdall-clang-plugin
    LIBRARY DESTINATION lib/heimdall/compiler
    COMPONENT clang-plugin
)

# Create a Clang plugin wrapper script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/heimdall-clang-wrapper.sh.in
    ${CMAKE_BINARY_DIR}/bin/heimdall-clang
    @ONLY
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/heimdall-clang
    DESTINATION bin
    COMPONENT clang-plugin
)

# Print build information
message(STATUS "Clang plugin will be built as: heimdall-clang-plugin.so")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include dirs: ${CLANG_INCLUDE_DIRS}")
message(STATUS "LLVM libraries: ${LLVM_LIBS}")

# Add custom target for easy building
add_custom_target(clang-plugin DEPENDS heimdall-clang-plugin)

# Test the plugin compilation
add_custom_target(test-clang-plugin
    COMMAND ${CMAKE_COMMAND} -E echo "Testing Clang plugin compilation..."
    COMMAND clang++ -load $<TARGET_FILE:heimdall-clang-plugin> 
            -plugin heimdall-sbom
            -plugin-arg-heimdall-sbom-verbose
            -c -o /dev/null -x c++ /dev/null
    DEPENDS heimdall-clang-plugin
    COMMENT "Testing Clang plugin functionality"
)

# Add to main build if enabled
if(HEIMDALL_BUILD_COMPILER_PLUGINS)
    message(STATUS "Clang plugin build enabled")
else()
    message(STATUS "Clang plugin build available but not enabled")
    message(STATUS "Use -DHEIMDALL_BUILD_COMPILER_PLUGINS=ON to enable")
endif()