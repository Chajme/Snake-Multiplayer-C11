#ifndef UDSP_SEMESTRALKA_3_GAME_H
#define UDSP_SEMESTRALKA_3_GAME_H

#include <SDL2/SDL.h>
#include "../common/protocol.h"

#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

void SetDirection(PlayerState *player, int newDirection);
int CheckSelfCollision(PlayerState *player);
void ResetGame(PlayerState *player);
int UpdateGame(PlayerState *player, GameState *state);

#endif //UDSP_SEMESTRALKA_3_GAME_H