#pragma once
#include <GLFW/glfw3.h>



namespace BackEnd {

    enum class WindowedMode { WINDOWED, FULLSCREEN };
    // Core
    void Init();
    void BeginFrame();
    void UpdateSubSystems();
    void EndFrame();
    void CleanUp();

    // Window
    GLFWwindow* GetWindowPtr();
    const WindowedMode& GetWindowMode();
    void SetWindowPointer(GLFWwindow* window);
    void CreateGLFWWindow(const WindowedMode& windowedMode);
    void SetWindowedMode(const WindowedMode& windowedMode);
    void ToggleFullscreen();
    void ForceCloseWindow();
    bool WindowIsOpen();
    bool WindowHasFocus();
    bool WindowHasNotBeenForceClosed();
    bool WindowIsMinimized();
    int GetWindowedWidth();
    int GetWindowedHeight();
    int GetCurrentWindowWidth();
    int GetCurrentWindowHeight();
    int GetFullScreenWidth();
    int GetFullScreenHeight();

    // Render Targets
    void SetPresentTargetSize(int width, int height);
    int GetPresentTargetWidth();
    int GetPresentTargetHeight();
}