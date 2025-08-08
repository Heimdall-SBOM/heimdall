cmake_minimum_required(VERSION 3.16)
project(HeimdallInstallableExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

add_library(installable_greeter STATIC installable_greeter.cpp)
target_include_directories(installable_greeter PUBLIC .)

add_executable(installable-main main.cpp)
target_link_libraries(installable-main PRIVATE installable_greeter)

heimdall_enable_sbom(installable_greeter FORMAT spdx-2.3 VERBOSE ON)
heimdall_enable_sbom(installable-main FORMAT cyclonedx-1.6 VERBOSE ON)

install(TARGETS installable_greeter installable-main
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib) 