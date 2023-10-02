cmake_minimum_required(VERSION 3.18)

project(gaya-lang
    LANGUAGES CXX
    DESCRIPTION "Programming language to solve AoC problems.")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)

include_directories(include)
add_subdirectory(lib)
add_subdirectory(src)

include(CTest)
enable_testing()
add_subdirectory(tests)
