cmake_minimum_required(VERSION 3.0)

project(DataStream++)
file(GLOB DataStream_SRC
    "*.h"
    "*.cpp"
)
set(DataStream_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "DataStream++ include directories")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Tests)
