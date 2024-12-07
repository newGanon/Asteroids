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
		Entity e = {
			.pos = player->pos,
			.vel = vec2_from_ang(player->ang, 0.5f),
			.type = BULLET,
		};
		add_entity(manager, e);
		player->input.shoot = false;
	}
}