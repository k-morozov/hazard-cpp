cmake_minimum_required(VERSION 3.27)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

include_directories("${HAZARD_DIR}")

add_library(sync-containers
        stack/stack.cpp
)

target_link_libraries(sync-containers PRIVATE
        hazard
        project_sanitizers
        project_warnings
)


target_include_directories(sync-containers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(sync-containers PROPERTIES LINKER_LANGUAGE CXX)