cmake_minimum_required(VERSION 4.0)
project(Udav)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest 1.17.0 EXACT REQUIRED)

enable_testing()

add_executable(UdavInterpreterTest)
target_sources(UdavInterpreterTest
  PRIVATE
    src/lexer/lexer_test.cpp
  PRIVATE
    FILE_SET CXX_MODULES
    FILES
      src/lexer/lexer.cppm
      src/lexer/token.cppm)
target_include_directories(UdavInterpreterTest PRIVATE src/)
target_link_libraries(UdavInterpreterTest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UdavInterpreterTest)
