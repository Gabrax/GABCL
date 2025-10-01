#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>
#include <stdint.h>

typedef struct {
    HWND hwnd;
    HDC hdc;
    BITMAPINFO bmi;
    int width;
    int height;
} Window;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline void window_init(Window* win, const char* name, int width, int height)
{
  HINSTANCE hInstance = GetModuleHandle(NULL);

  WNDCLASS wc = { 0 };
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "MyWindowClass";
  RegisterClass(&wc);

  // Adjust for window borders
  RECT wr = { 0, 0, width, height };
  AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
  int winWidth = wr.right - wr.left;
  int winHeight = wr.bottom - wr.top;

  // Get screen dimensions
  int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

  int posX = (screenWidth  - winWidth) / 2;
  int posY = (screenHeight - winHeight) / 2;

  win->hwnd = CreateWindow(
      wc.lpszClassName,
      name,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      posX, posY, winWidth, winHeight,
      NULL, NULL, hInstance, NULL
  );

  win->hdc = GetDC(win->hwnd);
  win->width = width;
  win->height = height;

  win->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  win->bmi.bmiHeader.biWidth = width;
  win->bmi.bmiHeader.biHeight = -height; // top-down
  win->bmi.bmiHeader.biPlanes = 1;
  win->bmi.bmiHeader.biBitCount = 32;
  win->bmi.bmiHeader.biCompression = BI_RGB;
}

inline void window_render(Window* win, uint32_t* pixels)
{
    StretchDIBits(win->hdc, 0, 0, win->width, win->height, 0, 0, win->width, win->height, pixels, &win->bmi, DIB_RGB_COLORS, SRCCOPY);
}

#endif // WINDOW_H
