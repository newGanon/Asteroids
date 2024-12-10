#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <tchar.h>

#include "util.h"
#include "graphic.h"
#include "player.h"
#include "entity.h"


const TCHAR sz_window_class[] = _T("DesktopApp");
const TCHAR sz_title[] = _T("Asteroids");
const TCHAR error_string[] = _T("Call to RegisterClassEx failed!");

static HINSTANCE hInst;
static HWND Wnd;

LRESULT CALLBACK WndProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

// Game specific variables
static Render_Buffer render_buffer;
static bool running = true;
static Player player;
static EntityManager entity_man;

static u64 last_time;
static u64 last_asteroid_spawn;

// Windows specific variables
static BITMAPINFO win32_bitmap_info;


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
    if (delta_time >= 500) delta_time = 500;
    // spawn asteroid every 5 seconds
    last_asteroid_spawn += delta_time;
    if(last_asteroid_spawn > 1000) {
        spawn_asteroid(&entity_man);
        last_asteroid_spawn = 0;
    }
    
    // player tick
    if (!player.dead) {
        update_player(&player, delta_time, &entity_man);
        player_collisions(&player, &entity_man);
    }

    // entity tick
    update_entities(&entity_man, delta_time);
    entity_collisions(&entity_man);
}

void render() {
    clear_screen(render_buffer, 0);
    if (!player.dead) {
        draw_player(render_buffer, player);
    }
    draw_entities(render_buffer, entity_man);
}


int init_window(_In_ HINSTANCE hInstance,
                _In_opt_ HINSTANCE hPrevInstance,
                _In_ LPWSTR lpCmdLine,
                _In_ int nShowCmd) {

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

    Wnd = hWnd;

}


int client_offline_main(_In_ HINSTANCE hInstance,
                        _In_opt_ HINSTANCE hPrevInstance,
                        _In_ LPWSTR lpCmdLine,
                        _In_ int nShowCmd) {

    init_window(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    // Random numbers
    srand(time(NULL));

    // Timing
    last_time = get_milliseconds();

    // Game
    player.pos = (vec2){ 1.77f / 2, 0.5f };
    player.acceleration = 0.5f;
    player.velocity = (vec2){ 0 };
    player.ang = 0;
    player.size = 0.004f;
    player.dead = false;
    player.mesh = create_player_mesh(player.size);

    last_asteroid_spawn = 0;

    //TODO make memory allocation for entities dynamic
    entity_man.entities = (Entity*)malloc(1000 * sizeof(Entity));

    // Main message loop
    while (running) {
        //input
        MSG msg;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch (msg.message) {
            case WM_KEYUP:
            case WM_KEYDOWN: {
                bool was_down = (msg.lParam >> 30) & 1;
                bool is_down = ((msg.lParam >> 31) & 1) == 0;

                switch (msg.wParam) {
                case VK_LEFT: player.input.turn_left = is_down; break;
                case VK_RIGHT: player.input.turn_right = is_down; break;
                case VK_UP: player.input.accelerate = is_down; break;
                case VK_SPACE:
                    if (!was_down && is_down) {
                        player.input.shoot = true;
                        break;
                    }
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

        // Simulate
        tick();

        // Render
        render();
        InvalidateRect(Wnd, NULL, FALSE);
    }
    return 0;

}


int client_online_main(_In_ HINSTANCE hInstance,
                        _In_opt_ HINSTANCE hPrevInstance,
                        _In_ LPWSTR lpCmdLine,
                        _In_ int nShowCmd) {
    init_window(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    // Random numbers
    srand(time(NULL));

    // Timing
    last_time = get_milliseconds();


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
    _In_ int nShowCmd) {

    bool offline = false;
    int ret;

    if (offline) {
        ret = client_offline_main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    }
    else {
        ret = client_online_main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    }
}


LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM w_param, _In_ LPARAM l_param) {

    //wchar_t text_buffer[20] = { 0 }; //temporary buffer
    //swprintf(text_buffer, _countof(text_buffer), L"%d", message); // convert
    //OutputDebugString(text_buffer); // print

    switch (message)
    {
        case WM_SIZE: if (!(w_param == SIZE_MAXIMIZED)) return 0; 
        case WM_EXITSIZEMOVE: {
            init_render_buffer(hWnd);
            break;
        }
                           
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