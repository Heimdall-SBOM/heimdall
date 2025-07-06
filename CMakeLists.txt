cmake_minimum_required(VERSION 3.16)
project(Heimdall VERSION 1.0.0 LANGUAGES CXX C)

# Configuration options
option(BUILD_LLD_PLUGIN "Build LLD plugin" ON)
option(BUILD_GOLD_PLUGIN "Build Gold plugin" ON)
option(BUILD_SHARED_CORE "Build shared core library" ON)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example projects" ON)
option(ENABLE_DEBUG "Enable debug output" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -DHEIMDALL_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(ENABLE_DEBUG)
    add_definitions(-DHEIMDALL_DEBUG_ENABLED)
endif()

if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

# Coverage configuration
if(ENABLE_COVERAGE)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    message(STATUS "Code coverage enabled")
endif()

# Platform detection
if(APPLE)
    set(PLATFORM_MACOS TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
endif()

find_package(PkgConfig QUIET)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found - required for SHA256 checksums")
endif()

# Find libelf for ELF parsing
find_library(ELF_LIBRARY NAMES elf)
if(ELF_LIBRARY)
    message(STATUS "Found libelf: ${ELF_LIBRARY}")
    set(ELF_FOUND TRUE)
else()
    message(WARNING "libelf not found - ELF parsing will be limited")
    set(ELF_FOUND FALSE)
endif()

# Find LLVM for DWARF debug info extraction
# Try to find Homebrew LLVM first (macOS)
set(LLVM_DWARF_FOUND FALSE)
if(APPLE)
    set(LLVM_HOMEBREW_PREFIX "/opt/homebrew/opt/llvm")
    if(EXISTS "${LLVM_HOMEBREW_PREFIX}")
        message(STATUS "Using Homebrew LLVM for DWARF support")
        set(LLVM_VERSION "20.1.7")
        set(LLVM_LIBS "LLVM")
        set(LLVM_CXXFLAGS "-I${LLVM_HOMEBREW_PREFIX}/include -std=c++17 -stdlib=libc++ -fno-exceptions -funwind-tables -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")
        set(LLVM_LDFLAGS "-L${LLVM_HOMEBREW_PREFIX}/lib")
        set(LLVM_DWARF_FOUND TRUE)
        message(STATUS "Found Homebrew LLVM ${LLVM_VERSION}")
        message(STATUS "LLVM libraries: ${LLVM_LIBS}")
        message(STATUS "LLVM CXX flags: ${LLVM_CXXFLAGS}")
        message(STATUS "LLVM LD flags: ${LLVM_LDFLAGS}")
    endif()
endif()

# Fallback to llvm-config if Homebrew LLVM not found
if(NOT LLVM_DWARF_FOUND)
    find_program(LLVM_CONFIG llvm-config)
    if(LLVM_CONFIG)
        execute_process(
            COMMAND ${LLVM_CONFIG} --version
            OUTPUT_VARIABLE LLVM_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${LLVM_CONFIG} --libs --system-libs
            OUTPUT_VARIABLE LLVM_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${LLVM_CONFIG} --cxxflags
            OUTPUT_VARIABLE LLVM_CXXFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${LLVM_CONFIG} --ldflags
            OUTPUT_VARIABLE LLVM_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        message(STATUS "Found LLVM ${LLVM_VERSION}")
        message(STATUS "LLVM libraries: ${LLVM_LIBS}")
        message(STATUS "LLVM CXX flags: ${LLVM_CXXFLAGS}")
        message(STATUS "LLVM LD flags: ${LLVM_LDFLAGS}")
        set(LLVM_DWARF_FOUND TRUE)
    else()
        message(WARNING "llvm-config not found - debug info extraction will be limited")
        set(LLVM_DWARF_FOUND FALSE)
    endif()
endif()

# Use LLVM 19.1 (most recent version) to avoid conflicts
if(EXISTS "/usr/lib64/libLLVM.so.19.1")
    message(STATUS "Using LLVM 19.1 library (most recent version)")
    set(LLVM_LIBRARY_PATH "/usr/lib64/libLLVM.so.19.1")
    set(LLVM_VERSION "19.1.0")
    set(LLVM_DWARF_FOUND TRUE)
    # Use LLVM 19.1 explicitly to avoid conflicts with 18.1
    set(LLVM_LIBS "/usr/lib64/libLLVM.so.19.1")
    set(LLVM_CXXFLAGS "-I/usr/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")
    set(LLVM_LDFLAGS "-L/usr/lib64")
    # Ensure we only link to 19.1, not 18.1
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
endif()

# Find LLVM for LLD plugin
if(BUILD_LLD_PLUGIN)
    # Try to find Homebrew LLVM first
    set(LLVM_HOMEBREW_PREFIX "/opt/homebrew/opt/llvm")
    if(EXISTS "${LLVM_HOMEBREW_PREFIX}")
        set(LLVM_DIR "${LLVM_HOMEBREW_PREFIX}/lib/cmake/llvm")
        find_package(LLVM CONFIG)
        if(LLVM_FOUND)
            message(STATUS "Found Homebrew LLVM ${LLVM_PACKAGE_VERSION}")
            set(LLVM_INCLUDE_DIRS "${LLVM_HOMEBREW_PREFIX}/include")
            set(LLVM_LIBRARY_DIR "${LLVM_HOMEBREW_PREFIX}/lib")
        endif()
    endif()
    
    # Fallback to system LLVM
    if(NOT LLVM_FOUND)
        find_package(LLVM QUIET CONFIG)
        if(LLVM_FOUND)
            message(STATUS "Found system LLVM ${LLVM_PACKAGE_VERSION}")
        endif()
    endif()
    
    # Look for LLD libraries in multiple locations
    find_library(LLD_ELF_LIB lldELF 
        HINTS 
        ${LLVM_LIBRARY_DIR}
        /opt/homebrew/opt/lld/lib
        /usr/local/opt/lld/lib
    )
    find_library(LLD_COMMON_LIB lldCommon 
        HINTS 
        ${LLVM_LIBRARY_DIR}
        /opt/homebrew/opt/lld/lib
        /usr/local/opt/lld/lib
    )
    
    if(LLD_ELF_LIB AND LLD_COMMON_LIB)
        set(LLD_AVAILABLE TRUE)
        message(STATUS "LLD libraries found - will build LLD plugin")
        message(STATUS "LLD_ELF_LIB: ${LLD_ELF_LIB}")
        message(STATUS "LLD_COMMON_LIB: ${LLD_COMMON_LIB}")
    else()
        set(LLD_AVAILABLE FALSE)
        message(WARNING "LLD libraries not found")
        message(STATUS "Searched for lldELF and lldCommon in:")
        message(STATUS "  - ${LLVM_LIBRARY_DIR}")
        message(STATUS "  - /opt/homebrew/opt/lld/lib")
        message(STATUS "  - /usr/local/opt/lld/lib")
    endif()
endif()

# Find BFD library for Gold plugin
find_library(BFD_LIBRARY NAMES bfd)
if(BFD_LIBRARY)
    message(STATUS "Found BFD library: ${BFD_LIBRARY}")
    set(BFD_FOUND TRUE)
else()
    message(WARNING "BFD library not found - Gold plugin will have limited functionality")
    set(BFD_FOUND FALSE)
endif()

# Find Gold linker
if(BUILD_GOLD_PLUGIN)
    find_program(GOLD_LINKER NAMES ld.gold gold)
    if(GOLD_LINKER)
        set(GOLD_AVAILABLE TRUE)
        message(STATUS "Gold linker found at: ${GOLD_LINKER}")
        
        find_path(GOLD_PLUGIN_HEADERS plugin-api.h
            PATHS /usr/include /usr/local/include
            PATH_SUFFIXES gold bfd
        )
        
        if(NOT GOLD_PLUGIN_HEADERS)
            set(GOLD_PLUGIN_HEADERS "${CMAKE_SOURCE_DIR}/include/gold")
        endif()
        
        # Check for BFD headers
        include(CheckIncludeFile)
        check_include_file(bfd.h BFD_HEADER_FOUND)
        check_include_file(elf-bfd.h ELF_BFD_HEADER_FOUND)
        
        if(BFD_HEADER_FOUND AND ELF_BFD_HEADER_FOUND AND BFD_FOUND)
            set(GOLD_BFD_AVAILABLE TRUE)
            message(STATUS "BFD support available for Gold plugin")
        else()
            set(GOLD_BFD_AVAILABLE FALSE)
            message(STATUS "BFD support not available for Gold plugin")
        endif()
    else()
        set(GOLD_AVAILABLE FALSE)
        message(WARNING "Gold linker not found")
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

# Heimdall core library
if(BUILD_SHARED_CORE)
    add_library(heimdall-core SHARED
        src/common/SBOMGenerator.cpp
        src/common/ComponentInfo.cpp
        src/common/MetadataExtractor.cpp
        src/common/DWARFExtractor.cpp
        src/common/Utils.cpp
        src/common/PluginInterface.cpp
    )
    
    target_include_directories(heimdall-core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
        ${OPENSSL_INCLUDE_DIR}
    )
    
    # Add LLVM include directories if DWARF support is enabled
    if(LLVM_DWARF_FOUND)
        if(APPLE AND EXISTS "/opt/homebrew/opt/llvm")
            target_include_directories(heimdall-core PRIVATE /opt/homebrew/opt/llvm/include)
        endif()
    endif()
    
    target_compile_definitions(heimdall-core PRIVATE
        HEIMDALL_CORE_BUILDING
        HEIMDALL_VERSION="${PROJECT_VERSION}"
    )
    
    target_link_libraries(heimdall-core PRIVATE ${OPENSSL_LIBRARIES})
    
    if(ELF_FOUND)
        target_link_libraries(heimdall-core PRIVATE ${ELF_LIBRARY})
    endif()
    
    if(LLVM_DWARF_FOUND)
        # Parse LLVM libraries string into list
        string(REPLACE " " ";" LLVM_LIB_LIST "${LLVM_LIBS}")
        
        target_link_libraries(heimdall-core PRIVATE ${LLVM_LIB_LIST})
        target_compile_definitions(heimdall-core PRIVATE LLVM_DWARF_AVAILABLE)
        target_compile_options(heimdall-core PRIVATE ${LLVM_CXXFLAGS})
        target_link_options(heimdall-core PRIVATE ${LLVM_LDFLAGS})
    endif()
    
    set_target_properties(heimdall-core PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Heimdall LLD Plugin
if(BUILD_LLD_PLUGIN AND LLD_AVAILABLE)
    add_library(heimdall-lld SHARED
        src/lld/LLDPlugin.cpp
        src/lld/LLDAdapter.cpp
    )
    
    target_include_directories(heimdall-lld PRIVATE
        ${LLVM_INCLUDE_DIRS}
        /opt/homebrew/opt/llvm/include
        /opt/homebrew/opt/lld/include
        /usr/local/opt/lld/include
        ${CMAKE_SOURCE_DIR}/src
    )
    
    target_compile_definitions(heimdall-lld PRIVATE
        ${LLVM_DEFINITIONS}
        HEIMDALL_LLD_PLUGIN
        LLVM_AVAILABLE
    )
    
    # Link to the appropriate LLVM library based on platform
    if(APPLE AND EXISTS "/opt/homebrew/opt/llvm")
        target_link_libraries(heimdall-lld PRIVATE
            /opt/homebrew/opt/llvm/lib/libLLVM.dylib
            ${LLD_ELF_LIB}
            ${LLD_COMMON_LIB}
        )
    else()
        # Fallback to detected LLVM library
        target_link_libraries(heimdall-lld PRIVATE
            ${LLVM_LIBS}
            ${LLD_ELF_LIB}
            ${LLD_COMMON_LIB}
        )
    endif()
    
    # Add platform-specific linker flags
    if(PLATFORM_LINUX)
        target_link_options(heimdall-lld PRIVATE
            -Wl,--as-needed
            -Wl,--no-undefined
        )
    endif()
    
    if(BUILD_SHARED_CORE)
        target_link_libraries(heimdall-lld PRIVATE heimdall-core)
    endif()
    
    set_target_properties(heimdall-lld PROPERTIES
        OUTPUT_NAME "heimdall-lld"
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
    )
    
    if(PLATFORM_MACOS)
        set_target_properties(heimdall-lld PROPERTIES
            SUFFIX ".dylib"
            MACOSX_RPATH ON
        )
    endif()
endif()

# Heimdall Gold Plugin
if(BUILD_GOLD_PLUGIN AND GOLD_AVAILABLE AND GOLD_BFD_AVAILABLE)
    add_library(heimdall-gold SHARED
        src/gold/GoldPlugin.cpp
        src/gold/GoldAdapter.cpp
    )
    
    target_include_directories(heimdall-gold PRIVATE
        ${GOLD_PLUGIN_HEADERS}
        ${CMAKE_SOURCE_DIR}/src
    )
    
    target_compile_definitions(heimdall-gold PRIVATE
        HEIMDALL_GOLD_PLUGIN
        BFD_AVAILABLE=1
        # Disable LLVM DWARF for Gold plugin to avoid conflicts
        # LLVM_DWARF_AVAILABLE=0
    )
    
    target_link_libraries(heimdall-gold PRIVATE dl)
    
    if(BFD_FOUND)
        target_link_libraries(heimdall-gold PRIVATE ${BFD_LIBRARY})
    endif()
    
    # ENABLE heimdall-core linkage for Gold plugin
    if(BUILD_SHARED_CORE)
        target_link_libraries(heimdall-gold PRIVATE heimdall-core)
    endif()
    
    set_target_properties(heimdall-gold PROPERTIES
        OUTPUT_NAME "heimdall-gold"
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
    )
    
    if(PLATFORM_LINUX)
        set_target_properties(heimdall-gold PROPERTIES
            LINK_FLAGS "-Wl,--no-undefined"
        )
    endif()
elseif(BUILD_GOLD_PLUGIN AND GOLD_AVAILABLE AND NOT GOLD_BFD_AVAILABLE)
    message(WARNING "Gold plugin not built: BFD headers not available")
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)

if(BUILD_SHARED_CORE)
    install(TARGETS heimdall-core
        EXPORT HeimdallCoreTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY src/common/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heimdall
        FILES_MATCHING PATTERN "*.hpp"
    )
endif()

if(TARGET heimdall-lld)
    install(TARGETS heimdall-lld
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/heimdall-plugins
    )
endif()

if(TARGET heimdall-gold)
    install(TARGETS heimdall-gold
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/heimdall-plugins
    )
endif()

# Documentation: Markdown files are included in the repository
# No build step required for documentation

# Summary
message(STATUS "")
message(STATUS "=== Heimdall Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "")
message(STATUS "Plugins to build:")
if(TARGET heimdall-lld)
    message(STATUS "  ✓ LLD Plugin (LLVM ${LLVM_PACKAGE_VERSION})")
else()
    message(STATUS "  ✗ LLD Plugin (LLVM not available)")
endif()

if(TARGET heimdall-gold)
    message(STATUS "  ✓ Gold Plugin")
else()
    message(STATUS "  ✗ Gold Plugin (Gold linker not available)")
endif()

if(BUILD_SHARED_CORE)
    message(STATUS "  ✓ Heimdall Core Library")
endif()

message(STATUS "")
message(STATUS "Documentation:")
message(STATUS "  LLD Integration Rationale: ${CMAKE_SOURCE_DIR}/docs/lld-integration-rationale.md")
message(STATUS "  Gold Installation Guide: ${CMAKE_SOURCE_DIR}/docs/gold-installation.md")
message(STATUS "")

# =====================
# Static Analysis Tools
# =====================
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CPPCHECK_EXE NAMES cppcheck)

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE SOURCE_FILES 
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
    )
    add_custom_target(
        clang-tidy
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on all source files"
        VERBATIM
    )
endif()

if(CPPCHECK_EXE)
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK_EXE} --enable=all --inconclusive --std=c++17 --force --quiet --project=${CMAKE_BINARY_DIR}/compile_commands.json src/
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck on all source files"
        VERBATIM
    )
endif()
