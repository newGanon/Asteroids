#include "network.h"

message_status get_message_from_buffer(u8* buffer, u32* bytes_used, Message* msg) {
	if (*bytes_used < sizeof(MessageHeader)) {
		return MESSAGE_EMPTY;
	}
	MessageHeader head;
	memcpy(&head, buffer, sizeof(MessageHeader));

	if (*bytes_used < head.size) {
		return MESSAGE_EMPTY;
	}
	*bytes_used -= head.size;

	memcpy(msg, buffer, head.size);
	memmove(buffer, buffer + head.size, *bytes_used);
	return MESSAGE_SUCCESS;
}

// returns: 1 if message has been read, 
//			0 if there is no message in buffer,
//			-1 if an error occured and socket should be closed 
message_status recieve_message(NetworkSocket* s, Message* msg) {

	int flags = 0;
	int rc = recv(s->sock, &s->buffer[s->bytes_used], sizeof(Message) - s->bytes_used, flags);

	if (rc == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK) {
			return MESSAGE_EMPTY;
		}
		closesocket(s->sock);
		return MESSAGE_ERROR;
	}
	//socket closed 
	if (rc == 0) {
		closesocket(s->sock);
		return MESSAGE_ERROR;
	}
	s->bytes_used += rc;
	return get_message_from_buffer(s->buffer, &s->bytes_used, msg);
}


// returns: 1 if message has been read, 
//			0 if there is no message in buffer,
//			-1 if an error occured and socket should be closed 
message_status send_message(NetworkSocket* s, Message* msg) {
	int flags = 0;
	int bs = send(s->sock, (char *)msg, msg->msg_header.size, flags);

	if (bs == SOCKET_ERROR) {
		i32 err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK) {
			// Package thrown away, maybe save a few and then send them all if not blocking anymore
			return MESSAGE_EMPTY;
		}
		return MESSAGE_ERROR;
	}
	//connection close
	if (bs == 0) {
		return MESSAGE_ERROR;
	}
	return true;
}