#include "network.h"


bool get_message_from_buffer(u8* buffer, u32* bytes_used, Message* msg) {
	if (*bytes_used < sizeof(MessageHeader)) {
		return false;
	}
	MessageHeader* head = (MessageHeader*) (buffer);

	if (*bytes_used < head->size) {
		return false;
	}
	*bytes_used -= head->size;

	memcpy(msg, buffer, head->size);
	memcpy(buffer, buffer + head->size, *bytes_used);
	return true;
}


bool recieve_message(NetworkSocket* s, Message* msg) {
	int flags = 0;
	int rc = recv(s->sock, &s->buffer[s->bytes_used], sizeof(Message) - s->bytes_used, flags);

	if (rc == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK) {
			return false;
		}
		//TODO ERROR HANDLING;
		return false;
	}
	//socket closed 
	if (rc == 0) {
		//TODO CLOSE CONNECTION
		return false;
	}
	s->bytes_used += rc;
	return get_message_from_buffer(s->buffer, &s->bytes_used, msg);
}

bool send_message(NetworkSocket* s, Message* msg) {
	int flags = 0;
	int bs = send(s->sock, (char *)msg, msg->msg_header.size, flags);

	if (bs == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			// Package thrown away, maybe save a few and then send them all if not blocking anymore
			return false;
		}
		//TODO ERROR HANDLING;
		return false;
	}
	return true;
}



void clean_up() {
	WSACleanup();
}
