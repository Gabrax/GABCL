cmake_minimum_required(VERSION 3.5)

include(FetchContent)

# Project name and version
project(vk_engine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Assimp 
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.3.0
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests" FORCE)

# Meshoptimizer
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG v0.18
)

# TracyProfiler
FetchContent_Declare (
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.11.1
)

# ImGuizmo
FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG master
)

FetchContent_MakeAvailable(assimp meshoptimizer tracy imguizmo)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw-3.3.2)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/glText)
add_subdirectory(vendor/raudio)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui-docking)
add_subdirectory(vendor/tinyobj)
add_subdirectory(vendor/EnTT)
add_subdirectory(vendor/nlohmann-json)
add_subdirectory(vendor/physx)
add_subdirectory(vendor/Vulkan)

# Define the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add executable
add_executable("${PROJECT_NAME}")

#file(GLOB IMGUIMO_SRC "${imguizmo_SOURCE_DIR}/ImGuizmo.cpp")

#file(GLOB IMGUIMO_HEADERS "${imguizmo_SOURCE_DIR}/ImGuizmo.h")

#target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE ${imguizmo_SOURCE_DIR})

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

# Set resource folder as a definition for runtime access
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    RESOURCE_DIR="${CMAKE_SOURCE_DIR}/res"
)

# Set default configuration to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug or Release)" FORCE)
endif()

# Target link libraries
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw stb_image raudio imgui assimp glText tinyOBJ meshoptimizer EnTT JSONparser TracyClient PhysX Vulkan)