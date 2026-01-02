#include "server.h"
#include "../game/game.h"             // defines Game and GameState, game logic
#include "../common/game_protocol.h"  // defines SerializedGameState
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

struct Server {
    int port;
    int server_fd;
    int client_fds[MAX_CLIENTS];
    pthread_t threads[MAX_CLIENTS];
    pthread_mutex_t lock;
    bool running;

    // Simple input queue
    int input_client_idx[1024];
    int input_value[1024];
    int input_head;
    int input_tail;
};

typedef struct {
    Server* server;
    int client_idx;
} ClientThreadData;

static void *client_thread(void *arg) {
    ClientThreadData *data = (ClientThreadData*)arg;
    Server *srv = data->server;
    int idx = data->client_idx;
    free(data);

    char buffer[BUFFER_SIZE];

    while (srv->running) {
        int n = read(srv->client_fds[idx], buffer, sizeof(buffer - 1));
        if (n <= 0) break;

        buffer[n] = '\0';
        int input = atoi(buffer);
        server_push_input(srv, idx, input);
    }

    close(srv->client_fds[idx]);

    pthread_mutex_lock(&srv->lock);
    srv->client_fds[idx] = -1;
    pthread_mutex_unlock(&srv->lock);

    return NULL;
}

static void* server_start_thread(void* arg) {
    server_start((Server*)arg);
    return NULL;
}

Server* server_create(int port) {
    Server* srv = calloc(1, sizeof(Server));
    srv->port = port;
    srv->running = true;
    pthread_mutex_init(&srv->lock, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++)
        srv->client_fds[i] = -1;

    srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->server_fd < 0) {
        perror("Socket creation failed");
        free(srv);
        return NULL;
    }

    int opt = 1;
    setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(srv->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(srv->server_fd);
        free(srv);
        return NULL;
    }

    if (listen(srv->server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(srv->server_fd);
        free(srv);
        return NULL;
    }

    printf("Server listening on port %d\n", port);
    return srv;
}

void server_destroy(Server* srv) {
    if (!srv) return;

    srv->running = false;
    close(srv->server_fd);

    for (int i = 0; i < MAX_CLIENTS; i++)
        if (srv->client_fds[i] != -1)
            close(srv->client_fds[i]);

    pthread_mutex_destroy(&srv->lock);
    free(srv);
}

void server_start(Server* srv) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (srv->running) {
        int client_fd = accept(srv->server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&srv->lock);
        int idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (srv->client_fds[i] == -1) {
                srv->client_fds[i] = client_fd;
                idx = i;
                break;
            }
        }
        pthread_mutex_unlock(&srv->lock);

        if (idx == -1) {
            close(client_fd);
            printf("Max clients reached, connection refused\n");
            continue;
        }

        ClientThreadData* data = malloc(sizeof(ClientThreadData));
        data->server = srv;
        data->client_idx = idx;
        pthread_create(&srv->threads[idx], NULL, client_thread, data);
        pthread_detach(srv->threads[idx]);

        printf("Client %d connected\n", idx);
    }
}

void server_broadcast_state(Server* srv, SerializedGameState* s) {
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->client_fds[i] != -1) {
            write(srv->client_fds[i], s, sizeof(*s));  // send actual struct bytes
        }
    }
    pthread_mutex_unlock(&srv->lock);
}



// Pushes client input to server queue
void server_push_input(Server* srv, int client_idx, int input) {
    pthread_mutex_lock(&srv->lock);
    srv->input_client_idx[srv->input_tail] = client_idx;
    srv->input_value[srv->input_tail] = input;
    srv->input_tail = (srv->input_tail + 1) % 1024;
    pthread_mutex_unlock(&srv->lock);
}

// Gets next input from queue (for game logic)
bool server_get_next_input(Server* srv, int* client_idx, int* input) {
    bool has_input = false;
    pthread_mutex_lock(&srv->lock);
    if (srv->input_head != srv->input_tail) {
        *client_idx = srv->input_client_idx[srv->input_head];
        *input = srv->input_value[srv->input_head];
        srv->input_head = (srv->input_head + 1) % 1024;
        has_input = true;
    }
    pthread_mutex_unlock(&srv->lock);
    return has_input;
}

int main(void) {
    Server* srv = server_create(1337);
    if (!srv) {
        fprintf(stderr, "Failed to create server\n");
        return -1;
    }

    // Start listening for clients
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_start_thread, srv);

    // Create the game
    Game* g = game_create(60, 45);

    Snake* snakes[MAX_CLIENTS] = {0};

    // Game loop
    while (srv->running) {
        int client_idx, input;
        while (server_get_next_input(srv, &client_idx, &input)) {
            if (client_idx >= 0 && client_idx < MAX_CLIENTS) {
                // If this client has no snake yet, create it
                if (!snakes[client_idx]) {
                    snakes[client_idx] = game_add_snake(g, 2 + client_idx * 3, 2 + client_idx * 2);
                    printf("Assigned snake to client %d\n", client_idx);
                }
                snake_set_direction(snakes[client_idx], input);
            }
        }

        game_update(g);

        // Send game state to all clients
        GameState* state = game_get_state(g);
        SerializedGameState s;
        s.num_snakes = gamestate_get_num_snakes(state);
        for (int i = 0; i < s.num_snakes; i++) {
            s.snake_lengths[i] = gamestate_get_snake_length(state, i);
            for (int j = 0; j < s.snake_lengths[i]; j++) {
                s.snake_x[i][j] = gamestate_get_snake_segment_x(state, i, j);
                s.snake_y[i][j] = gamestate_get_snake_segment_y(state, i, j);
            }
            s.snake_scores[i] = gamestate_get_snake_score(state, i);
        }
        s.fruit_x = gamestate_get_fruit_x(state);
        s.fruit_y = gamestate_get_fruit_y(state);
        s.game_over = gamestate_is_game_over(state);

        server_broadcast_state(srv, &s);
        free(state);

        usleep(100000); // ~10 FPS
    }

    game_destroy(g);
    server_destroy(srv);
    return 0;
}