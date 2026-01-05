#include "server.h"
#include "game_controller.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t running = 1;

static void on_sigint(int sig) {
    (void)sig;
    running = 0;
}

static int parse_arguments(int argc, char* argv[], char* out_ip, const size_t ip_len, int* out_port) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <bind_ip> <port>\n", argv[0]);
        return 0; // failure
    }

    strncpy(out_ip, argv[1], ip_len - 1);
    out_ip[ip_len - 1] = '\0'; // ensure null-terminated

    *out_port = atoi(argv[2]);
    if (*out_port <= 0 || *out_port > 65535) {
        fprintf(stderr, "Invalid port: %d\n", *out_port);
        return 0;
    }

    return 1; // success
}

int main(int argc, char* argv[]) {
    signal(SIGINT, on_sigint);

    // char ip[64];
    // int port;
    //
    // if (!parse_arguments(argc, argv, ip, sizeof(ip), &port)) {
    //     return 1;
    // }

    // Server* server = server_create(ip, port);
    Server* server = server_create("127.0.0.1", 1338);
    server_start_async(server);

    GameController* ctrl = game_controller_create(server, 60, 45);
    if (!ctrl) {
        server_stop(server);
        server_destroy(server);
        return 1;
    }

    while (running && server_is_running(server)) {
        game_controller_tick(ctrl);
    }

    game_controller_destroy(ctrl);
    server_stop(server);
    server_destroy(server);

    return 0;
}
