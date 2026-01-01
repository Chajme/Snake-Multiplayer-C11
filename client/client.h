
#ifndef UDSP_SEMESTRALKA_3_CLIENT_H
#define UDSP_SEMESTRALKA_3_CLIENT_H

#include <bits/pthreadtypes.h>

#include "../common/protocol.h"
#include "client_network.h"

typedef struct {
    ClientNet net;
    GameState state;
    pthread_mutex_t mutex;
    int running;
} ClientData;

void *network_thread(void *arg);
void HandleInput(ClientData *data);

#endif