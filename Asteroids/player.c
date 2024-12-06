#include "player.h"
#include "math.h"

void update_player(Player* player, Input input, u64 delta_time) {
	if (input.accelerate) player->velocity = 1.0;
	else player->velocity = 0.0;
	if (input.turn_left) player->ang += delta_time / 200.0f;
	if (input.turn_right) player->ang -= delta_time / 200.0f;

	player->pos = vec2_add(player->pos, vec2_from_ang(player->ang + PI_2, player->velocity));
}