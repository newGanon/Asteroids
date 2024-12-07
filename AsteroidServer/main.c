#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "stdio.h"
#include "util.h"

#define DEFAULT_PORT "27015"
#define MAX_CLIENTS 4

SOCKET listen_socket;
SOCKET accept_socket[4];
size accept_socket_amt;


bool init_network() {
	WSADATA wsaData;
	return !WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool init_server() {
	accept_socket_amt = 0;
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
	listen_socket = INVALID_SOCKET;
	listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET) return false;

	if (bind(listen_socket, result->ai_addr, (i32)result->ai_addrlen) != SOCKET_ERROR) {
		if (listen(listen_socket, MAX_CLIENTS) != SOCKET_ERROR) {
			u_long mode = 1;
			if (ioctlsocket(listen_socket, FIONBIO, &mode) != SOCKET_ERROR) {
				freeaddrinfo(result);
				return true;
			}
		}
	}
	// failed, free everything
	freeaddrinfo(result);
	closesocket(listen_socket);
	return false;
}

bool accept_connection() { 
	struct sockaddr sa_client;
	int i_client_size = sizeof(sa_client);
	accept_socket[accept_socket_amt] = WSAAccept(listen_socket, &sa_client, &i_client_size, NULL, NULL);
	if (accept_socket[accept_socket_amt] == INVALID_SOCKET) return false;
	accept_socket_amt++;
	u_long mode = 1;
	if (ioctlsocket(accept_socket[accept_socket_amt], FIONBIO, &mode) == SOCKET_ERROR) {
		closesocket(accept_socket[accept_socket_amt]);
		return false;
	}
	accept_socket_amt++;
	return true;
}

void clean_up() {
	for (size_t i = 0; i < accept_socket_amt; i++) {
		closesocket(accept_socket);
	}
	closesocket(listen_socket);
	WSACleanup();
}

i32 server_main() {
	if (!init_network()) return 1;
	if (!init_server()) return 1;

	while (true) {
		if (accept_connection()) {
			printf("NEUER CLIENT");
		}
	}
	return 0;
}


int main() {
	i32 exit_code = server_main();
	clean_up();
	return exit_code;
}