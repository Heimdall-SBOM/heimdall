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

# Heimdall Compiler Common Infrastructure

# Find required packages
find_package(OpenSSL REQUIRED)

# Create compiler common library
add_library(heimdall-compiler-common STATIC
    CompilerMetadata.cpp
)

# Set target properties
set_target_properties(heimdall-compiler-common PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(heimdall-compiler-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../common
    ../../detectors
    ../../utils
    ../../compat
    ${CMAKE_SOURCE_DIR}/external/json/include
)

# Link libraries
target_link_libraries(heimdall-compiler-common PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Add dependencies on heimdall core components if available
if(TARGET heimdall-core)
    target_link_libraries(heimdall-compiler-common PUBLIC heimdall-core)
endif()

# Compiler definitions
target_compile_definitions(heimdall-compiler-common PUBLIC
    HEIMDALL_COMPILER_COMMON=1
)

# Installation
install(TARGETS heimdall-compiler-common
    ARCHIVE DESTINATION lib/heimdall
    COMPONENT compiler-common
)

install(FILES
    CompilerMetadata.hpp
    DESTINATION include/heimdall/compiler
    COMPONENT compiler-common
)

message(STATUS "Heimdall compiler common infrastructure configured")