cmake_minimum_required(VERSION 3.5)

project(vk_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/MP)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries only" FORCE)

add_subdirectory(vendor/glfw-3.3.2)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/raudio)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui-docking)
add_subdirectory(vendor/Vulkan)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${PROJECT_NAME}")

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

# Target link libraries
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw stb_image raudio imgui Vulkan)
