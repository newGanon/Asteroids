#include "graphic.h"
#include "math.h"

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
    vec2 bl;
    vec2 tr;
    if (v0.x < v1.x) { bl.x = v0.x; tr.x = v1.x; }
    else { bl.x = v1.x; tr.x = v0.x; }
    if (v0.y < v1.y) { bl.y = v0.y; tr.y = v1.y; }
    else { bl.y = v1.y; tr.y = v0.y; }
    if (!rect_overlap_rect(bl, tr, (vec2) { 0, 0 }, (vec2) { rb.width, rb.height })) return;

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

void draw_line_in_rect(BitMap rb, ivec2 v0, ivec2 v1, u32 color, irect rect) {
    vec2 bl;
    vec2 tr;
    if (v0.x < v1.x) { bl.x = v0.x; tr.x = v1.x; }
    else { bl.x = v1.x; tr.x = v0.x; }
    if (v0.y < v1.y) { bl.y = v0.y; tr.y = v1.y; }
    else { bl.y = v1.y; tr.y = v0.y; }
    if (!rect_overlap_rect(bl, tr, (vec2) { 0, 0 }, (vec2) { rb.width, rb.height })) return;

    f32 dx = abs(v1.x - v0.x);
    f32 sx = (v0.x < v1.x) ? 1 : -1;
    f32 dy = -abs(v1.y - v0.y);
    f32 sy = (v0.y < v1.y) ? 1 : -1;
    f32 err = dx + dy, e2;

    for (;;) {
        if (v0.x >= 0 && v0.x < rb.width && v0.y >= 0 && v0.y < rb.height && v0.x > rect.bl.x && v0.x < rect.tr.x && v0.y > rect.bl.y && v0.y < rect.tr.y) {
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

void fill_rectangle_in_rect(BitMap rb, ivec2 v0, ivec2 v1, u32 color, irect rect) {
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

    v0.x = CLAMP(v0.x, 0, rb.width - 1);
    v1.x = CLAMP(v1.x, 0, rb.width - 1);
    v0.y = CLAMP(v0.y, 0, rb.height - 1);
    v1.y = CLAMP(v1.y, 0, rb.height - 1);

    for (i32 x = v0.x; x < v1.x; x++) {
        for (i32 y = v0.y; y < v1.y; y++) {
            if (x > rect.bl.x && x < rect.tr.x && y > rect.bl.y && y < rect.tr.y) {
                rb.pixels[x + y * rb.width] = color;
            }
        }
    }
}


void draw_character(BitMap rb, BitMap font, ivec2 pos, vec2 size, const unsigned char c) {
    if (c > 255) return;
    i32 row_elements = (font.width / 8);
    i32 bx = ((i32)c % row_elements) * 8;
    i32 by = (((i32)c / row_elements)) * 8;
    for (size_t y = 0; y <= (i32)(8 * size.y); y++) {
        for (size_t x = 0; x < (i32)(8 * size.x); x++) {
            f32 c_x = (bx + (x / size.x));
            f32 c_y = (by + (y / size.y));
            u32 color = font.pixels[(i32)c_y * font.width + (i32)c_x];
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
        const unsigned char c = *string++;
        if (c == '\n') cursor.y -= size.y * 8;
        draw_character(rb, font, cursor, size, c);
        cursor.x += size.x * 8;
    }
}

static void draw_mesh_in_rect(BitMap rb, f32 p_size, WireframeMesh m, vec2 pos, f32 angle, f32 size, u32 color, irect map_rect_screen_cords) {
    ivec2 p0 = pos_to_screen_relative_rotate(vec2_scale(m.points[0], size), p_size, pos, angle, rb.height, rb.width);
    ivec2 first = p0;
    for (size_t i = 1; i < m.point_amt; i++) {
        ivec2 p1 = pos_to_screen_relative_rotate(vec2_scale(m.points[i], size), p_size, pos, angle, rb.height, rb.width);
        draw_line_in_rect(rb, p0, p1, color, map_rect_screen_cords);
        p0 = p1;
    }
    draw_line_in_rect(rb, p0, first, color, map_rect_screen_cords);
}

static void draw_mesh(BitMap rb, f32 p_size, WireframeMesh m, vec2 pos, f32 angle, f32 size, u32 color) {
    ivec2 p0 = pos_to_screen_relative_rotate(vec2_scale(m.points[0], size), p_size, pos, angle, rb.height, rb.width);
    ivec2 first = p0;
    for (size_t i = 1; i < m.point_amt; i++) {
        ivec2 p1 = pos_to_screen_relative_rotate(vec2_scale(m.points[i], size), p_size, pos, angle, rb.height, rb.width);
        draw_line(rb, p0, p1, color);
        p0 = p1;
    }
    draw_line(rb, p0, first, color);
}


void draw_player(BitMap rb, Player player, f32 map_size) {
    Entity p = player.p;
    vec2 pos_trans = vec2_transform_relative_player(p.pos, p.size, p.pos);

    // player pos if in the middle of the screen, because screenn has 16/9 resolution
    irect rel_screen_map_rect = get_screen_map_rect(map_size, p.pos, p.size, rb.height, rb.width);
    draw_mesh_in_rect(rb, player.p.size, p.mesh, pos_trans, p.ang, p.size, 0x00FFFFFF, rel_screen_map_rect);

    if (player.input.accelerate) {
        i32 r = random_between(7, 9);
        ivec2 p5 = pos_to_screen_relative_rotate((vec2) { -0.4f * p.size, 0.3f * p.size }, player.p.size, pos_trans, p.ang, rb.height, rb.width);
        ivec2 p6 = pos_to_screen_relative_rotate((vec2) { -0.4f * p.size, -0.3f * p.size }, player.p.size, pos_trans, p.ang, rb.height, rb.width);
        ivec2 p7 = pos_to_screen_relative_rotate((vec2) { (-r * 0.1f)* p.size, 0.0f * p.size }, player.p.size, pos_trans, p.ang, rb.height, rb.width);

        draw_line_in_rect(rb, p5, p7, 0x00FFFFFF, rel_screen_map_rect);
        draw_line_in_rect(rb, p6, p7, 0x00FFFFFF, rel_screen_map_rect);
    }
}


void draw_entities(BitMap rb, EntityManager manager, Player player, NetworkPlayerInfo* players_info, BitMap font, f32 map_size) {
    irect rel_screen_map_rect = get_screen_map_rect(map_size, player.p.pos, player.p.size, rb.height, rb.width);
    // draw body of the entity
    for (size_t i = 0; i < manager.entity_amt; i++)
    {
        Entity e = manager.entities[i];
        // temporarily set enitty position relative to player to create the illusion of a moving camera
        e.pos = vec2_transform_relative_player(e.pos, player.p.size ,player.p.pos);

        switch (e.type)
        {
            case BULLET: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, player.p.size, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, player.p.size, rb.height, rb.width);
                fill_rectangle_in_rect(rb, p0, p1, 0x00FFFFFF, rel_screen_map_rect);
                break;
            }
            case ASTEROID: {
                if (e.mesh.point_amt == 0) return;
                draw_mesh_in_rect(rb, player.p.size, e.mesh, e.pos, 0, e.size, 0x00FFFFFF, rel_screen_map_rect);
                break;
            }
            case PARTICLE_SQUARE: {
                ivec2 p0 = pos_to_screen((vec2) { e.pos.x - e.size, e.pos.y - e.size }, player.p.size, rb.height, rb.width);
                ivec2 p1 = pos_to_screen((vec2) { e.pos.x + e.size, e.pos.y + e.size }, player.p.size, rb.height, rb.width);
                fill_rectangle_in_rect(rb, p0, p1, 0x00FFFFFF, rel_screen_map_rect);
                break;
            }
            case PLAYER: {
                //u32 color = 0x00FFFFFF;
                u32 color = 0x003B7F3E;
                if (e.size > player.p.size) color = 0x008E1616;
                draw_mesh_in_rect(rb, player.p.size, e.mesh, e.pos, e.ang, e.size, color, rel_screen_map_rect);
                if (e.accelerating) {
                    i32 r = random_between(7, 9);
                    ivec2 p5 = pos_to_screen_relative_rotate((vec2) { -0.4f * e.size, 0.3f * e.size }, player.p.size, e.pos, e.ang, rb.height, rb.width);
                    ivec2 p6 = pos_to_screen_relative_rotate((vec2) { -0.4f * e.size, -0.3f * e.size }, player.p.size, e.pos, e.ang, rb.height, rb.width);
                    ivec2 p7 = pos_to_screen_relative_rotate((vec2) { (-r * 0.1f) * e.size, 0.0f * e.size }, player.p.size, e.pos, e.ang, rb.height, rb.width);
                     
                    draw_line_in_rect(rb, p5, p7, color, rel_screen_map_rect);
                    draw_line_in_rect(rb, p6, p7, color, rel_screen_map_rect);
                }
            }
            default: break;
        }
    }
    // draw playernames
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (!players_info[i].connected || players_info[i].dead) continue;
        Entity p;
        if (player.p.id == i) {
            p = player.p;
        } 
        else {
            i32 idx = get_entity_idx(manager, i);
            if (idx == -1) continue;
            p = manager.entities[idx];
        }
        vec2 font_size = { p.size / player.p.size * (rb.width / 1280.0f), p.size / player.p.size * (rb.height / 720.0f) };
        i32 name_length = strlen(players_info[i].name);
        f32 pixel_width = name_length * font_size.x * 8;

        vec2 offset = screen_to_pos((ivec2) {-pixel_width/2 + 2, -40 * font_size.y}, player.p.size, rb.height, rb.width);

        vec2 string_start_pos = (vec2){ p.pos.x + offset.x, p.pos.y + offset.y};
        
        draw_string(rb, font, pos_to_screen(vec2_transform_relative_player(string_start_pos, player.p.size, player.p.pos), player.p.size, rb.height, rb.width), font_size, players_info[i].name);
    }
}


void draw_outline_and_grid(BitMap rb, EntityManager manager, Player player, f32 map_size) {
    // draw grid
    i32 lines = map_size * 10;
    f32 step_size = map_size / lines;

    //horizontal gridlines
    ivec2 horizontal_start_screen, horizontal_end_screen;
    f32 x_start = CLAMP(player.p.pos.x - (1.77f / 2.0f) * (20 * player.p.size), -map_size, map_size);
    f32 x_end = CLAMP(player.p.pos.x + (1.77f / 2.0f) * (20 * player.p.size), -map_size, map_size);
    for (f32 cur_y = -map_size; cur_y < map_size; cur_y += step_size) {
        if (fabs(cur_y - player.p.pos.y) > (1.0f/2.0f) * (20 * player.p.size)) continue;
        horizontal_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_start, cur_y }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
        horizontal_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_end, cur_y }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
        draw_line(rb, horizontal_start_screen, horizontal_end_screen, 0x00252626);
    }

    //vertical gridlines
    ivec2 vertical_start_screen, vertical_end_screen;
    f32 y_start = CLAMP(player.p.pos.y - (1.0f / 2.0f) * (20 * player.p.size), -map_size, map_size);
    f32 y_end = CLAMP(player.p.pos.y + (1.0f / 2.0f) * (20 * player.p.size), -map_size, map_size);
    for (f32 cur_x = -map_size; cur_x < map_size; cur_x += step_size) {
        if (fabs(cur_x - player.p.pos.x) > (1.77f / 2.0f) * (20 * player.p.size)) continue;
        vertical_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { cur_x, y_start }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
        vertical_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { cur_x, y_end }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
        draw_line(rb, vertical_start_screen, vertical_end_screen, 0x00252626);
    }

    // bottom borderline
    horizontal_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_start, -map_size }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    horizontal_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_end, -map_size }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    draw_line(rb, horizontal_start_screen, horizontal_end_screen, 0x00FFFFFF);
    // top borderline
    horizontal_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_start, map_size }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    horizontal_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { x_end, map_size }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    draw_line(rb, horizontal_start_screen, horizontal_end_screen, 0x00FFFFFF);

    // left borderline
    horizontal_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, y_start }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    horizontal_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { -map_size, y_end }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    draw_line(rb, horizontal_start_screen, horizontal_end_screen, 0x00FFFFFF);
    // right borderline
    horizontal_start_screen = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, y_start }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    horizontal_end_screen = pos_to_screen(vec2_transform_relative_player((vec2) { map_size, y_end }, player.p.size, player.p.pos), player.p.size, rb.height, rb.width);
    draw_line(rb, horizontal_start_screen, horizontal_end_screen, 0x00FFFFFF);
}


