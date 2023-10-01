cmake_minimum_required(VERSION 3.24)

project(Algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Werror -Wall -pedantic -g)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
)

target_sources(${PROJECT_NAME}
PUBLIC
    src/sync/wait_group.cpp
)

set(IS_ROOT_DIR OFF)
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IS_ROOT_DIR ON)
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARK "Build benchmarks" OFF)

if (BUILD_TESTS AND IS_ROOT_DIR)
    message("Building tests")
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

IF (BUILD_BENCHMARK AND IS_ROOT_DIR)
    message("Building benchmark")
    add_subdirectory(benchmark EXCLUDE_FROM_ALL)
endif()
