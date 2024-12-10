#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "util.h"
#include "message.h"

#define MAX_SERVER_SOCKETS 4

typedef struct NetworkSocket_s {
	SOCKET sock;

	u8 buffer[sizeof(Message)];
	u32 bytes_used;
} NetworkSocket;

typedef struct ClientSocket_s {
	NetworkSocket sock;
}ClientSocket;

typedef struct ServerSocket_s {
	NetworkSocket socks[MAX_SERVER_SOCKETS];
}ServerSocket;


bool recieve_message(NetworkSocket* s, Message* msg);
bool send_message(NetworkSocket* s, Message* msg);