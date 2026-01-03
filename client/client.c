#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL2/SDL.h>
#include "../gui/game_render.h"
#include "../common/game_protocol.h"

#define WIDTH 60
#define HEIGHT 45
#define CELL_SIZE 20

struct Client {
    int sock;
    int player_id;
    bool connected;
};

Client* client_create(const char* ip, int port) {
    Client* c = malloc(sizeof(Client));
    c->sock = socket(AF_INET, SOCK_STREAM, 0);
    c->connected = false;
    if (c->sock < 0) { free(c); return NULL; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(c->sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(c->sock);
        free(c);
        return NULL;
    }

    c->player_id = -1;
    c->connected = true;
    return c;
}

void client_destroy(Client* c) {
    if (!c) return;
    if (c->connected) close(c->sock);
    free(c);
}

void client_send_input(Client* c, int input) {
    if (!c || !c->connected) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", input);
    send(c->sock, buf, strlen(buf), 0);
}

bool client_receive_state(Client* c, void* buffer, size_t size) {
    char* ptr = (char*)buffer;
    size_t received = 0;
    while (received < size) {
        int n = recv(c->sock, ptr + received, size - received, 0);
        if (n <= 0) return false; // disconnected or error
        received += n;
    }
    return true;
}

bool client_is_connected(Client* c) {
    if (!c) return false;
    return c->connected;
}

int main(void) {
    Client* c = client_create("127.0.0.1", 1337);
    if (!c) {
        fprintf(stderr, "Failed to create client\n");
        return -1;
    }

    AssignPlayerMsg id_msg;
    if (!client_receive_state(c, &id_msg, sizeof(id_msg))) {
        fprintf(stderr, "Failed to receive player ID\n");
        client_destroy(c);
        return -1;
    }

    c->player_id = id_msg.player_id;
    printf("Assigned player ID: %d\n", c->player_id);

    GameRenderer* gr = NULL;
    if (renderer_init(&gr, "Snake Multiplayer", WIDTH, HEIGHT, CELL_SIZE) != 0) {
        fprintf(stderr, "Renderer init failed\n");
        client_destroy(c);
        return -1;
    }

    SDL_Event event;
    bool quit = false;

    SerializedGameState s;

    while (!quit) {
        // Poll SDL events for input
        while (SDL_PollEvent(&event)) {
            int dir = -1;
            if (event.type == SDL_QUIT) quit = true;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: dir = 0; break;
                    case SDLK_RIGHT: dir = 1; break;
                    case SDLK_DOWN: dir = 2; break;
                    case SDLK_LEFT: dir = 3; break;
                }
                if (dir != -1) client_send_input(c, dir);
            }
        }

        // Receive serialized state
        if (client_receive_state(c, &s, sizeof(s))) {
            renderer_draw_serialized(gr, &s, c->player_id);
        }

        SDL_Delay(16); // ~60 FPS
    }

    renderer_destroy(gr);
    client_destroy(c);
    return 0;
}
