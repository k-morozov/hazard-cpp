cmake_minimum_required(VERSION 3.27)

include_directories(${CONAN_INCLUDE_DIRS_GTEST})
include_directories(${CONAN_INCLUDE_DIRS_BENCHMARK})

set(TEST_APP_NAME unit_test_simple_hazard)

add_executable(${TEST_APP_NAME}
        main.cpp
        test_simple_hazard.cpp
)

target_link_libraries(${TEST_APP_NAME}
        simple_hazard
        ${CONAN_LIBS_GTEST}
        project_sanitizers
        project_warnings
)

enable_testing()