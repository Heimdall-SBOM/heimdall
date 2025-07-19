cmake_minimum_required(VERSION 3.26)
project(heimdall-spdx-validation-example)

# Set C++ standard
set(CMAKE_CXX_STANDARD17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a comprehensive test application with multiple components
add_library(validation_lib STATIC
    src/validation_lib.cpp
    src/validation_utils.cpp
)

target_include_directories(validation_lib PUBLIC src)

# Create a shared library component
add_library(validation_shared SHARED
    src/shared_component.cpp
)

target_include_directories(validation_shared PUBLIC src)

# Create the main executable
add_executable(spdx_validation_app
    src/main.cpp
    src/processor.cpp
    src/validator.cpp
)

target_link_libraries(spdx_validation_app 
    PRIVATE validation_lib 
    validation_shared
)

# Set output directories
set_target_properties(spdx_validation_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(validation_lib validation_shared PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Create validation script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_spdx_sboms.sh
    ${CMAKE_BINARY_DIR}/validate_spdx_sboms.sh
    @ONLY
)

# Make the script executable
execute_process(
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/validate_spdx_sboms.sh
) 