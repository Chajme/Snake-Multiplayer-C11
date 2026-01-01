// server_network.c
#include "server_network.h"
#include <sys/socket.h>
#include <string.h>

size_t send_all(int sock, void *buf, size_t len){
    size_t total = 0;
    char *p = buf;
    while(total < len){
        ssize_t n = send(sock, p + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

void BroadcastState(GameState *state, int *clients) {
    ServerMessage msg = {.type = MSG_STATE, .state = *state};

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] >= 0) {
            send_all(clients[i], &msg, sizeof(msg));
        }
    }
}

