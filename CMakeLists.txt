cmake_minimum_required(VERSION 3.22)
project(orc++ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        src/assert
        src/core
        src/utilities
        src/expected
        src/time
        src/other
        src/strings
        src/optional
        src/iterators
        src/containers
)

add_library(orc++ SHARED src/library.cpp)
target_compile_definitions(orc++ PRIVATE ORC_EXPORT)
add_executable(orc++tests src/tests.cpp)
target_compile_definitions(orc++tests PRIVATE ORC_EXPORT)