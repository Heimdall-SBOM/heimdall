cmake_minimum_required(VERSION 3.16)
project(heimdall-macho-enhanced-example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Heimdall package
find_package(Heimdall REQUIRED)

# Create executable
add_executable(macho-enhanced-example main.cpp)

# Link against Heimdall
target_link_libraries(macho-enhanced-example Heimdall::Heimdall)

# Set include directories
target_include_directories(macho-enhanced-example PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Platform-specific settings
if(APPLE)
    # Add macOS-specific flags if needed
    target_compile_definitions(macho-enhanced-example PRIVATE __APPLE__)
endif()

# Installation
install(TARGETS macho-enhanced-example
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib) 