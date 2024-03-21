cmake_minimum_required(VERSION 3.27)

include_directories(${CONAN_INCLUDE_DIRS_GTEST})
include_directories(${CONAN_INCLUDE_DIRS_BENCHMARK})

set(TEST_APP_NAME unit_test_hazard_containers)

add_executable(${TEST_APP_NAME}
        main.cpp
        test_simple_hazard.cpp
        test_time_simple_hazard.cpp
        test_stack.cpp
)

target_link_libraries(${TEST_APP_NAME}
        hazard
        sync-containers
        ${CONAN_LIBS_GTEST}
        project_sanitizers
        project_warnings
)

enable_testing()