void draw_minimap(BitMap rb, EntityManager manager, Player player, f32 map_size) {
    f32 minimap_size = 0.3;
    vec2 offset = { 0.05f, 0.95f };
    ivec2 pi0 = pos_to_screen((vec2) { offset.x, offset.y - minimap_size }, 0.05f, rb.height, rb.width);
    ivec2 pi1 = pos_to_screen((vec2) { offset.x, offset.y }, 0.05f, rb.height, rb.width);
    ivec2 pi2 = pos_to_screen((vec2) { minimap_size + offset.x, offset.y }, 0.05f, rb.height, rb.width);
    ivec2 pi3 = pos_to_screen((vec2) { minimap_size + offset.x, offset.y - minimap_size }, 0.05f, rb.height, rb.width);

    fill_rectangle(rb, pi0, pi2, 0x00000000);

    draw_line(rb, pi0, pi1, 0x00FFFFFF);
    draw_line(rb, pi1, pi2, 0x00FFFFFF);
    draw_line(rb, pi2, pi3, 0x00FFFFFF);
    draw_line(rb, pi3, pi0, 0x00FFFFFF);

    irect rel_screen_rect = {
        .bl = pos_to_screen((vec2) { offset.x, offset.y - minimap_size }, 0.05f, rb.height, rb.width),
        .tr = pos_to_screen((vec2) { offset.x + minimap_size, offset.y }, 0.05f, rb.height, rb.width),
    };

    //draw entities
    f32 minimap_scale = minimap_size / (map_size * 2.0f);
    for (size_t i = 0; i < manager.entity_amt; i++) {
        Entity e = manager.entities[i];
        switch (e.type)
        {
        case ASTEROID: {
            vec2 p0 = pos_to_minimap((vec2) { e.pos.x, e.pos.y }, offset, minimap_size, map_size);
            draw_mesh_in_rect(rb, 0.05f, e.mesh, p0, e.ang, minimap_scale * e.size, 0x00FFFFFF, rel_screen_rect);
            break;

        }
        case PLAYER: {
            vec2 p0 = pos_to_minimap((vec2) { e.pos.x, e.pos.y }, offset, minimap_size, map_size);
            draw_mesh_in_rect(rb, 0.05f, e.mesh, p0, e.ang, minimap_scale * e.size, 0x00FFFFFF, rel_screen_rect);
            break;
        }
        default: break;
        }
    }

    // draw main player 
    if (player.dead) return;
    vec2 p0 = pos_to_minimap((vec2) { player.p.pos.x, player.p.pos.y }, offset, minimap_size, map_size);
    draw_mesh_in_rect(rb, 0.05f, player.p.mesh, p0, player.p.ang, minimap_scale * 1.5f * player.p.size, 0x009C0909, rel_screen_rect);
}

