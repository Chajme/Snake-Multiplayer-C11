#ifndef UDSP_SEMESTRALKA_3_CLIENT_NETWORK_H
#define UDSP_SEMESTRALKA_3_CLIENT_NETWORK_H

#include "protocol.h"
#include <stdint.h>

typedef struct {
    int sockfd;
    int playerId;
} ClientNet;

int ConnectToServer(ClientNet *net, const char *ip, int port);
int SendDirection(ClientNet *net, uint8_t dir);
int ReceiveGameState(ClientNet *net, GameState *state);

#endif