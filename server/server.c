#include "server.h"
#include "../game/game.h"             // defines Game and GameState, game logic
#include "../common/game_protocol.h"// defines SerializedGameState
#include "../util/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

struct Server {
    int port;
    int server_fd;
    int client_fds[MAX_CLIENTS];
    pthread_t threads[MAX_CLIENTS];
    pthread_t accept_thread;
    pthread_mutex_t lock;
    bool running;

    Queue *input_queue;
};

typedef struct {
    Server* server;
    ClientDisconnectCallback on_disconnect;
    int client_idx;
} ClientThreadData;

typedef struct {
    int client_id;
    int direction;
} PlayerInput;

static volatile sig_atomic_t shutdown_requested = 0;

static void handle_signal(int sig) {
    (void)sig;
    shutdown_requested = 1;
}

static void *client_thread(void *arg) {
    ClientThreadData *data = (ClientThreadData*)arg;
    Server *srv = data->server;
    int idx = data->client_idx;
    free(data);

    char buffer[BUFFER_SIZE];

    while (srv->running) {
        int n = read(srv->client_fds[idx], buffer, sizeof(buffer) - 1);
        if (n <= 0) break;

        buffer[n] = '\0';
        int input = atoi(buffer);
        server_push_input(srv, idx, input);
    }

    pthread_mutex_lock(&srv->lock);
    int fd = srv->client_fds[idx];
    srv->client_fds[idx] = -1;
    pthread_mutex_unlock(&srv->lock);

    if (fd != -1)
        close(fd);

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

    srv->input_queue = queue_new(sizeof(PlayerInput), 1024);
    if (!srv->input_queue) {
        perror("Failed to create input queue");
        close(srv->server_fd);
        pthread_mutex_destroy(&srv->lock);
        free(srv);
        return NULL;
    }

    return srv;
}

void server_destroy(Server* srv) {
    if (!srv) return;

    queue_free(srv->input_queue);

    pthread_mutex_destroy(&srv->lock);
    free(srv);
}

void server_start(Server* srv) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (srv->running) {
        int client_fd = accept(srv->server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
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

        AssignPlayerMsg msg = { .player_id = idx };
        write(client_fd, &msg, sizeof(msg));

        ClientThreadData* data = malloc(sizeof(ClientThreadData));
        data->server = srv;
        data->client_idx = idx;
        pthread_create(&srv->threads[idx], NULL, client_thread, data);
        // pthread_detach(srv->threads[idx]);

        printf("Client %d connected\n", idx);
    }
}

void server_start_async(Server* srv) {
    pthread_create(&srv->accept_thread, NULL, server_start_thread, srv);
}

void server_stop(Server *srv) {
    if (!srv) return;

    pthread_mutex_lock(&srv->lock);
    srv->running = false;
    pthread_mutex_unlock(&srv->lock);

    // This unblocks accept()
    shutdown(srv->server_fd, SHUT_RDWR);
    close(srv->server_fd);

    // Close all client sockets (client threads will exit)
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->client_fds[i] != -1) {
            shutdown(srv->client_fds[i], SHUT_RDWR);
            close(srv->client_fds[i]);
            srv->client_fds[i] = -1;
        }
    }
    pthread_mutex_unlock(&srv->lock);

    // Wait for accept thread to exit
    pthread_join(srv->accept_thread, NULL);

    // Wait for all client threads to exit
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->threads[i]) {
            pthread_join(srv->threads[i], NULL);
        }
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
    if (!srv) return;

    PlayerInput ev = {
        .client_id = client_idx,
        .direction = input
    };

    pthread_mutex_lock(&srv->lock);
    queue_push(srv->input_queue, &ev);
    pthread_mutex_unlock(&srv->lock);
}

// Gets next input from queue (for game logic)
bool server_get_next_input(Server* srv, int* client_idx, int* input) {
    if (!srv || !client_idx || !input) return false;

    PlayerInput ev;

    pthread_mutex_lock(&srv->lock);
    int ok = queue_pop(srv->input_queue, &ev);
    pthread_mutex_unlock(&srv->lock);

    if (!ok) return false;

    *client_idx = ev.client_id;
    *input = ev.direction;
    return true;
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    Server* srv = server_create(1337);
    if (!srv) {
        fprintf(stderr, "Failed to create server\n");
        return -1;
    }

    server_start_async(srv);

    // Create the game
    Game* g = game_create(60, 45);

    Snake* snakes[MAX_CLIENTS] = {0};

    // Game loop
    while (srv->running && !shutdown_requested) {
        int client_idx, input;

        while (server_get_next_input(srv, &client_idx, &input)) {
            if (client_idx >= 0 && client_idx < MAX_CLIENTS) {
                // If this client has no snake yet, create it
                if (!snakes[client_idx]) {
                    snakes[client_idx] = game_reset_snake(g, client_idx);
                    // snakes[client_idx] = game_add_snake(g, 2 + client_idx * 3, 2 + client_idx * 2);
                    printf("Assigned snake to client %d\n", client_idx);
                    // game_set_snake_alive(g, client_idx);
                }

                if (snakes[client_idx]) {
                    game_snake_set_direction(g,client_idx, input);
                }
            }
        }

        game_update(g);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (snakes[i] && srv->client_fds[i] == -1) {   //  && gamestate_is_snake_alive(state, i
                game_set_snake_dead(g, i);
                snakes[i] = NULL;
                printf("Snake for client %d destroyed due to disconnect\n", i);
            }
        }

        // Send game state to all clients
        GameState* state = game_get_state(g);
        SerializedGameState s;
        gamestate_serialize(state, &s);
        server_broadcast_state(srv, &s);
        game_free_state(state);

        usleep(100000); // ~10 FPS
    }

    server_stop(srv);
    game_destroy(g);
    server_destroy(srv);
    return 0;
}