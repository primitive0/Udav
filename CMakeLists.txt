cmake_minimum_required(VERSION 4.0)
project(Udav)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(CLI11 REQUIRED)

enable_testing()

add_library(Udav OBJECT)
target_sources(Udav
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      src/
  PUBLIC
    FILE_SET CXX_MODULES
    FILES
      src/support/convert.cppm
      src/ast/ast.cppm
      src/ast/builder.cppm
      src/text/utf8/utf8.cppm
      src/lexer/token.cppm
      src/lexer/lexer.cppm
      src/lexer/lexer_stats.cppm
      src/parsing/parser.cppm)
target_link_libraries(Udav PUBLIC utf8cpp)

add_executable(UdavTest)
target_sources(UdavTest
  PRIVATE
    src/lexer/lexer_test.cpp
    src/lexer/lexer_stats_test.cpp
    src/parsing/parsing_test.cpp)
target_link_libraries(UdavTest PRIVATE Udav GTest::gmock_main)

add_executable(UdavMain)
target_sources(UdavMain
  PRIVATE
    src/main.cpp)
target_link_libraries(UdavMain PRIVATE Udav CLI11::CLI11)

include(GoogleTest)
gtest_discover_tests(UdavTest)
