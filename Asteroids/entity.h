#pragma once
#include "math.h"

typedef enum entity_type_e {
	ASTEROID,
	BULLET,
	PARTICLE_SQUARE,
} entity_type;

typedef struct WireframeMesh_s {
	vec2* points;
	size point_amt;
} WireframeMesh;

typedef struct Entity_s {
	vec2 pos;
	vec2 vel;
	f32 size;

	entity_type type;
	//type specific varaiables
	i32 lifetime;

	WireframeMesh mesh;
} Entity;

typedef struct EntityManager_s {
	Entity* entities;
	size entity_amt;
}EntityManager;

void add_entity(EntityManager* entity_manager, Entity e);
void remove_entity(EntityManager* manager, size idx);

void update_entities(EntityManager* manager, u32 delta_time);
void entity_collisions(EntityManager* manager);

void spawn_asteroid(EntityManager* manager);
void spawn_explosion(EntityManager* manager, vec2 pos, size amt);

Entity create_asteroid(vec2 pos, vec2 vel, f32 size);
Entity create_bullet(vec2 pos, vec2 vel, f32 size);
Entity create_particle_round(vec2 pos, vec2 vel, f32 size, i32 lifetime);

WireframeMesh create_asteroid_mesh(f32 size);

void destroy_entity(EntityManager* manager, i32 idx);

bool has_hitbox(entity_type t);
