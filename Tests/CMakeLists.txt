cmake_minimum_required(VERSION 3.0)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB DataStreamTest_SRC
    "*.h"
    "*.cpp"
)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}Test ${DataStreamTest_SRC})
target_include_directories(${PROJECT_NAME}Test PRIVATE ${DataStream_DIR} .)
target_link_libraries(${PROJECT_NAME}Test ${GTEST_BOTH_LIBRARIES})
