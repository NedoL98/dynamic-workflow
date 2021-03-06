cmake_minimum_required(VERSION 2.8.8)

project(DynamicWorkflow)

add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wshadow")

set(DEBUG_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
set(SANITIZE_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address")
set(RELEASE_FLAGS "${CMAKE_CXX_FLAGS} -O2") 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(SOURCE_LIST 
    "src/argument_parser.cpp"
    "src/common/common.cpp"
    "src/common/parse_log.cpp"
    "src/platform_generator.cpp"
    "src/platform/host.cpp"
    "src/platform/platform.cpp"
    "src/platform/cloud_platform.cpp"
    "src/prototypes/scheduler.cpp"
    "src/prototypes/cloud_simulator.cpp"
    "src/schedule/schedule.cpp"
    "src/schedule/schedule_action.cpp"
    "src/scheduler_factory.cpp"
    "src/schedulers/baseline_scheduler.cpp"
    "src/schedulers/genetic_scheduler.cpp"
    "src/schedulers/naive_scheduler.cpp"
    "src/schedulers/mao_scheduler.cpp"
    "src/schedulers/scheduler_base.cpp"
    "src/task.cpp"
    "src/tasks_graph.cpp"
    "src/view/viewer.cpp"
    "src/vm_list.cpp"
    "src/workflow/graph.cpp"
    "src/workflow/graph_iterator.cpp"
    "src/workflow/task.cpp"
    "src/files/manager.cpp"
)


find_library(YAML_LIBRARY NAMES yaml-cpp)
find_package(SimGrid 3.20 REQUIRED) # This template requires SimGrid v3.20
find_package(GTest QUIET)

include_directories(${SimGrid_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS} src)

add_executable(
    dynamic-workflow
    ${SOURCE_LIST}
    src/main.cpp
)
set_target_properties(
    dynamic-workflow
    PROPERTIES COMPILE_FLAGS ${RELEASE_FLAGS}
)

add_executable(
    dynamic-workflow-debug
    ${SOURCE_LIST}
    src/main.cpp
)

set_target_properties(
    dynamic-workflow-debug
    PROPERTIES COMPILE_FLAGS ${DEBUG_FLAGS}
)

add_executable(
    dynamic-workflow-test
    src/common/parse_log.cpp
    src/tests/simple.cpp
)

set_target_properties(
    dynamic-workflow-test
    PROPERTIES COMPILE_FLAGS ${DEBUG_FLAGS}
)

target_link_libraries(dynamic-workflow ${SimGrid_LIBRARY} ${YAML_LIBRARY})
target_link_libraries(dynamic-workflow-debug ${SimGrid_LIBRARY} ${YAML_LIBRARY})
target_link_libraries(dynamic-workflow-test  ${SimGrid_LIBRARY} ${YAML_LIBRARY} ${GTEST_LIBRARIES} -lpthread)
#target_link_libraries(dynamic-workflow-fsanitize ${SimGrid_LIBRARY} ${YAML_LIBRARY})

add_custom_target(
    run_example
    COMMAND echo dynamic-workflow
    ${CMAKE_SOURCE_DIR}/resources/workflows/CyberShake_100.yml
    ${CMAKE_SOURCE_DIR}/resources/vms.yml
    COMMAND dynamic-workflow 
    ${CMAKE_SOURCE_DIR}/resources/workflows/CyberShake_100.yml
    ${CMAKE_SOURCE_DIR}/resources/vms.yml
)

add_custom_target(
    release
    COMMAND make dynamic-workflow -j5
)

add_custom_target(
    debug
    COMMAND make dynamic-workflow-debug -j5
)

add_custom_target(
    run_test
    COMMAND make dynamic-workflow-test -j5
    COMMAND dynamic-workflow-test
)

add_dependencies(run_example release)
add_dependencies(run_test   debug)

add_custom_target(
    parse_all_workflows
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/xml_yaml_parser.py
    ${CMAKE_SOURCE_DIR}/resources/workflows
    ${CMAKE_SOURCE_DIR}/resources/workflows/CyberShake_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Epigenomics_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Inspiral_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/Montage_100.xml
    ${CMAKE_SOURCE_DIR}/resources/workflows/montage_1_5.dax
    ${CMAKE_SOURCE_DIR}/resources/workflows/montage_2_0.dax
    ${CMAKE_SOURCE_DIR}/resources/workflows/1000genome.dax
)


