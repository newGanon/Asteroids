#pragma once
#include "util.h"
#include "player.h"
#include "entity.h"

typedef struct BitMap_s {
    u32 width, height;
    u32* pixels;
} BitMap;

void clear_screen(BitMap rb);
void fill_screen(BitMap rb, u32 color);

// Bresenham line algorithm
void draw_line(BitMap rb, ivec2 v0, ivec2 v1, u32 color); 

void draw_player(BitMap rb, Player player);
void draw_entities(BitMap rb, EntityManager manager, Player player);
void draw_outline_and_grid(BitMap rb, EntityManager manager, Player player, f32 map_size);
void draw_minimap(BitMap rb, EntityManager manager, Player player, f32 map_size);

void draw_character(BitMap rb, BitMap font, ivec2 pos, i32 size, const char c);