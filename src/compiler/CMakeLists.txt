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

# Heimdall Compiler Plugins

# Option to build compiler plugins (inherit from parent)
set(HEIMDALL_BUILD_COMPILER_PLUGINS ${BUILD_COMPILER_PLUGINS})

message(STATUS "Configuring Heimdall compiler plugins...")

# Always build the common infrastructure (needed by EnhancedGoldAdapter etc.)
add_subdirectory(common)

# Build GCC and Clang plugins only if requested and available
if(HEIMDALL_BUILD_COMPILER_PLUGINS)
    message(STATUS "Building compiler plugins...")
    
    # Try to build GCC plugin
    add_subdirectory(gcc)
    
    # Try to build Clang plugin
    add_subdirectory(clang)
    
    # Create unified targets
    add_custom_target(compiler-plugins)
    
    if(TARGET heimdall-gcc-plugin)
        add_dependencies(compiler-plugins heimdall-gcc-plugin)
        message(STATUS "GCC plugin will be built")
    else()
        message(STATUS "GCC plugin not available")
    endif()
    
    if(TARGET heimdall-clang-plugin)
        add_dependencies(compiler-plugins heimdall-clang-plugin)
        message(STATUS "Clang plugin will be built")
    else()
        message(STATUS "Clang plugin not available")
    endif()
    
else()
    message(STATUS "Compiler plugins disabled")
    message(STATUS "Use -DHEIMDALL_BUILD_COMPILER_PLUGINS=ON to enable")
endif()

# Create a combined test target
if(HEIMDALL_BUILD_COMPILER_PLUGINS)
    add_custom_target(test-compiler-plugins
        COMMENT "Testing all available compiler plugins"
    )
    
    if(TARGET test-gcc-plugin)
        add_dependencies(test-compiler-plugins test-gcc-plugin)
    endif()
    
    if(TARGET test-clang-plugin)
        add_dependencies(test-compiler-plugins test-clang-plugin)
    endif()
endif()

# Install compiler plugin integration files
if(HEIMDALL_BUILD_COMPILER_PLUGINS)
    # Install CMake integration functions
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/HeimdallCompilerPlugin.cmake
        DESTINATION lib/cmake/Heimdall
        COMPONENT cmake-files
    )
endif()