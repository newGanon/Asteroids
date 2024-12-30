
#include "network.h"

#include <stdio.h>

#define DEFAULT_PORT "27015"
#define TICKSPERSECOND 100
#define TIMEPERUPDATE 1000/TICKSPERSECOND
#define ASTEROIDSPAWNTIME 1000

typedef struct ServerState_s {
	ServerSocket sockets;
	EntityManager entity_manager;
	EntityManager entity_queue;

	u64 last_time;
	i32 last_asteroid_spawn;
} ServerState;

ServerState state;

void broadcast_message(ServerSocket* s, EntityManager* man, Message* msg);
void handle_message_error(ServerSocket* s, EntityManager* man, i32 id);
void send_welcome_message(ServerSocket* s, EntityManager* man, u32 id);
 
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
	
	SOCKET* s = &state.sockets.listen;
	*s = INVALID_SOCKET;
	*s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (*s == INVALID_SOCKET) {
		closesocket(*s);
		return false;
	}
	if (bind(*s, result->ai_addr, (i32)result->ai_addrlen) != SOCKET_ERROR) {
		if (listen(*s, 32) != SOCKET_ERROR) {
			u_long mode = 1;
			if (ioctlsocket(*s, FIONBIO, &mode) != SOCKET_ERROR) {
				state.sockets.listen = *s;
				freeaddrinfo(result);
				return true;
			}
		}
	}
	// failed, free everything
	freeaddrinfo(result);
	closesocket(*s);
	return false;
}

i32 seach_empty_slot(ServerSocket* s) {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		if (s->connections[i].sock == 0) {
			return i;
		}
	}
}


