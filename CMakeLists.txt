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
option(ENABLE_PROFILING "Enable performance profiling" OFF)
option(ENABLE_BENCHMARKS "Enable performance benchmarks" OFF)

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
        set(HEIMDALL_CPP11_AVAILABLE 1 CACHE INTERNAL "C++11 available")
        set(ENABLE_CPP11_14 ON CACHE INTERNAL "Enable C++11/14 mode")
        message(STATUS "C++11 mode: HEIMDALL_NO_DWARF set to: ${HEIMDALL_NO_DWARF}")
        if(NOT DEFINED USE_BOOST_FILESYSTEM)
            set(USE_BOOST_FILESYSTEM ON CACHE INTERNAL "Use Boost.Filesystem")
        endif()
    else()
        # C++14 mode
        set(HEIMDALL_CPP14_AVAILABLE 1 CACHE INTERNAL "C++14 available")
        set(ENABLE_CPP11_14 ON CACHE INTERNAL "Enable C++11/14 mode")
        message(STATUS "C++14 mode: HEIMDALL_BASIC_DWARF set to: ${HEIMDALL_BASIC_DWARF}")
        if(NOT DEFINED USE_BOOST_FILESYSTEM)
            set(USE_BOOST_FILESYSTEM ON CACHE INTERNAL "Use Boost.Filesystem")
        endif()
    endif()
else()
    # C++17+ mode
    set(HEIMDALL_CPP17_AVAILABLE 1 CACHE INTERNAL "C++17 available")
    set(HEIMDALL_FULL_DWARF 1 CACHE INTERNAL "Full DWARF support")
    message(STATUS "C++17+ mode: HEIMDALL_FULL_DWARF set to: ${HEIMDALL_FULL_DWARF}")
    
    # C++20+ features
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
        set(HEIMDALL_CPP20_AVAILABLE 1 CACHE INTERNAL "C++20 available")
    endif()
    
    # C++23+ features
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
        set(HEIMDALL_CPP23_AVAILABLE 1 CACHE INTERNAL "C++23 available")
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
    
    # Get LLVM version from the config
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
        OUTPUT_VARIABLE LLVM_VERSION_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE ".*LLVM version ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" LLVM_PACKAGE_VERSION "${LLVM_VERSION_RAW}")
    string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" LLVM_VERSION_MAJOR "${LLVM_PACKAGE_VERSION}")
    
    # Get LLVM paths
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
        OUTPUT_VARIABLE LLVM_INCLUDE_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
        OUTPUT_VARIABLE LLVM_LIBRARY_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
        OUTPUT_VARIABLE LLVM_CXXFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
        OUTPUT_VARIABLE LLVM_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
        OUTPUT_VARIABLE LLVM_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Set LLVM definitions
    set(LLVM_DEFINITIONS "-DLLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}")
    
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} (major: ${LLVM_VERSION_MAJOR})")
    message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "LLVM library dirs: ${LLVM_LIBRARY_DIRS}")
    message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
else()
    # Fallback to find_package if LLVM_CONFIG not provided
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    add_definitions(${LLVM_DEFINITIONS})
endif()

# Fallback: if LLVM_LIBRARIES is empty, set it manually based on LLVM version (always run this)
if(NOT LLVM_LIBRARIES OR LLVM_LIBRARIES STREQUAL "")
    set(LLVM_LIBRARIES "-lLLVM-${LLVM_VERSION_MAJOR}" CACHE STRING "LLVM libraries fallback" FORCE)
    message(STATUS "LLVM_LIBRARIES was empty, setting to: ${LLVM_LIBRARIES}")
endif()

# Detect available LLVM targets
execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --components
    OUTPUT_VARIABLE LLVM_AVAILABLE_COMPONENTS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\n" " " LLVM_AVAILABLE_COMPONENTS "${LLVM_AVAILABLE_COMPONENTS}")
foreach(target IN ITEMS x86 arm aarch64 mips powerpc systemz sparc hexagon lanai avr msp430 loongarch riscv ve amdgpu bpf webassembly nvptx xcore)
    string(FIND "${LLVM_AVAILABLE_COMPONENTS}" "${target}" _found)
    if(NOT _found EQUAL -1)
        string(TOUPPER "${target}" _target_upper)
        set(HAVE_LLVM_TARGET_${_target_upper} 1)
        message(STATUS "LLVM target available: ${target}")
    endif()
endforeach()

