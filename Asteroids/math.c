#include "math.h"

vec2 vec2_add(vec2 v1, vec2 v2) {
	return (vec2) { v1.x + v2.x, v1.y + v2.y };
}

vec2 vec2_from_ang(f32 ang, f32 len) {
	return (vec2) { cos(ang) * len, sin(ang) * len};
}