#pragma once
#include "util.h"
#include <math.h>

vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_scale(vec2 v, f32 factor);
vec2 vec2_from_ang(f32 ang, f32 len);
vec2 vec2_transform_relative_player(vec2 v, vec2 player_pos);
ivec2 vec2_to_ivec2(vec2 v);
vec2 ivec2_to_vec2(ivec2 v);
f32 vec2_length(vec2 v);

ivec2 pos_to_screen(vec2 p, i32 s_height, i32 s_width);
ivec2 pos_to_screen_relative_rotate(vec2 point, vec2 relative_to, f32 angle, i32 screen_height, i32 screen_width);
vec2 pos_to_minimap(vec2 pos, vec2 minimap_offset, f32 minimap_size, f32 map_size);

vec2 vec2_rotate(vec2 v, f32 ang);

bool point_outside_rect(vec2 p, vec2 v0, vec2 v1);
bool rect_overlap_rect(vec2 r0, vec2 r1, vec2 r2, vec2 r3);

bool circle_intersect(vec2 c0, f32 r0, vec2 c1, f32 r1);

i32 random_between(i32 lower, i32 upper);

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CLAMP(x, lower, upper) (MIN(upper, MAX(x, lower)))
#define CLAMP_MIN(x, lower) (MAX(x, lower))
#define CLAMP_MAX(x, upper) (MIN(x, upper))

#define SIGN(x) ( (x) < 0 ? 0 : 1 )
