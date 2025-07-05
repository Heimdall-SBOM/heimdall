# Heimdall Test Configuration
cmake_minimum_required(VERSION 3.16)

include(FetchContent)

enable_testing()

# Download GoogleTest if not found
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GoogleTest not found, downloading with FetchContent...")
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Create test executable
add_executable(heimdall_tests
    main.cpp
    test_utils.cpp
    test_component_info.cpp
    test_sbom_generator.cpp
    test_metadata_extractor.cpp
    test_linux_support.cpp
    test_package_manager_and_archive.cpp
)

# Link GoogleTest and other dependencies
if(TARGET GTest::gtest AND TARGET GTest::gtest_main)
    target_link_libraries(heimdall_tests
        GTest::gtest
        GTest::gtest_main
        heimdall-core
        ${OPENSSL_LIBRARIES}
    )
else()
    # Fallback for older CMake/GoogleTest
    target_link_libraries(heimdall_tests
        gtest
        gtest_main
        heimdall-core
        ${OPENSSL_LIBRARIES}
    )
endif()

# Include directories
# Only add project and OpenSSL includes
# GoogleTest includes are handled by the imported target

target_include_directories(heimdall_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${OPENSSL_INCLUDE_DIR}
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

# === Test Archive Generation ===
set(TESTDATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/testdata)
set(CONAN_LIB_DIR ${TESTDATA_DIR}/conan/lib)
set(SPACK_LIB_DIR ${TESTDATA_DIR}/spack/opt/spack/linux-ubuntu20.04-x86_64/gcc-9.3.0/zlib-1.2.11-abcdef/lib)
set(VCPKG_LIB_DIR ${TESTDATA_DIR}/vcpkg/installed/x64-linux/lib)

file(MAKE_DIRECTORY ${CONAN_LIB_DIR})
file(MAKE_DIRECTORY ${SPACK_LIB_DIR})
file(MAKE_DIRECTORY ${VCPKG_LIB_DIR})

add_custom_command(
    OUTPUT ${TESTDATA_DIR}/test_lib.o
    COMMAND ${CMAKE_C_COMPILER} -c ${TESTDATA_DIR}/test_lib.c -o ${TESTDATA_DIR}/test_lib.o
    DEPENDS ${TESTDATA_DIR}/test_lib.c
    COMMENT "Compiling test_lib.o for test archives"
)

add_custom_command(
    OUTPUT ${TESTDATA_DIR}/libtest.a
    COMMAND ar rcs ${TESTDATA_DIR}/libtest.a ${TESTDATA_DIR}/test_lib.o
    DEPENDS ${TESTDATA_DIR}/test_lib.o
    COMMENT "Creating libtest.a archive"
)

add_custom_command(
    OUTPUT ${TESTDATA_DIR}/libtest_with_syms.a
    COMMAND ar rcs ${TESTDATA_DIR}/libtest_with_syms.a ${TESTDATA_DIR}/test_lib.o
    DEPENDS ${TESTDATA_DIR}/test_lib.o
    COMMENT "Creating libtest_with_syms.a archive"
)

add_custom_command(
    OUTPUT ${CONAN_LIB_DIR}/libz.a
    COMMAND ${CMAKE_COMMAND} -E echo "dummy conan libz" > ${CONAN_LIB_DIR}/libz.a
    COMMENT "Creating dummy conan libz.a"
)

add_custom_command(
    OUTPUT ${SPACK_LIB_DIR}/libz.a
    COMMAND ${CMAKE_COMMAND} -E echo "dummy spack libz" > ${SPACK_LIB_DIR}/libz.a
    COMMENT "Creating dummy spack libz.a"
)

add_custom_command(
    OUTPUT ${VCPKG_LIB_DIR}/libz.a
    COMMAND ${CMAKE_COMMAND} -E echo "dummy vcpkg libz" > ${VCPKG_LIB_DIR}/libz.a
    COMMENT "Creating dummy vcpkg libz.a"
)

add_custom_target(generate_test_archives ALL
    DEPENDS
        ${TESTDATA_DIR}/libtest.a
        ${TESTDATA_DIR}/libtest_with_syms.a
        ${CONAN_LIB_DIR}/libz.a
        ${SPACK_LIB_DIR}/libz.a
        ${VCPKG_LIB_DIR}/libz.a
)
add_dependencies(heimdall_tests generate_test_archives)

# Coverage targets
if(ENABLE_COVERAGE)
    # Find gcov
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(WARNING "gcov not found - coverage reports will not be generated")
    endif()
    
    # Coverage target
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "Running tests with coverage..."
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose
        COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage report..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND ${GCOV_PATH} -r -b -s ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/CMakeFiles/heimdall-core.dir/src/common/*.gcno
        COMMAND ${GCOV_PATH} -r -b -s ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/CMakeFiles/heimdall_tests.dir/*.gcno
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage data generated in ${CMAKE_BINARY_DIR}/coverage/"
        DEPENDS heimdall_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
    )
    
    # Clean coverage target
    add_custom_target(coverage-clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -delete
        COMMENT "Cleaning coverage data"
    )
    
    message(STATUS "Coverage targets available: coverage, coverage-clean")
endif()
