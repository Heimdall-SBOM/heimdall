# CMakeLists.txt for tools directory
# Build standalone tools

find_package(OpenSSL REQUIRED)
find_library(ELF_LIBRARY NAMES elf)
find_package(LLVM REQUIRED CONFIG)

# Build validation tools
add_executable(debug_validator debug_validator.cpp)
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

add_executable(sbom_loader sbom_loader.cpp)
target_link_libraries(sbom_loader PRIVATE llvm_symbols ${OPENSSL_LIBRARIES} ${ELF_LIBRARY})
target_include_directories(sbom_loader PRIVATE ${CMAKE_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

install(TARGETS debug_validator heimdall-validate
    RUNTIME DESTINATION bin
) 