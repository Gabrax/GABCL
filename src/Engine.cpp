#include "Engine.h"
#include <vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h> 
#include <iostream>
#include <fstream>
#include <vector>

std::vector<char> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    return std::vector<char>(std::istreambuf_iterator<char>(file), {});
}

bool init_vulkan(GLFWwindow*& window, VkSurfaceKHR& surface, vkb::Instance& vkb_inst, vkb::Device& vkb_device) {
    vkb::InstanceBuilder builder;
    auto inst_ret =
        builder.set_app_name("Example Vulkan Application").request_validation_layers().use_default_debug_messenger().build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }
    vkb_inst = inst_ret.value();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan Triangle", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        return false;
    }

    VkResult glfw_result = glfwCreateWindowSurface(vkb_inst, window, nullptr, &surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return false;
    }

    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    auto phys_ret = selector.set_surface(surface).select();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return false;
    }

    vkb::DeviceBuilder device_builder{ phys_ret.value() };
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return false;
    }
    vkb_device = dev_ret.value();

    return true;
}

void cleanup(VkDevice device, vkb::Device vkb_device, 
             vkb::Instance vkb_inst, VkSurfaceKHR surface, GLFWwindow* window) {
    vkb::destroy_device(vkb_device);
    vkb::destroy_surface(vkb_inst, surface);
    vkb::destroy_instance(vkb_inst);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Engine::Run() {
    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkb::Instance vkb_inst;
    vkb::Device vkb_device;

    if (!init_vulkan(window, surface, vkb_inst, vkb_device)) {
        std::cerr << "Vulkan initialization failed\n";
        return;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Poll events (e.g., window close, resizing)

        // Render a frame (Placeholder code, you would call Vulkan commands here)
        // This is where you will use Vulkan to render to the screen.
        // For now, we'll assume a placeholder render loop.

        // For example: Present the swapchain, submit command buffers, etc.

        // Swap buffers after rendering the frame
        // vkQueuePresentKHR(graphics_queue, &present_info); // Example

        // Ensure that the window updates (or Vulkan does it automatically).
    }

    cleanup(vkb_device.device, vkb_device, vkb_inst, surface, window);
}
