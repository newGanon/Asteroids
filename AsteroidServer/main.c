
#include "network.h"

#include <stdio.h>

#define DEFAULT_PORT "27015"
#define TICKSPERSECOND 100
#define TIMEPERUPDATE 1000/TICKSPERSECOND

typedef struct ServerState_s {
	ServerSocket sockets;
	EntityManager entity_manager;
	EntityManager entity_queue;

	u64 last_time;
	i32 last_asteroid_spawn;
	f32 map_size;
} ServerState;

ServerState state;

//TODO MAKE THIS INDIVUDUAL FOR EACH CONNECTION
#define BLOCKEDLENGTH 1000
typedef struct Blocket_Message_s {
	Message msg;
	u32 id;
}Blocket_Message;
Blocket_Message blocked_messages[BLOCKEDLENGTH];
size_t blocked_len = 0;

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

Entity create_player(EntityManager* man, i32 id) {
	Entity player = (Entity){
	.ang = 0.0,
	.id = id,
	.despawn = false,
	.dirty = true,
	.pos = (vec2) {0, 0},
	.size = 0.05,
	.type = PLAYER,
	.vel = 0,
	.accelerating = false,
	};
	return player;
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
	serv->sockets.connections[empty_slot].sock = s;
	serv->sockets.con_amt++;
	serv->sockets.player_status[empty_slot].connected = true;

	Entity player = create_player(&serv->entity_manager, empty_slot);
	add_entity(&serv->entity_manager, player);

	send_welcome_message(&serv->sockets, &serv->entity_manager, empty_slot, player);
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
	p->accelerating = msg->c_player.accelerate;
	p->dirty = true;
	if (msg->c_player.shooting) {
		vec2 angle_vec2 = vec2_from_ang(p->ang, 1.0f);
		vec2 bullet_pos = vec2_add(p->pos, (vec2_scale(angle_vec2, p->size * 0.9f)));
		f32 bullet_size = 0.003f * 20.0f * p->size;
		if (!rect_overlap_rect((vec2) { bullet_pos.x - bullet_size, bullet_pos.y - bullet_size },
							   (vec2) { bullet_pos.x + bullet_size, bullet_pos.y + bullet_size },
							   (vec2) { -state.map_size, -state.map_size },
							   (vec2) { state.map_size, state.map_size })) {
			return;
		}
		Entity bullet = create_bullet(bullet_pos, vec2_scale(angle_vec2, 0.8f), bullet_size, id);
		add_entity(man, bullet);
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
	message_status msg_code = send_message(&s->connections[id], msg);
	if (msg_code == MESSAGE_ERROR) handle_message_error(s, man, id);
	else if (msg_code == MESSAGE_EMPTY && blocked_len < BLOCKEDLENGTH) {
		if (!msg->e_state.entity.despawn) return;
		blocked_messages[blocked_len].msg = *msg;
		blocked_messages[blocked_len].id = id;
		blocked_len++;
	}
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


void send_welcome_message(ServerSocket* s, EntityManager* man, u32 id, Entity player) {
	Message msg;
	msg = (Message){
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

		msg = (Message){
			.msg_header = {
				.size = sizeof(MessageHeader) + sizeof(MessageNewClient),
				.type = CLIENT_NEW,
			},
			.c_new.id = i,
		};
		memcpy(msg.c_new.name, s->player_status[i].name, MAX_NAME_LENGTH);
		message_to_player(s, man, &msg, id);
	}

	msg = (Message){
				.msg_header = {
					.size = sizeof(MessageHeader) + sizeof(MessageEntityState),
					.type = ENTITY_STATE,
				},
				.e_state.entity = player,
	};
	message_to_player(s, man, &msg, id);

	NetworkPlayerInfo info = s->player_status[id];
	msg = (Message){
	.msg_header = {
			.size = sizeof(MessageHeader) + sizeof(MessageClientState),
			.type = CLIENT_STATE,
		},
		.c_state.id = id,
		.c_state.score = info.score,
		.c_state.dead = info.dead,
		.c_state.dead_timer = info.dead_timer,
	};

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
			.c_state.dead = s->player_status[i].dead,
			.c_state.dead_timer = s->player_status[i].dead_timer,
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

void resend_blocked_messages(ServerSocket* s, EntityManager* man, Blocket_Message* blocked_msgs, size_t* blocked_len) {
	for (i32 i = 0; i < *blocked_len; i++) {
		Message* msg = &blocked_messages[i];
		u32 id = blocked_messages[i].id;
		message_status stat = send_message(&s->connections[id], msg);
		switch (stat)
		{
		case (MESSAGE_EMPTY): { break; }
		case (MESSAGE_ERROR): { handle_message_error(s, man, id); break; }
		case (MESSAGE_SUCCESS): {
			memmove(&blocked_messages[i], &blocked_messages[i + 1], sizeof(Blocket_Message) * (*blocked_len - (i + 1))); 
			*blocked_len -= 1;
			i--;
			break; 
		}
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
			status->dead_timer -= delta_time;
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
	state.map_size = 2.0f;

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
			update_entities(&state.entity_manager, &state.entity_queue, TIMEPERUPDATE, state.map_size);
			entity_collisions(&state.entity_manager, &state.entity_queue, state.sockets.player_status);
		}

		// spawn asteroids
		if (state.last_asteroid_spawn > ASTEROIDSPAWNTIME) {
			spawn_asteroid(&state.entity_manager, state.map_size);
			state.last_asteroid_spawn = 0;
		}
		
		//revive players
		for (size_t i = 0; i < MAX_CLIENTS; i++) {
			NetworkPlayerInfo* player_info = &state.sockets.player_status[i];
			if (player_info->connected) {
				if (player_info->dead_timer < 0) {
					player_info->dead_timer = 0;
					player_info->dead = false;

					Entity player = create_player(&state.entity_manager, i);
					add_entity(&state.entity_manager, player);
				}
			}
		}

		// Send Messages
		send_entities_to_clients(&state.sockets, &state.entity_manager);
		send_player_states_to_client(&state.sockets, &state.entity_manager);

		resend_blocked_messages(&state.sockets, &state.entity_manager, blocked_messages, &blocked_len);
	}
	return 0;
}

int main() {
	i32 exit_code = server_main();
	WSACleanup();
	return exit_code;
}