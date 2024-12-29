#include "graphic.h"
#include "math.h"
#include <string.h>

void clear_screen(BitMap rb) {
    memset(rb.pixels, 0, rb.width * rb.height * sizeof(u32));
}

void fill_screen(BitMap rb, u32 color) {
    for (size_t i = 0; i < rb.width; i++) {
        for (size_t j = 0; j < rb.height; j++) {
            rb.pixels[i + rb.width * j] = color;
        }
    }
}

// TODO make the line algorithm more efficient by checking of line is outside of screen and only draw the part that lies in screen
void draw_line(BitMap rb, ivec2 v0, ivec2 v1, u32 color) {
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


static void draw_mesh(BitMap rb, WireframeMesh m, vec2 pos, f32 angle, f32 size, u32 color) {
    ivec2 p0 = pos_to_screen_relative_rotate(vec2_scale(m.points[0], size), pos, angle, rb.height, rb.width);
    ivec2 first = p0;
    for (size_t i = 1; i < m.point_amt; i++) {
        ivec2 p1 = pos_to_screen_relative_rotate(vec2_scale(m.points[i], size), pos, angle, rb.height, rb.width);
        draw_line(rb, p0, p1, color);
        p0 = p1;
    }
    draw_line(rb, p0, first, color);
}


void draw_player(BitMap rb, Player player) {
    Entity p = player.p;
    // player pos if in the middle of the screen, because screenn has 16/9 resolution
    draw_mesh(rb, p.mesh, (vec2) {1.77f/2.0f, 1.0f/2.0f}, p.ang, 200.0f * p.size, 0x00FFFFFF);

    if (player.input.accelerate) {
        i32 r = random_between(7, 9);
        ivec2 p5 = pos_to_screen_relative_rotate((vec2) { -4.0f * p.size, 3.0f * p.size }, (vec2) { 1.77f / 2.0f, 1.0f/2.0f }, p.ang, rb.height, rb.width);
        ivec2 p6 = pos_to_screen_relative_rotate((vec2) { -4.0f * p.size, -3.0f * p.size }, (vec2) { 1.77f / 2.0f, 1.0f/2.0f }, p.ang, rb.height, rb.width);
        ivec2 p7 = pos_to_screen_relative_rotate((vec2) { -r * p.size, 0 * p.size }, (vec2) { 1.77f / 2.0f, 1.0f/2.0f }, p.ang, rb.height, rb.width);

        draw_line(rb, p5, p7, 0x00FFFFFF);
        draw_line(rb, p6, p7, 0x00FFFFFF);
    }
}


void fill_rectangle(BitMap rb, ivec2 v0, ivec2 v1, u32 color) {
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
            rb.pixels[x + y * rb.width] = color;
        }
    }
}


void draw_entities(BitMap rb, EntityManager manager, Player player, NetworkPlayerInfo* players_info) {
    for (size_t i = 0; i < manager.entity_amt; i++)
    {
        Entity e = manager.entities[i];
        // temporarily set enitty position relative to player to create the illusion of a moving camera
        e.pos = vec2_transform_relative_player(e.pos, player.p.pos);

        switch (e.type)
        {
            case BULLET: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, rb.height, rb.width);
                fill_rectangle(rb, p0, p1, 0x00FFFFFF);
                break;
            }
            case ASTEROID: {
                if (e.mesh.point_amt == 0) return;
                draw_mesh(rb, e.mesh, e.pos, 0, 1.0f, 0x00FFFFFF);
                break;
            }
            case PARTICLE_SQUARE: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, rb.height, rb.width);
                fill_rectangle(rb, p0, p1, 0x00FFFFFF);
                break;
            }
            case PLAYER: {
                //ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, rb.height, rb.width);
                //ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, rb.height, rb.width);
                //draw_rectangle(rb, p0, p1);

                draw_mesh(rb, e.mesh, e.pos, e.ang, 200.0f * e.size, 0x00FFFFFF);
                if (players_info[e.id].accelerate) {
                    i32 r = random_between(7, 9);
                    ivec2 p5 = pos_to_screen_relative_rotate((vec2) { -4.0f * e.size, 3.0f * e.size }, (vec2) { e.pos.x, e.pos.y }, e.ang, rb.height, rb.width);
                    ivec2 p6 = pos_to_screen_relative_rotate((vec2) { -4.0f * e.size, -3.0f * e.size }, (vec2) { e.pos.x, e.pos.y  }, e.ang, rb.height, rb.width);
                    ivec2 p7 = pos_to_screen_relative_rotate((vec2) { -r * e.size, 0 * e.size }, (vec2) { e.pos.x, e.pos.y  }, e.ang, rb.height, rb.width);

                    draw_line(rb, p5, p7, 0x00FFFFFF);
                    draw_line(rb, p6, p7, 0x00FFFFFF);
                }
            }
            default: break;
        }
    }
}


