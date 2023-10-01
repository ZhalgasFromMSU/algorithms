cmake_minimum_required(VERSION 3.24)

FetchContent_Declare(
  cameron314_queue
  GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
  GIT_TAG 6dd38b8a1dbaa7863aa907045f32308a56a6ff5d
)

FetchContent_MakeAvailable(
  cameron314_queue
)
