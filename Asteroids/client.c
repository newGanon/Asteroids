#include "client.h"
#include <stdio.h>

bool init_client(Client* client, char* port) {
	client->id = -1;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// get socket
	if (getaddrinfo("localhost", port, &hints, &result)) return false;
	SOCKET* s = &client->socket.connection.sock;
	*s = INVALID_SOCKET;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		*s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (*s == INVALID_SOCKET) {
			continue;
		}
		if (connect(*s, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
			closesocket(*s);
			*s = INVALID_SOCKET;
			continue;
		}
		u_long mode = 1;
		if (ioctlsocket(*s, FIONBIO, &mode) == SOCKET_ERROR) {
			continue;
		}
		break;

	}
	freeaddrinfo(result);

	if (*s == INVALID_SOCKET) return false;
	return true;
}

bool send_player_state_to_server(Client* c) {
	Message msg = {
		.msg_header = {
			.type = CLIENT_PLAYER,
			.size = sizeof(MessageHeader) + sizeof(MessageClientPlayer)
		},
		.c_player.ang = c->player.p.ang,
		.c_player.pos = c->player.p.pos,
		.c_player.shooting = c->player.input.shoot,
		.c_player.accelerate = c->player.input.accelerate,
	};

	c->player.input.shoot = false;
 	message_status st = send_message(&c->socket.connection, &msg);

	if (st == MESSAGE_ERROR)
		return false;
	return true;
}


bool send_client_connect(Client* c, const char* name) {
	Message msg = {
		.msg_header = {
			.type = CLIENT_CONNECT,
			.size = sizeof(MessageHeader) + sizeof(MessageClientConnect)
		},
	};

	memcpy(msg.c_connect.name, name, 16);

	message_status st = send_message(&c->socket.connection, &msg);

	if (st == MESSAGE_ERROR)
		return false;
	return true;
}


bool recieve_server_messages(Client* c, EntityManager* man, NetworkPlayerInfo* players) {
	Message msg;
	message_status st;
	while (st = recieve_message(&c->socket.connection, &msg)) {
		switch (msg.msg_header.type)
		{
		case ENTITY_STATE: {
			Entity e = msg.e_state.entity;
			// if entity is own player entity only copy certain attributes
			if (e.id == c->id) {
				c->player.p.size = e.size;
				c->player.p.id = e.id;
				continue;
			}
			i32 idx = get_entity_idx(*man, e.id);
			if (idx == -1 && e.despawn) continue;
			// entity not found, create new entity
			if (idx == -1) {
				add_entity(man, msg.e_state.entity);
				man->entities[man->entity_amt - 1].mesh = create_entity_mesh(e.type, e.size);
			}
			// entity found, update entity
			else {
				if (e.despawn) { 
					remove_entity(man, idx); 
				}
				else {
					// HACK!!! TODO: SAFE MESH SOMEWHERE ELSE 
					e.mesh = man->entities[idx].mesh;
					overwrite_entity_idx(man, e, idx);
				}
			}
			break;
		}
		case CLIENT_STATE: {
			players[msg.c_state.id].score = msg.c_state.score;
			players[msg.c_state.id].dead = msg.c_state.dead;
			if (msg.c_state.id == c->id) c->player.dead = msg.c_state.dead;
			break;
		}
		case CLIENT_WELCOME: {
			c->id = msg.c_welcome.id;
			break;
		}
		case CLIENT_DISCONNECT: {
			u32 id = msg.c_disconnect.id;
			if (id == c->id) return false;
			players[id] = (NetworkPlayerInfo){0};
			break;
		}
		case CLIENT_NEW: {
			memcpy(players[msg.c_new.id].name, msg.c_new.name, MAX_NAME_LENGTH);
			players[msg.c_new.id].connected = true;
			break;
		}
		default: break;
		}
	}
	if (st == MESSAGE_ERROR)
		return false;
	return true;
}

