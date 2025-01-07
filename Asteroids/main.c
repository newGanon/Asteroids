#include "util.h"
#include "gui.h"
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
static HWND global_window;

static const TCHAR sz_window_class[] = _T("DesktopApp");
static const TCHAR sz_title[] = _T("Asteroids");
static const TCHAR error_string[] = _T("Call to RegisterClassEx failed!");

typedef struct GameState_s {
    bool running;
    Client client;
    char host[MAX_HOSTNAME_LENGTH + 1];
    char port[MAX_PORT_LENGTH + 1];
    EntityManager entity_man;
    NetworkPlayerInfo players[MAX_CLIENTS];

    BitMap render_buffer;
    BitMap font;

    u64 last_time;
    f32 map_size;
}GameState;

static i32 gui_element_focused = -1;

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
    if (delta_time >= TIMEPERUPDATE) {
        state.last_time += delta_time;
        if (!c->player.dead) update_player(&c->player, TIMEPERUPDATE, man, map_size);
        delta_time -= TIMEPERUPDATE;
    }
}

void render(BitMap rb, BitMap font, Player* p, EntityManager* man, f32 map_size, NetworkPlayerInfo* p_info) {
    clear_screen(rb);
    draw_outline_and_grid(rb, *man, *p, map_size);
    if (!p->dead) { draw_player(rb, *p, map_size); }
    draw_entities(rb, font, *man, *p, p_info, map_size);
    draw_minimap(rb, *man, *p, map_size);
    draw_scoreboard(rb, font, p_info);

    InvalidateRect(global_window, NULL, FALSE);
}

u64 connection_timer = 0;
void render_connecting_screen(BitMap rb, BitMap font, u64 dt) {
    clear_screen(rb);
    char str[100];
    memset(str, 0, 100);
    strcpy(str, "CONNECTING");
    connection_timer += dt;
    if (connection_timer >= 4000) connection_timer -= 4000;
    if (connection_timer >= 1000) str[strnlen(str, 98)] = '.';
    if (connection_timer >= 2000) str[strnlen(str, 98)] = '.';
    if (connection_timer >= 3000) str[strnlen(str, 98)] = '.';
    vec2 font_size = { (rb.width / 1280.0f) * 5.0f, (rb.height / 720.0f) * 5.0f };
    irect screen_rect = { (ivec2) { 0, 0 }, (ivec2) { rb.width, rb.height } };
    draw_string(rb, font, (ivec2) { rb.width/2.0f - font_size.x * (5 * 8), rb.height / 2.0f + font_size.y * 8/2}, (vec2) { font_size.x, font_size.y }, str, 0x00FFFFFF, screen_rect);

    InvalidateRect(global_window, NULL, FALSE);
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

    global_window = hWnd;
    return 0;
}


