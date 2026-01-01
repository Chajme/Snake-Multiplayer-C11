#ifndef UDSP_SEMESTRALKA_3_CLIENT_HANDLER_H
#define UDSP_SEMESTRALKA_3_CLIENT_HANDLER_H

typedef struct {
    int socket;
    int playerId;
} ClientInfo;

void *handle_client(void *arg);

#endif