#include "calc.h"

vec2 vec2_add(vec2 v1, vec2 v2) { return (vec2) { v1.x + v2.x, v1.y + v2.y }; }
vec2 vec2_scale(vec2 v, f32 factor) {return (vec2) { v.x * factor, v.y * factor};}
vec2 vec2_from_ang(f32 ang, f32 len) { return (vec2) { cos(ang) * len, sin(ang) * len}; }
vec2 ivec2_to_vec2(ivec2 v) { return (vec2) { (f32)v.x, (f32)v.y }; }

ivec2 vec2_to_ivec2(vec2 v) { return (ivec2) { (i32)v.x, (i32)v.y };}

f32 vec2_length(vec2 v) { return sqrt(v.x * v.x + v.y * v.y); }

vec2 vec2_rotate(vec2 v, f32 ang) {
	return (vec2) {v.x * cos(ang) - v.y * sin(ang), v.x * sin(ang) + v.y * cos(ang) };
}

ivec2 pos_to_screen(vec2 p, i32 screen_height, i32 screen_width) {
	p.x *= screen_width / 1.77f;
	p.y *= screen_height;
	return (ivec2) { (i32)p.x, (i32)p.y };
}

ivec2 pos_to_screen_relative_rotate( vec2 point, vec2 relative_to, f32 angle, i32 screen_height, i32 screen_width ) {
	return pos_to_screen(vec2_add(vec2_rotate(point, angle), relative_to), screen_height, screen_width);
}


bool point_outside_rect(vec2 p, vec2 v0, vec2 v1) {
    if (v0.x > v1.x) {
        f32 tmp = v0.x;
        v0.x = v1.x;
        v1.x = tmp;
    }
    if (v0.y > v1.y) {
        f32 tmp = v0.y;
        v0.y = v1.y;
        v1.y = tmp;
    }
    return (p.x < v0.x || p.x > v1.x || p.y < v0.y || p.y > v1.y);
}

f32 circle_distance(vec2 c0, vec2 c1) {
    return sqrtf(powf(c0.x-c1.x, 2) + powf(c0.y - c1.y, 2));
}

bool circle_intersect(vec2 c0, f32 r0, vec2 c1, f32 r1) {
    return (circle_distance(c0, c1) < (r0 + r1));
}

i32 random_between(i32 lower, i32 upper) {
    return (rand() % (upper - lower + 1)) + lower;
}