cmake_minimum_required(VERSION 3.16)
project(HeimdallInterfaceLibExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

# Interface library (header only)
add_library(interface_greeter INTERFACE)
target_include_directories(interface_greeter INTERFACE .)

# Real implementation (STATIC)
add_library(interface_greeter_impl STATIC interface_greeter.cpp)
target_link_libraries(interface_greeter_impl PUBLIC interface_greeter)
target_include_directories(interface_greeter_impl PUBLIC .)

add_executable(interface-main main.cpp)
target_link_libraries(interface-main PRIVATE interface_greeter_impl)

heimdall_enable_sbom(interface_greeter_impl FORMAT spdx-2.3 VERBOSE ON)
heimdall_enable_sbom(interface-main FORMAT cyclonedx-1.6 VERBOSE ON) 