#pragma once
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t size;

#define PI 3.14159265358979323846
#define PI_2 PI/2
#define PI_4 PI/4

#define MAX_NAME_LENGTH 16
#define MAX_CLIENTS 10
#define ASTEROIDSPAWNTIME 1000
#define PLAYERDEATHTIME 1000

typedef struct vec2_s {
	f32 x, y;
}vec2;

typedef struct ivec2_s {
	i32 x, y;
}ivec2;

typedef struct rect_s {
	vec2 bl, tr;
}rect;

typedef struct irect_s {
	ivec2 bl, tr;
}irect;