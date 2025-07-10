# Heimdall SBOM CMake Integration Template
# Copy this into your CMakeLists.txt and adjust as needed

# Add the Heimdall CMake module directory to your CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include Heimdall configuration and SBOM functions
include(HeimdallConfig)
include(HeimdallSBOM)

# --- Example: Library and Executable ---
add_library(mylib STATIC lib/mylib.cpp)
target_include_directories(mylib PUBLIC lib)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE mylib)

# Enable SBOM generation for the library (library target)
heimdall_enable_sbom(mylib FORMAT spdx-2.3 VERBOSE ON)

# Enable SBOM generation for the executable (multi-target)
heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6 VERBOSE ON)

# To force Gold or LLD, use:
# heimdall_enable_sbom(myapp FORMAT cyclonedx-1.6 LINKER gold)
# heimdall_enable_sbom(mylib FORMAT spdx LINKER lld)

# To generate multiple formats for a target:
# heimdall_enable_sbom(myapp FORMAT spdx)
# heimdall_enable_sbom(myapp FORMAT cyclonedx)

# For advanced options, see the Heimdall documentation or cmake/HeimdallConfig.cmake 