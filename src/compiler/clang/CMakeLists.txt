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

# Find Clang and LLVM
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(NOT LLVM_FOUND)
    message(WARNING "LLVM not found, skipping Clang plugin build")
    return()
endif()

if(NOT Clang_FOUND)
    message(WARNING "Clang not found, skipping Clang plugin build")
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
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
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

# Link libraries
# Try to use the consolidated libclang-cpp if available (LLVM 19+)
find_library(CLANG_CPP_LIB clang-cpp PATHS /usr/lib64 /usr/lib)
if(CLANG_CPP_LIB)
    target_link_libraries(heimdall-clang-plugin PRIVATE
        ${LLVM_LIBS}
        clang-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
    )
else()
    # Fall back to individual libraries for older LLVM versions
    target_link_libraries(heimdall-clang-plugin PRIVATE
        ${LLVM_LIBS}
        clangFrontend
        clangAST
        clangLex
        clangBasic
        clangSerialization
        clangTooling
        OpenSSL::SSL
        OpenSSL::Crypto
    )
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
if(TARGET heimdall-core)
    target_link_libraries(heimdall-clang-plugin PRIVATE heimdall-core)
endif()

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