#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "util.h"
#include "graphics.h"


TCHAR sz_window_class[] = _T("DesktopApp");
TCHAR sz_title[] = _T("Asteroids");
TCHAR error_string[] = _T("Call to RegisterClassEx failed!");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

// Game specific variables
Render_Buffer render_buffer;
bool running = true;

// Windows specific variables
BITMAPINFO win32_bitmap_info;

/// <summary>
/// Windows program entry point
/// </summary>
/// <param name="hInstance">Handle to an instance, some windows function need it</param>
/// <param name="hPrevInstance">Not used</param>
/// <param name="pCmdLine">Command line args asa Unicode string</param>
/// <param name="nCmdShow">Flag that indicates whether the main application window is minimixed, maximized or shown normally</param>
/// <returns>int value as a status code</returns>
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Store handle
    hInst = hInstance;

    // Create window class 
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);;
    //wcex.hCursor = LoadCursor(NULL, IDC_WAIT);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = sz_window_class;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(
            NULL,
            error_string,
            sz_title,
            NULL);
        return 1;
    }


    // Create Window
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        sz_window_class,
        sz_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        MessageBox(NULL,
            error_string,
            sz_title,
            NULL);

        return 1;
    }

    // Show the window
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    HDC hdc = GetDC(hWnd);

    // Main message loop
    while (running) {
        //input
        MSG msg;
        if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //simulation
        
    }
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam) {
   switch (message)
   {
   case WM_SIZE: {
       RECT rect;
       GetWindowRect(hWnd, &rect);
       render_buffer.width = rect.right - rect.left;
       render_buffer.height = rect.bottom - rect.top;

       if (render_buffer.pixels) {
           VirtualFree(render_buffer.pixels, NULL, MEM_RELEASE);
       }
       render_buffer.pixels = (u32*)VirtualAlloc(NULL, render_buffer.width * render_buffer.height * sizeof(u32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

       win32_bitmap_info.bmiHeader.biSize = sizeof(win32_bitmap_info.bmiHeader);
       win32_bitmap_info.bmiHeader.biWidth = render_buffer.width;
       win32_bitmap_info.bmiHeader.biHeight = render_buffer.height;
       win32_bitmap_info.bmiHeader.biPlanes = 1;
       win32_bitmap_info.bmiHeader.biBitCount = 32;
       win32_bitmap_info.bmiHeader.biCompression = BI_RGB;
       break;
   }
   //render
   case WM_PAINT: {
       PAINTSTRUCT ps;
       HDC hdc = BeginPaint(hWnd, &ps);

       clear_screen(&render_buffer, 255);

       StretchDIBits(
           hdc,
           0, 0,
           render_buffer.width,
           render_buffer.height,
           0, 0,
           render_buffer.width,
           render_buffer.height,
           render_buffer.pixels,
           &win32_bitmap_info,
           DIB_RGB_COLORS,
           SRCCOPY
       );

       EndPaint(hWnd, &ps);
       break; 
   }
   case WM_DESTROY: {
       running = false;
       break;
   }
   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
   }

   return 0;
}