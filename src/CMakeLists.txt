cmake_minimum_required(VERSION 2.8)
add_definitions(-std=c++11)
add_definitions(-Wall)

project("Mr-SEQL")

## Install the executable to bin fodler
set(EXECUTABLE_OUTPUT_PATH bin)

add_executable(sax_convert sax_converter.cpp)
add_executable(mr_seql mr_seql.cpp seql.h seql_learn.cpp SNode.cpp)
