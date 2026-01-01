// #include <SDL2/SDL.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <pthread.h>
//
// #include "client_network.h"
// #include "../common/protocol.h"
// #include "../client/game_render.h"
//
// PlayerState player;
// GameState state;
// int sockfd;
// pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;
// int playerId = 0;
// int hasState = 0;
//
// volatile int running = 1;
//
// size_t recv_all(int sock, void *buf, size_t len){
//     size_t total = 0;
//     char *p = (char*)buf;
//     while(total < len){
//         ssize_t n = recv(sock, p + total, len - total, 0);
//         if(n <= 0) return n;
//         total += n;
//     }
//     return total;
// }
//
// void *network_thread(void *arg){
//     while(running) {
//         ServerMessage msg;
//         int bytes = recv_all(sockfd, &msg, sizeof(msg));
//         if (bytes <= 0) {
//             running = 0;
//             break;
//         }
//
//         if (msg.type == MSG_GAME_OVER) {
//             pthread_mutex_lock(&stateMutex);
//             int finalScore = player.score;
//             pthread_mutex_unlock(&stateMutex);
//
//             printf("Game over!\n");
//             printf("Your score was: %d\n", finalScore);
//             fflush(stdout);
//
//             running = 0;
//             return NULL;
//         }
//
//         if (msg.type == MSG_STATE) {
//             pthread_mutex_lock(&stateMutex);
//             memset(&state, 0, sizeof(GameState));
//             state = msg.state;
//             if (playerId < state.numPlayers) {
//                 // Clamp tail length to MAX_TAIL to avoid reading past arrays
//                 if (state.players[playerId].tail_length > MAX_TAIL)
//                     state.players[playerId].tail_length = MAX_TAIL;
//                 player = state.players[playerId];
//                 hasState = 1;
//             } else {
//                 hasState = 0;
//             }
//             pthread_mutex_unlock(&stateMutex);
//         }
//     }
//     return NULL;
// }
//
// // int main(){
// //     sockfd = socket(AF_INET, SOCK_STREAM, 0);
// //     if(sockfd<0){ perror("socket"); return -1; }
// //
// //     struct sockaddr_in serverAddr;
// //     serverAddr.sin_family = AF_INET;
// //     serverAddr.sin_port = htons(PORT);
// //     inet_pton(AF_INET,"127.0.0.1",&serverAddr.sin_addr);
// //
// //     if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){ perror("connect"); return -1; }
// //
// //     if(recv(sockfd, &playerId, sizeof(int), 0) <= 0){
// //         perror("recv playerId");
// //         return -1;
// //     }
// //     printf("Assigned player ID: %d\n", playerId);
// //
// //     SDL_Init(SDL_INIT_VIDEO);
// //     SDL_Window *win = SDL_CreateWindow(
// //         "Snake Client",
// //         SDL_WINDOWPOS_CENTERED,
// //         SDL_WINDOWPOS_CENTERED,
// //         GRID_WIDTH*CELL_SIZE,
// //         GRID_HEIGHT*CELL_SIZE,
// //         0
// //         );
// //     if (!win) {
// //         fprintf(stderr, "SDL_CreateWindow failed\n");
// //         SDL_Quit();
// //         return -1;
// //     }
// //     SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
// //     if (!r) {
// //         fprintf(stderr, "SDL_CreateRenderer failed\n");
// //         SDL_DestroyWindow(win);  // <- Fix
// //         SDL_Quit();
// //         return -1;
// //     }
// //
// //     char title[64];
// //     sprintf(title, "Snake Client - Player %d", playerId);
// //     SDL_SetWindowTitle(win, title);
// //
// //     pthread_t netThread;
// //     if (pthread_create(&netThread, NULL, network_thread, NULL) != 0) {
// //         perror("pthread_create");
// //         close(sockfd);
// //         return -1;
// //     }
// //
// //
// //     SDL_Event e;
// //     while(running){
// //         while(SDL_PollEvent(&e)){
// //             if(e.type==SDL_QUIT) running=0;
// //             if(e.type==SDL_KEYDOWN){
// //                 char dir = 0;
// //                 switch(e.key.keysym.sym){
// //                     case SDLK_a: dir=1; break;
// //                     case SDLK_d: dir=2; break;
// //                     case SDLK_w: dir=3; break;
// //                     case SDLK_s: dir=4; break;
// //                 }
// //
// //                 if(dir){
// //                     uint8_t d = (uint8_t)dir;      // ensure unsigned
// //                     ssize_t sent = send(sockfd, &d, sizeof(d), 0);
// //                     if(sent != sizeof(d)) {
// //                         perror("send");
// //                     }
// //                 }
// //             }
// //         }
// //
// //         pthread_mutex_lock(&stateMutex);
// //         if (hasState)
// //             DrawGame(r, playerId, &state);
// //         pthread_mutex_unlock(&stateMutex);
// //
// //         SDL_Delay(50);
// //     }
// //
// //     running = 0;
// //     shutdown(sockfd, SHUT_RDWR);
// //     pthread_join(netThread, NULL);
// //     close(sockfd);
// //
// //     SDL_DestroyRenderer(r);
// //     SDL_DestroyWindow(win);
// //     SDL_Delay(100);
// //     SDL_Quit();
// //     return 0;
// // }
// int main(){
//     ClientNet net;
//     GameState state;
//     pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//
//     if(ConnectToServer(&net, "127.0.0.1", PORT) < 0) return -1;
//
//     SDL_Init(SDL_INIT_VIDEO);
//     SDL_Window *win = SDL_CreateWindow(...);
//     SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
//
//     pthread_t netThread;
//     pthread_create(&netThread, NULL, network_thread, &net);
//
//     while(running){
//         HandleInput(&net);
//         pthread_mutex_lock(&mutex);
//         DrawGame(r, net.playerId, &state);
//         pthread_mutex_unlock(&mutex);
//         SDL_Delay(50);
//     }
//
//     pthread_join(netThread, NULL);
//     SDL_DestroyRenderer(r);
//     SDL_DestroyWindow(win);
//     SDL_Quit();
//     return 0;
// }
// client.c
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "client_network.h"
#include "game_render.h"
#include "../common/protocol.h"
#include "client.h"

