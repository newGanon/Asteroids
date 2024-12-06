#pragma once
#include "util.h"
#include "player.h"

typedef struct Render_Buffer_s {
    u32 width, height;
    u32* pixels;
} Render_Buffer;

void clear_screen(Render_Buffer rb, u32 color);

// Bresenham line algorithm
void draw_line(Render_Buffer rb, ivec2 v0, ivec2 v1, u32 color); 

void draw_player(Render_Buffer rb, Player player);