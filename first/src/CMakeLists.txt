cmake_minimum_required(VERSION 3.6)
project(src)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

set(SOURCE_FILES main.cpp Solver.h test.inc data.inc devs.h)
add_executable(src ${SOURCE_FILES})