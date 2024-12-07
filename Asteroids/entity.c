#include "entity.h"
#include "math.h"

void add_entity(EntityManager* manager, Entity e) {
	manager->entities[manager->entity_amt++] = e;
}

void remove_entity(EntityManager* manager, size idx) {
	manager->entities[idx] = manager->entities[manager->entity_amt - 1];
	manager->entity_amt--;
}

Entity create_asteroid(vec2 pos, vec2 vel, f32 size) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = ASTEROID,
		.size = size,
		.mesh = create_asteroid_mesh(size),
	};	
}

Entity create_particle_round(vec2 pos, vec2 vel, f32 size, i32 lifetime) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = PARTICLE_SQUARE,
		.size = size,
		.lifetime = lifetime,
	};
}

Entity create_bullet(vec2 pos, vec2 vel, f32 size) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = BULLET,
		.size = 0.003f,
	};
}

WireframeMesh create_player_mesh(f32 size) {
	WireframeMesh m;
	m.point_amt = 4;
	m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));
	m.points[0] = (vec2){ -0.05f * size, 0.05f * size };
	m.points[1] = (vec2){ 0.1f * size, 0 * size };
	m.points[2] = (vec2){ -0.05f * size, -0.05f * size };
	m.points[3] = (vec2){ -0.02f * size, 0 * size };
	return m;
}


WireframeMesh create_asteroid_mesh(f32 size) {
	WireframeMesh m;
	m.point_amt = 4;
	m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));
	m.points[0] = (vec2){ -size, -size };
	m.points[1] = (vec2){ -size, size };
	m.points[2] = (vec2){ size,  size };
	m.points[3] = (vec2){ size, -size };
	return m;
}

bool has_mesh(entity_type t) {
	return t == ASTEROID;
}

bool can_collide(entity_type t0, entity_type t1) {
	return (t0 == ASTEROID && t1 == BULLET || t0 == BULLET && t1 == ASTEROID);
}

bool has_hitbox(entity_type t) {
	return t == BULLET || t == ASTEROID;
}

bool has_life_time(entity_type t) {
	return t == PARTICLE_SQUARE;
}


void destroy_entity(EntityManager* manager, i32 idx) {
	Entity e = manager->entities[idx];
	remove_entity(manager, idx);
	if (has_mesh(e.type)) {
		free(e.mesh.points);
	}
}

void update_entities(EntityManager* manager, u32 delta_time) {
	// Update movement
	f32 dt = delta_time / 1000.0f;
	for (i32 i = manager->entity_amt - 1; i >= 0; i--) {
		Entity* e = &manager->entities[i];
		if (has_life_time(e->type)) {
			e->lifetime -= delta_time;
			if (e->lifetime < 0) {
				destroy_entity(manager, i);
				continue;
			}
		}

		e->pos = vec2_add(e->pos, (vec2) { e->vel.x* dt, e->vel.y* dt });
		f32 around = e->size * 3;
		if (point_outside_rect(e->pos, (vec2) { 0 - around, 0 - around}, (vec2) { 1.77f + around, 1.0f + around})) {
			destroy_entity(manager, i);
		}
	}
}


void create_explosion(EntityManager* manager, vec2 pos, size amt) {
	for (size_t i = 0; i < amt; i++) {
		f32 ang = (random_between(0, (314 * 2)) / 100.0f);
		f32 speed = (random_between(90, 110) / 1000.0f);
		vec2 vel = vec2_from_ang(ang, speed);

		add_entity(manager, create_particle_round(pos, vel, 0.01f, 1000));
	}
}

void entity_collisions(EntityManager* manager) {
	for (i32 i = manager->entity_amt - 1; i > 0; i--) {
		Entity e0 = manager->entities[i];
		if (!has_hitbox(e0.type)) continue;
		for (i32 j = i-1; j >= 0; j--) {
			Entity e1 = manager->entities[j];
			if (!has_hitbox(e1.type)) continue;

			if (can_collide(e0.type, e1.type) && circle_intersect(e0.pos, e0.size, e1.pos, e1.size)) {
				create_explosion(manager, e0.pos, random_between(3, 5));
				destroy_entity(manager, i);
				destroy_entity(manager, j);
				i--;
				break;
			}
		}
	}
}

typedef enum side_e {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
}side;

void spawn_asteroid(EntityManager* manager) {
	vec2 pos;
	vec2 vel;
	f32 speed = 0.1f;
	f32 size = random_between(50,150) / 1000.0f;

	side s = random_between(0, 3);
	switch (s)
	{
		case TOP: {
			pos = (vec2){ random_between(0, 177) / 100.0f , 1.0f + 0.2f };
			vel = vec2_from_ang(random_between(393, 550) / 100.0f, speed);
			break;
		}
		case BOTTOM: {
			pos = (vec2){ random_between(0, 177) / 100.0f , - 0.2f };
			vel = vec2_from_ang(random_between(78, 235) / 100.0f, speed);
			break;
		}
		case LEFT: {
			pos = (vec2){ -0.2f , random_between(0, 100) / 100.0f };
			vel = vec2_from_ang(random_between(550, 707) / 100.0f, speed);
			break;
		}
		case RIGHT: {
			pos = (vec2){ 1.77f +0.2f , random_between(0, 100) / 100.0f };
			vel = vec2_from_ang(random_between(235, 393) / 100.0f, speed);
			break;
		}
		default:
			return;
	}

	add_entity(manager, create_asteroid(pos, vel, size));
}
