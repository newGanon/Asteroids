#pragma once
#include "math.h"

typedef struct WireframeMesh_s {
	vec2* points;
	size point_amt;
} WireframeMesh;


typedef enum entity_type_e {
	ASTEROID,
	BULLET
} entity_type;

typedef struct Entity_s {
	vec2 pos;
	vec2 vel;

	entity_type type;

	WireframeMesh mesh;

} Entity;

typedef struct EntityManager_s {
	Entity* entities;
	size entity_amt;
}EntityManager;



void add_entity(EntityManager* entity_manager, Entity e);
void update_entities(EntityManager* manager, u32 delta_time);