cmake_minimum_required(VERSION 3.16)
project(HeimdallCMakeModuleExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Heimdall CMake Module Integration ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake")
include(HeimdallConfig)
include(HeimdallSBOM)

# Add the greeter library (static)
add_library(greeter STATIC lib/greeter.cpp)
target_include_directories(greeter PUBLIC lib)

# Add the main executable, link to greeter
add_executable(heimdall-cmake-module-example main.cpp)
target_link_libraries(heimdall-cmake-module-example PRIVATE greeter)

# Enable SBOM generation for the library (demonstrates library target)
heimdall_enable_sbom(greeter FORMAT spdx-2.3 VERBOSE ON)

# Enable SBOM generation for the main executable (multi-target)
heimdall_enable_sbom(heimdall-cmake-module-example FORMAT cyclonedx-1.6 VERBOSE ON)

# To force Gold or LLD, use:
# heimdall_enable_sbom(heimdall-cmake-module-example FORMAT cyclonedx-1.6 LINKER gold)
# heimdall_enable_sbom(greeter FORMAT spdx LINKER lld)

# To generate multiple formats for a target:
# heimdall_enable_sbom(heimdall-cmake-module-example FORMAT spdx)
# heimdall_enable_sbom(heimdall-cmake-module-example FORMAT cyclonedx) 