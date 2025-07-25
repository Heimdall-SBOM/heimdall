# Source directory CMakeLists.txt

# Find OpenSSL (required for Utils.cpp)
if(APPLE)
    # Try Homebrew OpenSSL@3 first
    if(EXISTS "/opt/homebrew/opt/openssl@3")
        set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
    elseif(EXISTS "/usr/local/opt/openssl@3")
        set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl@3")
    endif()
endif()
find_package(OpenSSL REQUIRED)

# Core library (shared)
message(STATUS "LLVM_LIBRARIES: ${LLVM_LIBRARIES}")

# Define source files
set(HEIMDALL_CORE_SOURCES
    common/SBOMGenerator.cpp
    common/ComponentInfo.cpp
    common/MetadataExtractor.cpp
    common/AdaExtractor.cpp
    common/LazySymbolExtractor.cpp
    common/LightweightDWARFParser.cpp
    common/Utils.cpp
    common/PluginInterface.cpp
    common/SBOMValidator.cpp
    common/SBOMComparator.cpp
    common/Profiler.cpp
)

# Conditionally add DWARFExtractor.cpp
if(HEIMDALL_FULL_DWARF)
    list(APPEND HEIMDALL_CORE_SOURCES common/DWARFExtractor.cpp)
elseif(HEIMDALL_BASIC_DWARF)
    # For C++11/14, don't include DWARFExtractor.cpp (use lightweight parser instead)
    # The lightweight parser is always included and provides DWARF functionality
    message(STATUS "Using lightweight DWARF parser for C++${CMAKE_CXX_STANDARD} compatibility")
else()
    # No DWARF support
    message(STATUS "DWARF support disabled")
endif()

