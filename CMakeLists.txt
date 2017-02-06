cmake_minimum_required (VERSION 2.6)
project (player)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x -O2 -pthread")

add_subdirectory(benchmarks)
add_subdirectory(player)
add_subdirectory(tests)
