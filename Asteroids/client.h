#pragma once
#include "player.h"
#include "network.h"

typedef struct Client_s {
	u32 id;
	Player player;
	ClientSocket socket;
} Client;

bool init_client(Client* c, const char* port, const char* host);
bool check_connection_status(SOCKET s);
bool send_player_state_to_server(Client* c);
bool send_client_connect(Client* c, const char* name);
bool recieve_server_messages(Client* c, EntityManager* man, NetworkPlayerInfo* players);