add_library(heimdall-core SHARED ${HEIMDALL_CORE_SOURCES})

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
    ${OPENSSL_INCLUDE_DIR}
    ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(heimdall-core PUBLIC
    ${LLVM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

# Only link ELF library on Linux (not macOS)
if(UNIX AND NOT APPLE AND ELF_LIBRARY)
    target_link_libraries(heimdall-core PUBLIC ${ELF_LIBRARY})
endif()

target_compile_definitions(heimdall-core PRIVATE
    ${LLVM_DEFINITIONS}
)

# Add debug support if enabled
if(ENABLE_DEBUG)
    target_compile_definitions(heimdall-core PRIVATE HEIMDALL_DEBUG_ENABLED=1)
endif()

target_link_libraries(heimdall-core PRIVATE nlohmann_json_schema_validator)


# Add DWARF support definitions
if(HEIMDALL_FULL_DWARF)
    target_compile_definitions(heimdall-core PRIVATE LLVM_DWARF_AVAILABLE=1)
elseif(HEIMDALL_BASIC_DWARF)
    # For C++11/14, use lightweight parser instead of LLVM to avoid compatibility issues
    if(CMAKE_CXX_STANDARD LESS 17)
        target_compile_definitions(heimdall-core PRIVATE LLVM_DWARF_AVAILABLE=0)
    else()
        target_compile_definitions(heimdall-core PRIVATE LLVM_DWARF_AVAILABLE=1)
    endif()
elseif(HEIMDALL_NO_DWARF)
    # Explicitly disable DWARF support
    target_compile_definitions(heimdall-core PRIVATE LLVM_DWARF_AVAILABLE=0)
endif()

# LLD Plugin (compatible with C++11+)
if(CMAKE_CXX_STANDARD GREATER_EQUAL 11)
    # Check if LLD is available
    set(LLD_AVAILABLE FALSE)
    set(LLD_INCLUDE_DIR "")
    set(LLD_LIBRARY_DIR "")
    
    if(APPLE)
        # Check for Homebrew LLD
        if(EXISTS "/opt/homebrew/opt/lld/include/lld/Common/Driver.h")
            set(LLD_INCLUDE_DIR "/opt/homebrew/opt/lld/include")
            set(LLD_LIBRARY_DIR "/opt/homebrew/opt/lld/lib")
            set(LLD_AVAILABLE TRUE)
            message(STATUS "Found Homebrew LLD headers at ${LLD_INCLUDE_DIR}")
        elseif(EXISTS "/usr/local/opt/lld/include/lld/Common/Driver.h")
            set(LLD_INCLUDE_DIR "/usr/local/opt/lld/include")
            set(LLD_LIBRARY_DIR "/usr/local/opt/lld/lib")
            set(LLD_AVAILABLE TRUE)
            message(STATUS "Found Homebrew LLD headers at ${LLD_INCLUDE_DIR}")
        endif()
        
        # Also check if LLD is available via llvm-config
        find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config)
        if(LLVM_CONFIG_EXECUTABLE)
            execute_process(
                COMMAND ${LLVM_CONFIG_EXECUTABLE} --components
                OUTPUT_VARIABLE LLVM_COMPONENTS
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(LLVM_COMPONENTS MATCHES "lld")
                set(LLD_AVAILABLE TRUE)
                message(STATUS "LLD component found in LLVM installation")
            endif()
        endif()
    endif()
    
    if(LLD_AVAILABLE)
        message(STATUS "Building LLD plugin - LLD headers found")
        
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
            ${LLD_INCLUDE_DIR}
        )

        target_compile_definitions(heimdall-lld PRIVATE LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1)

        if(HEIMDALL_FULL_DWARF)
            target_compile_definitions(heimdall-lld PRIVATE LLVM_DWARF_AVAILABLE=1)
        elseif(HEIMDALL_BASIC_DWARF)
            # For C++11/14, use lightweight parser instead of LLVM to avoid compatibility issues
            if(CMAKE_CXX_STANDARD LESS 17)
                target_compile_definitions(heimdall-lld PRIVATE LLVM_DWARF_AVAILABLE=0)
            else()
                target_compile_definitions(heimdall-lld PRIVATE LLVM_DWARF_AVAILABLE=1)
            endif()
        elseif(HEIMDALL_NO_DWARF)
            # Explicitly disable DWARF support
            target_compile_definitions(heimdall-lld PRIVATE LLVM_DWARF_AVAILABLE=0)
        endif()

        # Try to get LLD libraries from llvm-config if available
        if(LLVM_CONFIG_EXECUTABLE)
            execute_process(
                COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs lld
                OUTPUT_VARIABLE LLD_LIBS_RAW
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_VARIABLE LLD_LIBS_ERROR
            )
            if(NOT LLD_LIBS_ERROR)
                string(REPLACE "\n" " " LLD_LIBS "${LLD_LIBS_RAW}")
                separate_arguments(LLD_LIBS)
                message(STATUS "LLD libraries from llvm-config: ${LLD_LIBS}")
            else()
                message(STATUS "LLD libraries not available via llvm-config, using basic LLVM libraries")
                set(LLD_LIBS "")
            endif()
        else()
            set(LLD_LIBS "")
        endif()

        target_link_libraries(heimdall-lld PRIVATE
            heimdall-core
            ${LLVM_LIBRARIES}
            ${LLD_LIBS}
        )
    else()
        message(WARNING "LLD headers not found - skipping LLD plugin build")
        message(STATUS "To enable LLD plugin, install LLD: brew install lld")
    endif()
endif()

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

# Check for <format> support in C++20 and C++23 builds
if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
            message(FATAL_ERROR "GCC 13 or newer is required for C++20/C++23 builds due to <format> support.")
        endif()
        # GCC 13+ supports <format>, so we can proceed
        message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION} supports <format> for C++${CMAKE_CXX_STANDARD}")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
            message(FATAL_ERROR "Clang 14 or newer is required for C++20/C++23 builds due to <format> support.")
        endif()
        # Clang 14+ supports <format>, so we can proceed
        message(STATUS "Clang ${CMAKE_CXX_COMPILER_VERSION} supports <format> for C++${CMAKE_CXX_STANDARD}")
    else()
        # For other compilers, let them try and fail gracefully if needed
        message(STATUS "Using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} for C++${CMAKE_CXX_STANDARD}")
    endif()
endif()

add_subdirectory(tools)

# Install
install(TARGETS heimdall-core
    EXPORT HeimdallTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 11)
    install(TARGETS heimdall-lld
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

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