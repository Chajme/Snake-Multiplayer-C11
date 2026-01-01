// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <pthread.h>
// #include "../common/protocol.h"
// #include "client_handler.h"
// #include "../game/game.h"
// #include <signal.h>
//
// volatile sig_atomic_t server_running = 1;
//
// GameState gameState;
// pthread_mutex_t gameMutex = PTHREAD_MUTEX_INITIALIZER;
// int clientSockets[MAX_CLIENTS];
// int server_fd;
//
// void handle_sigint(int sig) {
//     (void)sig;
//     server_running = 0;
//     shutdown(server_fd, SHUT_RDWR);
// }
//
// size_t send_all(int sock, void *buf, size_t len) {
//     size_t total = 0;
//     char *p = buf;
//     while (total < len) {
//         ssize_t n = send(sock, p + total, len - total, 0);
//         if (n <= 0) return n;
//         total += n;
//     }
//     return total;
// }
//
// /* ---------- CLIENT ACCEPT THREAD ---------- */
// void *accept_thread(void *arg) {
//     server_fd = *(int *)arg;
//
//     while (server_running) {
//         struct sockaddr_in clientAddr;
//         socklen_t len = sizeof(clientAddr);
//         int client_fd = accept(server_fd, (struct sockaddr*)&clientAddr, &len);
//         if (client_fd < 0) {
//             if (!server_running) break;
//             if (errno == EINTR) continue; // interrupted by signal, retry
//             perror("accept");
//             continue;
//         }
//
//         pthread_mutex_lock(&gameMutex);
//
//         if (gameState.numPlayers >= MAX_CLIENTS) {
//             pthread_mutex_unlock(&gameMutex);
//             close(client_fd);
//             continue;
//         }
//
//         int id = gameState.numPlayers;
//         PlayerState *player_state = &gameState.players[id];
//         player_state->id = id;
//         player_state->alive = 1;
//
//         static const uint8_t colors[][3] = {
//             {0, 255, 0},     // green
//             {0, 0, 255},     // blue
//             {255, 255, 0},   // yellow
//             {255, 0, 255},   // magenta
//             {0, 255, 255},   // cyan
//         };
//
//         int colorIndex = id % 5;
//         player_state->r = colors[colorIndex][0];
//         player_state->g = colors[colorIndex][1];
//         player_state->b = colors[colorIndex][2];
//
//
//         player_state->x = rand() % GRID_WIDTH;
//         player_state->y = rand() % GRID_HEIGHT;
//         player_state->direction = 2;
//         player_state->tail_length = 3;
//
//         for (int i = 0; i < player_state->tail_length; i++) {
//             player_state->tailX[i] = (player_state->x - (i + 1) + GRID_WIDTH) % GRID_WIDTH;
//             player_state->tailY[i] = player_state->y;
//         }
//
//         clientSockets[id] = client_fd;
//         gameState.numPlayers++;
//
//         pthread_mutex_unlock(&gameMutex);
//
//         send_all(client_fd, &id, sizeof(int));
//
//         ServerMessage msg;
//         memset(&msg, 0, sizeof(msg));
//         msg.type = MSG_STATE;
//         msg.state = gameState;
//         send_all(client_fd, &msg, sizeof(msg));
//
//         ClientInfo *info = malloc(sizeof(ClientInfo));
//         if (!info) {
//             close(client_fd);
//             pthread_mutex_unlock(&gameMutex);
//             continue;
//         }
//
//         info->socket = client_fd;
//         info->playerId = id;
//
//         pthread_t t;
//         if (pthread_create(&t, NULL, handle_client, info) != 0) {
//             perror("pthread_create");
//             close(client_fd);
//             free(info);
//             pthread_mutex_unlock(&gameMutex);
//             continue;
//         }
//         pthread_detach(t);
//
//         printf("Client %d connected\n", id);
//     }
//     printf("Accept thread exiting\n");
//     return NULL;
// }
//
// /* ---------- MAIN ---------- */
// int main() {
//     signal(SIGINT, handle_sigint);
//     srand(time(NULL));
//
//     memset(&gameState, 0, sizeof(GameState));
//
//
//     // Initialize each player
//     for (int i = 0; i < MAX_CLIENTS; i++) {
//         gameState.players[i].alive = 0;
//         gameState.players[i].tail_length = 0;
//
//         // Initialize tail arrays to avoid uninitialized memory
//         for (int t = 0; t < MAX_TAIL; t++) {
//             gameState.players[i].tailX[t] = 0;
//             gameState.players[i].tailY[t] = 0;
//         }
//     }
//
//     // Initialize fruit position
//     gameState.fruitX = rand() % GRID_WIDTH;
//     gameState.fruitY = rand() % GRID_HEIGHT;
//
//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     int opt = 1;
//     setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//
//     struct sockaddr_in addr = {0};
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = INADDR_ANY;
//     addr.sin_port = htons(PORT);
//
//     bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
//     listen(server_fd, MAX_CLIENTS);
//
//     pthread_t acceptThread;
//     pthread_create(&acceptThread, NULL, accept_thread, &server_fd);
//
//     /* ---------- GAME LOOP ---------- */
//     while (server_running) {
//         usleep(200000);
//
//         pthread_mutex_lock(&gameMutex);
//
//         for (int i = 0; i < gameState.numPlayers; i++) {
//             PlayerState *p = &gameState.players[i];
//             if (!p->alive) continue;
//
//             if (UpdateGame(p)) {
//                 printf("Player %d died\n", i);
//                 ServerMessage msg;
//                 memset(&msg, 0, sizeof(msg));
//                 msg.type = MSG_GAME_OVER;
//                 send_all(clientSockets[i], &msg, sizeof(msg));
//
//                 /* Close connection */
//                 shutdown(clientSockets[i], SHUT_RDWR);
//                 close(clientSockets[i]);
//
//                 gameState.players[i].alive = 0;
//                 continue;
//             }
//
//             if (p->x < 0) p->x = GRID_WIDTH - 1;
//             if (p->x >= GRID_WIDTH) p->x = 0;
//             if (p->y < 0) p->y = GRID_HEIGHT - 1;
//             if (p->y >= GRID_HEIGHT) p->y = 0;
//
//             if (p->x == gameState.fruitX && p->y == gameState.fruitY) {
//                 p->tail_length++;
//                 gameState.fruitX = rand() % GRID_WIDTH;
//                 gameState.fruitY = rand() % GRID_HEIGHT;
//                 p->score++;
//             }
//         }
//
//         for (int i = 0; i < gameState.numPlayers; i++) {
//             //send_all(clientSockets[i], &gameState, sizeof(GameState));
//             ServerMessage msg;
//             memset(&msg, 0, sizeof(msg));
//             msg.type = MSG_STATE;
//             msg.state = gameState;
//
//             send_all(clientSockets[i], &msg, sizeof(msg));
//         }
//
//         pthread_mutex_unlock(&gameMutex);
//     }
//
//     close(server_fd);
//     pthread_join(acceptThread, NULL);
//
//     for (int i = 0; i < gameState.numPlayers; i++) {
//         if (gameState.players[i].alive) {
//             shutdown(clientSockets[i], SHUT_RDWR);
//             close(clientSockets[i]);
//         }
//     }
//     printf("Server shutdown complete\n");
// }
// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include "../common/protocol.h"
#include "client_handler.h"
#include "server_network.h"
#include "../game/game.h"
#include "server.h"

