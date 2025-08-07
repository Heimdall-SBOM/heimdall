cmake_minimum_required(VERSION 3.16)
project(HeimdallSharedLibExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

add_library(shared_greeter SHARED shared_greeter.cpp)
target_include_directories(shared_greeter PUBLIC .)

add_executable(sharedlib-main main.cpp)
target_link_libraries(sharedlib-main PRIVATE shared_greeter)

heimdall_enable_sbom(shared_greeter FORMAT spdx-2.3 VERBOSE ON)
heimdall_enable_sbom(sharedlib-main FORMAT cyclonedx-1.6 VERBOSE ON) 