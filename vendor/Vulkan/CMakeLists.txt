cmake_minimum_required(VERSION 3.5)

project(Vulkan)

# Vulkan include and library paths
set(VULKAN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(VULKAN_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib") # lib folder is at the same level as CMakeLists.txt

# Vulkan libraries
set(VULKAN_LIBS
  vulkan-1
  shaderc_combined
)

# Add Vulkan include directories
include_directories(${VULKAN_INCLUDE_DIR})

# Create an interface target for Vulkan
add_library(Vulkan INTERFACE)

# Link Vulkan libraries to the interface target
foreach(lib IN LISTS VULKAN_LIBS)
    target_link_libraries(Vulkan INTERFACE "${VULKAN_LIB_DIR}/${lib}.lib")
endforeach()

# Make include paths available for dependent targets
target_include_directories(Vulkan INTERFACE ${VULKAN_INCLUDE_DIR})
