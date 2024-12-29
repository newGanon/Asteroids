#pragma once
#include "player.h"
#include "network.h"

typedef struct Client_s {
	Player player;
	ClientSocket socket;
	u32 id;
} Client;

bool init_client(Client* c, char* port);
bool send_player_state_to_server(Client* c);
bool send_client_connect(Client* c, const char* name);
bool recieve_server_messages(Client* c, EntityManager* man, NetworkPlayerInfo* players);
