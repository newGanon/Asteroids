#pragma once
#include "util.h"

typedef struct Input_s {
	vec2 mouse_pos;
	vec2 mouse_move;

	bool turn_left;
	bool turn_right;
	bool accelerate;
	bool shoot;
}Input;

typedef struct Player_s {
	vec2 pos;
	vec2 velocity;
	f32 acceleration;
	f32 ang;

	Input input;
} Player;

void update_player(Player* player, u64 delta_time);