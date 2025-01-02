#include "calc.h"

vec2 vec2_add(vec2 v1, vec2 v2) { return (vec2) { v1.x + v2.x, v1.y + v2.y }; }
vec2 vec2_sub(vec2 v1, vec2 v2) { return (vec2) { v1.x - v2.x, v1.y - v2.y }; }
vec2 vec2_scale(vec2 v, f32 factor) {return (vec2) { v.x * factor, v.y * factor};}
vec2 vec2_from_ang(f32 ang, f32 len) { return (vec2) { cos(ang) * len, sin(ang) * len}; }
vec2 vec2_transform_relative_player(vec2 v, f32 p_size, vec2 player_pos) { return (vec2){ v.x - (player_pos.x - (1.77f * (20.0f * p_size)/ 2.0f)), v.y - (player_pos.y - (1.0f * (20.0f * p_size) / 2.0f)) }; }
f32 vec2_length(vec2 v) { return sqrt(v.x * v.x + v.y * v.y); }

ivec2 ivec2_add(ivec2 v1, ivec2 v2) { return (ivec2) { v1.x + v2.x, v1.y + v2.y }; }
ivec2 ivec2_sub(ivec2 v1, ivec2 v2) { return (ivec2) { v1.x - v2.x, v1.y - v2.y }; }

vec2 ivec2_to_vec2(ivec2 v) { return (vec2) { (f32)v.x, (f32)v.y }; }
ivec2 vec2_to_ivec2(vec2 v) { return (ivec2) { (i32)v.x, (i32)v.y }; }

vec2 pos_to_minimap(vec2 pos, vec2 minimap_offset, f32 minimap_size, f32 map_size) {
    pos.x += map_size;
    pos.y += map_size;
    f32 minimap_scale = minimap_size / (map_size * 2.0f);
    vec2 minimap_pos = vec2_scale(pos, minimap_scale);
    minimap_pos.x += minimap_offset.x;
    minimap_pos.y = minimap_offset.y - (minimap_size - minimap_pos.y);
    return minimap_pos;
}

vec2 vec2_rotate(vec2 v, f32 ang) {
	return (vec2) {v.x * cos(ang) - v.y * sin(ang), v.x * sin(ang) + v.y * cos(ang) };
}

ivec2 pos_to_screen(vec2 p, f32 p_size, i32 screen_height, i32 screen_width) {
    // player should be able to see more if he's bigger
    f32 size_scale = 20.0f * p_size;
	p.x *= screen_width / (1.77f * size_scale);
	p.y *= screen_height / size_scale;
	return (ivec2) { (i32)p.x, (i32)p.y };
}

vec2 screen_to_pos(ivec2 p, f32 p_size, i32 screen_height, i32 screen_width) {
    f32 size_scale = 20.0f * p_size;
    vec2 ret;
    ret.x = ((f32)p.x / screen_width * 1.77f * size_scale);
    ret.y = ((f32)p.y / screen_height * size_scale);
    return ret;
}

ivec2 pos_to_screen_relative_rotate( vec2 point, f32 p_size, vec2 relative_to, f32 angle, i32 screen_height, i32 screen_width ) {
	return pos_to_screen(vec2_add(vec2_rotate(point, angle), relative_to), p_size, screen_height, screen_width);
}

vec2 screen_to_pos_relative(ivec2 point, f32 p_size, ivec2 relative_to, f32 angle, i32 screen_height, i32 screen_width) {
    return screen_to_pos(ivec2_add(point, relative_to), p_size, screen_height, screen_width);
}

bool point_outside_rect(vec2 p, vec2 r0, vec2 r1) {
    if (r0.x > r1.x) {
        f32 tmp = r0.x;
        r0.x = r1.x;
        r1.x = tmp;
    }
    if (r0.y > r1.y) {
        f32 tmp = r0.y;
        r0.y = r1.y;
        r1.y = tmp;
    }
    return (p.x < r0.x || p.x > r1.x || p.y < r0.y || p.y > r1.y);
}


bool rect_overlap_rect(vec2 r0, vec2 r1, vec2 r2, vec2 r3) {
    return (r0.x < r3.x && r1.x > r2.x && r0.y < r3.y && r1.y > r2.y);
}

bool rect_inside_rect(vec2 r0, vec2 r1, vec2 r2, vec2 r3) {
    return (r0.x < r3.x && r1.x > r2.x && r0.y < r3.y && r1.y > r2.y && (r1.x < r3.x && r0.x > r2.x && r1.y < r3.y && r0.y > r2.y));
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

irect get_screen_map_rect(f32 map_size, vec2 p_pos, f32 p_size, f32 rb_height, f32 rb_width) {
    ivec2 bl = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, -map_size }, p_size, p_pos), p_size, rb_height, rb_width);
    ivec2 tr = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, map_size }, p_size, p_pos), p_size, rb_height, rb_width);
    return (irect) { bl, tr };
}