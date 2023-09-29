cmake_minimum_required(VERSION 3.14)

project(Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -g -Wall -Werror -pedantic -fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=address -fsanitize=undefined)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${PROJECT_NAME}
    bigint.cpp
    string.cpp
    sync/wait_group.cpp
    sync/queue.cpp
    # field.cpp
)

target_link_libraries(${PROJECT_NAME} GTest::gtest_main ${CMAKE_PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
