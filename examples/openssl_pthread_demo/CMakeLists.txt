cmake_minimum_required(VERSION 3.16)
project(OpenSSLPthreadDemo)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Create the executable
add_executable(openssl_pthread_demo main.c)

# Link with OpenSSL and pthreads
target_link_libraries(openssl_pthread_demo 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    Threads::Threads
)

# Set compile definitions
target_compile_definitions(openssl_pthread_demo PRIVATE
    _REENTRANT  # For pthread support
)

# Set C standard
set_target_properties(openssl_pthread_demo PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Add debug flags for DWARF information (important for SBOM generation)
target_compile_options(openssl_pthread_demo PRIVATE
    -g                    # Generate debug information
    -O0                   # No optimization for better debug info
    -fno-omit-frame-pointer  # Keep frame pointers for better debugging
    -gdwarf-4             # Force DWARF 4 for better LLVM compatibility
)

# Install target - commented out to prevent installation
# install(TARGETS openssl_pthread_demo
#     RUNTIME DESTINATION bin
# )

# Print configuration info
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "Threads library: ${CMAKE_THREAD_LIBS_INIT}")
message(STATUS "Debug flags enabled for DWARF information") 