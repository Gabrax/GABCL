#include "BackEnd.h"
#include <iostream>
#include <string>
//#include "Vulkan/VK_backEnd.h"
#include "../Input/Input.h"

namespace BackEnd {

    GLFWwindow* _window = NULL;
    WindowedMode _windowedMode = WindowedMode::WINDOWED;
    GLFWmonitor* _monitor;
    const GLFWvidmode* _mode;
    bool _forceCloseWindow = false;
    bool _windowHasFocus = true;
    int _windowedWidth = 0;
    int _windowedHeight = 0;
    int _fullscreenWidth = 0;
    int _fullscreenHeight = 0;
    int _currentWindowWidth = 0;
    int _currentWindowHeight = 0;
    int _presentTargetWidth = 0;
    int _presentTargetHeight = 0;
    int _scrollWheelYOffset = 0;
    int _mouseScreenX = 0;
    int _mouseScreenY = 0;

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void window_focus_callback(GLFWwindow* window, int focused);


    ////////////////////
    //                //
    //      Core      //

    void Init() {

        //VulkanBackEnd::CreateVulkanInstance();

        int width = 1920 * 1.5f;
        int height = 1080 * 1.5f;

        glfwInit();
        glfwSetErrorCallback([](int error, const char* description) { std::cout << "GLFW Error (" << std::to_string(error) << "): " << description << "\n";});

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

        // Resolution and window size
        _monitor = glfwGetPrimaryMonitor();
        _mode = glfwGetVideoMode(_monitor);
        glfwWindowHint(GLFW_RED_BITS, _mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, _mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, _mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, _mode->refreshRate);
        _fullscreenWidth = _mode->width;
        _fullscreenHeight = _mode->height;
        _windowedWidth = width;
        _windowedHeight = height;
        if (_windowedWidth > _fullscreenWidth || _windowedHeight > _fullscreenHeight) {
            _windowedWidth = static_cast<int>(_fullscreenWidth * 0.75f);
            _windowedHeight = static_cast<int>(_fullscreenHeight * 0.75f);
        }
        CreateGLFWWindow(WindowedMode::WINDOWED);
        if (_window == NULL) {
            std::cout << "Failed to create GLFW window\n";
            glfwTerminate();
            return;
        }
        glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);
        glfwSetWindowFocusCallback(_window, window_focus_callback);

        //VulkanBackEnd::InitMinimum();
        // VulkanRenderer minimum init is tangled in the above function
        
        //AssetManager::LoadFont();

        /*if (GetAPI() == API::VULKAN) {
            AssetManager::FindAssetPaths();
        }*/

        // Init sub-systems
        Input::Init();
        //Audio::Init();
        //Physics::Init();
        
        glfwShowWindow(BackEnd::GetWindowPtr());
    }

    void BeginFrame() {
        glfwPollEvents();
    }

    void EndFrame() {

    }

    void UpdateSubSystems() {
         Input::Update();
        //Audio::Update();
        //Scene::Update();
    }

    void CleanUp() {
        if (GetWindowMode() == WindowedMode::FULLSCREEN) {
            ToggleFullscreen();
        }
        glfwTerminate();
    }

    // Window
    GLFWwindow* GetWindowPtr() {
        return _window;
    }

    void SetWindowPointer(GLFWwindow* window) {
        _window = window;
    }

    void CreateGLFWWindow(const WindowedMode& windowedMode) {
        if (windowedMode == WindowedMode::WINDOWED) {
            _currentWindowWidth = _windowedWidth;
            _currentWindowHeight = _windowedHeight;
            _window = glfwCreateWindow(_windowedWidth, _windowedHeight, "VK_Engine", NULL, NULL);
            glfwSetWindowPos(_window, 100, 100);
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            _currentWindowWidth = _fullscreenWidth;
            _currentWindowHeight = _fullscreenHeight;
            _window = glfwCreateWindow(_fullscreenWidth, _fullscreenHeight, "VK_Engine", _monitor, NULL);
        }
        _windowedMode = windowedMode;
    }

    void SetWindowedMode(const WindowedMode& windowedMode) {
        if (windowedMode == WindowedMode::WINDOWED) {
            _currentWindowWidth = _windowedWidth;
            _currentWindowHeight = _windowedHeight;
            glfwSetWindowMonitor(_window, nullptr, 0, 0, _windowedWidth, _windowedHeight, _mode->refreshRate);
            glfwSetWindowPos(_window, 0, 0);
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            _currentWindowWidth = _fullscreenWidth;
            _currentWindowHeight = _fullscreenHeight;
            glfwSetWindowMonitor(_window, _monitor, 0, 0, _fullscreenWidth, _fullscreenHeight, _mode->refreshRate);
        }
        _windowedMode = windowedMode;
    }

    void ToggleFullscreen() {
        if (_windowedMode == WindowedMode::WINDOWED) {
            SetWindowedMode(WindowedMode::FULLSCREEN);
        }
        else {
            SetWindowedMode(WindowedMode::WINDOWED);
        }
        /*else {
            //VulkanBackEnd::HandleFrameBufferResized();
        }*/
    }

    void ForceCloseWindow() {
        _forceCloseWindow = true;
    }

    bool WindowHasFocus() {
        return _windowHasFocus;
    }

    bool WindowHasNotBeenForceClosed() {
        return !_forceCloseWindow;
    }

    int GetWindowedWidth() {
        return _windowedWidth;
    }

    int GetWindowedHeight() {
        return _windowedHeight;
    }

    int GetFullScreenWidth() {
        return _fullscreenWidth;
    }

    int GetFullScreenHeight() {
        return _fullscreenHeight;
    }

    int GetCurrentWindowWidth() {
        return _currentWindowWidth;
    }

    int GetCurrentWindowHeight() {
        return _currentWindowHeight;
    }

    bool WindowIsOpen() {
        return !(glfwWindowShouldClose(_window) || _forceCloseWindow);
    }

    bool WindowIsMinimized() {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        return (width == 0 || height == 0);
    }

    const WindowedMode& GetWindowMode() {
        return _windowedMode;
    }

    //////////////////////////////
    //                          //
    //      Render Targets      //

    void SetPresentTargetSize(int width, int height) {
        _presentTargetWidth = width;
        _presentTargetHeight = height;
        /*else {
            //VulkanBackEnd::SetPresentTargetSize(width, height);
        }*/
    }

    int GetPresentTargetWidth() {
        return _presentTargetWidth;
    }

    int GetPresentTargetHeight() {
        return _presentTargetHeight;
    }


    /////////////////////////
    //                     //
    //      Callbacks      //

    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
        
       //VulkanBackEnd::MarkFrameBufferAsResized();
    }

    void window_focus_callback(GLFWwindow* /*window*/, int focused) {
        if (focused) {
            BackEnd::_windowHasFocus = true;
        }
        else {
            BackEnd::_windowHasFocus = false;
        }
    }
}

