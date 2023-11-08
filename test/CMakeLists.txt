cmake_minimum_required(VERSION 3.0)
project(obj_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

set(OFF 0)
option(AETHER_CONFIG "AETHER_CONFIG" OFF)
if(NOT ${AETHER_CONFIG} EQUAL OFF)
  add_definitions(-include ${AETHER_CONFIG})
endif()

include_directories(
  ../third_party/aether/stream
  ../third_party/Catch2/single_include
  ../third_party/crc32
  ../third_party/cppformat
  ../)
set(SOURCES
  ../obj/obj_test.cpp
  ../third_party/cppformat/format.cc
)

add_executable(obj_test ${SOURCES})
