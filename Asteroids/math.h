#pragma once
#include "util.h"
#include <math.h>

vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_from_ang(f32 ang, f32 len);
ivec2 vec2_to_ivec2(vec2 v);

vec2 ivec2_to_vec2(ivec2 v);

vec2 pos_to_screen(vec2 p, i32 s_height, i32 s_width);
vec2 vec2_rotate(vec2 v, f32 ang);

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
