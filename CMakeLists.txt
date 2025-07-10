cmake_minimum_required(VERSION 3.16)
project(Heimdall VERSION 1.0.0 LANGUAGES CXX C)

# Configuration options (default ON)
option(BUILD_LLD_PLUGIN "Build LLD plugin" ON)
option(BUILD_GOLD_PLUGIN "Build Gold plugin" ON)
option(BUILD_SHARED_CORE "Build shared core library" ON)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example projects" ON)
option(ENABLE_DEBUG "Enable debug output" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Set default C++ standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

# Ensure C++ standard is set
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_CPP11_14 "Enable C++11/14 compatibility mode" OFF)
option(USE_BOOST_FILESYSTEM "Use Boost.Filesystem for C++11/14 compatibility" OFF)

# C++ Standard compatibility matrix
if(CMAKE_CXX_STANDARD LESS 17)
    if(CMAKE_CXX_STANDARD LESS 14)
        if(CMAKE_CXX_STANDARD LESS 11)
            message(FATAL_ERROR "C++11 or later is required")
        endif()
        # C++11 mode
        set(HEIMDALL_CPP11_AVAILABLE 1)
        set(HEIMDALL_NO_DWARF 1)
        set(ENABLE_CPP11_14 ON)
        if(NOT DEFINED USE_BOOST_FILESYSTEM)
            set(USE_BOOST_FILESYSTEM ON)
        endif()
    else()
        # C++14 mode
        set(HEIMDALL_CPP14_AVAILABLE 1)
        set(HEIMDALL_BASIC_DWARF 1)
        set(ENABLE_CPP11_14 ON)
        if(NOT DEFINED USE_BOOST_FILESYSTEM)
            set(USE_BOOST_FILESYSTEM ON)
        endif()
    endif()
else()
    # C++17+ mode
    set(HEIMDALL_CPP17_AVAILABLE 1)
    set(HEIMDALL_FULL_DWARF 1)
    
    # C++20+ features
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
        set(HEIMDALL_CPP20_AVAILABLE 1)
    endif()
    
    # C++23+ features
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
        set(HEIMDALL_CPP23_AVAILABLE 1)
    endif()
endif()

# Boost dependency for C++11/14 compatibility
if(USE_BOOST_FILESYSTEM)
    find_package(Boost REQUIRED COMPONENTS filesystem system)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost.Filesystem is required for C++11/14 compatibility")
    endif()
    add_definitions(-DUSE_BOOST_FILESYSTEM=1)
endif()

# LLVM dependency
if(DEFINED LLVM_CONFIG)
    # Use specific LLVM config if provided
    set(LLVM_CONFIG_EXECUTABLE "${LLVM_CONFIG}")
    message(STATUS "Using LLVM config: ${LLVM_CONFIG_EXECUTABLE}")
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})

# LLVM version compatibility check
if(ENABLE_CPP11_14)
    # For C++11/14, we need LLVM 7-18
    if(LLVM_VERSION_MAJOR GREATER_EQUAL 19)
        message(WARNING "LLVM ${LLVM_VERSION_MAJOR} may not be fully compatible with C++11/14. Consider using LLVM 7-18.")
    endif()
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Coverage
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)
include_directories(src)

# Add src directory
add_subdirectory(src)

# Add examples directory (for test binary)
add_subdirectory(examples)

# Add tests if enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Enable Tests: ${ENABLE_TESTS}")
message(STATUS "  Enable Coverage: ${ENABLE_COVERAGE}")
message(STATUS "  C++11/14 Mode: ${ENABLE_CPP11_14}")
message(STATUS "  Use Boost.Filesystem: ${USE_BOOST_FILESYSTEM}")
message(STATUS "  LLVM Version: ${LLVM_PACKAGE_VERSION}")
