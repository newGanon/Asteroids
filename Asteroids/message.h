#pragma once
#include "util.h"
#include "entity.h"

typedef enum message_type_e {
	PLAYER_STATE,
	ENTITY_STATE
} message_type;

typedef struct MessagePlayerState_s {
	Entity player_ent;
	bool shooting;
} MessagePlayerState;

typedef struct MessageEntityState_s {
	Entity ent;
} MessageEntityState;

typedef struct MessageHeader_s {
	message_type type;
	u32 size;
} MessageHeader;

typedef struct Message_s {
	MessageHeader msg_header;
	union {
		MessagePlayerState p_state;
		MessageEntityState e_state;
	};
} Message;