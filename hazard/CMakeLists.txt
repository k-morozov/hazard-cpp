cmake_minimum_required(VERSION 3.27)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(hazard
    simple/hazard.cpp
)

target_include_directories(hazard PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(hazard PROPERTIES LINKER_LANGUAGE CXX)