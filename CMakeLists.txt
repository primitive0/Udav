cmake_minimum_required(VERSION 3.28)

project(Udav LANGUAGES CXX)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# By default Clang 20/21 uses full BMI. Due to some bugs in Clang, it
# is not possible to build Udav with full BMI. Therefore, we fallback
# to reduced BMI.
#
# TODO: probably this should be enabled per target.
include(ClangEnableReducedBMI)
include(IPOSupport)


find_package(Catch2 REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(lyra REQUIRED)
find_package(magic_enum REQUIRED)
find_package(Boost REQUIRED)

include(Catch)


option(UDAV_BUILD_EXTRA "Build Udav extra modules" OFF)
if(UDAV_BUILD_EXTRA)
  message(STATUS "Udav extra: ON")
else()
  message(STATUS "Udav extra: OFF")
endif()

option(UDAV_BUILD_UNITTESTS "Build Udav unit tests" ON)
if(UDAV_BUILD_UNITTESTS)
  message(STATUS "Udav unit tests: ON")
else()
  message(STATUS "Udav unit tests: OFF")
endif()


add_library(UdavLib OBJECT)
target_sources(UdavLib
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      src/
  PUBLIC
    FILE_SET CXX_MODULES
    FILES
      src/support/convert.cppm
      src/support/functional.cppm
      src/ast/visitor.cppm
      src/ast/nodes.cppm
      src/ast/recursive_visitor.cppm
      src/ast/format.cppm
      src/ast/ast.cppm
      src/text/utf8/utf8.cppm
      src/lexer/token.cppm
      src/lexer/lexer.cppm
      src/parsing/token_stream.cppm
      src/parsing/parser.cppm
      src/runtime/exceptions.cppm
      src/runtime/string.cppm
      src/runtime/integer.cppm
      src/runtime/boolean.cppm
      src/runtime/value.cppm
      src/runtime/runtime.cppm
      src/sema/exceptions.cppm
      src/sema/annotations.cppm
      src/sema/literal_parsing.cppm
      src/sema/eval_literals_pass.cppm
      src/sema/sema.cppm
      src/eval/variable_table.cppm
      src/eval/eval.cppm
      src/cli/cli.cppm)
target_link_libraries(UdavLib
  PUBLIC
    Catch2::Catch2
    utf8cpp
    magic_enum::magic_enum
    Boost::headers
    bfg::lyra)
if(NOT UDAV_BUILD_UNITTESTS)
  target_compile_definitions(UdavLib PUBLIC CATCH_CONFIG_DISABLE)
endif()


if(UDAV_BUILD_EXTRA)
  add_subdirectory(extra/lexical_stats/)
endif()


if(UDAV_BUILD_UNITTESTS)
  add_executable(UdavTestRunner)
  target_link_libraries(UdavTestRunner
    PRIVATE
      UdavLib
      Catch2::Catch2WithMain)

  catch_discover_tests(UdavTestRunner
    TEST_PREFIX "UnitTest|")
endif()


add_executable(Udav src/main.cpp)
target_link_libraries(Udav PRIVATE UdavLib)


add_subdirectory(test/)
