#include "server.h"
#include "game_controller.h"
#include <signal.h>

static volatile sig_atomic_t running = 1;

static void on_sigint(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGINT, on_sigint);

    Server* server = server_create(1338);
    server_start_async(server);

    GameController* ctrl =
        game_controller_create(server, 60, 45);

    while (running && server_is_running(server)) {
        game_controller_tick(ctrl);
    }

    game_controller_destroy(ctrl);
    server_stop(server);
    server_destroy(server);

    return 0;
}
