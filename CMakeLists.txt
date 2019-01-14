cmake_minimum_required(VERSION 2.8.8)

project(DynamicWorkflow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wshadow")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

find_library(YAML_LIBRARY NAMES yaml-cpp)
find_package(SimGrid 3.20 REQUIRED) # This template requires SimGrid v3.20

include_directories(${SimGrid_INCLUDE_DIR})

add_executable(
    dynamic-workflow
    common/common.cpp
    main.cpp
    schedulers/scheduler_base.cpp
    schedulers/naive_scheduler.cpp
    task.cpp
    tasks_graph.cpp
)

target_link_libraries(dynamic-workflow ${SimGrid_LIBRARY} ${YAML_LIBRARY})

add_custom_target(
    run_parser
    COMMAND python3.6 ../xml_yaml_parser/parser.py
)