volatile sig_atomic_t server_running = 1;
GameState gameState;
pthread_mutex_t gameMutex = PTHREAD_MUTEX_INITIALIZER;
int clientSockets[MAX_CLIENTS];
int server_fd;

void handle_sigint(int sig) {
    (void)sig;
    server_running = 0;
    shutdown(server_fd, SHUT_RDWR);
}

static int find_free_player_slot(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!gameState.players[i].alive)
            return i;
    }
    return -1;
}


void *accept_thread(void *arg) {
    server_fd = *(int *)arg;

    while (server_running) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &len);

        if (client_fd < 0) {
            if (!server_running) break;
            continue;
        }

        pthread_mutex_lock(&gameMutex);

        if (gameState.numPlayers >= MAX_CLIENTS) {
            pthread_mutex_unlock(&gameMutex);
            close(client_fd);
            continue;
        }

        int id = find_free_player_slot();
        if (id == -1) {
            pthread_mutex_unlock(&gameMutex);
            close(client_fd);
            continue;
        }

        PlayerState *p = &gameState.players[id];
        memset(p, 0, sizeof(*p));

        p->id = id;
        p->alive = 1;
        p->x = rand() % GRID_WIDTH;
        p->y = rand() % GRID_HEIGHT;
        p->direction = RIGHT;
        p->tail_length = 3;

        static const uint8_t colors[][3] = {
            {0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255}
        };
        p->r = colors[id % 5][0];
        p->g = colors[id % 5][1];
        p->b = colors[id % 5][2];

        for (int i = 0; i < p->tail_length; i++) {
            p->tailX[i] = (p->x - (i + 1) + GRID_WIDTH) % GRID_WIDTH;
            p->tailY[i] = p->y;
        }

        clientSockets[id] = client_fd;

        pthread_mutex_unlock(&gameMutex);

        send_all(client_fd, &id, sizeof(int));

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

int main(void) {
    signal(SIGINT, handle_sigint);
    srand(time(NULL));

    memset(&gameState, 0, sizeof(gameState));
    gameState.fruitX = rand() % GRID_WIDTH;
    gameState.fruitY = rand() % GRID_HEIGHT;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, MAX_CLIENTS);

    pthread_t acceptThread;
    pthread_create(&acceptThread, NULL, accept_thread, &server_fd);

    while (server_running) {
        usleep(200000);

        pthread_mutex_lock(&gameMutex);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            PlayerState *p = &gameState.players[i];
            if (!p->alive) continue;

            if (UpdateGame(p)) {
                ServerMessage msg = {.type = MSG_GAME_OVER};
                send_all(clientSockets[i], &msg, sizeof(msg));
                shutdown(clientSockets[i], SHUT_RDWR);
                close(clientSockets[i]);
                p->alive = 0;
                continue;
            }

            if (p->x == gameState.fruitX && p->y == gameState.fruitY) {
                p->tail_length++;
                p->score++;
                gameState.fruitX = rand() % GRID_WIDTH;
                gameState.fruitY = rand() % GRID_HEIGHT;
            }
        }

        BroadcastState(&gameState, clientSockets);
        pthread_mutex_unlock(&gameMutex);
    }

    close(server_fd);
    pthread_join(acceptThread, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientSockets[i] >= 0) {
            shutdown(clientSockets[i], SHUT_RDWR);
            close(clientSockets[i]);
            clientSockets[i] = -1;
        }
        gameState.players[i].alive = 0;
    }

    printf("Server shutdown complete\n");
    return 0;
}
