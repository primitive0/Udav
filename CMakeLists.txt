cmake_minimum_required(VERSION 3.28)

project(Udav LANGUAGES CXX)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)
find_package(Catch2 REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(CLI11 REQUIRED)
find_package(magic_enum REQUIRED)

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
target_link_libraries(Udav
  PUBLIC
    Catch2::Catch2
    utf8cpp
    magic_enum::magic_enum)

if(UDAV_BUILD_UNITTESTS)
    message(STATUS "Udav unit tests: ON")

    add_executable(UdavTestRunner)
    target_link_libraries(UdavTestRunner
      PRIVATE
        Udav
        Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(UdavTestRunner
      TEST_PREFIX "UnitTest|")
else()
    message(STATUS "Udav unit tests: OFF")
    target_compile_definitions(Udav PUBLIC CATCH_CONFIG_DISABLE)
endif()

add_executable(UdavMain src/main.cpp)
target_link_libraries(UdavMain PRIVATE Udav CLI11::CLI11)

# TODO: remove GoogleTest
add_executable(UdavGTestRunner)
target_sources(UdavGTestRunner
  PRIVATE
    src/lexer/lexer_test.cpp)
target_link_libraries(UdavGTestRunner PRIVATE Udav GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(UdavGTestRunner
  TEST_PREFIX "UnitTest.")

# TODO: improve lit compatibility
add_test(
  NAME Lit
  COMMAND lit ${CMAKE_CURRENT_SOURCE_DIR}/test/)
