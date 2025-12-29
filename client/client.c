#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "../common/protocol.h"
#include "game.h"

PlayerState player;
GameState state;
int sockfd;
pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;
int playerId = 0;
int hasState = 0;


size_t recv_all(int sock, void *buf, size_t len){
    size_t total = 0;
    char *p = (char*)buf;
    while(total < len){
        ssize_t n = recv(sock, p + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

void *network_thread(void *arg){
    while(1){
        GameState newState;
        int bytes = recv_all(sockfd, &newState, sizeof(GameState));
        if(bytes <= 0) break;

        pthread_mutex_lock(&stateMutex);
        state = newState;
        player = state.players[playerId];
        hasState = 1;
        pthread_mutex_unlock(&stateMutex);

    }
    return NULL;
}

int main(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){ perror("socket"); return -1; }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&serverAddr.sin_addr);

    if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){ perror("connect"); return -1; }

    if(recv(sockfd, &playerId, sizeof(int), 0) <= 0){
        perror("recv playerId");
        return -1;
    }
    printf("Assigned player ID: %d\n", playerId);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow(
        "Snake Client",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GRID_WIDTH*CELL_SIZE,
        GRID_HEIGHT*CELL_SIZE,
        0
        );
    SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    char title[64];
    sprintf(title, "Snake Client - Player %d", playerId);
    SDL_SetWindowTitle(win, title);

    pthread_t netThread;
    pthread_create(&netThread, NULL, network_thread, NULL);

    SDL_Event e;
    int running=1;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=0;
            if(e.type==SDL_KEYDOWN){
                char dir = 0;
                switch(e.key.keysym.sym){
                    case SDLK_a: dir=1; break;
                    case SDLK_d: dir=2; break;
                    case SDLK_w: dir=3; break;
                    case SDLK_s: dir=4; break;
                }

                if(dir){
                    uint8_t d = (uint8_t)dir;      // ensure unsigned
                    ssize_t sent = send(sockfd, &d, sizeof(d), 0);
                    if(sent != sizeof(d)) {
                        perror("send");
                    }
                }
            }
        }

        pthread_mutex_lock(&stateMutex);
        if (hasState)
            DrawGame(r, playerId, &state);
        pthread_mutex_unlock(&stateMutex);


        SDL_Delay(50);
    }

    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
    close(sockfd);
    return 0;
}
