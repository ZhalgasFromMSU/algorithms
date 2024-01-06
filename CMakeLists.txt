cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(algorithms CXX)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC FILE_SET modules TYPE CXX_MODULES FILES
  algorithms.cpp
  src/sync/wait_group.cpp
  src/sync/queue.cpp
  src/sync/thread_pool.cpp
)

include(CTest)
if (BUILD_TESTING)
  add_compile_options(-fsanitize=thread -g)
  add_link_options(-fsanitize=thread)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # release-1.14.0
  )
  FetchContent_MakeAvailable(googletest)
  set(TESTS_TARGET "algorithms_tests")
  add_executable(${TESTS_TARGET}
    tests/wait_group_ut.cpp
    tests/lfqueue_ut.cpp
  )

  target_link_libraries(${TESTS_TARGET} GTest::gtest_main ${PROJECT_NAME})
  include(GoogleTest)
  gtest_discover_tests(${TESTS_TARGET})
endif()