void *network_thread(void *arg) {
    ClientData *data = arg;

    while (data->running) {
        pthread_mutex_lock(&data->mutex);
        // int res = ReceiveGameState(&data->net, &data->state);
        int res = ReceiveGameState(&data->net, &data->state);
        if (res <= 0) {
            printf("Disconnected from server\n");
            data->running = 0;
            pthread_mutex_unlock(&data->mutex);
            break;
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (data->state.players[i].tail_length < 0)
                data->state.players[i].tail_length = 0;
            if (data->state.players[i].tail_length > MAX_TAIL)
                data->state.players[i].tail_length = MAX_TAIL;
        }

        pthread_mutex_unlock(&data->mutex);

        usleep(5000);
    }
    return NULL;
}

void HandleInput(ClientData *data) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            data->running = 0;

        if (e.type == SDL_KEYDOWN) {
            uint8_t dir = 0;
            switch (e.key.keysym.sym) {
                case SDLK_a: dir = LEFT; break;
                case SDLK_d: dir = RIGHT; break;
                case SDLK_w: dir = UP; break;
                case SDLK_s: dir = DOWN; break;
            }
            if (dir)
                SendDirection(&data->net, dir);
        }
    }
}

int main(void) {
    ClientData client = {0};
    client.running = 1;
    pthread_mutex_init(&client.mutex, NULL);

    memset(&client.state, 0, sizeof(GameState));

    if (ConnectToServer(&client.net, "127.0.0.1", PORT) < 0) {
        fprintf(stderr, "Failed to connect\n");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
        "Snake Client",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GRID_WIDTH * CELL_SIZE,
        GRID_HEIGHT * CELL_SIZE,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    pthread_t netThread;
    pthread_create(&netThread, NULL, network_thread, &client);

    while (client.running) {
        HandleInput(&client);

        pthread_mutex_lock(&client.mutex);
        DrawGame(renderer, client.net.playerId, &client.state);
        pthread_mutex_unlock(&client.mutex);

        SDL_Delay(50);
    }

    pthread_join(netThread, NULL);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    close(client.net.sockfd);
    pthread_mutex_destroy(&client.mutex);

    return 0;
}
