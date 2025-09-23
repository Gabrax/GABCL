cmake_minimum_required(VERSION 3.5)

include(FetchContent)

# Set this to false if you don’t use clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded") # Or "MultiThreadedDLL" if you prefer /MD

    if(RELEASE)
        add_compile_options(/MP /O2)
    endif()

    if(DEBUG)
        add_compile_options(/MP /Zi /Od) 
        add_link_options(/DEBUG /INCREMENTAL)
    endif()

    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-function
        -Wno-unused-parameter
        -ferror-limit=0
        -std=c++20
    )
endif()

project(${EXE_NAME})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries only" FORCE)

# ImGuizmo
FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG master
)

# Assimp 
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.3.0
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "Disable Assimp documentation" FORCE)

# Meshoptimizer
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG v0.18
)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0 
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as static library" FORCE)

FetchContent_MakeAvailable(assimp imguizmo meshoptimizer spdlog)

add_subdirectory(vendor/glfw-3.3.2)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui-docking)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

file(GLOB IMGUIMO_SRC "${imguizmo_SOURCE_DIR}/ImGuizmo.cpp")
file(GLOB IMGUIMO_HEADERS "${imguizmo_SOURCE_DIR}/ImGuizmo.h")

target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE ${imguizmo_SOURCE_DIR})

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} ${IMGUIMO_SRC})

# if(RELEASE)
#   if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "-Wl,-subsystem:windows -Wl,-entry:mainCRTStartup")
#   else()
#     set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#   endif()
# else()
#     add_compile_definitions(DEBUG)
# endif()

set(IMGUI_INI_FILE "${CMAKE_BINARY_DIR}/imgui.ini")
set(RES_FOLDER "${CMAKE_BINARY_DIR}/../res")
file(MAKE_DIRECTORY ${RES_FOLDER})

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw glad stb_image imgui assimp meshoptimizer spdlog::spdlog)
