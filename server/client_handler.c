#include "client_handler.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include  "../game/game.h"

extern GameState gameState;
extern pthread_mutex_t gameMutex;

void *handle_client(void *arg) {
    ClientInfo *info = (ClientInfo*)arg;
    uint8_t dir;

    while(1){
        size_t n = recv(info->socket, &dir, sizeof(dir), 0);
        if(n <= 0) {
            // Client disconnected, mark player as dead
            pthread_mutex_lock(&gameMutex);
            gameState.players[info->playerId].alive = 0;
            pthread_mutex_unlock(&gameMutex);

            printf("Client %d disconnected\n", info->playerId);

            break;
        }

        if(dir >= 1 && dir <= 4) {
            pthread_mutex_lock(&gameMutex);
            SetDirection(&gameState.players[info->playerId], dir);
            pthread_mutex_unlock(&gameMutex);
        }
    }

    shutdown(info->socket, SHUT_RDWR);
    close(info->socket);
    free(info);
    return NULL;
}

