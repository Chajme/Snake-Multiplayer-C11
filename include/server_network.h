#ifndef UDSP_SEMESTRALKA_3_SERVER_NETWORKS_H
#define UDSP_SEMESTRALKA_3_SERVER_NETWORKS_H

#include <stddef.h>
#include "protocol.h"

size_t send_all(int sock, void *buf, size_t len);
void BroadcastState(GameState *state, int *clients);

#endif