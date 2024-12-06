#include "math.h"

vec2 vec2_add(vec2 v1, vec2 v2) { return (vec2) { v1.x + v2.x, v1.y + v2.y }; }
vec2 vec2_from_ang(f32 ang, f32 len) { return (vec2) { cos(ang) * len, sin(ang) * len}; }
vec2 ivec2_to_vec2(ivec2 v) { return (vec2) { (f32)v.x, (f32)v.y }; }

ivec2 vec2_to_ivec2(vec2 v) { return (ivec2) { (i32)v.x, (i32)v.y };}

vec2 pos_to_screen(vec2 p, i32 s_height, i32 s_width) {
	p.x *= s_width / 1.77f;
	p.y *= s_height;
	return (vec2) { p.x, p.y };
}

vec2 vec2_rotate(vec2 v, f32 ang) {
	return (vec2) {v.x * cos(ang) - v.y * sin(ang), v.x * sin(ang) + v.y * cos(ang) };
}