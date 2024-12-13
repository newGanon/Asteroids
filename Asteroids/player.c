#include "player.h"

void update_player(Player* player, u64 delta_time, EntityManager* manager) {
	f32 ds = (delta_time / 1000.0f);
	Input input = player->input;
	Entity* p = &player->p;

	if (input.turn_left) p->ang += 5 * ds;
	if (input.turn_right) p->ang -= 5 * ds;

	if (player->input.accelerate) {
		vec2 acc = vec2_from_ang(p->ang, player->acceleration * ds);
		p->vel = vec2_add(p->vel, acc);
	}
	p->pos = vec2_add(p->pos, (vec2) { p->vel.x * ds, p->vel.y * ds});

	vec2 slow_down = (vec2){ -p->vel.x * ds, -p->vel.y * ds };
	if (fabs(slow_down.x) > fabs(p->vel.x)) {
		slow_down.x = -p->vel.x;
	}
	if (fabs(slow_down.y) > fabs(p->vel.y)) {
		slow_down.y = -p->vel.y;
	}

	p->vel = vec2_add(p->vel, slow_down);

	if (p->pos.x > 1.0f) p->pos.x -= 1.77f;
	if (p->pos.x < 0.0f) p->pos.x += 1.77f;
	if (p->pos.y > 1.0f) p->pos.y -= 1.0f;
	if (p->pos.y < 0.0f) p->pos.y += 1.0f;
}

void player_collisions(Player* player, EntityManager* manager) {
	Entity* p = &player->p;
	for (i32 i = manager->entity_amt-1; i >= 0; i--) {
		Entity e = manager->entities[i];
		if(has_hitbox(e.type) && e.type == ASTEROID) {
			if (circle_intersect(p->pos, p->size * 6.0f, e.pos, e.size)) {
				player->dead = true;
				spawn_explosion(manager, p->pos, 200);
			}
		}
	}
}