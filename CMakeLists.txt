cmake_minimum_required(VERSION 3.27)
project(hazard-ptr)

set(CMAKE_CXX_STANDARD 20)

option(CHECK_SANITIZE "enable sanitizers checks on debug mode" OFF)

# https://llvm.org/docs/CMake.html
#set(LLVM_ENABLE_LIBCXX ON)

# Adding sanitizers
add_library(project_sanitizers INTERFACE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_macros.html
    add_compile_options(-D_GLIBCXX_ASSERTIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1)

    add_compile_options(-O0)

    if (CHECK_SANITIZE)
        include(${CMAKE_SOURCE_DIR}/cmake/sanitizers.cmake)
        message(ERROR "Check sanitize")
        enable_sanitizers(project_sanitizers)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize-memory-track-origins -fPIE -pie -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize-memory-track-origins -fPIE -pie -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    endif()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
endif ()

# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# https://clang.llvm.org/docs/DiagnosticsReference.html
add_compile_options(-Wall -Wextra -Wpedantic)

include(${CMAKE_SOURCE_DIR}/cmake/warnings.cmake)
add_library(project_warnings INTERFACE)
enable_warnings(project_warnings)

get_directory_property(CMAKE_CXX_FLAGS_LIST COMPILE_OPTIONS)
list(JOIN CMAKE_CXX_FLAGS_LIST " " CMAKE_CXX_FLAGS)
message("Compiler flags: ${CMAKE_CXX_FLAGS}")

set(ENV{PYTHONPATH} "${CMAKE_CURRENT_SOURCE_DIR}/env/bin/python3")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(hazard)
add_subdirectory(test)
