cmake_minimum_required(VERSION 3.16)
project(HeimdallCMakeModule)

# Install CMake modules
install(FILES
    FindHeimdall.cmake
    HeimdallConfig.cmake
    HeimdallSBOM.cmake
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/Heimdall
)

# Install templates
install(DIRECTORY templates/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/Heimdall/templates
    FILES_MATCHING PATTERN "*.cmake" PATTERN "*.json"
) 