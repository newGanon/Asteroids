#pragma once
#include "util.h"
#include "entity.h"

#define MAX_NAME_LENGTH 16

typedef enum message_type_e {
	// CLIENT TO SERVER
	CLIENT_PLAYER,
	CLIENT_CONNECT,

	// SERVER TO CLIENT
	ENTITY_STATE,
	CLIENT_STATE,
	CLIENT_WELCOME,
	CLIENT_DISCONNECT,
	CLIENT_NEW,
} message_type;

// CLIENT TO SERVER
typedef struct MessageClientPlayer_s {
	vec2 pos;
	f32 ang;
	bool shooting;
} MessageClientPlayer;

typedef struct MessageClientConnect_s {
	char name[MAX_NAME_LENGTH];
} MessageClientConnect;

// SERVER TO CLIENT
typedef struct MessageEntityState_s {
	Entity entity;
} MessageEntityState;

typedef struct MessageClientState_s {
	u32 id;
	u32 score;
}MessageClientState;

typedef struct MessageClientDisconnect_s {
	u32 id;
}MessageClientDisconnect;

typedef struct MessageClientWelcome_s {
	u32 id;
}MessageClientWelcome;

typedef struct MessageNewClient_s {
	u32 id;
	char name[MAX_NAME_LENGTH];
} MessageNewClient;



typedef struct MessageHeader_s {
	message_type type;
	u32 size;
} MessageHeader;

typedef struct Message_s {
	MessageHeader msg_header;
	union {
		// CLIENT TO SERVER
		MessageClientPlayer c_player;
		MessageClientConnect c_connect;
		// SERVER TO CLIENT
		MessageEntityState e_state;
		MessageClientState c_state;
		MessageNewClient c_new;
		MessageClientWelcome c_welcome;
		MessageClientDisconnect c_disconnect;
	};
} Message;