# CMakeLists.txt for tools directory
# Build standalone tools

find_package(OpenSSL REQUIRED)
find_library(ELF_LIBRARY NAMES elf)
find_package(LLVM REQUIRED CONFIG)


# Build validation tools
add_executable(debug_validator debug_validator.cpp)
target_link_libraries(debug_validator PRIVATE nlohmann_json_schema_validator)
target_link_libraries(debug_validator PRIVATE heimdall-core ${OPENSSL_LIBRARIES} ${ELF_LIBRARY})
target_include_directories(debug_validator PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

add_executable(heimdall-validate heimdall-validate.cpp)
target_link_libraries(heimdall-validate PRIVATE heimdall-core ${OPENSSL_LIBRARIES} ${ELF_LIBRARY})
target_include_directories(heimdall-validate PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

# Create a shared library with LLVM symbols
add_library(llvm_symbols SHARED llvm_symbols.cpp)
set_target_properties(llvm_symbols PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET default
)

add_executable(heimdall-sbom sbom_loader.cpp)
target_link_libraries(heimdall-sbom PRIVATE llvm_symbols ${OPENSSL_LIBRARIES} ${ELF_LIBRARY})
target_include_directories(heimdall-sbom PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

# Performance benchmark tool (only build if profiling is enabled)
if(ENABLE_BENCHMARKS)
    add_executable(heimdall-benchmark PerformanceBenchmark.cpp)
    target_link_libraries(heimdall-benchmark PRIVATE heimdall-core ${OPENSSL_LIBRARIES} ${ELF_LIBRARY})
    target_include_directories(heimdall-benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})
    
    install(TARGETS heimdall-benchmark
        RUNTIME DESTINATION bin
    )
endif()

install(TARGETS debug_validator heimdall-validate
    RUNTIME DESTINATION bin
) 