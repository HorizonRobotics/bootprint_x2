cmake_minimum_required(VERSION 2.8)
option(PARENT_BUILD "is build from parent" OFF)
if(NOT ${PARENT_BUILD})
  include(cmake/hobot_util.cmake)
endif()
project(xroc-framework-example)

option(RELEASE_LIB "build version of release" ON)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

if (${RELEASE_LIB})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()


add_definitions(-DHR_POSIX)
add_definitions(-DHR_LINUX)

list(APPEND CMAKE_C_FLAGS "-march=armv8-a -mcpu=cortex-a53 ")

message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}" )

set(CMAKE_CXX_STANDARD 11)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message("building release")
    list(APPEND CMAKE_C_FLAGS " -O3")
else ()
    list(APPEND CMAKE_C_FLAGS " -Og -g ")
endif()

list(APPEND CMAKE_C_FLAGS " -Wall -fPIC")
list(APPEND CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
string(REGEX REPLACE ";" " " CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
string(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

message("add xroc lib  ...")
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
link_directories("${CMAKE_CURRENT_LIST_DIR}/deps/${suffix}/xroc-framework/lib")

add_subdirectory(example_bbox_filter)


if(NOT ${PARENT_BUILD})
  set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})





