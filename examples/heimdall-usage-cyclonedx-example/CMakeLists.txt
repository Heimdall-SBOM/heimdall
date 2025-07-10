cmake_minimum_required(VERSION 3.10)
project(HeimdallExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(example-app
    main.cpp
    utils.cpp
    math.cpp
)

# Include directories
target_include_directories(example-app PRIVATE .)

# Example of how to use Heimdall with CMake
# Note: This is for demonstration purposes. In practice, you would:
# 1. Set CMAKE_LINKER to use LLD or Gold
# 2. Add plugin options via CMAKE_EXE_LINKER_FLAGS

# Example for LLD:
# set(CMAKE_LINKER "ld.lld")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --plugin-opt=load:${HEIMDALL_PLUGIN_PATH} --plugin-opt=sbom-output:${CMAKE_CURRENT_BINARY_DIR}/sbom.json")

# Example for Gold:
# set(CMAKE_LINKER "ld.gold")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --plugin ${HEIMDALL_PLUGIN_PATH} --plugin-opt sbom-output=${CMAKE_CURRENT_BINARY_DIR}/sbom.json")

# Print information about the build
message(STATUS "Building Heimdall example with:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Linker: ${CMAKE_LINKER}")
message(STATUS "")
message(STATUS "To use Heimdall with this CMake project:")
message(STATUS "1. Set CMAKE_LINKER to 'ld.lld' or 'ld.gold'")
message(STATUS "2. Add plugin options to CMAKE_EXE_LINKER_FLAGS")
message(STATUS "3. Build with: cmake .. && make") 