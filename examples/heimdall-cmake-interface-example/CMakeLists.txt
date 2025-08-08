cmake_minimum_required(VERSION 3.16)
project(HeimdallInterfaceLibExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

add_library(interface_greeter INTERFACE)
target_include_directories(interface_greeter INTERFACE .)

add_executable(interface-main main.cpp)
target_link_libraries(interface-main PRIVATE interface_greeter)

heimdall_enable_sbom(interface-main FORMAT cyclonedx-1.6 VERBOSE ON) 