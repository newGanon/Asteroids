#pragma once
#include "player.h"
#include "network.h"

typedef struct client_s {
	Player player;
	ClientSocket socket;

}client;