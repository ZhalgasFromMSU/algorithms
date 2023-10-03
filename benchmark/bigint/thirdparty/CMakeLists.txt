cmake_minimum_required(VERSION 3.24)

set(EXTERNAL_LIB ${CMAKE_CURRENT_BINARY_DIR}/external)

file(DOWNLOAD
  https://github.com/faheel/BigInt/releases/download/v0.5.0-dev/BigInt.hpp
  ${EXTERNAL_LIB}/BigInt.hpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
  ${EXTERNAL_LIB}
)
