cmake_minimum_required(VERSION 2.6)

project(OpenCL_Practice)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OpenCL_Practice_SOURCE_DIR}/cmake")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c99")

find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

add_subdirectory(util)

