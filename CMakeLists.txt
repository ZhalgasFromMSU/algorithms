cmake_minimum_required(VERSION 3.14)

project(Algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
)

target_sources(${PROJECT_NAME}
PUBLIC
    src/sync/wait_group.cpp
)

add_compile_options(-Werror -Wall -pedantic -g)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message("Building tests")
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()
