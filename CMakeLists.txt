cmake_minimum_required(VERSION 3.22)
project(mount_daemon)

set(IS_DEBUG_BUILD CMAKE_BUILD_TYPE STREQUAL "Debug")

# Indication to the code that this is a debug build
if (${IS_DEBUG_BUILD})
add_compile_definitions(__DEBUG__)
endif ()

option(examples_option "Build examples." ON)

message(STATUS "Build examples '${examples_option}'")

add_subdirectory(src)

if(examples_option)
  add_subdirectory( examples )
endif()