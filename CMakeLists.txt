cmake_minimum_required(VERSION 3.5)

include(FetchContent)

project(vk_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/MP)
endif()

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

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${PROJECT_NAME}")

#file(GLOB IMGUIMO_SRC "${imguizmo_SOURCE_DIR}/ImGuizmo.cpp")

#file(GLOB IMGUIMO_HEADERS "${imguizmo_SOURCE_DIR}/ImGuizmo.h")

#target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE ${imguizmo_SOURCE_DIR})

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

file(COPY
    "${CMAKE_BINARY_DIR}/_deps/assimp-build/bin/assimp-vc143-mt.dll"
    "${CMAKE_SOURCE_DIR}/vendor/physx/lib/release/PhysX_64.dll"
    "${CMAKE_SOURCE_DIR}/vendor/physx/lib/release/PhysXCommon_64.dll"
    "${CMAKE_SOURCE_DIR}/vendor/physx/lib/release/PhysXCooking_64.dll"
    "${CMAKE_SOURCE_DIR}/vendor/physx/lib/release/PhysXFoundation_64.dll"
    "${CMAKE_SOURCE_DIR}/vendor/physx/lib/release/PVDRuntime_64.dll"

    DESTINATION "${CMAKE_BINARY_DIR}"
)

# Target link libraries
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw stb_image raudio imgui assimp glText tinyOBJ meshoptimizer EnTT JSONparser TracyClient PhysX Vulkan)