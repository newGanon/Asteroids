#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "stdio.h"
#include "util.h"

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

SOCKET connect_socket;

bool init_network() {
	WSADATA wsaData;
	return !WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool init_client() {
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// get socket
	if (getaddrinfo("localhost", DEFAULT_PORT, &hints, &result)) return false;
	SOCKET connect_socket = INVALID_SOCKET;

	ptr = result;
	connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (connect_socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		return 0;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		if (connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
			closesocket(connect_socket);
			connect_socket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (connect_socket == INVALID_SOCKET) return false;
	return true;
}


void clean_up() {
	WSACleanup();
}

/*
bool send_data() {
	const char* sendbuf = "this is the client";
	WSABUF buf = { .buf = sendbuf , .len = (int)strlen(sendbuf) };
	DWORD bytesSent = 0;

	int res;
	if (res = WSASendTo(connect_socket, &buf, 1, 0, 0, 0, &bytesSent, 0, 0, 0) == SOCKET_ERROR) {
		close_socket(connect_socket);
		return false;
	}

	printf("Sent %lu bytes: %s\n", bytesSent, sendbuf);
	return true;
}*/

i32 client_main() {
	if (!init_network()) return 1;
	if (!init_client()) return 1;

	while (true) {

	}
	return 0;
}

int main() {
	i32 exit_code = client_main();
	clean_up();
	return exit_code;
}