void draw_scoreboard(BitMap rb, BitMap font, NetworkPlayerInfo* players_info) {
    vec2 loeaderboard_size = {0.25f, 0.5f};
    vec2 offset = {1.47, 0.95};
    ivec2 pi0 = pos_to_screen((vec2) { offset.x, offset.y - loeaderboard_size.y }, 0.05f, rb.height, rb.width);
    ivec2 pi1 = pos_to_screen((vec2) { offset.x, offset.y }, 0.05f, rb.height, rb.width);
    ivec2 pi2 = pos_to_screen((vec2) { loeaderboard_size.x + offset.x, offset.y }, 0.05f, rb.height, rb.width);
    ivec2 pi3 = pos_to_screen((vec2) { loeaderboard_size.x + offset.x, offset.y - loeaderboard_size.y }, 0.05f, rb.height, rb.width);

    fill_rectangle(rb, pi0, pi2, 0x00000000);

    draw_line(rb, pi0, pi1, 0x00FFFFFF);
    draw_line(rb, pi1, pi2, 0x00FFFFFF);
    draw_line(rb, pi2, pi3, 0x00FFFFFF);
    draw_line(rb, pi3, pi0, 0x00FFFFFF);

    vec2 font_size = { rb.width / 1280.0f, rb.height / 720.0f };
    const char* title = "SCOREBOARD";
    draw_string(rb, font, pos_to_screen((vec2){ 1.49, 0.90 }, 0.05f, rb.height, rb.width), vec2_scale(font_size, 2.0f), title);
    //draw underscore
    size_t tile_len = strlen(title);
    vec2 underline_size = { 2.0f, 1.5f };
    // draw underline, -1 because im drawing all underscor symbols twice as thick to make it consistent with very small screens
    for (size_t i = 0; i < tile_len-1; i++) {
        ivec2 pos = pos_to_screen((vec2) { 1.485, 0.88 }, 0.05f, rb.height, rb.width);
        draw_character(rb, font, (ivec2) { (i32)(pos.x + i * underline_size.x * font_size.x * 8), pos.y }, (vec2) {font_size.x * underline_size.x * 2.0f, font_size.y * underline_size.y}, 196);
    }

    vec2 cur_pos = { 1.50, 0.85 };
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (!players_info[i].connected) continue;
        // draw name
        draw_string(rb, font, pos_to_screen(cur_pos, 0.05f, rb.height, rb.width), font_size, players_info[i].name);
        // draw score
        char str[100];

        _itoa_s(players_info[i].score, str, 10, 10);
        draw_string(rb, font, pos_to_screen((vec2) { cur_pos.x + 0.15, cur_pos.y }, 0.05f, rb.height, rb.width), font_size, str);
        cur_pos.y -= 0.04;
    }
}