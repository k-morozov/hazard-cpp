cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

add_library(simple_hazard hazard.h)

set_target_properties(simple_hazard PROPERTIES LINKER_LANGUAGE CXX)