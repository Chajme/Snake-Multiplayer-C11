#ifndef SNAKEGAME_CONTROLLER_H
#define SNAKEGAME_CONTROLLER_H

#include "server.h"

typedef struct GameController GameController;

GameController* game_controller_create(Server* server, int width, int height);
void game_controller_destroy(GameController* ctrl);

void game_controller_run(GameController* ctrl);
void game_controller_tick(GameController* c);

#endif