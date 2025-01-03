#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "util.h"
#include "message.h"

typedef enum message_status_e {
	MESSAGE_ERROR = -1,
	MESSAGE_EMPTY = 0,
	MESSAGE_SUCCESS = 1,
} message_status;

typedef struct NetworkSocket_s {
	SOCKET sock;

	u8 buffer[sizeof(Message)];
	u32 bytes_used;
} NetworkSocket;

typedef struct ClientSocket_s {
	NetworkSocket connection;
}ClientSocket;

typedef struct ServerSocket_s {
	SOCKET listen;
	NetworkSocket connections[MAX_CLIENTS];
	NetworkPlayerInfo player_status[MAX_CLIENTS];
	size con_amt;
}ServerSocket;

message_status recieve_message(NetworkSocket* s, Message* msg);
message_status send_message(NetworkSocket* s, Message* msg);
