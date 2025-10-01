cmake_minimum_required(VERSION 3.16)
project(${EXE_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  OpenCL_SDK
  GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-SDK.git
  GIT_TAG        main
)

set(OPENCL_SDK_BUILD_SAMPLES OFF CACHE BOOL "Disable SDK samples" FORCE)
set(OPENCL_SDK_BUILD_OPENGL_SAMPLES OFF CACHE BOOL "Disable OpenGL samples" FORCE)
set(OPENCL_SDK_BUILD_VULKAN_SAMPLES OFF CACHE BOOL "Disable Vulkan samples" FORCE)
set(OPENCL_SDK_TEST_SAMPLES OFF CACHE BOOL "Disable sample tests" FORCE)
set(OPENCL_SDK_FETCH_DEPENDENCIES ON CACHE BOOL "Fetch dependencies automatically" FORCE)

FetchContent_MakeAvailable(OpenCL_SDK)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable("${CMAKE_PROJECT_NAME}")

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE OpenCL::OpenCL)
