#pragma once
#include "util.h"
#include "entity.h"
#include "calc.h"

typedef struct Input_s {
	vec2 mouse_pos;
	vec2 mouse_move;

	bool turn_left;
	bool turn_right;
	bool accelerate;
	bool shoot;
}Input;

typedef struct Player_s {
	Entity p;
	f32 acceleration;

	bool dead;
	Input input;
} Player;

void update_player(Player* player, u64 delta_time, EntityManager* manager);
void player_collisions(Player* player, EntityManager* manager);
WireframeMesh create_player_mesh(f32 size);
