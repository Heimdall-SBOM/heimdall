# Examples CMakeLists.txt
# This file includes all example projects

# Always build essential examples that are part of the main build
# Add the OpenSSL/pthread demo
add_subdirectory(openssl_pthread_demo)

# Add the CMake example if the src directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake_example/src")
    add_subdirectory(cmake_example)
endif()

# All other examples are built as standalone projects when --examples is used
# They require the main build to complete first to find the heimdall-sbom tool
# (These are commented out because they're built standalone by build.sh --examples)
#
# if(BUILD_EXAMPLES)
#     add_subdirectory(heimdall-cmake-install-example)
#     add_subdirectory(heimdall-cmake-installable-example)
#     add_subdirectory(heimdall-cmake-interface-example)
#     add_subdirectory(heimdall-cmake-module-example)
#     add_subdirectory(heimdall-cmake-sharedlib-example)
#     add_subdirectory(heimdall-dwarf-rich-cyclonedx-example)
#     add_subdirectory(heimdall-macho-enhanced-example)
#     add_subdirectory(heimdall-spdx-validation-example)
#     add_subdirectory(heimdall-usage-cyclonedx-example)
#     add_subdirectory(heimdall-usage-example)
#     add_subdirectory(heimdall-usage-spdx-example)
# endif()

# Add BOM reference demo
add_executable(bom_ref_demo bom_ref_demo.cpp)
target_link_libraries(bom_ref_demo PRIVATE heimdall-core ${OPENSSL_LIBRARIES} ${LLVM_LIBRARIES})
target_include_directories(bom_ref_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(DEFINED LLVM_LIBRARY_DIRS)
    target_link_directories(bom_ref_demo PRIVATE ${LLVM_LIBRARY_DIRS})
endif()

install(TARGETS bom_ref_demo
    RUNTIME DESTINATION bin
)

# Add profiling example (only if profiling is enabled)
if(ENABLE_PROFILING)
    add_executable(profiling_example profiling_example.cpp)
    target_link_libraries(profiling_example PRIVATE heimdall-core ${LLVM_LIBRARIES})
    target_include_directories(profiling_example PRIVATE ${CMAKE_SOURCE_DIR}/src)
    if(DEFINED LLVM_LIBRARY_DIRS)
        target_link_directories(profiling_example PRIVATE ${LLVM_LIBRARY_DIRS})
    endif()
    
    install(TARGETS profiling_example
        RUNTIME DESTINATION bin
    )
endif()
