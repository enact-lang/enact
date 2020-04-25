cmake_minimum_required(VERSION 3.9.2)
project(enact CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(DEBUG)

include_directories(include)
add_subdirectory(lib)
add_subdirectory(src)
