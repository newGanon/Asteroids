#pragma once
#include "util.h"

typedef struct Render_Buffer_s {
    u32 width, height;
    u32* pixels;
} Render_Buffer;

void clear_screen(Render_Buffer* rb, u32 color);