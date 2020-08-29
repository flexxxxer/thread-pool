cmake_minimum_required (VERSION 3.17)

project ("thread_pool")
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("source")

file(GLOB_RECURSE thread_pool_src ${CMAKE_CURRENT_SOURCE_DIR}/source/thread-pool.h)
add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE ${thread_pool_src})
set(thread_pool_src ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")