void draw_outline_and_grid(BitMap rb, EntityManager manager, Player player, f32 map_size) {

    // draw grid
    i32 lines = map_size * 10;
    f32 step_size = map_size / lines;
    for (i32 i = -(lines-1); i < lines; i++) {
        //horizontal lines
        ivec2 hs = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, i* step_size }, player.p.pos), rb.height, rb.width);
        ivec2 he = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, i* step_size }, player.p.pos), rb.height, rb.width);
        draw_line(rb, hs, he, 0x00252626);
        //vertical lines
        ivec2 vs = pos_to_screen(vec2_transform_relative_player((vec2) { i* step_size, -map_size }, player.p.pos), rb.height, rb.width);
        ivec2 ve = pos_to_screen(vec2_transform_relative_player((vec2) { i* step_size, map_size }, player.p.pos), rb.height, rb.width);
        draw_line(rb, vs, ve, 0x00252626);
    }

    // points of the outline rectangle
    ivec2 p0 = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, -map_size }, player.p.pos), rb.height, rb.width);
    ivec2 p1 = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, map_size }, player.p.pos), rb.height, rb.width);
    ivec2 p2 = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, map_size }, player.p.pos), rb.height, rb.width);
    ivec2 p3 = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, -map_size }, player.p.pos), rb.height, rb.width);

    draw_line(rb, p0, p1, 0x00FFFFFF);
    draw_line(rb, p1, p2, 0x00FFFFFF);
    draw_line(rb, p2, p3, 0x00FFFFFF);
    draw_line(rb, p3, p0, 0x00FFFFFF);

}


void draw_minimap(BitMap rb, EntityManager manager, Player player, f32 map_size) {
    f32 minimap_size = 0.3;
    vec2 offset = { 0.05f, 0.95f };
    ivec2 pi0 = pos_to_screen((vec2) { offset.x, offset.y - minimap_size }, rb.height, rb.width);
    ivec2 pi1 = pos_to_screen((vec2) { offset.x, offset.y }, rb.height, rb.width);
    ivec2 pi2 = pos_to_screen((vec2) { minimap_size + offset.x, offset.y }, rb.height, rb.width);
    ivec2 pi3 = pos_to_screen((vec2) { minimap_size + offset.x, offset.y - minimap_size }, rb.height, rb.width);

    fill_rectangle(rb, pi0, pi2, 0x00000000);

    draw_line(rb, pi0, pi1, 0x00FFFFFF);
    draw_line(rb, pi1, pi2, 0x00FFFFFF);
    draw_line(rb, pi2, pi3, 0x00FFFFFF);
    draw_line(rb, pi3, pi0, 0x00FFFFFF);

    //draw entities
    f32 minimap_scale = minimap_size / (map_size * 2.0f);
    for (size_t i = 0; i < manager.entity_amt; i++) {
        Entity e = manager.entities[i];
        switch (e.type)
        {
        case ASTEROID: {
            //ivec2 p0 = pos_to_screen(pos_to_minimap((vec2) { e.pos.x - e.size, e.pos.y - e.size }, offset, minimap_size, map_size), rb.height, rb.width);
            //ivec2 p1 = pos_to_screen(pos_to_minimap((vec2) { e.pos.x + e.size, e.pos.y + e.size }, offset, minimap_size, map_size), rb.height, rb.width);
            //fill_rectangle(rb, p0, p1, 0x00FFFFFF);

            vec2 p0 = pos_to_minimap((vec2) { e.pos.x, e.pos.y }, offset, minimap_size, map_size);
            draw_mesh(rb, e.mesh, p0, e.ang, minimap_scale, 0x00FFFFFF);
            break;

        }
        case PLAYER: {
            vec2 p0 = pos_to_minimap((vec2) { e.pos.x, e.pos.y }, offset, minimap_size, map_size);
            draw_mesh(rb, e.mesh, p0, e.ang, minimap_scale, 0x00FFFFFF);
            break;
        }
        default: break;
        }
    }

    // draw main player 
    vec2 p0 = pos_to_minimap((vec2) { player.p.pos.x, player.p.pos.y }, offset, minimap_size, map_size);
    draw_mesh(rb, player.p.mesh, p0, player.p.ang, 0.2f, 0x009C0909);

}


