cmake_minimum_required(VERSION 3.24)

project(Benchmark)

add_subdirectory(
  thirdparty
)

add_executable(${PROJECT_NAME}
    queue.cpp
)

target_link_libraries(
    ${PROJECT_NAME}

    benchmark::benchmark_main
    concurrentqueue
    ${CMAKE_PROJECT_NAME}
)

