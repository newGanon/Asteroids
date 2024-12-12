#pragma once
#include "player.h"
#include "network.h"

typedef struct Client_s {
	Player player;
	ClientSocket socket;

} Client;

bool init_client(Client* c, char* port);
bool send_player_state_to_server(Client* c);
bool revieve_server_messages(Client* c, EntityManager* man);
