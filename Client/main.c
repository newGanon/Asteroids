#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "stdio.h"
#include "util.h"

#define DEFAULT_PORT "27015"

SOCKET socket;

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

	// TODO finish init
	return true;
}


bool connect_client() {
	//TODO connect to server
	return true;
}

void clean_up() {
	WSACleanup();
}


i32 client_main() {
	if (!init_network()) return 1;
	if (!init_client()) return 1;

	while (true) {
		connect_client();
	}
	return 0;
}

int main() {
	i32 exit_code = client_main();
	clean_up();
	return exit_code;
}
