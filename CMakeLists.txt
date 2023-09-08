cmake_minimum_required(VERSION 3.18)

project(gaya-lang
    LANGUAGES CXX
    DESCRIPTION "Programming language to solve AoC problems.")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party/fmt)

include_directories(include)
add_subdirectory(src)


