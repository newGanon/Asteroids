#include "util.h"
#include "graphic.h"
#include "entity.h"
#include "client.h"

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <tchar.h>

// Windows specific variables
LRESULT CALLBACK WndProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

static BITMAPINFO win32_bitmap_info;
static HINSTANCE hInst;
static HWND Wnd;

static const TCHAR sz_window_class[] = _T("DesktopApp");
static const TCHAR sz_title[] = _T("Asteroids");
static const TCHAR error_string[] = _T("Call to RegisterClassEx failed!");

typedef struct GameState_s {
    bool running;
    RenderBuffer render_buffer;
    Client client;
    EntityManager entity_man;

    u64 last_time;
    f32 map_size;
}GameState;

// Game specific variables
static GameState state = { .running = true };

#define FRAMESPERSECOND 100
#define TIMEPERUPDATE 1000/FRAMESPERSECOND

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
    state.render_buffer.width = rect.right - rect.left;
    state.render_buffer.height = rect.bottom - rect.top;

    if (state.render_buffer.pixels) {
        VirtualFree(state.render_buffer.pixels, NULL, MEM_RELEASE);
    }
    state.render_buffer.pixels = (u32*)VirtualAlloc(NULL, state.render_buffer.width * state.render_buffer.height * sizeof(u32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    win32_bitmap_info.bmiHeader.biSize = sizeof(win32_bitmap_info.bmiHeader);
    win32_bitmap_info.bmiHeader.biWidth = state.render_buffer.width;
    win32_bitmap_info.bmiHeader.biHeight = state.render_buffer.height;
    win32_bitmap_info.bmiHeader.biPlanes = 1;
    win32_bitmap_info.bmiHeader.biBitCount = 32;
    win32_bitmap_info.bmiHeader.biCompression = BI_RGB;
}


void tick_player(Client* c, EntityManager* man, f32 map_size) {

    u64 delta_time = time_since(state.last_time);
    if(delta_time >= TIMEPERUPDATE) {\
        state.last_time += delta_time;
        if (!c->player.dead) update_player(c, TIMEPERUPDATE, man, map_size);
        delta_time -= TIMEPERUPDATE;
    }
}

void render(RenderBuffer rb, Player* p, EntityManager* man, f32 map_size) {
    clear_screen(rb);
    draw_outline_and_grid(rb, *man, *p, map_size);
    if (!p->dead) { draw_player(rb, *p); }
    draw_entities(rb, *man, *p);
    draw_minimap(rb, *man, *p, map_size);
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


int client_online_main(_In_ HINSTANCE hInstance,
                        _In_opt_ HINSTANCE hPrevInstance,
                        _In_ LPWSTR lpCmdLine,
                        _In_ int nShowCmd) {
    // Initialize game window
    init_window(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    // Try to connect to the server and send player state
    if (!init_client(&state.client, "27015")) {
        // TODO exit or try again;
    }   

    // Random numbers
    srand(time(NULL));

    // Timing
    state.last_time = get_milliseconds();

    // Game
    Player* p = &state.client.player;
    //p->p.pos = (vec2){ 1.77f / 2, 0.5f };
    p->p.pos = (vec2){ 0 };
    p->acceleration = 0.5f;
    p->p.vel = (vec2){ 0 };
    p->p.ang = 0;
    p->p.size = 0.004f;
    p->dead = false;
    p->p.mesh = create_entity_mesh(PLAYER, p->p.size);
    p->p.type = PLAYER;

    state.map_size = 2.0f;

    //TODO make memory allocation for entities dynamic
    state.entity_man.entities = (Entity*)malloc(1000 * sizeof(Entity));

    bool success;

    // Main message loop
    while (state.running) {
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
                case VK_LEFT:  p->input.turn_left = is_down; break;
                case VK_RIGHT:  p->input.turn_right = is_down; break;
                case VK_UP:  p->input.accelerate = is_down; break;
                case VK_SPACE:
                    if (!was_down && is_down) {
                        p->input.shoot = true;
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

        // Get messages from the server
        if(!revieve_server_messages(&state.client, &state.entity_man)) return 1;

        // Simulate
        tick_player(&state.client, &state.entity_man, state.map_size);

        // Render
        render(state.render_buffer, p, &state.entity_man, state.map_size);
        InvalidateRect(Wnd, NULL, FALSE);

        // Send messages to server
        if(!p->dead && !send_player_state_to_server(&state.client)) return 1;
    }
    return 0;
}


bool init_network() {
    WSADATA wsaData;
    return !WSAStartup(MAKEWORD(2, 2), &wsaData);
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

    init_network();

    bool offline = false;
    int ret;
    if (offline) {
        //ret = client_offline_main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    }
    else {
        ret = client_online_main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    }
    return ret;
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
                state.render_buffer.width, state.render_buffer.height, 0, 0,
                state.render_buffer.width, state.render_buffer.height,
                state.render_buffer.pixels, &win32_bitmap_info,
                DIB_RGB_COLORS, SRCCOPY);

            EndPaint(hWnd, &ps);
            break; 
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
        case WM_CLOSE: {
            PostQuitMessage(0);
            state.running = false;
            break;
        }
        default:
           return DefWindowProc(hWnd, message, w_param, l_param);
           break;
    }

    return 0;
}