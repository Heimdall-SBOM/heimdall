# Tests directory CMakeLists.txt

# Find GoogleTest
find_package(GTest REQUIRED)

# Test executable
add_executable(heimdall-tests
    test_cyclonedx_all_versions_cpp.cpp
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
)

# Link libraries
target_link_libraries(heimdall-tests PRIVATE
    heimdall-core
    GTest::gtest
    GTest::gtest_main
)

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

# Add test
add_test(NAME heimdall-tests COMMAND heimdall-tests)

# Enable CTest integration
enable_testing()
