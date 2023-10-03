cmake_minimum_required(VERSION 3.24)

#add_subdirectory(
  #thirdparty
#)

#add_executable(${PROJECT_NAME}
  #bigint.cpp
#)

#target_link_libraries(
  #${PROJECT_NAME}

  #benchmark::benchmark_main
  #${CMAKE_PROJECT_NAME}
#)

target_sources(${PROJECT_NAME}
PUBLIC
  bigint.cpp
)
