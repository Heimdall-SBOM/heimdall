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
    FetchContent_MakeAvailable(googletest)
endif()

find_package(OpenSSL REQUIRED)
find_library(ELF_LIBRARY NAMES elf)

# Test executable and test setup only for C++17+
if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    # Select test files conditionally
    if(UNIX AND NOT APPLE)
        # On Linux, include all test files including Gold tests
        file(GLOB TEST_SOURCES "*.cpp")
    else()
        # On macOS and other platforms, exclude Gold-related tests
        file(GLOB ALL_TEST_SOURCES "*.cpp")
        set(TEST_SOURCES "")
        foreach(source ${ALL_TEST_SOURCES})
            get_filename_component(filename ${source} NAME)
            if(NOT filename STREQUAL "test_gold_plugin.cpp" AND NOT filename STREQUAL "test_integration_gold.cpp")
                list(APPEND TEST_SOURCES ${source})
            endif()
        endforeach()
    endif()
    
    add_executable(heimdall-tests
        ${TEST_SOURCES}
    )

    # Set properties
    set_target_properties(heimdall-tests PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

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
                ${ELF_LIBRARY}
            )
        else()
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-lld
                GTest::gtest
                GTest::gtest_main
                ${OPENSSL_LIBRARIES}
                ${ELF_LIBRARY}
            )
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
                ${ELF_LIBRARY}
            )
        else()
            target_link_libraries(heimdall-tests PRIVATE
                heimdall-core
                heimdall-lld
                gtest
                gtest_main
                ${OPENSSL_LIBRARIES}
                ${ELF_LIBRARY}
            )
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

    # Link stdc++fs for C++17 on Linux only
    if(CMAKE_CXX_STANDARD EQUAL 17 AND UNIX AND NOT APPLE)
        target_link_libraries(heimdall-tests PRIVATE stdc++fs)
    endif()
endif()
