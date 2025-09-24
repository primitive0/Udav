cmake_minimum_required(VERSION 3.14)
project(LabIntroduction)

include(FetchContent)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(LabIntroductionTest src/ExampleTest.cpp)
target_include_directories(LabIntroductionTest PUBLIC src/)
target_link_libraries(LabIntroductionTest GTest::gtest_main)
# target_link_libraries(LabIntroductionTest GTest::gmock)

include(GoogleTest)
gtest_discover_tests(LabIntroductionTest)
