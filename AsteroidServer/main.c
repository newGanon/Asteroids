
#include "stdio.h"
#include "network.h"

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 4

SOCKET listen_socket;
NetworkSocket cons[MAX_CLIENTS];
size con_amt;



bool init_network() {
	WSADATA wsaData;
	return !WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool init_server() {
	con_amt = 0;
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
	if (listen_socket == INVALID_SOCKET) {
		closesocket(listen_socket);
		return false;
	}
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
	cons[con_amt].sock = accept(listen_socket, NULL, NULL);
	if (cons[con_amt].sock == INVALID_SOCKET) return false;
	u_long mode = 1;
	if (ioctlsocket(cons[con_amt].sock, FIONBIO, &mode) == SOCKET_ERROR) {
		closesocket(cons[con_amt].sock);
		cons[con_amt].sock = INVALID_SOCKET;
		return false;
	}
	con_amt++;
	return true;
}

i32 server_main() {
	if (!init_network()) return 1;
	if (!init_server()) return 1;

	bool running = true;

	while (running) {
		if (accept_connection()) printf("NEUER CLIENT\n");
		for (size_t i = 0; i < con_amt; i++) {
			if (listen_socket != INVALID_SOCKET) {
				//Sleep(10000);
				Message msg;
				while (recieve_message(&cons[i], &msg)) {
					printf("MESSAGE RECIEVED: ");
					printf("%f, %f\n", msg.pos.pos.x, msg.pos.pos.y);
				}
			}
		}
	}
	return 0;
}

int main() {
	i32 exit_code = server_main();
	clean_up();
	return exit_code;
}