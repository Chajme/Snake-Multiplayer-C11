
#include "client.h"

#include <SDL_ttf.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <bits/socket.h>
#include <netinet/in.h>

#include "gui/game_render.h"

struct Client {
    int sock;
    int player_id;
    bool connected;

    GameRenderer* renderer;
    SerializedGameState state;

    int width;
    int height;
    int cell_size;
};

void client_destroy(Client* c) {
    if (!c) return;
    if (c->renderer) renderer_destroy(c->renderer);
    if (c->connected) close(c->sock);
    free(c);
}

Client* client_create(const char* ip, int port, int width, int height, int cell_size) {
    Client* c = malloc(sizeof(Client));
    if (!c) return NULL;

    c->sock = socket(AF_INET, SOCK_STREAM, 0);
    c->connected = false;
    if (c->sock < 0) {
        // free(c);
        client_destroy(c);
        return NULL;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(c->sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        client_destroy(c);
        return NULL;
    }

    // Receive player ID
    AssignPlayerMsg msg;
    if (recv(c->sock, &msg, sizeof(msg), 0) != sizeof(msg)) {
        fprintf(stderr, "Failed to receive player ID\n");
        // close(c->sock);
        // free(c);
        client_destroy(c);
        return NULL;
    }

    fprintf(stderr, "Received player ID %d\n", msg.player_id);

    c->player_id = msg.player_id;
    c->connected = true;
    c->width = width;
    c->height = height;
    c->cell_size = cell_size;
    c->renderer = NULL;

    // renderer_init(&c->renderer, "Snake Multiplayer", width, height, cell_size);
    c->renderer = renderer_create("Snake Multiplayer", width, height, cell_size);
    if (!c->renderer) {
        fprintf(stderr, "Renderer init failed\n");
        client_destroy(c);
        return NULL;
    }

    return c;
}

void client_send_input(Client* c, int dir) {
    if (!c || !c->connected) return;
    int net_dir = (int)htonl(dir);
    send(c->sock, &net_dir, sizeof(net_dir), 0);
}

bool client_receive_state(Client* c) {
    if (!c || !c->connected) return false;
    char* ptr = (char*)&c->state;
    size_t size = sizeof(c->state);
    size_t received = 0;

    while (received < size) {
        int n = (int)recv(c->sock, ptr + received, size - received, 0);
        if (n <= 0) {
            printf("Server shut down!\n");
            c->connected = false;
            return false;
        }
        received += n;
    }
    return true;
}

void client_handle_input(Client* c, const SDL_Event* event) {
    if (event->type != SDL_KEYDOWN) return;

    int dir = -1;
    switch (event->key.keysym.sym) {
        case SDLK_UP:    dir = 0; break;
        case SDLK_RIGHT: dir = 1; break;
        case SDLK_DOWN:  dir = 2; break;
        case SDLK_LEFT:  dir = 3; break;
        default: ;
    }
    if (dir != -1) client_send_input(c, dir);
}

void client_render(Client* c) {
    if (!c) return;
    renderer_draw_serialized(c->renderer, &c->state, c->player_id, c->connected);
}

bool client_is_connected(Client *c) {
    if (!c) return false;
    return c->connected;
}

void client_set_connected(Client *c, bool connected) {
    if (!c) return;
    c->connected = connected;
}



