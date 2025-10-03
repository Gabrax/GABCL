#pragma once

#include <Windows.h>
#include <stdint.h>

typedef struct { unsigned char r, g, b, a; } Pixel;

typedef struct {
    HWND hwnd;
    HDC hMemDC;
    HBITMAP hBitmap;
    Pixel* pixelBuffer;
    float width;
    float height;
} Window;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            Window* window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(window && window->hMemDC)
                BitBlt(hdc, 0, 0, window->width, window->height, window->hMemDC, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
        } break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

Window createWindow(int width, int height)
{
    Window win = {0};
    win.width = width;
    win.height = height;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OpenCLCube";
    RegisterClass(&wc);

    win.hwnd = CreateWindowEx(
        0,
        "OpenCLCube",
        "3D Cube Rasterizer",
        WS_OVERLAPPEDWINDOW,
        0, 0, // temporary, will move later
        width, height,
        NULL, NULL,
        wc.hInstance,
        NULL
    );

    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth  - width)  / 2;
    int y = (screenHeight - height) / 2;
    SetWindowPos(win.hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetWindowLongPtr(win.hwnd, GWLP_USERDATA, (LONG_PTR)&win);
    ShowWindow(win.hwnd, SW_SHOW);

    HDC hdc = GetDC(win.hwnd);
    win.hMemDC = CreateCompatibleDC(hdc);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    win.hBitmap = CreateDIBSection(win.hMemDC, &bmi, DIB_RGB_COLORS, (void**)&win.pixelBuffer, NULL, 0);
    SelectObject(win.hMemDC, win.hBitmap);

    return win;
}

void updateWindow(Window* win)
{
  InvalidateRect(win->hwnd,NULL,FALSE);
}

void destroyWindow(Window* win) {
    DeleteObject(win->hBitmap);
    DeleteDC(win->hMemDC);
    ReleaseDC(win->hwnd, GetDC(win->hwnd));
    DestroyWindow(win->hwnd);
}
