cmake_minimum_required(VERSION 2.8.8)

project(DynamicWorkflow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wshadow")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

find_library(YAML_LIBRARY NAMES yaml-cpp)
find_package(SimGrid 3.20 REQUIRED) # This template requires SimGrid v3.20

include_directories(${SimGrid_INCLUDE_DIR} src)

add_executable(
    dynamic-workflow
    src/argument_parser.cpp
    src/common/common.cpp
    src/main.cpp
    src/platform_generator.cpp
    src/prototypes/scheduler.cpp
    src/schedule/schedule.cpp
    src/schedule/schedule_action.cpp
    src/schedulers/naive_scheduler.cpp
    src/schedulers/scheduler_base.cpp
    src/task.cpp
    src/tasks_graph.cpp
)

target_link_libraries(dynamic-workflow ${SimGrid_LIBRARY} ${YAML_LIBRARY})

add_custom_target(
    run_example
    COMMAND dynamic-workflow ${CMAKE_SOURCE_DIR}/resources/workflows/CyberShake_100.yml
)

add_custom_target(
    parse_all_workflows
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/xml_yaml_parser.py
    ${CMAKE_SOURCE_DIR}/resources/workflows
    ${CMAKE_SOURCE_DIR}/resources/workflows/CyberShake_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Epigenomics_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Inspiral_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Montage_100.xml
)
