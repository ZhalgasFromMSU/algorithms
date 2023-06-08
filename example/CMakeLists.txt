cmake_minimum_required(VERSION 3.14)

project(Example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./../ Algorithm)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC Algorithm)
target_include_directories(${PROJECT_NAME}
PUBLIC
    build/depends
)
