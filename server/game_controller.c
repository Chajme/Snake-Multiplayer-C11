#include "game_controller.h"
#include "server.h"
#include "game/game.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../util/vector.h"

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

static void process_inputs(const GameController* c) {
    int player, dir;

    while (server_poll_input(c->server, &player, &dir)) {
        if (player < 0 || player >= MAX_CLIENTS)
            continue;

        GameState *state = game_get_state(c->game);
        // Spawn or reset player
        if (!gamestate_is_snake_alive(state, player)) {
            game_spawn_player(c->game, player);

        }
        // Set direction
        game_snake_set_direction(c->game, player, dir);

        game_free_state(state);
    }
}

static void handle_disconnects(const GameController* c) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!server_is_client_connected(c->server, i)) {
            // Only reset the snake if it exists/alive
            GameState* state = game_get_state(c->game);
            if (gamestate_is_snake_alive(state, i)) {
                game_set_snake_dead(c->game, i); // Mark dead in Game
                printf("Client %d disconnected, snake removed\n", i);
            }
            game_free_state(state);
        }
    }
}

static void handle_snake_death(Game* g, int idx) {
    printf("Snake %d died!\n", idx);
    game_set_snake_dead(g, idx);
}

void game_controller_tick(GameController* c) {
    // ensure_snakes_exist(c);
    process_inputs(c);
    game_update(c->game, handle_snake_death);
    handle_disconnects(c);

    GameState* state = game_get_state(c->game);
    SerializedGameState s;
    gamestate_serialize(state, &s);

    server_broadcast_state(c->server, &s);

    game_free_state(state);
    usleep(TICK_USEC);
}