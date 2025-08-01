# Tests directory CMakeLists.txt

# Try to find system GTest first
find_package(GTest)

# If not found, use FetchContent to download and build GTest
if(NOT GTest_FOUND)
    message(STATUS "GTest not found, using FetchContent to download GoogleTest.")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Prevent Google Test from being installed
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Fix RPATH for macOS to ensure dynamic libraries are found
    if(APPLE)
        set_target_properties(gtest PROPERTIES
            INSTALL_RPATH "@loader_path/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
        set_target_properties(gtest_main PROPERTIES
            INSTALL_RPATH "@loader_path/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
endif()

find_package(OpenSSL REQUIRED)

# ELF library - Linux only
if(UNIX AND NOT APPLE)
    find_library(ELF_LIBRARY NAMES elf)
else()
    set(ELF_LIBRARY "")
endif()

# Test executable and test setup for C++11+
if(CMAKE_CXX_STANDARD GREATER_EQUAL 11)
    # Select test files conditionally
    file(GLOB ALL_TEST_SOURCES "*.cpp")
    set(TEST_SOURCES "")
    
    foreach(source ${ALL_TEST_SOURCES})
        get_filename_component(filename ${source} NAME)
        
        # For C++11/14 builds, exclude tests that use C++17+ features
        if(CMAKE_CXX_STANDARD LESS 17)
            # Exclude tests that use std::filesystem or other C++17+ features
            if(filename STREQUAL "test_sbom_validation.cpp" OR
               filename STREQUAL "test_metadata_extractor_extended.cpp" OR
               filename STREQUAL "test_sbom_generator.cpp" OR
               filename STREQUAL "test_integration_gold.cpp" OR
               filename STREQUAL "test_integration_lld.cpp" OR
               filename STREQUAL "test_dwarf_advanced.cpp" OR
               filename STREQUAL "test_dwarf_cross_platform.cpp" OR
               filename STREQUAL "test_dwarf_integration.cpp" OR
               filename STREQUAL "test_plugin_sbom_consistency.cpp" OR
               filename STREQUAL "test_component_info.cpp" OR
               filename STREQUAL "test_dwarf_extractor.cpp" OR
               filename STREQUAL "test_linux_support.cpp" OR
               filename STREQUAL "test_gold_plugin.cpp" OR
               filename STREQUAL "test_utils.cpp" OR
               filename STREQUAL "test_metadata_extractor.cpp" OR
               filename STREQUAL "test_lld_plugin.cpp" OR
               filename STREQUAL "test_package_manager_and_archive.cpp" OR
               filename STREQUAL "test_plugin_interface.cpp" OR
               filename STREQUAL "test_utils_extended.cpp" OR
               filename STREQUAL "test_sbom_signer.cpp" OR
               filename STREQUAL "test_sbom_signer_extended.cpp" OR
               filename STREQUAL "test_cyclonedx_enhanced_fields.cpp" OR
               filename STREQUAL "test_signature_integration.cpp")
                # Skip these files for C++11/14
                continue()
            endif()
        endif()
        
        # Platform-specific exclusions
        if(APPLE)
            # On macOS, exclude ELF-related tests (Gold plugin and Linux-specific tests)
            if(filename STREQUAL "test_gold_plugin.cpp" OR 
               filename STREQUAL "test_integration_gold.cpp" OR
               filename STREQUAL "test_linux_support.cpp" OR
               filename STREQUAL "test_dwarf_cross_platform.cpp")
                continue()
            endif()
        else()
            # On non-Apple platforms, exclude Mach-O specific tests
            if(filename STREQUAL "test_enhanced_macho.cpp")
                continue()
            endif()
        endif()
        
        # Docker container exclusions
        # Skip package manager tests in Docker containers where they may fail
        # Note: This has been modified to use mock files instead of system binaries
        # if(filename STREQUAL "test_package_manager_and_archive.cpp")
        #     continue()
        # endif()
        
        list(APPEND TEST_SOURCES ${source})
    endforeach()
    
    add_executable(heimdall-tests
        ${TEST_SOURCES}
    )

    # Set properties
    set_target_properties(heimdall-tests PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    
    # Fix RPATH for macOS to find Google Test dynamic libraries
    if(APPLE)
        # Get the directory where GTest libraries are located
        if(GTest_FOUND AND TARGET GTest::gtest)
            get_target_property(GTEST_LOCATION GTest::gtest LOCATION)
            get_filename_component(GTEST_LIB_DIR "${GTEST_LOCATION}" DIRECTORY)
        else()
            set(GTEST_LIB_DIR "@loader_path/../lib")
        endif()
        
        set_target_properties(heimdall-tests PROPERTIES
            INSTALL_RPATH "@executable_path;@executable_path/../lib;@loader_path;@loader_path/../lib;${GTEST_LIB_DIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
            MACOSX_RPATH TRUE
        )
    endif()

    # Include directories
    target_include_directories(heimdall-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/src
        ${OPENSSL_INCLUDE_DIR}
    )

    # Link libraries
    # Support both system and FetchContent GTest targets
    if(TARGET GTest::gtest_main)
        if(UNIX AND NOT APPLE)
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-gold
                heimdall-lld
                GTest::gtest
                GTest::gtest_main
                ${OPENSSL_LIBRARIES}
            )
            if(ELF_LIBRARY)
                target_link_libraries(heimdall-tests PRIVATE ${ELF_LIBRARY})
            endif()
        else()
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-lld
                GTest::gtest
                GTest::gtest_main
                ${OPENSSL_LIBRARIES}
            )
            if(ELF_LIBRARY)
                target_link_libraries(heimdall-tests PRIVATE ${ELF_LIBRARY})
            endif()
        endif()
    elseif(TARGET gtest_main)
        if(UNIX AND NOT APPLE)
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-gold
                heimdall-lld
                gtest
                gtest_main
                ${OPENSSL_LIBRARIES}
            )
            if(ELF_LIBRARY)
                target_link_libraries(heimdall-tests PRIVATE ${ELF_LIBRARY})
            endif()
        else()
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-lld
                gtest
                gtest_main
                ${OPENSSL_LIBRARIES}
            )
            if(ELF_LIBRARY)
                target_link_libraries(heimdall-tests PRIVATE ${ELF_LIBRARY})
            endif()
        endif()
    else()
        message(FATAL_ERROR "No suitable GTest target found!")
    endif()

    # Add compatibility definitions
    if(ENABLE_CPP11_14)
        target_compile_definitions(heimdall-tests PRIVATE
            HEIMDALL_CPP11_14_MODE=1
        )
    endif()

    # Always enable debug output for tests
    target_compile_definitions(heimdall-tests PRIVATE HEIMDALL_DEBUG_ENABLED=1)

    # Add Boost if needed
    if(USE_BOOST_FILESYSTEM)
        target_link_libraries(heimdall-tests PRIVATE
            Boost::filesystem
            Boost::system
        )
        target_compile_definitions(heimdall-tests PRIVATE
            USE_BOOST_FILESYSTEM=1
        )
    endif()

    # Find or fetch fmt for C++20 (not needed for C++23)
    if (CMAKE_CXX_STANDARD EQUAL 20)
        find_package(fmt QUIET)
        if (NOT fmt_FOUND)
            include(FetchContent)
            FetchContent_Declare(
                fmt
                GIT_REPOSITORY https://github.com/fmtlib/fmt.git
                GIT_TAG 10.2.1 # or latest stable
            )
            FetchContent_MakeAvailable(fmt)
        endif()
    endif()

    # Link fmt for C++20
    if (CMAKE_CXX_STANDARD EQUAL 20)
        target_link_libraries(heimdall-tests PRIVATE fmt::fmt)
    endif()

    # Enable CTest integration
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(heimdall-tests)

    target_link_libraries(heimdall-tests PRIVATE nlohmann_json_schema_validator)
    # Only add dependency if the target exists
    if(TARGET copy_json_schema_header)
        add_dependencies(heimdall-tests copy_json_schema_header)
    endif()
    # Add LLVM flags to this specific target
    target_compile_options(heimdall-tests PRIVATE ${LLVM_CXXFLAGS_CLEAN})
    # Add LLVM linker flags to this specific target
    if(DEFINED LLVM_LINKER_FLAGS)
        target_link_options(heimdall-tests PRIVATE ${LLVM_LINKER_FLAGS})
    endif()

    # Link stdc++fs for C++17 on Linux only
    if(CMAKE_CXX_STANDARD EQUAL 17 AND UNIX AND NOT APPLE)
        target_link_libraries(heimdall-tests PRIVATE stdc++fs)
    endif()
endif()
