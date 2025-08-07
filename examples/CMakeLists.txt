# Examples CMakeLists.txt
# This file includes all example projects

# Add the OpenSSL/pthread demo
add_subdirectory(openssl_pthread_demo)

# Add the CMake example if the src directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake_example/src")
    add_subdirectory(cmake_example)
endif()

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

 

 

 
