# Heimdall Test Configuration
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Create test executable
add_executable(heimdall_tests
    main.cpp
    test_utils.cpp
    test_component_info.cpp
    test_sbom_generator.cpp
    test_metadata_extractor.cpp
)

# Force static linking for Google Test on macOS
if(APPLE)
    # Find static libraries explicitly
    find_library(GTEST_LIBRARY
        NAMES libgtest.a gtest
        PATHS /opt/homebrew/Cellar/googletest/*/lib
        NO_DEFAULT_PATH
    )
    find_library(GTEST_MAIN_LIBRARY
        NAMES libgtest_main.a gtest_main
        PATHS /opt/homebrew/Cellar/googletest/*/lib
        NO_DEFAULT_PATH
    )
    
    if(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
        target_link_libraries(heimdall_tests
            ${GTEST_LIBRARY}
            ${GTEST_MAIN_LIBRARY}
            heimdall-core
            ${OPENSSL_LIBRARIES}
        )
    else()
        # Fallback to CMake's find_package
        target_link_libraries(heimdall_tests
            GTest::gtest
            GTest::gtest_main
            heimdall-core
            ${OPENSSL_LIBRARIES}
        )
    endif()
    
    # Set rpath to include build directory
    set_target_properties(heimdall_tests PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path;@executable_path/../lib"
    )
else()
    # Non-Apple platforms
    target_link_libraries(heimdall_tests
        GTest::gtest
        GTest::gtest_main
        heimdall-core
        ${OPENSSL_LIBRARIES}
    )
endif()

# Include directories
target_include_directories(heimdall_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${OPENSSL_INCLUDE_DIR}
    /opt/homebrew/Cellar/googletest/1.17.0/include
)

# Add test
add_test(NAME heimdall_unit_tests COMMAND heimdall_tests)

# Set test properties
set_tests_properties(heimdall_unit_tests PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "HEIMDALL_TEST_DATA_DIR=${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# Create test data directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/test_files)

# Copy test data files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/test_files
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data)

# Set working directory for tests
set_tests_properties(heimdall_unit_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Enable CTest integration
include(CTest)
if(BUILD_TESTING)
    enable_testing()
endif()

# Add custom test targets
add_custom_target(test-all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS heimdall_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test-utils
    COMMAND heimdall_tests --gtest_filter="UtilsTest.*"
    DEPENDS heimdall_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test-component-info
    COMMAND heimdall_tests --gtest_filter="ComponentInfoTest.*"
    DEPENDS heimdall_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test-sbom-generator
    COMMAND heimdall_tests --gtest_filter="SBOMGeneratorTest.*"
    DEPENDS heimdall_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test-metadata-extractor
    COMMAND heimdall_tests --gtest_filter="MetadataExtractorTest.*"
    DEPENDS heimdall_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
