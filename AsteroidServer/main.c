
#include "network.h"

#include <stdio.h>

#define DEFAULT_PORT "27015"
#define MAX_CLIENTS 4
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


void broadcast_message(ServerSocket* s, EntityManager* man, Message* msg, i32 exclude_id);
void handle_message_error(ServerSocket* s, EntityManager* man, i32 idx);
 
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
		if (listen(*s, MAX_CLIENTS) != SOCKET_ERROR) {
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


void handle_message_player_state(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {
	if (s->player_status[id].dead) return;
	Entity p = msg->p_state.player_ent;
	p.dirty = true;
	p.id = id;

	i32 idx = get_entity_idx(*man, id);
	// player id not found, add player
	if (idx == -1) {
		add_entity(man, p);
	}
	// player id found, update player
	else {
		overwrite_entity_idx(man, p, idx);
	}
	// check if player is shooting
	if (msg->p_state.shooting) {
		vec2 angle_vec2 = vec2_from_ang(p.ang, 1.0f);
		Entity b = create_bullet(vec2_add(p.pos, vec2_scale(angle_vec2, 8.0 * p.size)), vec2_scale(angle_vec2, 0.8f), 0.003f);
		add_entity(man, b);
	}
}


void handle_message(ServerSocket* s, EntityManager* man, Message* msg, u32 id) {
	switch (msg->msg_header.type)
	{
	case PLAYER_STATE: handle_message_player_state(s, man, msg, id); break; 
	default: break;
	}
}

void message_to_player(ServerSocket* s, EntityManager* man, Message* msg, i32 idx, i32 exclude_id) {
	if (s->connections[idx].sock == 0 || idx == exclude_id) return;
	message_status success = send_message(&s->connections[idx], msg);
	if (success == MESSAGE_ERROR) handle_message_error(s, man, idx);
}

void broadcast_message(ServerSocket* s, EntityManager* man, Message* msg, i32 exclude_id) {
	for (i32 j = MAX_CLIENTS - 1; j >= 0; j--) {
		message_to_player(s, man, msg, j, exclude_id);
	}
}

void handle_message_error(ServerSocket* s, EntityManager* man , i32 idx) {
	s->con_amt--;
	s->connections[idx] = (NetworkSocket){ 0 };
	Entity* e = &man->entities[get_entity_idx(*man, idx)];
	e->dirty = true;
	e->despawn = true;
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
				.e_state.ent = *e,
			};
			// Exclude e->id, as player entity id correspons to their socket idx
			broadcast_message(s, man, &msg, e->id);

			// If player despawn is true and player still connected then send them a death message
			if (e->despawn) {
				if (e->type == PLAYER) {
					msg = (Message){
						.msg_header = {
							.size = sizeof(MessageHeader) + sizeof(MessagePlayerState),
							.type = PLAYER_STATE
						},
						.p_state.player_ent = *e,
						.p_state.dead = true,
					};
					message_to_player(s, man, &msg, e->id, -1);
					s->player_status[e->id].dead = true;
					s->player_status->dead_timer = 0;
				}
				remove_entity(man, i);
			}
		}
	}
}

send_revive_messages(ServerSocket* s) {
	// TODO
	for (size_t i = 0; i < MAX_CLIENTS; i++) {

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
		PlayerStatus* status = &s->sockets.connections[i];
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
			entity_collisions(&state.entity_manager, &state.entity_queue);
		}


		if (state.last_asteroid_spawn > ASTEROIDSPAWNTIME) {
			spawn_asteroid(&state.entity_manager, map_size);
			state.last_asteroid_spawn = 0;
		}

		// Send Messages
		send_entities_to_clients(&state.sockets, &state.entity_manager);
		send_revive_messages(&state.sockets);
	}
	return 0;
}

int main() {
	i32 exit_code = server_main();
	clean_up();
	return exit_code;
}