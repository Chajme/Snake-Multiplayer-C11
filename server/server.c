#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../common/protocol.h"
#include "client_handler.h"
#include "../game/game.h"

GameState gameState;
pthread_mutex_t gameMutex = PTHREAD_MUTEX_INITIALIZER;
int clientSockets[MAX_CLIENTS];

size_t send_all(int sock, void *buf, size_t len) {
    size_t total = 0;
    char *p = buf;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}

/* ---------- CLIENT ACCEPT THREAD ---------- */
void *accept_thread(void *arg) {
    int server_fd = *(int *)arg;

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        int client_fd = accept(server_fd, (struct sockaddr*)&clientAddr, &len);
        if (client_fd < 0) continue;


        pthread_mutex_lock(&gameMutex);

        if (gameState.numPlayers >= MAX_CLIENTS) {
            pthread_mutex_unlock(&gameMutex);
            close(client_fd);
            continue;
        }

        int id = gameState.numPlayers;
        PlayerState *p = &gameState.players[id];
        p->id = id;

        static const uint8_t colors[][3] = {
            {0, 255, 0},     // green
            {0, 0, 255},     // blue
            {255, 255, 0},   // yellow
            {255, 0, 255},   // magenta
            {0, 255, 255},   // cyan
        };

        int colorIndex = id % 5;
        p->r = colors[colorIndex][0];
        p->g = colors[colorIndex][1];
        p->b = colors[colorIndex][2];


        p->x = rand() % GRID_WIDTH;
        p->y = rand() % GRID_HEIGHT;
        p->direction = 2;
        p->tail_length = 3;

        for (int i = 0; i < p->tail_length; i++) {
            p->tailX[i] = (p->x - (i + 1) + GRID_WIDTH) % GRID_WIDTH;
            p->tailY[i] = p->y;
        }

        clientSockets[id] = client_fd;
        gameState.numPlayers++;

        pthread_mutex_unlock(&gameMutex);

        send_all(client_fd, &id, sizeof(int));
        // send_all(client_fd, &gameState, sizeof(GameState));

        ServerMessage msg;
        msg.type = MSG_STATE;
        msg.state = gameState;
        send_all(client_fd, &msg, sizeof(msg));

        ClientInfo *info = malloc(sizeof(ClientInfo));
        info->socket = client_fd;
        info->playerId = id;

        pthread_t t;
        pthread_create(&t, NULL, handle_client, info);
        pthread_detach(t);

        printf("Client %d connected\n", id);
    }
    return NULL;
}

/* ---------- MAIN ---------- */
int main() {
    srand(time(NULL));
    memset(&gameState, 0, sizeof(GameState));
    gameState.fruitX = rand() % GRID_WIDTH;
    gameState.fruitY = rand() % GRID_HEIGHT;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, MAX_CLIENTS);

    pthread_t acceptThread;
    pthread_create(&acceptThread, NULL, accept_thread, &server_fd);

    /* ---------- GAME LOOP ---------- */
    while (1) {
        usleep(200000);

        pthread_mutex_lock(&gameMutex);

        for (int i = 0; i < gameState.numPlayers; i++) {
            PlayerState *p = &gameState.players[i];

            if (UpdateGame(p)) {
                printf("Player %d died\n", i);

                ServerMessage msg;
                msg.type = MSG_GAME_OVER;
                send_all(clientSockets[i], &msg, sizeof(msg));

                close(clientSockets[i]);

                for (int j = i; j < gameState.numPlayers - 1; ++j) {
                    gameState.players[j] = gameState.players[j + 1];
                    clientSockets[j] = clientSockets[j + 1];
                }

                --gameState.numPlayers;
                --i;
                continue;
            }

            if (p->x < 0) p->x = GRID_WIDTH - 1;
            if (p->x >= GRID_WIDTH) p->x = 0;
            if (p->y < 0) p->y = GRID_HEIGHT - 1;
            if (p->y >= GRID_HEIGHT) p->y = 0;

            if (p->x == gameState.fruitX && p->y == gameState.fruitY) {
                p->tail_length++;
                gameState.fruitX = rand() % GRID_WIDTH;
                gameState.fruitY = rand() % GRID_HEIGHT;
            }
        }

        for (int i = 0; i < gameState.numPlayers; i++) {
            //send_all(clientSockets[i], &gameState, sizeof(GameState));
            ServerMessage msg;
            msg.type = MSG_STATE;
            msg.state = gameState;

            send_all(clientSockets[i], &msg, sizeof(msg));
        }

        pthread_mutex_unlock(&gameMutex);
    }
}
