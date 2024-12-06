#include "player.h"
#include "math.h"

void update_player(Player* player, u64 delta_time) {
	f32 ds = (delta_time / 1000.0f);
	Input input = player->input;
	if (input.turn_left) player->ang += 5 * ds;
	if (input.turn_right) player->ang -= 5 * ds;

	if (player->input.accelerate) {
		vec2 acc = vec2_from_ang(player->ang + PI/2, player->acceleration * ds);
		player->velocity = vec2_add(player->velocity, acc);
	}
	player->pos = vec2_add(player->pos, (vec2) {player->velocity.x * ds, player->velocity.y * ds});
	player->velocity = vec2_add(player->velocity, (vec2) { -player->velocity.x * ds, -player->velocity.y * ds });

	if (player->pos.x > 1.0f) player->pos.x -= 1.77f;
	if (player->pos.x < 0.0f) player->pos.x += 1.77f;
	if (player->pos.y > 1.0f) player->pos.y -= 1.0f;
	if (player->pos.y < 0.0f) player->pos.y += 1.0f;
}