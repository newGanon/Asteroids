#pragma once
#include "util.h"

typedef enum message_type_e {
	PLAYER_POSITION,
} message_type;

typedef struct MessagePlayerPostition_s {
	vec2 pos;
} MessagePlayerPostition;

typedef struct MessagePOS_s {
	vec2 pos1;
	vec2 pos2;
} MessagePOS;

typedef struct MessageHeader_s {
	message_type type;
	u32 size;
} MessageHeader;

typedef struct Message_s {
	MessageHeader msg_header;
	union {
		MessagePlayerPostition pos;
		MessagePOS p;
	};
} Message;