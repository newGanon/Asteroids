#include "entity.h"

//start id at 100 to make space for players, as a playerid corresponds to their server socket slot
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
		.mesh = 0,
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

Entity create_bullet(vec2 pos, vec2 vel, f32 size, u32 source_id) {
	return (Entity) {
		.pos = pos,
		.vel = vel,
		.type = BULLET,
		.size = size,
		.dirty = true,
		.source_id = source_id,
		.id = id++,
	};
}


WireframeMesh create_entity_mesh(entity_type t, f32 size) {
	WireframeMesh m = {0};
	switch (t)
	{
	case PLAYER: {
		m.point_amt = 4;
		m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));
		m.points[0] = (vec2){ -0.5f, 0.5f };
		m.points[1] = (vec2){ 0.8f , 0.0f };
		m.points[2] = (vec2){ -0.5f, -0.5f };
		m.points[3] = (vec2){ -0.2f, 0.0f };
		break;
	}
	case ASTEROID: {
		m.point_amt = 6;
		m.points = (vec2*)malloc(m.point_amt * sizeof(vec2));
		for (size_t i = 0; i < 6; i++) {
			m.points[i] = vec2_from_ang((PI_2 * (2.0f / 3.0f)) * i, 1.0);
		}
	}
	default: break;
	}

	return m;
	
}

bool has_mesh(entity_type t) {
	return t == ASTEROID || t == PLAYER;
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

void update_entities(EntityManager* manager, EntityManager* queue, u32 delta_time, f32 map_size) {
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
		f32 around = 0;
		vec2 border_bl = (vec2){ -(map_size + around), -(map_size + around) };
		vec2 border_tl = (vec2){ map_size + around, map_size + around };
		vec2 entity_bl = (vec2){ e->pos.x - e->size, e->pos.y - e->size };
		vec2 entity_tl = (vec2){ e->pos.x + e->size, e->pos.y + e->size };

		if (!rect_inside_rect(entity_bl, entity_tl, border_bl, border_tl) || point_outside_rect(e->pos, border_bl, border_tl)) {
			e->dirty = true;
			e->despawn = true;
		}
	}
	// add queued entities to main entity manager
	for (size_t i = 0; i < queue->entity_amt; i++) {
		add_entity(manager, queue->entities[i]);
		queue->entities[i] = (Entity){0};
	}
	queue->entity_amt = 0;
}


void spawn_explosion(EntityManager* manager, vec2 pos, size amt) {
	for (size_t i = 0; i < amt; i++) {
		f32 ang = (random_between(0, (314 * 2)) / 100.0f);
		f32 speed = (random_between(60, 120) / 100.0f);
		vec2 vel = vec2_from_ang(ang, speed);

		add_entity(manager, create_particle_round(pos, vel, 0.001f, 200));
	}
}

void entity_collisions(EntityManager* manager, EntityManager* queue, NetworkPlayerInfo* p_info) {
	for (size_t i = 0; i < manager->entity_amt; i++) {
		Entity* e1 = &manager->entities[i];
		if (e1->despawn) continue;
		for (size_t j = 0; j < manager->entity_amt; j++) {
			Entity* e2 = &manager->entities[j];
			if (e2->despawn) break;
			switch (e1->type)
			{
			case ASTEROID: {
				switch (e2->type)
				{
				case ASTEROID: break;
				case BULLET: {
					if (circle_intersect(e1->pos, e1->size, e2->pos, e2->size)) {
						e1->dirty = true;
						e1->despawn = true;
						e2->dirty = true;
						e2->despawn = true;
						// give player that shot an asteroids points
						if (e2->source_id < MAX_CLIENTS) {
							//p_info[e2->source_id].score += (u64)(e1->size * 1000);
							p_info[e2->source_id].score += 100;
							if (!p_info[e2->source_id].dead) {
								i32 idx = get_entity_idx(*manager, e2->source_id);
								if (idx != -1) {
									manager->entities[idx].size += e1->size * 0.1f;
								}
							}
						}
						// spawn smaller asteroids if destroyed asteroid is big enough
						if (e1->size > 0.05f) {
							for (size_t i = 0; i < 2; i++) {
								f32 speed = vec2_length(e1->vel);
								f32 angle = random_between(0, 6283) / 1000.0f;
								Entity e = create_asteroid(e1->pos, vec2_from_ang(angle, speed * 1.5f), e1->size / 2.0f);
								add_entity(queue, e);
							}
						}
					}
					break;
				}
				case PLAYER: {
					if (circle_intersect(e1->pos, e1->size, e2->pos, e2->size * 0.7f)) {
						if (e2->size * 0.5f > e1->size) {
							e1->dirty = true;
							e1->despawn = true;
							//p_info[e2->source_id].score += (u64)(e1->size * 1000);
							p_info[e2->source_id].score += 100;
							e2->size += e1->size * 0.1f;
						}
						else {
							e2->dirty = true;
							e2->despawn = true;
							p_info[e2->id].dead = true;
						}
					}
					break;
				}
				default: break;
				}
				break;
			}
			case BULLET: break;
			case PLAYER: break; 
			default: break;
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


bool try_spawn_asteroid(EntityManager* manager, f32 map_size, vec2 pos, f32 size) {
	for (size_t j = 0; j < manager->entity_amt; j++) {
		Entity e = manager->entities[j];
		if (e.type == PLAYER) {
			if (circle_intersect(e.pos, e.size, pos, size * 2.0f)) {
				return false;
			}
		}
	}
	return true;
}

void spawn_asteroid(EntityManager* manager, f32 map_size) {
	vec2 pos = { 0 };
	f32 speed = 0.1f;
	vec2 vel = vec2_from_ang(random_between(0 , 6283) / 1000.0f, speed);;
	f32 size = random_between(80, 130) / 1000.0f;
	// try spawning a asteroid 100 times before giving up
	bool found = false;
	for (size_t i = 0; i < 100; i++) {
		pos = (vec2){ (random_between(-5000, 5000) / 5000.0f) * (map_size * 0.9f),  (random_between(-5000, 5000) / (5000.0f)) * (map_size * 0.9f) };
		if (try_spawn_asteroid(manager, map_size, pos, size)) {
			found = true;
			break;
		}
	}
	if(found) add_entity(manager, create_asteroid(pos, vel, size));
}