cmake_minimum_required(VERSION 4.0)
project(Udav)

include(FetchContent)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(UdavInterpreterTest)
target_sources(UdavInterpreterTest
  PRIVATE
    src/lexer_test.cpp
  PRIVATE
    FILE_SET CXX_MODULES
    FILES
      src/lexer.cppm)
target_link_libraries(UdavInterpreterTest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UdavInterpreterTest)
