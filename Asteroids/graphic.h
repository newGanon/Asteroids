#pragma once
#include "util.h"
#include "player.h"
#include "entity.h"

typedef struct RenderBuffer_s {
    u32 width, height;
    u32* pixels;
} RenderBuffer;

void clear_screen(RenderBuffer rb);
void fill_screen(RenderBuffer rb, u32 color);

// Bresenham line algorithm
void draw_line(RenderBuffer rb, ivec2 v0, ivec2 v1, u32 color); 

void draw_player(RenderBuffer rb, Player player);
void draw_entities(RenderBuffer rb, EntityManager manager, Player player);
void draw_outline_and_grid(RenderBuffer rb, EntityManager manager, Player player, f32 map_size);
void draw_minimap(RenderBuffer rb, EntityManager manager, Player player, f32 map_size);

