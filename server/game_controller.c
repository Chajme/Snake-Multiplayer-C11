#include "game_controller.h"
#include "server.h"
#include "game/game.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define TICK_USEC 100000  // 10 FPS

struct GameController {
    Server* server;
    Game* game;
    int running;

    Snake* snakes[MAX_CLIENTS];
};

GameController* game_controller_create(Server* server,  int width, int height)
{
    GameController* c = malloc(sizeof(GameController));
    if (!c) return NULL;

    c->server = server;
    c->game = game_create(width, height);

    for (int i = 0; i < MAX_CLIENTS; i++)
        c->snakes[i] = NULL;

    return c;
}

void game_controller_destroy(GameController* c) {
    if (!c) return;
    game_destroy(c->game);
    free(c);
}

static void process_inputs(GameController* c) {
    int player, dir;

    while (server_poll_input(c->server, &player, &dir)) {
        if (player < 0 || player >= MAX_CLIENTS)
            continue;

        if (!c->snakes[player]) {
            c->snakes[player] = game_reset_snake(c->game, player);
            printf("Spawned snake for player %d\n", player);
        }

        game_snake_set_direction(c->game, player, dir);
    }
}

static void handle_disconnects(GameController* c) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c->snakes[i] && !server_is_client_connected(c->server, i)) {
            game_set_snake_dead(c->game, i);
            c->snakes[i] = NULL;
            printf("Client %d disconnected\n", i);
        }
    }
}

void game_controller_tick(GameController* c) {
    // ensure_snakes_exist(c);
    process_inputs(c);
    game_update(c->game);
    handle_disconnects(c);

    GameState* state = game_get_state(c->game);
    SerializedGameState s;
    gamestate_serialize(state, &s);

    server_broadcast_state(c->server, &s);

    game_free_state(state);
    usleep(TICK_USEC);
}