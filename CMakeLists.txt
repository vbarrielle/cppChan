cmake_minimum_required(VERSION 2.6)

# Project declaration
project(cppChan )

set(EXECUTABLE_OUTPUT_PATH .)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include ${gtest_SOURCE_DIR}/include)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(./3rdparty/gtest-1.6.0/)

add_definitions(-O0 -Wall -g -Wextra -std=c++11)

# Executables
add_executable(test_channel    ./tests/test_channel.cc)

# Linkage libraries
target_link_libraries(
  test_channel
  gtest
)

enable_testing()
add_test(TestChannel test_channel)