int load_resources(GameState* s) {
    // load font
    BitMap* font = &s->font;

    HBITMAP bm_handle = LoadImage(NULL, L"font.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (!bm_handle) return 1;

    BITMAP bmp;
    GetObject(bm_handle, sizeof(BITMAP), &bmp);

    font->pixels = (u32*)malloc(sizeof(u32) * bmp.bmHeight * bmp.bmWidth);
    font->height = bmp.bmHeight;
    font->width = bmp.bmWidth;

    BITMAPINFO bm_info = {
        .bmiHeader = {
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth = bmp.bmWidth,
            .biHeight = bmp.bmHeight,
            .biPlanes = 1,
            .biBitCount = 32,
            .biCompression = BI_RGB,
        }
    };
    HDC hdc = GetDC(global_window);
    GetDIBits(hdc, bm_handle, 0, bmp.bmHeight, font->pixels, &bm_info, DIB_RGB_COLORS);
    if (!font->pixels) return 1;
    ReleaseDC(global_window, hdc);
    DeleteObject(bm_handle);
    return 0;
}

int connect_to_server(const char* host, const char* port) {
    // Try to connect to the server and send player state
    if (!init_client(&state.client, host, port)) return 1;
    u64 retry_connection_timer = 0;
    while (state.running && !check_connection_status(state.client.socket.connection.sock)) {
        // get windows messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // handle connection
        u64 dt = time_since(state.last_time);
        state.last_time += dt;
        render_connecting_screen(state.render_buffer, state.font, dt);
        // enought time has passed, close old connection socked and retry connection
        retry_connection_timer += dt;
        if (retry_connection_timer >= 1000) {
            retry_connection_timer = 0;
            closesocket(state.client.socket.connection.sock);
            if (!init_client(&state.client, host, port)) return 1;
        }
    }
    return 0;
}

int init_game(char* name) {
    Player* p = &state.client.player;
    p->dead = true;
    p->acceleration = 0.5f;
    p->p.mesh = create_entity_mesh(PLAYER, p->p.size);

    state.map_size = 2.0f;

    //TODO make memory allocation for entities dynamic
    state.entity_man.entities = (Entity*)malloc(1000 * sizeof(Entity));

    send_client_connect(&state.client, name);
    //wait for player info from server
    while (state.client.player.p.size == 0.0f && state.client.player.dead) {
        if (!recieve_server_messages(&state.client, &state.entity_man, &state.players)) return 1;
    }
    return 0;
}

char get_ascii_from_vk(char vk, bool shift_pressed) {

    if ('0' <= vk && vk <= '9') return vk;
    if ('A' <= vk && vk <= 'Z') {
        if (shift_pressed) return vk;
        else return vk + 32;
    }
    if (vk == VK_BACK) return 8;
    return -1;
}

int main_menu(char* playername, char* host, char* port){
    
    bool confirmed = false;
    // mouse controls
    bool lmb_down = false;
    bool lmb_was_down = false;
    ivec2 mouse_pos = { -1, -1 };
    // gui elements
    TextBox inputs[3];
    char header_strings[3][50] = { "Player Name", "Host", "Port" };
    char default_textbox_values[3][50] = { "Insert Name", "Insert Host", "Insert Port" };
    char predefined_textbox_values[3][50] = { "", "localhost", "27015" };
    for (size_t i = 0; i < 3; i++) {
        inputs[i] = (TextBox){.id = i, .pos_rect = {.bl = (vec2) { 0.5f, 0.8f - (i * 0.2f)}, .tr = (vec2) {1.4f, 0.9f - (i * 0.2f)}}};
        strcpy(inputs[i].header_string, header_strings[i]);
        strcpy(inputs[i].input_text, predefined_textbox_values[i]);
        inputs[i].input_text_len = strlen(predefined_textbox_values[i]);
        inputs[i].cursor_pos = inputs[i].input_text_len;
        strcpy(inputs[i].default_text, default_textbox_values[i]);
    }
    Button connect_button = { .id = 4, .default_text = "CONNECT", .pos_rect = {.bl = (vec2) { 0.5f, 0.1f }, .tr = (vec2) { 1.4f, 0.2f } } };
    // TODO: make a queue for all keys pressed so no keys get lost
    char key_pressed = -1;

    while (state.running) {
        key_pressed = -1;
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
            case WM_KEYDOWN: {
                bool was_down = (msg.lParam >> 30) & 1;
                bool is_down = ((msg.lParam >> 31) & 1) == 0;
                // enter key
                if (msg.wParam == VK_RETURN && !was_down && is_down) { confirmed = true; }
                // other key
                else {
                    bool uppercase = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                    bool caps = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
                    key_pressed = get_ascii_from_vk(msg.wParam, uppercase != caps);
                }
                break;
            }
            case WM_MOUSEMOVE: {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(global_window, &p);
                mouse_pos.x = p.x;
                mouse_pos.y = state.render_buffer.height - p.y;
                break;
            }
            case WM_LBUTTONDOWN: {
                lmb_down = true;
                break;
            }
            case WM_LBUTTONUP: {
                lmb_down = false;
                break;
            }
            default: {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            }
        }
        // time for animating cursor
        u64 dt = time_since(state.last_time);
        state.last_time += dt;
        // update gui elements
        BitMap rb = state.render_buffer;
        bool updated = false;
        // if the user let go of the left mouse button over the connect button, then start connecting the player
        if (confirmed && lmb_was_down && !lmb_down) break;
        // button
        if (button_update(rb, &connect_button, mouse_pos, lmb_down, lmb_was_down, &gui_element_focused)) {
            confirmed = true;
            updated |= true;
        }
        // update text boxes
        for (size_t i = 0; i < 3; i++) { updated |= textbox_update(rb, &inputs[i], mouse_pos, lmb_down, &gui_element_focused, key_pressed); }

        // if user clicked on the screen but not on a gui element, unfocus
        if (lmb_down && !updated) gui_element_focused = -1;
        //  check if name has a value
        if (confirmed && inputs[0].input_text_len == 0) confirmed = false;
        // save last state of the left mouse button, so buttons can be activated when youi let go of your mouse
        lmb_was_down = lmb_down;

        // render gui elements
        clear_screen(rb);
        for (size_t i = 0; i < 3; i++) textbox_render(rb, state.font, inputs[i], lmb_down, gui_element_focused, dt);
        button_render(rb, state.font, connect_button, lmb_down, gui_element_focused);
        InvalidateRect(global_window, NULL, FALSE);
    }

    strcpy(playername, inputs[0].input_text, inputs[0].input_text_len);
    strcpy(host, inputs[1].input_text, inputs[1].input_text_len);
    strcpy(port, inputs[2].input_text, inputs[2].input_text_len);
    return 0;
}

int client_online_main() {
    //init time
    state.last_time = get_milliseconds();
    //draw the main menu where the client is asked for host, port and playername, if 1 is returned an error has occured
    if (state.running && main_menu(state.client.player.name, state.host, state.port)) return 1;

    // Establish connection to the server, if connect_to_server returns 1, an error has occured
    if (state.running && connect_to_server(state.host, state.port)) return 1;

    // intialize all game varaibles and wait for initial information from server, if init_game returns 1, an error has occured
    //char name[16] = { "TomTomTomTomTom" };
    if (state.running && init_game(state.client.player.name)) return 1;

    Player* p = &state.client.player;
    // Main message loop
    while (state.running) {
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
        if(!recieve_server_messages(&state.client, &state.entity_man, &state.players)) return 1;

        // Simulate
        tick_player(&state.client, &state.entity_man, state.map_size);

        // Render
        render(state.render_buffer, state.font, p, &state.entity_man, state.map_size, state.players);

        // Send messages to server
        if(!p->dead && !send_player_state_to_server(&state.client)) return 1;
    }
    return 0;
}

int init_network() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
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

    // Initialize the network 
    if(init_network()) return 1;

    // Initialize game window
    if(init_window(hInstance, hPrevInstance, lpCmdLine, nShowCmd)) return 1;

    // load resources
    if(load_resources(&state)) return 1;

    // Random numbers
    srand(time(NULL));

    // Timing
    state.last_time = get_milliseconds();

    bool offline = false;
    int ret = -1;
    if (offline) {
        // TODO: start the server in another thread than the client, so it appears that client plays offline
        ret = 0;
    }
    else {
        while (ret != 0) {
            ret = client_online_main();
            memset(&state.players, 0, sizeof(NetworkPlayerInfo) * MAX_CLIENTS);
        }
    }
    // error occured close the window and handle the error
    if (ret == 1) {
        PostMessage(global_window, WM_CLOSE, 0, 0);
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