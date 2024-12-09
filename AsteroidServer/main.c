#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#include "stdio.h"
#include "util.h"
#include "message.h"

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 4

SOCKET listen_socket;
SOCKET accept_socket[4];
size accept_socket_amt;

OVERLAPPED overlapped = { 0 };
char* char_buf[sizeof(Message)];
WSABUF buf = { .buf = char_buf, .len = sizeof(Message) };



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
	accept_socket[accept_socket_amt] = accept(listen_socket, NULL, NULL);
	if (accept_socket[accept_socket_amt] == INVALID_SOCKET) return false;
	u_long mode = 1;
	if (ioctlsocket(accept_socket[accept_socket_amt], FIONBIO, &mode) == SOCKET_ERROR) {
		closesocket(accept_socket[accept_socket_amt]);
		accept_socket[accept_socket_amt] = INVALID_SOCKET;
		return false;
	}
	accept_socket_amt++;
	return true;
}

void clean_up() {
	for (size_t i = 0; i < accept_socket_amt; i++) {
		closesocket(accept_socket);
	}
	WSACleanup();
}


bool recieve_data(i32 idx) {
	DWORD flags = 0;
	int err = 0;
	int rc = 0;
	DWORD bytes_revieved;

	bool still_revieving = true;

	while (still_revieving) {
		rc = WSAGetOverlappedResult(accept_socket[idx], &overlapped, &bytes_revieved, FALSE, &flags);
		if (rc == FALSE) {
			if ((rc == SOCKET_ERROR) && WSAGetLastError() != WSA_IO_INCOMPLETE) {
				return false;
			}
			// Asnchronous function hasn't completed yet return
			break;
		}
		if (bytes_revieved > 0) {
			Message msg;
			memcpy(&msg, buf.buf, bytes_revieved);
			printf("MESSAGE RECIEVED\n");
		}
		rc = WSARecv(accept_socket[idx], &buf, 1, NULL, &flags, &overlapped, NULL);
		if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
			return false;
		}
	}

	return true;
}

i32 server_main() {
	if (!init_network()) return 1;
	if (!init_server()) return 1;

	bool running = true;

	while (running) {
		if (accept_connection()) printf("NEUER CLIENT\n");
		for (size_t i = 0; i < accept_socket_amt; i++) {
			if (accept_socket != INVALID_SOCKET) {
				if (!recieve_data(i)) running = false;
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