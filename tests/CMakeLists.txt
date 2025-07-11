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
    file(GLOB TEST_SOURCES "*.cpp")
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
        target_link_libraries(heimdall-tests PRIVATE
            heimdall-core
            GTest::gtest
            GTest::gtest_main
            ${OPENSSL_LIBRARIES}
            ${ELF_LIBRARY}
        )
    elseif(TARGET gtest_main)
        target_link_libraries(heimdall-tests PRIVATE
            heimdall-core
            gtest
            gtest_main
            ${OPENSSL_LIBRARIES}
            ${ELF_LIBRARY}
        )
    else()
        message(FATAL_ERROR "No suitable GTest target found!")
    endif()

    # Add compatibility definitions
    if(ENABLE_CPP11_14)
        target_compile_definitions(heimdall-tests PRIVATE
            HEIMDALL_CPP11_14_MODE=1
        )
    endif()

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

    # Add test
    add_test(NAME heimdall-tests COMMAND heimdall-tests)

    # Enable CTest integration
    enable_testing()

    # Link stdc++fs for C++17
    if(CMAKE_CXX_STANDARD EQUAL 17)
        target_link_libraries(heimdall-tests PRIVATE stdc++fs)
    endif()
endif()
