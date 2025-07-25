# FindHeimdallStandalone.cmake
# CMake module to find Heimdall libraries and headers for standalone example builds

# Try to find Heimdall in the main build directory first
if(DEFINED HEIMDALL_BUILD_DIR AND EXISTS "${HEIMDALL_BUILD_DIR}")
    set(HEIMDALL_ROOT "${HEIMDALL_BUILD_DIR}")
    message(STATUS "Using Heimdall from build directory: ${HEIMDALL_ROOT}")
else()
    # Fallback to installed Heimdall
    find_path(HEIMDALL_ROOT NAMES "lib/libheimdall-core.so" "lib/libheimdall-core.a" 
              PATHS "/usr/local" "/opt/heimdall" ENV HEIMDALL_ROOT)
endif()

# Find Heimdall headers
find_path(HEIMDALL_INCLUDE_DIRS
    NAMES "heimdall/SBOMGenerator.h"
    PATHS 
        "${HEIMDALL_SOURCE_DIR}/src"
        "${HEIMDALL_ROOT}/src"
        "${HEIMDALL_ROOT}/include"
        "/usr/include/heimdall"
        "/usr/local/include/heimdall"
    NO_DEFAULT_PATH
)

# Find Heimdall libraries
find_library(HEIMDALL_CORE_LIBRARY
    NAMES "heimdall-core"
    PATHS
        "${HEIMDALL_ROOT}/lib"
        "/usr/lib"
        "/usr/local/lib"
    NO_DEFAULT_PATH
)

find_library(HEIMDALL_LLD_LIBRARY
    NAMES "heimdall-lld"
    PATHS
        "${HEIMDALL_ROOT}/lib"
        "/usr/lib"
        "/usr/local/lib"
    NO_DEFAULT_PATH
)

find_library(HEIMDALL_GOLD_LIBRARY
    NAMES "heimdall-gold"
    PATHS
        "${HEIMDALL_ROOT}/lib"
        "/usr/lib"
        "/usr/local/lib"
    NO_DEFAULT_PATH
)

# Set variables for compatibility
if(HEIMDALL_INCLUDE_DIRS AND HEIMDALL_CORE_LIBRARY)
    set(HEIMDALL_FOUND TRUE)
    set(HEIMDALL_INCLUDE_DIR ${HEIMDALL_INCLUDE_DIRS})
    set(HEIMDALL_LIBRARIES ${HEIMDALL_CORE_LIBRARY})
    
    if(HEIMDALL_LLD_LIBRARY)
        list(APPEND HEIMDALL_LIBRARIES ${HEIMDALL_LLD_LIBRARY})
    endif()
    
    if(HEIMDALL_GOLD_LIBRARY)
        list(APPEND HEIMDALL_LIBRARIES ${HEIMDALL_GOLD_LIBRARY})
    endif()
    
    message(STATUS "Found Heimdall:")
    message(STATUS "  Include directories: ${HEIMDALL_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${HEIMDALL_LIBRARIES}")
else()
    set(HEIMDALL_FOUND FALSE)
    message(WARNING "Heimdall not found. Examples may not build correctly.")
    message(STATUS "Searched in:")
    message(STATUS "  HEIMDALL_BUILD_DIR: ${HEIMDALL_BUILD_DIR}")
    message(STATUS "  HEIMDALL_ROOT: ${HEIMDALL_ROOT}")
endif()

# Mark as advanced
mark_as_advanced(HEIMDALL_INCLUDE_DIRS HEIMDALL_CORE_LIBRARY HEIMDALL_LLD_LIBRARY HEIMDALL_GOLD_LIBRARY) 