# Source directory CMakeLists.txt

# Core library
add_library(heimdall-core STATIC
    compat/compatibility.hpp
    llvm/llvm_detector.hpp
    llvm/llvm_detector.cpp
)

# Set properties
set_target_properties(heimdall-core PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(heimdall-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link libraries
target_link_libraries(heimdall-core PUBLIC
    ${LLVM_LIBRARIES}
)

# Add Boost if needed
if(USE_BOOST_FILESYSTEM)
    target_link_libraries(heimdall-core PUBLIC
        Boost::filesystem
        Boost::system
    )
    target_compile_definitions(heimdall-core PRIVATE
        USE_BOOST_FILESYSTEM=1
    )
endif()

# Add compatibility definitions
if(ENABLE_CPP11_14)
    target_compile_definitions(heimdall-core PRIVATE
        HEIMDALL_CPP11_14_MODE=1
    )
endif()

# Add LLVM definitions
target_compile_definitions(heimdall-core PRIVATE
    ${LLVM_DEFINITIONS}
)

# Install
install(TARGETS heimdall-core
    EXPORT HeimdallTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    compat/compatibility.hpp
    llvm/llvm_detector.hpp
    DESTINATION include/heimdall
)