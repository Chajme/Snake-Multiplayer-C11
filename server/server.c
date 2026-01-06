#include "server.h"
#include "../util/queue.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 256

typedef struct {
    int player_id;
    int direction;
} InputEvent;

typedef struct {
    Server* srv;
    int idx;
} ClientThreadData;

struct Server {
    int port;
    int server_fd;
    int client_fds[MAX_CLIENTS];

    pthread_t accept_thread;
    pthread_t client_threads[MAX_CLIENTS];

    pthread_mutex_t lock;
    bool running;

    Queue* input_queue;
};

static void* client_thread(void* arg) {
    ClientThreadData* ctx = arg;

    Server* srv = ctx->srv;
    int idx = ctx->idx;
    free(ctx);

    printf("Client %d connected\n", idx);

    while (srv->running) {
        int net_dir;
        int n = (int)read(srv->client_fds[idx], &net_dir, sizeof(net_dir));
        if (n != sizeof(net_dir)) break;

        int dir = (int)ntohl(net_dir);

        InputEvent ev = { idx, dir };

        pthread_mutex_lock(&srv->lock);
        queue_push(srv->input_queue, &ev);
        pthread_mutex_unlock(&srv->lock);
    }

    pthread_mutex_lock(&srv->lock);
    close(srv->client_fds[idx]);
    srv->client_fds[idx] = -1;
    pthread_mutex_unlock(&srv->lock);

    return NULL;
}

static void* accept_thread(void* arg) {
    Server* srv = arg;

    while (srv->running) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int fd = accept(srv->server_fd, (struct sockaddr*)&addr, &len);
        if (fd < 0) continue;

        pthread_mutex_lock(&srv->lock);
        int idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (srv->client_fds[i] == -1) {
                srv->client_fds[i] = fd;
                idx = i;
                break;
            }
        }
        pthread_mutex_unlock(&srv->lock);

        if (idx < 0) {
            close(fd);
            continue;
        }

        AssignPlayerMsg msg = { .player_id = idx };
        write(fd, &msg, sizeof(msg));

        ClientThreadData* ctx = malloc(sizeof(ClientThreadData));
        ctx->srv = srv;
        ctx->idx = idx;

        pthread_create(&srv->client_threads[idx], NULL, client_thread, ctx);
    }

    return NULL;
}

Server* server_create(const char* ip, const int port) {
    Server* srv = calloc(1, sizeof(Server));
    if (!srv) {
        server_destroy(srv);
        free(srv);
        return NULL;
    }

    srv->port = port;
    srv->running = true;

    pthread_mutex_init(&srv->lock, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        srv->client_fds[i] = -1;
    }

    srv->input_queue = queue_new(sizeof(InputEvent), 1024);

    srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->server_fd < 0) {
        perror("socket");
        server_destroy(srv);
        return NULL;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        server_destroy(srv);
        return NULL;
    }

    if (bind(srv->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        server_destroy(srv);
        return NULL;
    }

    if (listen(srv->server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        server_destroy(srv);
        return NULL;

    }

    printf("Server listening on port %d\n", srv->port);

    return srv;
}

void server_destroy(Server* srv) {
    if (!srv) return;

    server_stop(srv);

    // Server must already be stopped
    queue_free(srv->input_queue);
    pthread_mutex_destroy(&srv->lock);

    srv->input_queue = NULL;

    free(srv);
}


void server_stop(Server* srv) {
    if (!srv) return;

    pthread_mutex_lock(&srv->lock);
    if (!srv->running) {
        pthread_mutex_unlock(&srv->lock);
        return;
    }
    srv->running = false;
    pthread_mutex_unlock(&srv->lock);

    // Unblock accept()
    shutdown(srv->server_fd, SHUT_RDWR);
    close(srv->server_fd);

    // Close all client sockets to unblock client threads
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->client_fds[i] != -1) {
            shutdown(srv->client_fds[i], SHUT_RDWR);
            close(srv->client_fds[i]);
            srv->client_fds[i] = -1;
        }
    }
    pthread_mutex_unlock(&srv->lock);

    /* Wait for accept thread */
    pthread_join(srv->accept_thread, NULL);

    // Wait for all client threads
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->client_threads[i]) {
            pthread_join(srv->client_threads[i], NULL);
            srv->client_threads[i] = 0;
        }
    }
}


void server_start_async(Server* srv) {
    pthread_create(&srv->accept_thread, NULL, accept_thread, srv);
}

bool server_poll_input(Server* srv, int* player, int* dir) {
    InputEvent ev;
    pthread_mutex_lock(&srv->lock);
    int ok = queue_pop(srv->input_queue, &ev);
    pthread_mutex_unlock(&srv->lock);

    if (!ok) return false;

    *player = ev.player_id;
    *dir = ev.direction;
    return true;
}

void server_broadcast_state(Server* srv, const SerializedGameState* state) {
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->client_fds[i] != -1)
            write(srv->client_fds[i], state, sizeof(*state));
    }
    pthread_mutex_unlock(&srv->lock);
}

bool server_is_client_connected(const Server *srv, int player_id) {
    if (!srv) return false;
    if (player_id < 0 || player_id >= MAX_CLIENTS) return false;

    pthread_mutex_lock((pthread_mutex_t*)&srv->lock);
    bool connected = (srv->client_fds[player_id] != -1);
    pthread_mutex_unlock((pthread_mutex_t*)&srv->lock);

    return connected;
}

bool server_is_running(const Server* srv) {
    if (!srv) return false;
    return srv->running;
}
