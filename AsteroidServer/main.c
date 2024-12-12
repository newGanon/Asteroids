
#include "network.h"

#include <stdio.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 4

typedef struct ServerState_s {
	ServerSocket sockets;
	EntityManager entity_manager;

	u64 last_time;
	u64 last_asteroid_spawn;
} ServerState;

ServerState state;

bool init_network() {
	WSADATA wsaData;
	return !WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool init_server() {
	state.sockets.con_amt = 0;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result)) {
		return false;
	}
	SOCKET s;
	s = INVALID_SOCKET;
	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (s == INVALID_SOCKET) {
		closesocket(s);
		return false;
	}
	if (bind(s, result->ai_addr, (i32)result->ai_addrlen) != SOCKET_ERROR) {
		if (listen(s, MAX_CLIENTS) != SOCKET_ERROR) {
			u_long mode = 1;
			if (ioctlsocket(s, FIONBIO, &mode) != SOCKET_ERROR) {
				state.sockets.listen = s;
				freeaddrinfo(result);
				return true;
			}
		}
	}
	// failed, free everything
	freeaddrinfo(result);
	closesocket(s);
	return false;
}

bool accept_connection() { 
	SOCKET s = accept(state.sockets.listen, NULL, NULL);
	if (s == INVALID_SOCKET) return false;
	u_long mode = 1;
	if (ioctlsocket(s, FIONBIO, &mode) == SOCKET_ERROR) {
		closesocket(s);
		s = INVALID_SOCKET;
		return false;
	}
	state.sockets.connections[state.sockets.con_amt].sock = s;
	state.sockets.con_amt++;
	return true;
}

i32 get_player_idx(u32 id) {
	for (size_t i = 0; i < state.entity_manager.entity_amt; i++) {
		// found player already in entity array
		if (state.entity_manager.entities[i].type == PLAYER || state.entity_manager.entities[i].id == id) {
			return i;
		}
	}
	return -1;
}


void handle_message_player_state(EntityManager* man, Message* msg, u32 id) {
	Entity p = msg->p_state.player_ent;

	i32 idx = get_entity_idx(*man, id);
	// player id not found, add player
	if (idx == -1) {
		add_entity(man, p);
	}
	// player id found, update player
	else {
		overwrite_entity_idx(man, p, idx);
		man->entities[idx].dirty = true;
	}
	// check if player is shooting
	if (msg->p_state.shooting) {
		vec2 angle_vec2 = vec2_from_ang(p.ang, 1.0f);
		Entity b = create_bullet(vec2_add(p.pos, vec2_scale(angle_vec2, 8.0 * p.size)), vec2_scale(angle_vec2, 0.8f), 0.003f);
		add_entity(man, b);
	}
}


void handle_message(EntityManager* man, Message* msg, u32 id) {
	switch (msg->msg_header.type)
	{
	case PLAYER_STATE: { handle_message_player_state(man, msg, id); break; }
	default:
		break;
	}
}


bool send_entities_to_clients() {
	EntityManager* man = &state.entity_manager;
	for (size_t i = 0; i < man->entity_amt; i++) {
		Entity* e = &man->entities[i];
		if (e->dirty) {
			e->dirty = false;
			Message msg = {
				.msg_header = {
					.size = sizeof(MessageHeader) + sizeof(MessageEntityState),
					.type = e->type,
				},
				.e_state.ent = *e,
			};
			for (size_t j = 0; j < state.sockets.con_amt; j++) {
				if (j == e->id && e->type == PLAYER) continue;
				bool succ = send_message(&state.sockets.connections[j], &msg);
			}
			if (e->despawn) { destroy_entity(man, i); }
		}
	}
}


u64 get_milliseconds() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER counter_now;
	QueryPerformanceCounter(&counter_now);
	return (1000LL * counter_now.QuadPart) / freq.QuadPart;
}

u64 time_since(u64 last_time) {
	u64 now = get_milliseconds();
	return now - last_time;
}

i32 server_main() {
	state = (ServerState){ 0 };
	state.entity_manager.entities = (Entity*)malloc(1000 * sizeof(Entity));
	state.last_time = get_milliseconds();
	if (!init_network()) return 1;
	if (!init_server()) return 1;
	bool running = true;

	while (running) {
		if (accept_connection()) printf("NEUER CLIENT\n");
		for (u32 i = 0; i < state.sockets.con_amt; i++) {
			if (state.sockets.listen != INVALID_SOCKET) {
				u64 delta_time = time_since(state.last_time);
				state.last_time += delta_time;

				Message msg;
				// Recieve Messages
				while (recieve_message(&state.sockets.connections[i], &msg)) {
					handle_message(&state.entity_manager, &msg, i);
				}
				// Update Entities
				if (delta_time > 500) {
					update_entities(&state.entity_manager, 500);
					entity_collisions(&state.entity_manager);
					delta_time -= 500;
				}
				update_entities(&state.entity_manager, delta_time);
				entity_collisions(&state.entity_manager);


				// Send Messages
				send_entities_to_clients();
			}	
		}
	}
	return 0;
}

int main() {
	i32 exit_code = server_main();
	clean_up();
	return exit_code;
}