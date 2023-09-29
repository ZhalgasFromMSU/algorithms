cmake_minimum_required(VERSION 3.24)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(FETCHCONTENT_QUIET OFF)

project(Benchmark)

include(FetchContent)

FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG 344117638c8ff7e239044fd0fa7085839fc03021 # release-1.8.3
)

FetchContent_MakeAvailable(google_benchmark)

add_executable(${PROJECT_NAME}
    queue.cpp
)

target_link_libraries(
    ${PROJECT_NAME}

    benchmark::benchmark_main
    ${CMAKE_PROJECT_NAME}
)