bool accept_connection(ServerState* serv) { 
	SOCKET s;
	if (serv->sockets.con_amt >= MAX_CLIENTS) return false;
	s = accept(serv->sockets.listen, NULL, NULL);
	if (s == INVALID_SOCKET) return false;
	u_long mode = 1;
	if (ioctlsocket(s, FIONBIO, &mode) == SOCKET_ERROR) {
		closesocket(s);
		s = INVALID_SOCKET;
		return false;
	}
	i32 empty_slot = seach_empty_slot(serv);
	serv->sockets.player_status[empty_slot].dead = false;
	serv->sockets.connections[empty_slot].sock = s;
	serv->sockets.con_amt++;

	send_welcome_message(&serv->sockets, &serv->entity_manager, empty_slot);

	Entity player = (Entity){
	.ang = 0.0,
	.id = empty_slot,
	.despawn = false,
	.dirty = true,
	.pos = (vec2) {0, 0},
	.size = 0.05,
	.type = PLAYER,
	.vel = 0,
	};

	add_entity(&serv->entity_manager, player);

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

void broadcast_new_player_message(ServerSocket* s, EntityManager* man, u32 id) {
	Message msg = (Message){
		.msg_header = {
			.size = sizeof(MessageHeader) + sizeof(MessageNewClient),
			.type = CLIENT_NEW,
		},
		.c_new.id = id,
	};
	memcpy(msg.c_new.name, s->player_status[id].name, MAX_NAME_LENGTH);

	broadcast_message(s, man, &msg);
}

void handle_message_client_connect(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {
	if (!s->connections[id].sock) return;
	memcpy(s->player_status[id].name, msg->c_connect.name, MAX_NAME_LENGTH);
	broadcast_new_player_message(s, man, id);
}

void handle_message_client_player(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {

	if (s->player_status[id].dead) return;
	i32 idx = get_entity_idx(*man, id);
	if (idx == -1) return;

	Entity* p = &man->entities[idx];
	p->ang = msg->c_player.ang;
	p->pos = msg->c_player.pos;
	p->dirty = true;

	s->player_status[id].accelerate = msg->c_player.accelerate;
	if (msg->c_player.shooting) {
		vec2 angle_vec2 = vec2_from_ang(p->ang, 1.0f);
		Entity b = create_bullet(vec2_add(p->pos, vec2_scale(angle_vec2, p->size * 0.9f)), vec2_scale(angle_vec2, 0.8f), 0.003f, id);
		add_entity(man, b);
	}
}

void handle_message(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {
	switch (msg->msg_header.type)
	{
	case CLIENT_PLAYER: handle_message_client_player(s, man, msg, id); break;
	case CLIENT_CONNECT: handle_message_client_connect(s, man, msg, id); break;
	default: break;
	}
} 

void message_to_player(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {
	if (s->connections[id].sock == 0) return;
	message_status success = send_message(&s->connections[id], msg);
	if (success == MESSAGE_ERROR) handle_message_error(s, man, id);
}

void broadcast_message(ServerSocket* s, EntityManager* man, Message* msg) {
	for (i32 j = MAX_CLIENTS - 1; j >= 0; j--) {
		message_to_player(s, man, msg, j);
	}
}

void broadcast_client_disconnect_message(ServerSocket* s, EntityManager* man, u32 id) {
	Message msg = (Message){
			.msg_header = {
				.size = sizeof(MessageHeader) + sizeof(MessageClientDisconnect),
				.type = CLIENT_DISCONNECT,
			},
			.c_disconnect.id = id,
	};
	broadcast_message(s, man, &msg);
}

void handle_message_error(ServerSocket* s, EntityManager* man , u32 id) {
	s->con_amt--;
	s->connections[id] = (NetworkSocket){ 0 };

	Entity* e = &man->entities[get_entity_idx(*man, id)];
	e->dirty = true;
	e->despawn = true;

	broadcast_client_disconnect_message(s, man, id);
}


void send_welcome_message(ServerSocket* s, EntityManager* man, u32 id) {
	Message msg = (Message){
				.msg_header = {
					.size = sizeof(MessageHeader) + sizeof(MessageClientWelcome),
					.type = CLIENT_WELCOME,
				},
				.c_welcome.id = id,
	};
	message_to_player(s, man, &msg, id);

	// TODO: MAKE THIS OWN METHOD
	//send all already connected clients
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		if (s->connections[i].sock == 0 || id == i) continue;

		Message msg = (Message){
			.msg_header = {
				.size = sizeof(MessageHeader) + sizeof(MessageNewClient),
				.type = CLIENT_NEW,
			},
			.c_new.id = i,
		};
		memcpy(msg.c_new.name, s->player_status[i].name, MAX_NAME_LENGTH);
		message_to_player(s, man, &msg, id);
	}
}


void send_player_states_to_client(ServerSocket* s, EntityManager* man) {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		if (s->connections[i].sock == 0) continue;
		Message msg = (Message){
			.msg_header = {
				.size = sizeof(MessageHeader) + sizeof(MessageClientState),
				.type = CLIENT_STATE,
			},
			.c_state.id = i,
			.c_state.score = s->player_status[i].score,
			.c_state.accelerate = s->player_status[i].accelerate,
		};

		broadcast_message(s, man, &msg);
	}
}

void send_entities_to_clients(ServerSocket* s, EntityManager* man) {
	Message msg;
	for (i32 i = man->entity_amt-1; i >= 0; i--) {
		Entity* e = &man->entities[i];
		if (e->dirty) {
			e->dirty = false;
			msg = (Message){
				.msg_header = {
					.size = sizeof(MessageHeader) + sizeof(MessageEntityState),
					.type = ENTITY_STATE,
				},
				.e_state.entity = *e,
			};

			broadcast_message(s, man, &msg);

			// If despawn is true remove the entity
			if (e->despawn) remove_entity(man, i); 
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

void update_tickers(ServerState* s, u64 delta_time) {
	// update last update counter
	s->last_time += delta_time;
	// update asteroid spawn timer
	s->last_asteroid_spawn += delta_time;
	// update player death timers
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		NetworkPlayerInfo* status = &s->sockets.player_status[i];
		if (s->sockets.connections[i].sock != 0 && status->dead) {
			status->dead_timer += delta_time;
		}
	}
}


i32 server_main() {
	state = (ServerState){ 0 };
	// TODO: make entity_manager dynamic
	state.entity_manager.entities = (Entity*)malloc(1000 * sizeof(Entity));
	state.entity_queue.entities = (Entity*)malloc(1000 * sizeof(Entity));
	state.last_time = get_milliseconds();
	state.last_asteroid_spawn = 0;

	f32 map_size = 2.0f;

	if (!init_network()) return 1;
	if (!init_server()) return 1;
	bool running = true;

	while (running) {
		if (accept_connection(&state.sockets)) printf("NEUER CLIENT\n");
		for (u32 i = 0; i < MAX_CLIENTS; i++) {
			if (state.sockets.connections[i].sock != INVALID_SOCKET && state.sockets.connections[i].sock != 0) {
				Message msg;
				message_status status;
				// Recieve Messages
				status = recieve_message(&state.sockets.connections[i], &msg);
				switch (status) 
				{
				case MESSAGE_SUCCESS: handle_message(&state.sockets, &state.entity_manager, &msg, i); break; 
				case MESSAGE_EMPTY: break; 
				case MESSAGE_ERROR: handle_message_error(&state.sockets, &state.entity_manager, i); break;
				default: break;
				}
			}	
		}

		// Update Server
		u64 delta_time = time_since(state.last_time);
		while (delta_time >= TIMEPERUPDATE) {
			delta_time -= TIMEPERUPDATE;
			update_tickers(&state, TIMEPERUPDATE);
			update_entities(&state.entity_manager, &state.entity_queue, TIMEPERUPDATE, map_size);
			entity_collisions(&state.entity_manager, &state.entity_queue, state.sockets.player_status);
		}

		if (state.last_asteroid_spawn > ASTEROIDSPAWNTIME) {
			spawn_asteroid(&state.entity_manager, map_size);
			state.last_asteroid_spawn = 0;
		}

		// Send Messages
		send_entities_to_clients(&state.sockets, &state.entity_manager);
		send_player_states_to_client(&state.sockets, &state.entity_manager);
	}
	return 0;
}

int main() {
	i32 exit_code = server_main();
	WSACleanup();
	return exit_code;
}