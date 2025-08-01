# CMakeLists.txt for tools directory
# Build standalone tools

find_package(OpenSSL REQUIRED)

# ELF library - Linux only
if(UNIX AND NOT APPLE)
    find_library(ELF_LIBRARY NAMES elf)
else()
    set(ELF_LIBRARY "")
endif()


# Build validation tools
add_executable(debug_validator debug_validator.cpp)
target_link_libraries(debug_validator PRIVATE nlohmann_json_schema_validator)
# Only add dependency if the target exists
if(TARGET copy_json_schema_header)
    add_dependencies(debug_validator copy_json_schema_header)
endif()
target_link_libraries(debug_validator PRIVATE heimdall-core ${OPENSSL_LIBRARIES})
if(UNIX AND NOT APPLE AND ELF_LIBRARY)
    target_link_libraries(debug_validator PRIVATE ${ELF_LIBRARY})
endif()
target_include_directories(debug_validator PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})
# Add LLVM flags to this specific target
target_compile_options(debug_validator PRIVATE ${LLVM_CXXFLAGS_CLEAN})
# Add LLVM linker flags to this specific target
if(DEFINED LLVM_LINKER_FLAGS)
    target_link_options(debug_validator PRIVATE ${LLVM_LINKER_FLAGS})
endif()

add_executable(heimdall-validate heimdall-validate.cpp)
target_link_libraries(heimdall-validate PRIVATE heimdall-core ${OPENSSL_LIBRARIES})
if(UNIX AND NOT APPLE AND ELF_LIBRARY)
    target_link_libraries(heimdall-validate PRIVATE ${ELF_LIBRARY})
endif()
target_include_directories(heimdall-validate PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})
# Add LLVM flags to this specific target
target_compile_options(heimdall-validate PRIVATE ${LLVM_CXXFLAGS_CLEAN})
# Add LLVM linker flags to this specific target
if(DEFINED LLVM_LINKER_FLAGS)
    target_link_options(heimdall-validate PRIVATE ${LLVM_LINKER_FLAGS})
endif()

# Create a shared library with LLVM symbols
add_library(llvm_symbols SHARED llvm_symbols.cpp)
set_target_properties(llvm_symbols PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET default
)
# Add LLVM flags to this specific target
target_compile_options(llvm_symbols PRIVATE ${LLVM_CXXFLAGS_CLEAN})
# Add LLVM linker flags to this specific target
if(DEFINED LLVM_LINKER_FLAGS)
    target_link_options(llvm_symbols PRIVATE ${LLVM_LINKER_FLAGS})
endif()

add_executable(heimdall-sbom heimdall-sbom.cpp)
target_link_libraries(heimdall-sbom PRIVATE llvm_symbols heimdall-core ${OPENSSL_LIBRARIES})
if(UNIX AND NOT APPLE AND ELF_LIBRARY)
    target_link_libraries(heimdall-sbom PRIVATE ${ELF_LIBRARY})
endif()
target_include_directories(heimdall-sbom PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})
# Add LLVM flags to this specific target
target_compile_options(heimdall-sbom PRIVATE ${LLVM_CXXFLAGS_CLEAN})
# Add LLVM linker flags to this specific target
if(DEFINED LLVM_LINKER_FLAGS)
    target_link_options(heimdall-sbom PRIVATE ${LLVM_LINKER_FLAGS})
endif()

# Performance benchmark tool (only build if profiling is enabled)
if(ENABLE_BENCHMARKS)
    add_executable(heimdall-benchmark PerformanceBenchmark.cpp)
    target_link_libraries(heimdall-benchmark PRIVATE heimdall-core ${OPENSSL_LIBRARIES})
    if(UNIX AND NOT APPLE AND ELF_LIBRARY)
        target_link_libraries(heimdall-benchmark PRIVATE ${ELF_LIBRARY})
    endif()
    target_include_directories(heimdall-benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})
    # Add LLVM flags to this specific target
    target_compile_options(heimdall-benchmark PRIVATE ${LLVM_CXXFLAGS_CLEAN})
    # Add LLVM linker flags to this specific target
    if(DEFINED LLVM_LINKER_FLAGS)
        target_link_options(heimdall-benchmark PRIVATE ${LLVM_LINKER_FLAGS})
    endif()
    
    install(TARGETS heimdall-benchmark
        RUNTIME DESTINATION bin
    )
endif()

install(TARGETS heimdall-validate heimdall-sbom
    RUNTIME DESTINATION bin
) 