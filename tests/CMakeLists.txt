cmake_minimum_required(VERSION 3.14)

project(tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-g -Wall -Werror -pedantic -fsanitize=address)
add_link_options(-fsanitize=address)

include(FetchContent)
FetchContent_Declare(
    googletest
    # URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${PROJECT_NAME}
    bigint.cpp
    string.cpp
    # field.cpp
)

target_link_libraries(${PROJECT_NAME} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
