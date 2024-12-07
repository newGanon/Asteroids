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


static void draw_mesh(Render_Buffer rb, WireframeMesh m, vec2 pos, f32 angle) {
    ivec2 p0 = pos_to_screen_relative_rotate(m.points[0], pos, angle, rb.height, rb.width);
    ivec2 first = p0;
    for (size_t i = 1; i < m.point_amt; i++) {
        ivec2 p1 = pos_to_screen_relative_rotate(m.points[i], pos, angle, rb.height, rb.width);
        draw_line(rb, p0, p1, 0x00FFFFFF);
        p0 = p1;
    }
    draw_line(rb, p0, first, 0x00FFFFFF);
}



void draw_player(Render_Buffer rb, Player player) {

    draw_mesh(rb, player.mesh, player.pos, player.ang);

    if (player.input.accelerate) {
        f32 size = 0.4;
        i32 r = random_between(7, 9);
        ivec2 p5 = pos_to_screen_relative_rotate( (vec2) { -0.04f * size,  0.03f * size }, player.pos, player.ang, rb.height, rb.width);
        ivec2 p6 = pos_to_screen_relative_rotate( (vec2) { -0.04f * size, -0.03f * size }, player.pos, player.ang, rb.height, rb.width);
        ivec2 p7 = pos_to_screen_relative_rotate( (vec2) { -(r * 0.01f) * size, 0 * size }, player.pos, player.ang, rb.height, rb.width);

        draw_line(rb, p5, p7, 0x00FFFFFF);
        draw_line(rb, p6, p7, 0x00FFFFFF);
    }
}


void draw_rectangle(Render_Buffer rb, ivec2 v0, ivec2 v1) {
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

    v0.x = CLAMP(v0.x, 0, rb.width-1);
    v1.x = CLAMP(v1.x, 0, rb.width-1);
    v0.y = CLAMP(v0.y, 0, rb.height-1);
    v1.y = CLAMP(v1.y, 0, rb.height-1);

    for (size_t x = v0.x; x < v1.x; x++) {
        for (size_t y = v0.y; y < v1.y; y++) {
            rb.pixels[x + y * rb.width] = 0x00FFFFFF;
        }
    }
}


void draw_entities(Render_Buffer rb, EntityManager manager) {
    for (size_t i = 0; i < manager.entity_amt; i++)
    {
        Entity e = manager.entities[i];

        switch (e.type)
        {
            case BULLET: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, rb.height, rb.width);
                draw_rectangle(rb, p0, p1);
                break;
            }
            case ASTEROID: {
                if (e.mesh.point_amt == 0) return;
                draw_mesh(rb, e.mesh, e.pos, 0);
                break;
            }
            case PARTICLE_SQUARE: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, rb.height, rb.width);
                draw_rectangle(rb, p0, p1);
                break;
            }
            default:
                break;
        }
    }

}