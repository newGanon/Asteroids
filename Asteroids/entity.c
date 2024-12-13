#include "entity.h"

//start id at 100 to make space for players, as players a player id is their socket slot
static u64 id = 100;


void add_entity(EntityManager* manager, Entity e) {
	manager->entities[manager->entity_amt++] = e;
}

void remove_entity(EntityManager* manager, size idx) {
	manager->entities[idx] = manager->entities[manager->entity_amt - 1];
	manager->entity_amt--;
}

void overwrite_entity_idx(EntityManager* manager, Entity e, size idx) {
	manager->entities[idx] = e;
}


i32 get_entity_idx(EntityManager manager, u32 id) {
	for (size_t i = 0; i < manager.entity_amt; i++) {
		// found player already in entity array
		if (manager.entities[i].id == id) {
			return i;
		}
	}
	return -1;
}


Entity create_asteroid(vec2 pos, vec2 vel, f32 size) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = ASTEROID,
		.size = size,
		.mesh = create_asteroid_mesh(size),
		.dirty = true,
		.id = id++,
	};	
}

Entity create_particle_round(vec2 pos, vec2 vel, f32 size, i32 lifetime) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = PARTICLE_SQUARE,
		.size = size,
		.lifetime = lifetime,
		.dirty = true,
		.id = id++,
	};
}

Entity create_bullet(vec2 pos, vec2 vel, f32 size) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = BULLET,
		.size = 0.003f,
		.dirty = true,
		.id = id++,
	};
}

WireframeMesh create_asteroid_mesh(f32 size) {
	WireframeMesh m;
	m.point_amt = 6;
	m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));

	for (size_t i = 0; i < 6; i++){
		m.points[i] = vec2_from_ang((PI_2 * (2.0f/3.0f)) * i, size);
	}
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
	for (i32 i = 0; i < manager->entity_amt; i++) {
		Entity* e = &manager->entities[i];
		if (e->type == PLAYER) continue;
		if (has_life_time(e->type)) {
			e->lifetime -= delta_time;
			if (e->lifetime < 0) {
				e->dirty = true;
				e->despawn = true;
				continue;
			}
		}
		e->pos = vec2_add(e->pos, (vec2) { e->vel.x* dt, e->vel.y* dt });
		e->dirty = true;
		f32 around = e->size * 3;
		if (point_outside_rect(e->pos, (vec2) { 0 - around, 0 - around}, (vec2) { 1.77f + around, 1.0f + around})) {
			e->dirty = true;
			e->despawn = true;
		}
	}
}


void spawn_explosion(EntityManager* manager, vec2 pos, size amt) {
	for (size_t i = 0; i < amt; i++) {
		f32 ang = (random_between(0, (314 * 2)) / 100.0f);
		f32 speed = (random_between(60, 120) / 100.0f);
		vec2 vel = vec2_from_ang(ang, speed);

		add_entity(manager, create_particle_round(pos, vel, 0.001f, 200));
	}
}

void entity_collisions(EntityManager* manager) {
	// TODO: split if size is bigger than 0.06
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
	f32 size = random_between(80, 130) / 1000.0f;

	f32 angle_const = 78 * 0.3f;

	side s = random_between(0, 3);
	switch (s)
	{
		case TOP: {
			pos = (vec2){ random_between(0, 177) / 100.0f , 1.0f + 0.2f };
			vel = vec2_from_ang(random_between(393 + angle_const, 550 - angle_const) / 100.0f, speed);
			break;
		}
		case BOTTOM: {
			pos = (vec2){ random_between(0, 177) / 100.0f , - 0.2f };
			vel = vec2_from_ang(random_between(78 + angle_const, 235 - angle_const) / 100.0f, speed);
			break;
		}
		case LEFT: {
			pos = (vec2){ -0.2f , random_between(0, 100) / 100.0f };
			vel = vec2_from_ang(random_between(550 + angle_const, 707 - angle_const) / 100.0f, speed);
			break;
		}
		case RIGHT: {
			pos = (vec2){ 1.77f +0.2f , random_between(0, 100) / 100.0f };
			vel = vec2_from_ang(random_between(235 + angle_const, 393 - angle_const) / 100.0f, speed);
			break;
		}
		default:
			return;
	}

	add_entity(manager, create_asteroid(pos, vel, size));
}