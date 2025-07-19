cmake_minimum_required(VERSION 3.16)
project(TaskManagerExample)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(taskmgr
    main.cpp
    Task.cpp
    Project.cpp
    User.cpp
    TaskManager.cpp
    utils.cpp
)

target_include_directories(taskmgr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

