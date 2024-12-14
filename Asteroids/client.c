#include "client.h"
#include <stdio.h>

bool init_client(Client* client, char* port) {
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
			.type = PLAYER_STATE,
			.size = sizeof(MessageHeader) + sizeof(MessagePlayerState)
		},
		.p_state.player_ent = c->player.p,
		.p_state.shooting = c->player.input.shoot,
	};
	c->player.input.shoot = false;
	message_status st = send_message(&c->socket.connection, &msg);
	if (st == MESSAGE_ERROR)
		return false;
	return true;
}


bool revieve_server_messages(Client* c, EntityManager* man) {
	Message msg;
	message_status st;
	while (st = recieve_message(&c->socket.connection, &msg)) {
		switch (msg.msg_header.type)
		{
		case ENTITY_STATE: {
			Entity e = msg.e_state.ent;
			i32 idx = get_entity_idx(*man, e.id);
			if (idx == -1 && e.despawn) return;
			if (idx == -1) {
				add_entity(man, msg.e_state.ent);
				man->entities[man->entity_amt - 1].mesh = create_entity_mesh(e.type, e.size);
			}
			else {
				if (e.despawn) { remove_entity(man, idx); }
				else {
					// HACK!!! TODO: SAFE MESH SOMEWHERE ELSE 
					e.mesh = man->entities[idx].mesh;
					overwrite_entity_idx(man, e, idx);
				}
			}
			break;
		}
		case PLAYER_STATE: {
			Entity e = msg.p_state.player_ent;
			c->player.p = e;
			c->player.dead = msg.p_state.dead;
		}
		default: break;
		}
	}
	if (st == MESSAGE_ERROR)
		return false;
	return true;
}