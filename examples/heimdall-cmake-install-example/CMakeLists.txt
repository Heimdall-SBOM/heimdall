cmake_minimum_required(VERSION 3.16)
project(HeimdallInstallExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

add_library(install_greeter STATIC install_greeter.cpp)
target_include_directories(install_greeter PUBLIC .)

add_executable(install-main main.cpp)
target_link_libraries(install-main PRIVATE install_greeter)

# Install rules
install(TARGETS install_greeter DESTINATION lib)
install(TARGETS install-main DESTINATION bin)
install(FILES install_greeter.h DESTINATION include)

heimdall_enable_sbom(install_greeter FORMAT spdx-2.3 VERBOSE ON)
heimdall_enable_sbom(install-main FORMAT cyclonedx-1.6 VERBOSE ON) 