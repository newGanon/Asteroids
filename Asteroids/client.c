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
			freeaddrinfo(result);
			return 0;
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
	return send_message(&c->socket.connection, &msg);
}


bool revieve_server_messages(Client* c, EntityManager* man) {
	Message msg;
	while (recieve_message(&c->socket.connection, &msg)) {
		if (msg.msg_header.type == ENTITY_STATE) {
			Entity e = msg.e_state.ent;
			i32 idx = get_entity_idx(*man, e.id);
			if (idx == -1) { 
				add_entity(man, msg.e_state.ent); }
			else { 
				if (e.despawn) { destroy_entity(man, idx); }
				else { overwrite_entity_idx(man, e, idx);}
			}
		}
	}
	return true;
}