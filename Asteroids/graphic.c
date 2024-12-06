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
        rb.pixels[rb.width * v0.y + v0.x] = color;
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
    vec2 ahead;
    ahead.x = -50.0 * sin(player.ang);
    ahead.y = 50 * cos(player.ang);

    draw_line(rb, (ivec2) { player.pos.x, player.pos.y }, (ivec2) { player.pos.x + ahead.x, player.pos.y + ahead.y }, 0x00FF0000);
}