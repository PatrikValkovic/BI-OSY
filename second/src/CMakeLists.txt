cmake_minimum_required(VERSION 3.7)
project(src)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

set(SOURCE_FILES main.cpp implementation.h definitions.h)
add_executable(src ${SOURCE_FILES})