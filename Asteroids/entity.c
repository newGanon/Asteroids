#include "entity.h"
#include "math.h"

void add_entity(EntityManager* manager, Entity e) {
	manager->entities[manager->entity_amt++] = e;
}


void update_entities(EntityManager* manager, u32 delta_time) {

	f32 dt = delta_time / 1000.0f;
	for (i32 i = manager->entity_amt-1; i >= 0; i--) {
		Entity* e = &manager->entities[i];
		e->pos = vec2_add(e->pos, (vec2) {e->vel.x * dt, e->vel.y * dt});
		if (point_outside_rect(e->pos, (vec2) { 0, 0 }, (vec2) { 1.77f, 1.0f })) {
			manager->entities[i] = manager->entities[manager->entity_amt-1];
			manager->entity_amt--;
		}
	}
}
