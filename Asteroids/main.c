#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <tchar.h>

#include "util.h"
#include "graphic.h"
#include "player.h"


TCHAR sz_window_class[] = _T("DesktopApp");
TCHAR sz_title[] = _T("Asteroids");
TCHAR error_string[] = _T("Call to RegisterClassEx failed!");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

// Game specific variables
static Render_Buffer render_buffer;
static bool running = true;
static Player player;
static u64 last_time;


// Windows specific variables
BITMAPINFO win32_bitmap_info;



u64 get_milliseconds() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    LARGE_INTEGER counter_now;
    QueryPerformanceCounter(&counter_now);
    return (1000LL * counter_now.QuadPart) / freq.QuadPart;
}

u64 time_since(u64 last_time) {
    u64 now = get_milliseconds();
    return now - last_time;
}

void init_render_buffer(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
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
}


void tick() {
    u64 delta_time = time_since(last_time);
    last_time += delta_time;
    update_player(&player, delta_time);
}

void render() {
    clear_screen(render_buffer, 0);
    draw_player(render_buffer, player);
}

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

    // Init Render Buffer
    init_render_buffer(hWnd);

    // Random numbers
    srand(time(NULL));

    // Timing
    last_time = get_milliseconds();

    // Game
    player.pos = (vec2){ 1.77f / 2, 0.5f };
    player.acceleration = 0.5f;
    player.velocity = (vec2){ 0 };
    player.ang = 0;

    Input input = { 0 };
    
    // Main message loop
    while (running) {
        //input
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        {
            switch (msg.message) {
                case WM_KEYUP:
                case WM_KEYDOWN: {
                    bool was_down = (msg.lParam >> 30) & 1;
                    bool is_down = ((msg.lParam >> 31) & 1) == 0;

                    switch (msg.wParam) {
                        case VK_LEFT: input.turn_left = is_down; break;
                        case VK_RIGHT: input.turn_right = is_down; break;
                        case VK_UP: input.accelerate = is_down; break;
                        case VK_SPACE: if (!was_down) input.shoot = is_down; break;
                    }
                    break;
                }
                case WM_LBUTTONDOWN: {
                    break;
                }
                case WM_LBUTTONUP: {
                    break;
                }
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        player.input = input;



        // Simulation 
        tick();
        // Rendering
        render();
        InvalidateRect(hWnd, NULL, FALSE);
    }
    return 0;
}


LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM w_param, _In_ LPARAM l_param) {
    switch (message)
    {
        case WM_SIZE: {
            if (w_param == SIZE_MAXIMIZED) init_render_buffer(hWnd);
            break;
        }
       case WM_EXITSIZEMOVE: {
           init_render_buffer(hWnd);
           KillTimer(hWnd, 0);
           break;
       }
       case WM_ENTERSIZEMOVE: {
           SetTimer(hWnd, 0, 1, NULL);
           break;
       }
       case WM_TIMER:
           tick();
           render();
           InvalidateRect(hWnd, NULL, TRUE);
           break;
                           
       case WM_PAINT: {
           PAINTSTRUCT ps;
           HDC hdc = BeginPaint(hWnd, &ps); 
           StretchDIBits(hdc, 0, 0, 
               render_buffer.width, render_buffer.height, 0, 0, 
               render_buffer.width, render_buffer.height, 
               render_buffer.pixels, &win32_bitmap_info, 
               DIB_RGB_COLORS, SRCCOPY);

           EndPaint(hWnd, &ps);
           break; 
       }
       case WM_ERASEBKGND:
           return 1;
       case WM_DESTROY:
       case WM_CLOSE: {
           PostQuitMessage(0);
           running = false;
           break;
       }
       default:
          return DefWindowProc(hWnd, message, w_param, l_param);
          break;
   }

   return 0;
}