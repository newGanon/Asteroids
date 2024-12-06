#pragma once
#include "util.h"

typedef struct Player_s {
	vec2 pos;
	f32 velocity;
	f32 speed;
	f32 ang;
} Player;

typedef struct Input_s {
	vec2 mouse_pos;
	vec2 mouse_move;

	bool turn_left;
	bool turn_right;
	bool accelerate;
	bool shoot;
}Input;

void update_player(Player* player, Input input, u64 delta_time);