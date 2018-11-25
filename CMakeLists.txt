cmake_minimum_required(VERSION 2.8.8)

project(DynamicWorkflow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SimGrid 3.20 REQUIRED) # This template requires SimGrid v3.20

include_directories(${SimGrid_INCLUDE_DIR})

add_executable(
    dynamic-workflow 
    main.cpp
    scheduler.cpp
    task.cpp
)
target_link_libraries(dynamic-workflow ${SimGrid_LIBRARY})
