# Source directory CMakeLists.txt

# Core library (shared)
add_library(heimdall-core SHARED
    common/SBOMGenerator.cpp
    common/ComponentInfo.cpp
    common/MetadataExtractor.cpp
    common/DWARFExtractor.cpp
    common/Utils.cpp
    common/PluginInterface.cpp
    common/SBOMValidator.cpp
    common/SBOMComparator.cpp
)

set_target_properties(heimdall-core PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(heimdall-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(heimdall-core PUBLIC
    ${LLVM_LIBRARIES}
)

target_compile_definitions(heimdall-core PRIVATE
    ${LLVM_DEFINITIONS}
)

# LLD Plugin
add_library(heimdall-lld SHARED
    lld/LLDAdapter.cpp
    lld/LLDAdapter.hpp
    lld/LLDPlugin.cpp
    common/MetadataExtractor.hpp
    common/SBOMGenerator.hpp
    common/Utils.hpp
)

set_target_properties(heimdall-lld PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(heimdall-lld PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(heimdall-lld PRIVATE
    heimdall-core
    ${LLVM_LIBRARIES}
)

# Gold Plugin (Linux only)
if(UNIX AND NOT APPLE)
    add_library(heimdall-gold SHARED
        gold/GoldAdapter.cpp
        gold/GoldAdapter.hpp
        gold/GoldPlugin.cpp
        common/MetadataExtractor.hpp
        common/SBOMGenerator.hpp
        common/Utils.hpp
    )

    set_target_properties(heimdall-gold PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        PREFIX ""
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )

    target_include_directories(heimdall-gold PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_link_libraries(heimdall-gold PRIVATE
        heimdall-core
    )
endif()

add_subdirectory(tools)

# Install
install(TARGETS heimdall-core
    EXPORT HeimdallTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(TARGETS heimdall-lld
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    install(TARGETS heimdall-gold
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

install(FILES
    compat/compatibility.hpp
    llvm/llvm_detector.hpp
    DESTINATION include/heimdall
)