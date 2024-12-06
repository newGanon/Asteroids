#include "graphic.h"
#include "math.h"

void clear_screen(Render_Buffer rb, u32 color) {
	for (size_t i = 0; i < rb.width; i++) {
		for (size_t j = 0; j < rb.height; j++) {
			rb.pixels[i + rb.width * j] = color;
		}
	}
}

void draw_line(Render_Buffer rb, ivec2 v0, ivec2 v1, u32 color) {
    f32 dx = abs(v1.x - v0.x);
    f32 sx = (v0.x < v1.x) ? 1 : -1;
    f32 dy = -abs(v1.y - v0.y);
    f32 sy = (v0.y < v1.y) ? 1 : -1;
    f32 err = dx + dy, e2;

    for (;;) {
        if (v0.x >= 0 && v0.x < rb.width && v0.y >= 0 && v0.y < rb.height) {
            rb.pixels[rb.width * v0.y + v0.x] = color;
        }
        if (v0.x == v1.x && v0.y == v1.y) break;
        e2 = err * 2;
        if (e2 >= dy) {
            err += dy;
            v0.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            v0.y += sy;
        }
    }
}


void draw_player(Render_Buffer rb, Player player) {
    vec2 ahead = { 0.1 * cos(player.ang), 0.1 * sin(player.ang)};

    //draw player
    vec2 pp = player.pos;

    f32 size = 0.4f;

    ivec2 p1 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { -0.05f * size, - 0.05f * size }, player.ang), pp), rb.height, rb.width));
    ivec2 p2 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { 0,              + 0.1f  * size }, player.ang), pp), rb.height, rb.width));
    ivec2 p3 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { 0.05f * size,   - 0.05f * size }, player.ang), pp), rb.height, rb.width));
    ivec2 p4 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { 0,              - 0.02f * size }, player.ang), pp), rb.height, rb.width));

    draw_line(rb, p1, p2, 0x00FFFFFF);
    draw_line(rb, p2, p3, 0x00FFFFFF);
    draw_line(rb, p3, p4, 0x00FFFFFF);
    draw_line(rb, p4, p1, 0x00FFFFFF);

    if (player.input.accelerate) {
        i32 r = (rand() % (9 - 7 + 1)) + 7;
        ivec2 p5 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { -0.03f * size, -0.04f * size }, player.ang), pp), rb.height, rb.width));
        ivec2 p6 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { +0.03f * size, -0.04f * size }, player.ang), pp), rb.height, rb.width));
        ivec2 p7 = vec2_to_ivec2(pos_to_screen(vec2_add(vec2_rotate((vec2) { 0,       -(r * 0.01f) * size }, player.ang), pp), rb.height, rb.width));

        draw_line(rb, p5, p7, 0x00FFFFFF);
        draw_line(rb, p6, p7, 0x00FFFFFF);
    }
}