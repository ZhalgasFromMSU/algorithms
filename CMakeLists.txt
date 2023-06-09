cmake_minimum_required(VERSION 3.14)

project(Algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
)

target_sources(${PROJECT_NAME}
PUBLIC
    src/finite_field/field.cpp
)

add_compile_options(-Werror -Wall -pedantic -g -O2)
