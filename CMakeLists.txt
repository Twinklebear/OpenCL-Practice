cmake_minimum_required(VERSION 2.6)

project(OpenCL_Practice)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OpenCL_Practice_SOURCE_DIR}/cmake")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -ggdb -std=c99")
set(BIN_DIR "${OpenCL_Practice_SOURCE_DIR}/bin/")

find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
include_directories(util)

add_subdirectory(util)
add_subdirectory(opencl_programming_guide)
add_subdirectory(ray_test)