void draw_character(BitMap rb, BitMap font, ivec2 pos, vec2 size, const char c) {
    i32 row_elements = (font.width / 8);
    i32 bx = ((i32)c % row_elements ) * 8;
    i32 by = (((i32)c / row_elements)) * 8;
    for (size_t y = 0; y < (i32)(8 * size.y); y++) {
        for (size_t x = 0; x < (i32)(8 * size.x); x++) {
            u32 color = font.pixels[(i32)(by + (y / size.y)) * font.width + (i32)(bx + (x / size.x))];
            i32 rbx = (pos.x + x);
            i32 rby = (pos.y + y);
            if (color && rbx > 0 && rbx < rb.width && rby > 0 && rby < rb.height) {
                rb.pixels[rby * rb.width + rbx] = color;
            }
        }
    }
}

void draw_string(BitMap rb, BitMap font, ivec2 pos, vec2 size, const char* string) {
    ivec2 cursor = pos;
    while (*string) {
        const char c = *string++;
        if (c == '\n') cursor.y -= size.y * 8;
        draw_character(rb, font, cursor, size, c);
        cursor.x += size.x * 8;
    }
}


void draw_scoreboard(BitMap rb, BitMap font, NetworkPlayerInfo* players_info) {
    vec2 loeaderboard_size = {0.25f, 0.5f};
    vec2 offset = {1.47, 0.95};
    ivec2 pi0 = pos_to_screen((vec2) { offset.x, offset.y - loeaderboard_size.y }, rb.height, rb.width);
    ivec2 pi1 = pos_to_screen((vec2) { offset.x, offset.y }, rb.height, rb.width);
    ivec2 pi2 = pos_to_screen((vec2) { loeaderboard_size.x + offset.x, offset.y }, rb.height, rb.width);
    ivec2 pi3 = pos_to_screen((vec2) { loeaderboard_size.x + offset.x, offset.y - loeaderboard_size.y }, rb.height, rb.width);

    fill_rectangle(rb, pi0, pi2, 0x00000000);

    draw_line(rb, pi0, pi1, 0x00FFFFFF);
    draw_line(rb, pi1, pi2, 0x00FFFFFF);
    draw_line(rb, pi2, pi3, 0x00FFFFFF);
    draw_line(rb, pi3, pi0, 0x00FFFFFF);

    vec2 size = { rb.width / 1280.0f, rb.height / 720.0f };
    draw_string(rb, font, pos_to_screen((vec2){ 1.50, 0.90 }, rb.height, rb.width), vec2_scale(size, 2.0f), "SCOREBOARD");

    vec2 cur_pos = { 1.50, 0.85 };
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (!players_info[i].connected) continue;
        // draw name
        draw_string(rb, font, pos_to_screen(cur_pos, rb.height, rb.width), size, players_info->name);
        // draw score
        char str[10];
        _itoa_s(players_info[i].score, str, 10, 10);
        draw_string(rb, font, pos_to_screen((vec2) { cur_pos.x + 0.15, cur_pos.y }, rb.height, rb.width), size, str);
        cur_pos.y -= 0.04;
    }
}
