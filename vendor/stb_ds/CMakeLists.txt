cmake_minimum_required(VERSION 3.5)
project(stb_ds)

add_library(stb_ds INTERFACE)

target_include_directories(stb_ds INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
