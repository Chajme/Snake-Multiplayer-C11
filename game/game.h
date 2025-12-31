//
// Created by erikm on 27. 12. 2025.
//

#ifndef UDSP_SEMESTRALKA_3_GAME_H
#define UDSP_SEMESTRALKA_3_GAME_H

#include <SDL2/SDL.h>
#include "../common/protocol.h"

int CheckSelfCollision(PlayerState *player);
void ResetGame(PlayerState *player);
int UpdateGame(PlayerState *player);

#endif //UDSP_SEMESTRALKA_3_GAME_H