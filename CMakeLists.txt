cmake_minimum_required(VERSION 3.28)

project(Udav LANGUAGES CXX)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)
find_package(doctest REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(CLI11 REQUIRED)

option(UDAV_BUILD_UNITTESTS "Build Udav unit tests" ON)

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
target_link_libraries(Udav PUBLIC utf8cpp doctest::doctest)

if(UDAV_BUILD_UNITTESTS)
    message(STATUS "Udav unit tests: ON")
else()
    message(STATUS "Udav unit tests: OFF")
    target_compile_definitions(Udav PUBLIC DOCTEST_CONFIG_DISABLE)
endif()

add_executable(UdavTest src/doctest_runner.cpp)
target_link_libraries(UdavTest PRIVATE Udav)

include(doctest)
doctest_discover_tests(UdavTest
  TEST_PREFIX "UnitTest|")

add_executable(UdavMain)
target_sources(UdavMain
  PRIVATE
    src/main.cpp
    src/doctest_stub.cpp)
target_link_libraries(UdavMain PRIVATE Udav CLI11::CLI11)

# TODO: remove GoogleTest
add_executable(UdavGTestRunner)
target_sources(UdavGTestRunner
  PRIVATE
    src/lexer/lexer_test.cpp
    src/lexer/lexer_stats_test.cpp
    src/doctest_stub.cpp)
target_link_libraries(UdavGTestRunner PRIVATE Udav GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(UdavGTestRunner
  TEST_PREFIX "UnitTest.")

# TODO: improve lit compatibility
add_test(
  NAME Lit
  COMMAND lit ${CMAKE_CURRENT_SOURCE_DIR}/test/)