# Update DWARF support based on LLVM version for C++11/14
if(CMAKE_CXX_STANDARD LESS 17)
    if(CMAKE_CXX_STANDARD LESS 14)
        if(CMAKE_CXX_STANDARD LESS 11)
            # Already handled above
        else()
            # C++11 mode - use lightweight DWARF parser to avoid LLVM template issues
            set(HEIMDALL_BASIC_DWARF 1 CACHE INTERNAL "Basic DWARF support for C++11 (lightweight parser)" FORCE)
            unset(HEIMDALL_NO_DWARF CACHE)
            unset(HEIMDALL_FULL_DWARF CACHE)
            message(STATUS "C++11 mode: HEIMDALL_BASIC_DWARF enabled (using lightweight parser to avoid LLVM compatibility issues)")
        endif()
    else()
        # C++14 mode - use lightweight DWARF parser instead of LLVM
        set(HEIMDALL_BASIC_DWARF 1 CACHE INTERNAL "Basic DWARF support for C++14 (lightweight parser)" FORCE)
        unset(HEIMDALL_NO_DWARF CACHE)
        unset(HEIMDALL_FULL_DWARF CACHE)
        message(STATUS "C++14 mode: HEIMDALL_BASIC_DWARF enabled (using lightweight parser)")
    endif()
endif()

# ELF library dependency (for Gold plugin) - Linux only
if(UNIX AND NOT APPLE)
    find_library(ELF_LIBRARY NAMES elf)
    if(ELF_LIBRARY)
        message(STATUS "Found ELF library: ${ELF_LIBRARY}")
    else()
        message(WARNING "ELF library not found. Gold plugin may not work correctly.")
    endif()
else()
    # On macOS, ELF library is not needed (uses Mach-O format)
    set(ELF_LIBRARY "")
    message(STATUS "ELF library not needed on macOS (using Mach-O format)")
endif()

# LLVM version compatibility check
if(ENABLE_CPP11_14)
    # For C++11/14, we need LLVM 7-18
    if(LLVM_VERSION_MAJOR GREATER_EQUAL 19)
        message(WARNING "LLVM ${LLVM_VERSION_MAJOR} may not be fully compatible with C++11/14. Consider using LLVM 7-18.")
    endif()
endif()

# Compiler flags
# Extract LLVM flags but exclude -fno-exceptions for external libraries
string(REPLACE "-fno-exceptions" "" LLVM_CXXFLAGS_CLEAN "${LLVM_CXXFLAGS}")

# Note: LLVM was compiled with libc++ but we need to support both Clang and GCC
# We'll handle the ABI compatibility in the DWARFExtractor code
if(APPLE)
    message(STATUS "LLVM compatibility: Will handle ABI differences in DWARFExtractor")
endif()

# Don't set global flags - use target-specific flags instead
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Linker flags - store for target-specific use instead of setting globally
if(DEFINED LLVM_LIBRARY_DIRS)
    set(LLVM_LINKER_FLAGS "-L${LLVM_LIBRARY_DIRS}" CACHE INTERNAL "LLVM linker flags")
endif()

# Coverage
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Profiling
if(ENABLE_PROFILING)
    add_definitions(-DENABLE_PROFILING=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    endif()
endif()

# Benchmarks
if(ENABLE_BENCHMARKS)
    add_definitions(-DENABLE_BENCHMARKS=1)
endif()

# Ensure json-schema-validator public header is in the expected include path
# Only copy if the file doesn't exist or if the source is newer
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann/json-schema.hpp OR
   ${CMAKE_SOURCE_DIR}/external/json-schema-validator/src/nlohmann/json-schema.hpp IS_NEWER_THAN ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann/json-schema.hpp)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann/json-schema.hpp
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/external/json-schema-validator/src/nlohmann/json-schema.hpp
            ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann/json-schema.hpp
        DEPENDS ${CMAKE_SOURCE_DIR}/external/json-schema-validator/src/nlohmann/json-schema.hpp
        COMMENT "Copying json-schema.hpp to include directory"
    )
endif()

# Only create the target if the custom command was actually added
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_custom_target(copy_json_schema_header
        DEPENDS ${CMAKE_SOURCE_DIR}/external/json-schema-validator/include/nlohmann/json-schema.hpp
    )
endif()

# Include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/external/json/include)
include_directories(${CMAKE_SOURCE_DIR}/external/json-schema-validator/include)
include_directories(src)
# Add external directory for shared headers
include_directories(${CMAKE_SOURCE_DIR}/external)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add for external json validation library
add_subdirectory(external/json)
add_subdirectory(external/json-schema-validator)

# Add src directory
add_subdirectory(src)

# Add examples directory (for test binary)
add_subdirectory(examples)

# Add cmake directory for CMake modules
add_subdirectory(cmake)

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
message(STATUS "  Enable Profiling: ${ENABLE_PROFILING}")
message(STATUS "  Enable Benchmarks: ${ENABLE_BENCHMARKS}")
message(STATUS "  C++11/14 Mode: ${ENABLE_CPP11_14}")
message(STATUS "  Use Boost.Filesystem: ${USE_BOOST_FILESYSTEM}")
message(STATUS "  LLVM Version: ${LLVM_PACKAGE_VERSION}")
