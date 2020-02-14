cmake_minimum_required (VERSION 3.15)

project ("thread-pool")
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("source")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
add_library("thread-pool" SHARED source/thread-pool.h)