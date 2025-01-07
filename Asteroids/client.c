#include "client.h"
#include <stdio.h>

bool init_client(Client* client, const char* host, const char* port) {
	client->id = -1;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// get socket
	if (getaddrinfo(host, port, &hints, &result)) return false;
	SOCKET* s = &client->socket.connection.sock;
	*s = INVALID_SOCKET;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		*s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (*s == INVALID_SOCKET) {
			continue;
		}
		u_long mode = 1;
		if (ioctlsocket(*s, FIONBIO, &mode) == SOCKET_ERROR) {
			closesocket(*s);
			*s = INVALID_SOCKET;
			continue;
		}
		if (connect(*s, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				closesocket(*s);
				*s = INVALID_SOCKET;
				continue;
			}
		}
		break;

	}
	freeaddrinfo(result);
	if (*s == INVALID_SOCKET) return false;
	return true;
}

bool check_connection_status(SOCKET sock) {
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sock, &writefds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int result = select(0, NULL, &writefds, NULL, &timeout);
	if (result > 0) {
		int error = 0;
		int len = sizeof(error);
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error == 0) {
			return true;
		}
		else {
			//error
			return false;
		}
	}
	if (result < 0) {
		// error
		return false;
	}
	return false;
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
	while ((st = recieve_message(&c->socket.connection, &msg)) == MESSAGE_SUCCESS) {
		switch (msg.msg_header.type)
		{
		case ENTITY_STATE: {
			modify_entity(msg.e_state.entity, man);
			break;
		}
		case CLIENT_ENTITY_STATE: {
			MessageClientWithEntityState cont = msg.ce_state;
			players[cont.id].score = cont.score;
			players[cont.id].dead_timer = cont.dead_timer;
			players[cont.id].dead = cont.entity.despawn;
			// local cient
			if (cont.id == c->id) {
				// revive and copy all the entity values
				if(c->player.dead && !cont.entity.despawn) {
					WireframeMesh mesh = c->player.p.mesh;
					c->player.p = cont.entity;
					c->player.p.mesh = mesh;
				}
				// if player is not revived just set certain values
				else { c->player.p.size = cont.entity.size; }
				c->player.dead = cont.entity.despawn;
			}
			// remote client
			else { modify_entity(cont.entity, man); }
			break;
		}
		case CLIENT_STATE: {
			players[msg.c_state.id].score = msg.c_state.score;
			players[msg.c_state.id].dead_timer = msg.c_state.dead_timer;
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

