cmake_minimum_required(VERSION 3.5)

project(Vulkan)

# Vulkan include directories
set(VULKAN_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/dxc/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/glslang/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/shaderc/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/spirv-headers/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/spirv-tools/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/spirv-cross/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/vk_video/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/vma/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/"
)

# Vulkan library directory
set(VULKAN_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib") # lib folder is at the same level as CMakeLists.txt

# Add VkBootstrap sources
set(VKBOOTSTRAP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/VkBootstrap.cpp")
set(VKBOOTSTRAP_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/VkBootstrap.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/VkBootstrapDispatch.h"
)

# Create the VkBootstrap static library
add_library(VkBootstrap STATIC ${VKBOOTSTRAP_SOURCES} ${VKBOOTSTRAP_HEADERS})

# Include directory for VkBootstrap
target_include_directories(VkBootstrap PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/vulkan/")

# Create an interface target for Vulkan
add_library(Vulkan INTERFACE)

# Set include directories for Vulkan
target_include_directories(Vulkan INTERFACE ${VULKAN_INCLUDE_DIRS})

# Vulkan precompiled libraries (example)
set(VULKAN_LIBS
    vulkan-1
    shaderc_combined
)

# Link Vulkan libraries to the Vulkan interface target
foreach(lib IN LISTS VULKAN_LIBS)
    target_link_libraries(Vulkan INTERFACE "${VULKAN_LIB_DIR}/${lib}.lib")
endforeach()

# Link VkBootstrap with Vulkan
target_link_libraries(Vulkan INTERFACE VkBootstrap)
