cmake_minimum_required(VERSION 3.24)

set(EXTERNAL_LIB ${CMAKE_CURRENT_BINARY_DIR}/external)

# Faheel Bigint
file(DOWNLOAD
  https://github.com/faheel/BigInt/releases/download/v0.5.0-dev/BigInt.hpp
  ${EXTERNAL_LIB}/BigInt.hpp
)

# Cryptopp Integer
find_library(CRYPTOPP cryptopp)
if (CRYPTOPP_NOTFOUND)
  message("cryptopp was not found")
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    COMPILE_DEFINITIONS
    -DNCRYPTOPP
  )
else()
  target_link_libraries(${PROJECT_NAME}
    ${CRYPTOPP}
  )
endif()

target_include_directories(${PROJECT_NAME}
PUBLIC
  ${EXTERNAL_LIB}
)

