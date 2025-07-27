# Examples CMakeLists.txt
# This file includes all example projects

# Add the OpenSSL/pthread demo
add_subdirectory(openssl_pthread_demo)

# Add BOM reference demo
add_executable(bom_ref_demo bom_ref_demo.cpp)
target_link_libraries(bom_ref_demo PRIVATE heimdall-core)
target_include_directories(bom_ref_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)

install(TARGETS bom_ref_demo
    RUNTIME DESTINATION bin
)

# Add profiling example (only if profiling is enabled)
if(ENABLE_PROFILING)
    add_executable(profiling_example profiling_example.cpp)
    target_link_libraries(profiling_example PRIVATE heimdall-core)
    target_include_directories(profiling_example PRIVATE ${CMAKE_SOURCE_DIR}/src)
    
    install(TARGETS profiling_example
        RUNTIME DESTINATION bin
    )
endif() 