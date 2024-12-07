#include "player.h"
#include "math.h"

void update_player(Player* player, u64 delta_time, EntityManager* manager) {
	f32 ds = (delta_time / 1000.0f);
	Input input = player->input;
	if (input.turn_left) player->ang += 5 * ds;
	if (input.turn_right) player->ang -= 5 * ds;

	if (player->input.accelerate) {
		vec2 acc = vec2_from_ang(player->ang, player->acceleration * ds);
		player->velocity = vec2_add(player->velocity, acc);
	}
	player->pos = vec2_add(player->pos, (vec2) {player->velocity.x * ds, player->velocity.y * ds});

	vec2 slow_down = (vec2){ -player->velocity.x * ds, -player->velocity.y * ds };
	if (fabs(slow_down.x) > fabs(player->velocity.x)) {
		slow_down.x = -player->velocity.x;
	}
	if (fabs(slow_down.y) > fabs(player->velocity.y)) {
		slow_down.y = -player->velocity.y;
	}

	player->velocity = vec2_add(player->velocity, slow_down);

	if (player->pos.x > 1.0f) player->pos.x -= 1.77f;
	if (player->pos.x < 0.0f) player->pos.x += 1.77f;
	if (player->pos.y > 1.0f) player->pos.y -= 1.0f;
	if (player->pos.y < 0.0f) player->pos.y += 1.0f;

	if (player->input.shoot) {
		vec2 angle_vec2 = vec2_from_ang(player->ang, 1.0f);
		add_entity(manager, create_bullet(vec2_add(player->pos, vec2_scale(angle_vec2, 8.0 * player->size)), vec2_scale(angle_vec2, 0.8f), 0.003f));
		player->input.shoot = false;
	}
}


WireframeMesh create_player_mesh(f32 size) {
	WireframeMesh m;
	m.point_amt = 4;
	m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));
	m.points[0] = (vec2){ -5.0f * size, 5.0f * size };
	m.points[1] = (vec2){ 8.0f * size, 0 * size };
	m.points[2] = (vec2){ -5.0f * size, -5.0f * size };
	m.points[3] = (vec2){ -2.0f * size, 0 * size };
	return m;
}


void player_collisions(Player* player, EntityManager* manager) {
	for (i32 i = manager->entity_amt-1; i >= 0; i--) {
		Entity e = manager->entities[i];
		if(has_hitbox(e.type) && e.type != BULLET) {
			if (circle_intersect(player->pos, player->size * 6.0f, e.pos, e.size)) {
				player->dead = true;
				spawn_explosion(manager, player->pos, 100);
			}
		}
	}
}