#include "client_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

extern GameState gameState;  // from server.c
extern pthread_mutex_t gameMutex;

void *handle_client(void *arg) {
    ClientInfo *info = (ClientInfo*)arg;
    uint8_t dir;

    while(1){
        size_t n = recv(info->socket, &dir, sizeof(dir), 0);
        if(n <= 0) break;

        if(dir >= 1 && dir <= 4) {
            pthread_mutex_lock(&gameMutex);
            gameState.players[info->playerId].direction = dir;
            pthread_mutex_unlock(&gameMutex);
        }
    }

    close(info->socket);
    free(info);
    return NULL;
}

